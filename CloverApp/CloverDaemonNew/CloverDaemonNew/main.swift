//
//  main.swift
//  cloverDaemonSwift
//
//  Created by vector sigma on 03/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation
import CoreFoundation

let fm = FileManager.default
//let runLoop = CFRunLoopGetCurrent()

/*
 Store a session reference to contact Disk Arbitration,
 ..it will be usefull during the shut down.
 this is weird, but make some chances for the last stand (SIGTERM)
 */
let gSession  = DASessionCreate(kCFAllocatorDefault)
let gSession1 = DASessionCreate(kCFAllocatorDefault)
let gSession2 = DASessionCreate(kCFAllocatorDefault)
let gSession3 = DASessionCreate(kCFAllocatorDefault)
let gSession4 = DASessionCreate(kCFAllocatorDefault)
let gSession5 = DASessionCreate(kCFAllocatorDefault)

func execAttr() -> [FileAttributeKey : Any] {
  var attributes = [FileAttributeKey : Any]()
  attributes[.posixPermissions] = NSNumber(value: 755) //NSNumber object. Use the int16Value method to retrieve the integer value for the permissions.
  attributes[.groupOwnerAccountID] = NSNumber(value: 0) // NSNumber object containing an unsigned long.
  attributes[.groupOwnerAccountName] = "wheel" as NSString
  
  attributes[.ownerAccountID] = NSNumber(value: 0) //NSNumber object containing an unsigned long.
  attributes[.ownerAccountName] = "root" as NSString
  return attributes
}

func launchAttr() -> [FileAttributeKey : Any] {
  var attributes = [FileAttributeKey : Any]()
  attributes[.posixPermissions] = NSNumber(value: 420) //NSNumber object. Use the int16Value method to retrieve the integer value for the permissions.
  attributes[.groupOwnerAccountID] = NSNumber(value: 0) // NSNumber object containing an unsigned long.
  attributes[.groupOwnerAccountName] = "wheel" as NSString
  
  attributes[.ownerAccountID] = NSNumber(value: 0) //NSNumber object containing an unsigned long.
  attributes[.ownerAccountName] = "root" as NSString
  return attributes
}

func run(cmd: String) {
  let task = Process()
  if #available(OSX 10.13, *) {
    task.executableURL = URL(fileURLWithPath: "/bin/bash")
  } else {
    task.launchPath = "/bin/bash"
  }
  task.environment = ProcessInfo.init().environment
  task.arguments = ["-c", cmd]
  task.launch()
}

func doJob() {
  /*
   Don't lose time.. We're shutting down, so try to make root writable anyway
   */
  if !fm.isWritableFile(atPath: "/") {
    run(cmd: "mount -uw /")
  }
  if let nvram = getNVRAM() {
    // first time is to dump nvram (time is running out..)
 
    if fm.isWritableFile(atPath: "/") {
      saveNVRAM(volume: "/", nvram: nvram)
    } else {
      if let bootDevice = findBootPartitionDevice() {
        print("Boot Device is \(bootDevice).")
        mountSaveNVRAM(disk: bootDevice, at: nil, nvram: nvram)
      }
    }
    
    checkSleepProxyClient(nvram: nvram)
  } else {
    print("nvram not present in this machine.")
  }
  exit(EXIT_SUCCESS)
}

