//
//  main.swift
//  cloverDaemonSwift
//
//  Created by vector sigma on 03/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

let fm = FileManager.default
let daemonVersion = "1.0.3"

let wrapperPath = "/Library/Application Support/Clover/CloverWrapper.sh"
let loginwindow = "/var/root/Library/Preferences/com.apple.loginwindow.plist"
let lh = "/Library/Application Support/Clover/CloverLogOut"

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
  if let nvram = getNVRAM() {
    checkSleepProxyClient(nvram: nvram)
  } else {
    print("nvram not present in this machine.")
  }
  exit(EXIT_SUCCESS)
}

func checkSleepProxyClient(nvram: NSDictionary) {
  let mDNSResponderPath = "/System/Library/LaunchDaemons/com.apple.mDNSResponder.plist"
  let disableOption = "-DisableSleepProxyClient"
  
  if let nvdata = nvram.object(forKey: "Clover.DisableSleepProxyClient") as? Data {
    let value = String(decoding: nvdata, as: UTF8.self)
    print("Clover.DisableSleepProxyClient=\(value)")
    if !fm.fileExists(atPath: mDNSResponderPath) {
      print("Error: cannot found \(mDNSResponderPath)")
      return
    }
    if !fm.isWritableFile(atPath: "/") {
      print("Cannot go ahead as / is still read-only at this moment, will try again at power off.")
      return
    }
    // check if enabled or disabled
    if let mDNSResponder = NSDictionary(contentsOfFile: mDNSResponderPath) {
      if let ProgramArguments = mDNSResponder.object(forKey: "ProgramArguments") as? NSArray {
        var index : Int = -1
        var serviceDisabled = false
        for i in 0..<ProgramArguments.count {
          if let arg = ProgramArguments.object(at: i) as? String {
            if arg == disableOption {
              index = i
              serviceDisabled = true
              break
            }
          }
        }
        
        if value == "true" && !serviceDisabled {
          print("Trying to disable Sleep Proxy Client service as requested.")
          let cmd = "/usr/libexec/PlistBuddy -c \"Add ProgramArguments: string \(disableOption)\" \(mDNSResponderPath)"
          run(cmd: cmd)
        } else if value != "true" && serviceDisabled {
          print("Trying to enable Sleep Proxy Client service as requested.")
          if index >= 0 {
            let cmd = "/usr/libexec/PlistBuddy -c \"delete :ProgramArguments:\(index)\" \(mDNSResponderPath)"
            run(cmd: cmd)
          } else {
            print("Bug: cant find the index of '\(disableOption)'.\n")
          }
        } else {
          print("Sleep Proxy Client \(serviceDisabled ? "disabled" : "enabled") as requested.")
        }
      }
    }
  }
}

func getLogOutHook() -> String? {
  return NSDictionary(contentsOfFile: loginwindow)?.object(forKey: "LogoutHook") as? String
}

