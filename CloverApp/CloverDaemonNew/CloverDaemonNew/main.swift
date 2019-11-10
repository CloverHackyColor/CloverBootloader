//
//  main.swift
//  cloverDaemonSwift
//
//  Created by vector sigma on 03/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation
import CoreFoundation

let daemonVersion = "1.01"
let fm = FileManager.default

/*
 Store a session reference to contact Disk Arbitration,
 ..it will be usefull during the shut down.
 this is weird, but make some chances for the last stand (SIGTERM)
 */

var BootDeviceUUID : String? = nil
var BootDeviceFS   : String? = nil
var BootDevice     : String? = nil


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
    if (nvram.object(forKey: "EmuVariableUefiPresent") != nil ||
      nvram.object(forKey: "TestEmuVariableUefiPresent") != nil) {
      // mount -t "$filesystem" "$bootdev" "$mnt_pt"
      if let bootDevice = BootDevice, let fs = BootDeviceFS {
        saveNVRAM(bootDevice: bootDevice, filesystem: fs, nvram: nvram)
      }
    }
    
    checkSleepProxyClient(nvram: nvram)
  } else {
    print("nvram not present in this machine.")
  }
  exit(EXIT_SUCCESS)
}

func saveNVRAM(bootDevice: String, filesystem: String, nvram: NSMutableDictionary) {
  nvram.removeObject(forKey: "efi-backup-boot-device")
  nvram.removeObject(forKey: "efi-backup-boot-device-data")
  nvram.removeObject(forKey: "install-product-url")
  nvram.removeObject(forKey: "previous-system-uuid")
  nvram.write(toFile: "/tmp/nvramsaved.plist", atomically: false)
  
  let uuid = (BootDeviceUUID != nil) ? "\(BootDeviceUUID!)" : "NO UUID"
  let letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
  let mp = "/Volumes/\(String((0..<10).map{ _ in letters.randomElement()! }))"
  let type : String = (filesystem.hasPrefix("fat") || filesystem.hasPrefix("exf")) ? "msdos" : filesystem
  
  let cmd = """
  if [[ ! -f /tmp/nvramsaved.plist ]]; then
    echo "Error: nvram not found to a temporary location."
    exit 0
  fi
  launchctl load -F /System/Library/LaunchDaemons/com.apple.diskarbitrationd.plist > /dev/null 2>&1
  launchctl load -F /System/Library/LaunchDaemons/com.apple.diskmanagementd.plist > /dev/null 2>&1
  diskutil mount \(bootDevice) > /dev/null 2>&1
  mp=$(LC_ALL=C mount | egrep "^/dev/\(bootDevice) on" | sed 's/^.* on *//;s/ ([^(]*//')
  if [[ "${mp}" == "/"* ]]; then
    cat /tmp/nvramsaved.plist > "${mp}"/nvram.plist
    echo 'nvram saved to disk with UUID \(uuid)'
  else
    mkdir -p '\(mp)'
    attempts=1
    until [ $attempts -ge 11 ]
    do
      launchctl load -F /System/Library/LaunchDaemons/com.apple.diskarbitrationd.plist > /dev/null 2>&1
      launchctl load -F /System/Library/LaunchDaemons/com.apple.diskmanagementd.plist > /dev/null 2>&1
      sleep 0.2
      mount -t \(type) /dev/\(bootDevice) '\(mp)' > /dev/null 2>&1
      sleep 0.3
      if [ $? -eq 0 ]; then
        cat /tmp/nvramsaved.plist > '\(mp)/nvram.plist'
        echo 'nvram saved to disk with UUID \(uuid)'
        echo "$attempts attempts required."
        break
      else
        attempts=$[$attempts+1]
        if [ $attempts -eq 11 ]; then
          echo "Error: \(bootDevice) doesn't want to mount after $attempts attempts, try to save in /."
          cat /tmp/nvramsaved.plist > /nvram.plist
        fi
      fi
    done
  fi
  """
  let task = Process()
  task.launchPath = "/bin/bash"
  task.arguments = ["-c", cmd]
  
  task.launch()
}


func checkSleepProxyClient(nvram: NSDictionary) {
  let mDNSResponderPath = "/System/Library/LaunchDaemons/com.apple.mDNSResponder.plist"
  let disableOption = "-DisableSleepProxyClient"
  
  if let nvdata = nvram.object(forKey: "Clover.DisableSleepProxyClient") as? Data {
    let value = String(decoding: nvdata, as: UTF8.self)
    
    if value == "true" {
      if !fm.fileExists(atPath: mDNSResponderPath) {
        print("Error: cannot found \(mDNSResponderPath)")
        return
      }
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
  print("- CloverDaemonNew v\(daemonVersion)")
  print("- System start at \(now)")
  print("--------------------------------------------")
  var powerObserver : PowerObserver? = PowerObserver() // I want it to be var
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
   Clean some lines from clover.daemon.log.
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
  
  
  // get the boot device UUID, will be of help later
  if let bootDevice = findBootPartitionDevice() {
    BootDevice     = bootDevice
    BootDeviceUUID = getVolumeUUID(from: bootDevice)
    BootDeviceFS   = getFS(from: bootDevice)?.lowercased()
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
  
  /*
   Clean old nvram.plist user may have in other volumes,
   but not if it is our boot device
   */
  for v in getVolumes() {
    let nvramtPath = v.addPath("nvram.plist")
    let bootDevice = findBootPartitionDevice()
    let vbsd = getBSDName(of: v)
    if vbsd != bootDevice {
      if fm.fileExists(atPath: nvramtPath) {
        if fm.isDeletableFile(atPath: nvramtPath) {
          do {
            try fm.removeItem(atPath: nvramtPath)
            print("old '\(nvramtPath)' removed.")
          } catch {
            print("Error: can't remove '\(nvramtPath)'.")
          }
        } else {
          print("Error: '\(nvramtPath)' is not deletable.")
        }
      }
    }
  }
  
  // SIGTERM? This is a daemon so we are shutting down
  signal(SIGTERM, SIG_IGN)
  let termSource = DispatchSource.makeSignalSource(signal: SIGTERM)
  termSource.setEventHandler {
    now = df.string(from: Date())
    print("")
    print("- System power off at \(now)")
    print("- CloverDaemonNew v\(daemonVersion)")
    powerObserver = nil // no longer needed
    doJob()
  }
  termSource.resume()
  RunLoop.current.run()
  //dispatchMain()
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
    if fm.fileExists(atPath: "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist") {
      run(cmd: "launchctl unload /Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
    }
    run(cmd: "launchctl load /Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
    run(cmd: "launchctl start /Library/LaunchDaemons/com.slice.CloverDaemonNew.plist")
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