func mountSaveNVRAM(disk bsdName: String, at path: String?, nvram: NSDictionary) {
  print("mount called for \(bsdName)")
  var disk : String = bsdName
  if disk.hasPrefix("disk") || disk.hasPrefix("/dev/disk") {
    if disk.hasPrefix("/dev/disk") {
      disk = disk.components(separatedBy: "dev/")[1]
    }

    let allocator = kCFAllocatorDefault 
    let newsession = DASessionCreate(allocator)
    if let session = ((newsession == nil) ? gSession5 : newsession) {
      if let bsd = DADiskCreateFromBSDName(allocator, session, disk) {
        var url : CFURL? = nil
        if (path != nil) {
          url = CFURLCreateFromFileSystemRepresentation(allocator, path?.toPointer(), (path?.count)!, true)
        }
        var context = UnsafeMutablePointer<Int>.allocate(capacity: 1)
        context.initialize(repeating: 0, count: 1)
        context.pointee = 0
        
        DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), CFRunLoopMode.defaultMode.rawValue)
        
        DADiskMountWithArguments(bsd, url, DADiskMountOptions(kDADiskMountOptionDefault), { (o, dis, ctx) in
          if (dis != nil) && (ctx != nil) {
            print("mount failure: " + printDAReturn(r: DADissenterGetStatus(dis!)))
          }
        }, &context, nil)
        //CFRunLoopRun()
        let result : Bool = (context.pointee == 0)
        print("\(bsdName) mounted: \(result)")
        if result {
          let mountpoint = getMountPoint(from: disk)
          if ((mountpoint != nil)) {
            // already mounted
            saveNVRAM(volume: mountpoint!, nvram: nvram)
          }
        } else {
          saveNVRAM(volume: "/", nvram: nvram)
        }
        
        DASessionUnscheduleFromRunLoop(session,
                                       CFRunLoopGetCurrent(),
                                       CFRunLoopMode.defaultMode.rawValue)
        
        context.deallocate()
      } else {
        print("DADiskCreateFromBSDName() is null.")
        saveNVRAM(volume: "/", nvram: nvram)
      }
    } else {
      print("gSession in null.")
      saveNVRAM(volume: "/", nvram: nvram)
    }
    
  }
}


func saveNVRAM(volume: String, nvram: NSDictionary) {
  let withSlash = (volume == "/") ? volume : "\(volume)/"
  if !fm.isWritableFile(atPath: withSlash) {
    print("saveNVRAM(): '\(volume)' is read-only..\n")
    run(cmd: "mount -uw /")
  }
  if (nvram.object(forKey: "EmuVariableUefiPresent") != nil ||
    nvram.object(forKey: "TestEmuVariableUefiPresent") != nil) {
    if nvram.write(toFile: volume.addPath("nvram.plist"), atomically: true) {
      print("nvram saved correctly at '\(volume.addPath("nvram.plist"))'.")
      run(cmd: "chmod 644 \(volume.addPath("nvram.plist"))")
    } else {
      print("nvram not saved.")
    }
  }
}

func checkSleepProxyClient(nvram: NSDictionary) {
  let mDNSResponderPath = "/System/Library/LaunchDaemons/com.apple.mDNSResponder.plist"
  let disableOption = "-DisableSleepProxyClient"
  
  if let nvdata = nvram.object(forKey: "Clover.DisableSleepProxyClient") as? Data {
    let value = String(decoding: nvdata, as: UTF8.self)
    if value == "true" {
      if !fm.isWritableFile(atPath: "/") {
        print("try to making '/' writable as Clover.DisableSleepProxyClient=true.")
        run(cmd: "mount -uw /")
      }
      
      // check if already disabled
      if let mDNSResponder = NSDictionary(contentsOfFile: mDNSResponderPath) {
        if let ProgramArguments = mDNSResponder.object(forKey: "ProgramArguments") as? NSArray {
          var toDisable : Bool = true
          for a in ProgramArguments {
            if let arg = a as? String {
              if arg == disableOption {
                print("DisableSleepProxyClient: service already disabled\n")
                toDisable = false
                break
              }
            }
          }
          
          if toDisable {
            print("DisableSleepProxyClient: trying to disable the service.. SIP permitting.\n")
            let cmd = "/usr/libexec/PlistBuddy -c \"Add ProgramArguments: string \(disableOption)\" \(mDNSResponderPath)"
            run(cmd: cmd)
          }
        }
      }
    }
  }
}