func main() {
  let df = DateFormatter()
  df.dateFormat = "yyyy-MM-dd hh:mm:ss"
  var now = df.string(from: Date())

  print("\n\n--------------------------------------------")
  print("- CloverDaemonNew v\(daemonVersion)")
  print("- macOS \(ProcessInfo().operatingSystemVersionString)")
  print("- System start at \(now)")
  print("------")
  run(cmd: "kextunload /System/Library/Extensions/msdosfs.kext 2>/dev/null")
  let _ : PowerObserver? = PowerObserver()
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
  
  // check options in nvram
  var mountRWCalled = fm.isWritableFile(atPath: "/")
  
  if let nvram = getNVRAM() {
    if let nvdata = nvram.object(forKey: "Clover.RootRW") as? Data {
      let value = String(decoding: nvdata, as: UTF8.self)
      if value == "true" {
        if !mountRWCalled {
          mountRWCalled = true
          print("making '/' writable as Clover.RootRW=true.")
          run(cmd: "mount -uw /")
        }
      }
    }
    
    
    // using the logout Hook only if EmuVariableUefiPresent
    let loh = getLogOutHook()
    
    if (nvram.object(forKey: "EmuVariableUefiPresent") != nil ||
      nvram.object(forKey: "TestEmuVariableUefiPresent") != nil) {
      
      // if a hook exist and is not our one, then add a wrapper
      if (loh != nil) {
        if loh! != lh && fm.fileExists(atPath: loh!) {
          let wrapper =
          """
#!/bin/sh

'\(lh)'
          
# This file is automatically generated by CloverDaemonNew because
# it has detected you added a logout script somewhere else.
# if you edit this file, be aware that if you start to boot in UEFI
# CloverDaemonNew will take care to restore your custom logout script,
# but your script must be the last line to be detected again.
# The script path must be escaped within '' if contains spaces in the middle.
# NOTE: if your will is to add multiple scripts, please add them to
# the following script (and so not in this script):
          
'\(loh!)'
"""
          print("Detected user logout hook at \(loh!), merging with it with CloverLogOut in CloverWrapper.sh.")
          do {
            try wrapper.write(toFile: wrapperPath, atomically: false, encoding: .utf8)
            try fm.setAttributes(execAttr(), ofItemAtPath: wrapperPath)
            
            run(cmd: "defaults write com.apple.loginwindow LogoutHook '\(wrapperPath)'")
          } catch {
            print(error)
          }
        } else {
          run(cmd: "defaults write com.apple.loginwindow LogoutHook '\(lh)'")
        }
      } else {
        run(cmd: "defaults write com.apple.loginwindow LogoutHook '\(lh)'")
      }
      
      if !fm.isWritableFile(atPath: "/") {
        if !mountRWCalled {
          print("making '/' writable as EmuVariableUefiPresent is present to save nvram.")
        }
        run(cmd: "mount -uw /")
      }
    } else {
      // what to do? remove the logout hook if it's our
      if (loh != nil) {
        if loh! == lh {
          // it is CloverLogOut
          print("Removing CloverLogOut hook as EmuVariable is no longer present.")
          run(cmd: "defaults delete com.apple.loginwindow LogoutHook")
        } else if loh! == wrapperPath {
          print("Removing CloverWrapper.sh logout hook as EmuVariable is no longer present.")
          // it is CloverWrapper.sh. We have to mantain user script!
          // get the user script path:
          var script : String? = nil
          if let lines = try? String(contentsOf: URL(fileURLWithPath: wrapperPath),
            encoding: .utf8).components(separatedBy: .newlines) {
            if lines.count > 0 {
              script = lines.last
            }
          }
          if script != nil {
            print("..but taking user logout hook script at \(script!).")
            run(cmd: "defaults write com.apple.loginwindow LogoutHook \(script!)")
          } else {
            run(cmd: "defaults delete com.apple.loginwindow LogoutHook")
          }
        }
      }
    }
    
    if let nvdata = nvram.object(forKey: "Clover.DisableSleepProxyClient") as? Data {
      let value = String(decoding: nvdata, as: UTF8.self)
      if value == "true" {
        if !fm.isWritableFile(atPath: "/") {
          if !mountRWCalled {
            print("making '/' writable as Clover.DisableSleepProxyClient=true.")
          }
          run(cmd: "mount -uw /")
          sleep(2)
        }
      }
      checkSleepProxyClient(nvram: nvram)
    }
    
    /*
     Clean old nvram.plist user may have in all volumes
     */
    for v in getVolumes() {
      let nvramtPath = v.addPath("nvram.plist")
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
  } else {
    print("Error: nvram not present in this System.")
  }

  print("Logout hook is: \(getLogOutHook() ?? "none")")
  signal(SIGTERM, SIG_IGN) //preferred
  let sigtermSource = DispatchSource.makeSignalSource(signal: SIGTERM)
  sigtermSource.setEventHandler {
    now = df.string(from: Date())
    print("")
    print("SIGTERM received at \(now)")
    //print("- CloverDaemonNew v\(daemonVersion)")
    doJob()
  }
  sigtermSource.resume()
  
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
    
    
    let logouthookSrc = myPath.deletingLastPath.addPath("CloverLogOut")
    if fm.fileExists(atPath: lh) {
      try fm.removeItem(atPath: lh)
    }
    if fm.fileExists(atPath: logouthookSrc) {
      try fm.copyItem(atPath: logouthookSrc,
                      toPath: lh)
      try fm.setAttributes(execAttr(),
                           ofItemAtPath: lh)
    }
    
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
}

exit(EXIT_FAILURE)