func main() {
  let df = DateFormatter()
  df.dateFormat = "yyyy-MM-dd hh:mm:ss"
  var now = df.string(from: Date())
  print("--------------------------------------------")
  print("- System start at \(now)")
  print("--------------------------------------------")
  if let volname = getMediaName(from: "/") {
    print("root mount point is '/Volumes/\(volname)'")
  }
  // check Clover revision
  if let revision = findCloverRevision() {
    print("Started with Clover r\(revision).")
  } else {
    print("Started with an unknown firmware.")
  }
  
  // check if old daemon exist
  let myDir = (CommandLine.arguments[0] as NSString).deletingLastPathComponent
  if fm.fileExists(atPath: "\(myDir)/CloverDaemon") {
    print("unloading old CloverDaemon")
    run(cmd: "launchctl unload /Library/LaunchDaemons/com.projectosx.clover.daemon.plist")
  }
  
  /*
   Clean some lines fro clover.daemon.log.
   This is not going to go into the System log and is not
   controllable by the nvram.
   */
  let logLinesMax : Int = 500
  let logDir = "/Library/Logs/CloverEFI"
  let logPath = "/Library/Logs/CloverEFI/clover.daemon.log"
  
  if !fm.fileExists(atPath: logDir) {
    try? fm.createDirectory(atPath: logDir,
                            withIntermediateDirectories: false,
                            attributes: nil)
  }
  
  if fm.fileExists(atPath: logPath) {
    if let log : String? = try? String(contentsOfFile: logPath) {
      if let lines = log?.components(separatedBy: CharacterSet.newlines) {
        if lines.count > logLinesMax {
          // take only latests
          run(cmd: "tail -n \(logLinesMax) \(logPath) 2>/dev/null > \(logPath).tmp")
          run(cmd: "mv -f \(logPath).tmp \(logPath)")
        }
      }
    }
  }
  
  if let nvram = getNVRAM() {
    if let nvdata = nvram.object(forKey: "Clover.RootRW") as? Data {
      let value = String(decoding: nvdata, as: UTF8.self)
      if value == "true" && !fm.isWritableFile(atPath: "/") {
        print("making '/' writable as Clover.RootRW=true")
        run(cmd: "mount -uw /")
      }
    }
    checkSleepProxyClient(nvram: nvram)
  }
  
  // SIGTERM? This is a daemon so we are shutting down
  signal(SIGTERM, SIG_IGN)
  let termSource = DispatchSource.makeSignalSource(signal: SIGTERM)
  termSource.setEventHandler {
    now = df.string(from: Date())
    print("")
    print("- System power off at \(now)")
    doJob()
  }
  termSource.resume()
  //CFRunLoopRun()
  dispatchMain()
}

let myPath = CommandLine.arguments[0]
let myName = (myPath as NSString).lastPathComponent

if CommandLine.arguments.contains("--install") {
  print("Installing daemon...")
  // build the launch daemon
  let launch = NSMutableDictionary()
  launch.setValue(true, forKey: "KeepAlive")
  launch.setValue(true, forKey: "RunAtLoad")
  launch.setValue("com.slice.CloverDaemonNew", forKey: "Label")
  
  launch.setValue("/Library/Logs/CloverEFI/clover.daemon.log", forKey: "StandardErrorPath")
  launch.setValue("/Library/Logs/CloverEFI/clover.daemon.log", forKey: "StandardOutPath")
  
  let ProgramArguments = NSArray(object: "/Library/Application Support/Clover/CloverDaemonNew")
  
  launch.setValue(ProgramArguments, forKey: "ProgramArguments")
  
  do {
    if !fm.fileExists(atPath: "/Library/Application Support/Clover") {
      try fm.createDirectory(atPath: "/Library/Application Support/Clover",
                             withIntermediateDirectories: false,
                             attributes: nil)
    }
    
    if fm.fileExists(atPath: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist") {
      try fm.removeItem(atPath: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
    }
    
    if fm.fileExists(atPath: "/Library/Application Support/Clover/\(myName)") {
      try fm.removeItem(atPath: "/Library/Application Support/Clover/\(myName)")
    }
    
    launch.write(toFile: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist",
                 atomically: true)
    
    try fm.copyItem(atPath: myPath, toPath: "/Library/Application Support/Clover/\(myName)")
    
    try fm.setAttributes(execAttr(),
                         ofItemAtPath: "/Library/Application Support/Clover/\(myName)")
    
    
    try fm.setAttributes(launchAttr(),
                         ofItemAtPath: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
    
    run(cmd: "launchctl load /Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
    exit(EXIT_SUCCESS)
  } catch {
    print(error)
  }
} else if CommandLine.arguments.contains("--uninstall") {
  print("uninstalling daemon...")
  do {
    if fm.fileExists(atPath: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist") {
      run(cmd: "launchctl unload /Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
      try fm.removeItem(atPath: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
    }
    
    if fm.fileExists(atPath: "/Library/Application Support/Clover/CloverDaemonNew") {
      try fm.removeItem(atPath: "/Library/Application Support/Clover/CloverDaemonNew")
    }
    exit(EXIT_SUCCESS)
  } catch {
    print(error)
  }
} else {
  main()
  CFRunLoopRun()
}

exit(EXIT_FAILURE)

