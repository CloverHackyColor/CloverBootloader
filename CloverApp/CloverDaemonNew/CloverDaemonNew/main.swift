//
//  main.swift
//  cloverDaemonSwift
//
//  Created by vector sigma on 03/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

let fm = FileManager.default
let daemonVersion = "1.0.8"

let wrapperPath = "/Library/Application Support/Clover/CloverWrapper.sh"
let frameworksPath = "/Library/Application Support/Clover/Frameworks"
let loginWindowPath = "/var/root/Library/Preferences/com.apple.loginwindow.plist"
let cloverLogOut = "/Library/Application Support/Clover/CloverLogOut"
let cloverDaemonNewPath = "/Library/Application Support/Clover/CloverDaemonNew"
let launchPlistPath = "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist"

let oldLaunchPlistPath = "/Library/LaunchDaemons/com.projectosx.clover.daemon.plist"


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
  
  var value = "false"
  
  if let nvdata = nvram.object(forKey: "Clover.DisableSleepProxyClient") as? Data {
    value = String(decoding: nvdata, as: UTF8.self)
    print("Clover.DisableSleepProxyClient=\(value)")
  } else {
    print("Clover.DisableSleepProxyClient is not set.")
  }
  
  if !fm.fileExists(atPath: mDNSResponderPath) {
    print("Error: cannot found \(mDNSResponderPath)")
    return
  }
  if !fm.isWritableFile(atPath: "/") {
    print("Cannot go ahead as / is read-only")
    return
  }
  
  let toDisable : Bool = value == "true"
  // check if enabled or disabled
  if let mDNSResponder = NSDictionary(contentsOfFile: mDNSResponderPath) {
    if let ProgramArguments = mDNSResponder.object(forKey: "ProgramArguments") as? NSArray {
      var index : Int = -1
      var serviceIsDisabled = false
      for i in 0..<ProgramArguments.count {
        if let arg = ProgramArguments.object(at: i) as? String {
          if arg == disableOption {
            index = i
            serviceIsDisabled = true
            break
          }
        }
      }
      
      // no need to do anything if already as user wants.
      if toDisable && serviceIsDisabled {
        print("Sleep Proxy Client is already disabled.")
        return
      } else if !toDisable && !serviceIsDisabled {
        print("Sleep Proxy Client is already enabled as default.")
        return
      }
      
      
      if toDisable {
        print("Trying to disable Sleep Proxy Client service.")
        let cmd = "/usr/libexec/PlistBuddy -c \"Add ProgramArguments: string \(disableOption)\" \(mDNSResponderPath)"
        run(cmd: cmd)
      } else {
        print("Trying to enable Sleep Proxy Client service as default.")
        if index >= 0 {
          let cmd = "/usr/libexec/PlistBuddy -c \"delete :ProgramArguments:\(index)\" \(mDNSResponderPath)"
          run(cmd: cmd)
        }
      }
    }
  }
}

func getLogOutHook() -> String? {
  return NSDictionary(contentsOfFile: loginWindowPath)?.object(forKey: "LogoutHook") as? String
}

func removeCloverRCScripts() {
  if fm.fileExists(atPath: oldLaunchPlistPath) {
    print("unloading old CloverDaemon..")
    run(cmd: "launchctl unload \(oldLaunchPlistPath)")
    try? fm.removeItem(atPath: oldLaunchPlistPath)
    if fm.fileExists(atPath: "/Library/Application Support/Clover/CloverDaemon") {
      try? fm.removeItem(atPath: "/Library/Application Support/Clover/CloverDaemon")
    }
    
    if fm.fileExists(atPath: "/Library/Application Support/Clover/CloverDaemon-stopservice") {
      try? fm.removeItem(atPath: "/Library/Application Support/Clover/CloverDaemon-stopservice")
    }
  }
  
  let oldRCScripts : [String] = ["/etc/rc.boot.d/10.save_and_rotate_boot_log.local",
                                 "/etc/rc.boot.d/20.mount_ESP.local",
                                 "/etc/rc.boot.d/70.disable_sleep_proxy_client.local.disabled",
                                 "/etc/rc.boot.d/70.disable_sleep_proxy_client.local",
                                 "/etc/rc.clover.lib",
                                 "/etc/rc.shutdown.d/80.save_nvram_plist.local"];
  
  for rc in oldRCScripts {
    if fm.fileExists(atPath: rc) {
      if !fm.isWritableFile(atPath: "/") {
        run(cmd: "mount -uw /")
        sleep(2)
      }
      print("Removing \(rc).")
      do {
        try fm.removeItem(atPath: rc)
      } catch {
        print(error)
      }
    }
  }
}

func main() {
  let df = DateFormatter()
  df.locale = Locale(identifier: "en_US")
  df.dateFormat = "yyyy-MM-dd hh:mm:ss"
  var now = df.string(from: Date())

  print("\n\n--------------------------------------------")
  print("- CloverDaemonNew v\(daemonVersion)")
  print("- macOS \(ProcessInfo().operatingSystemVersionString)")
  print("- System start at \(now)")
  print("------")
  run(cmd: "kextunload /System/Library/Extensions/msdosfs.kext 2>/dev/null")
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
  removeCloverRCScripts()

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
        if loh! != cloverLogOut && fm.fileExists(atPath: loh!) {
          let wrapper =
          """
#!/bin/sh

'\(cloverLogOut)'
          
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
          run(cmd: "defaults write com.apple.loginwindow LogoutHook '\(cloverLogOut)'")
        }
      } else {
        run(cmd: "defaults write com.apple.loginwindow LogoutHook '\(cloverLogOut)'")
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
        if loh! == cloverLogOut {
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
    }
    
    checkSleepProxyClient(nvram: nvram)
    /*
     Clean old nvram.plist user may have in all volumes
     Note: never delete in / as this will be done at shut down/restart
     if the nvram is correctly saved somewhere else (e.g. in the ESP).
     
     Also don't delete nvram.plist from external devices as this is not or
     shouldn't be our business.
     */
    
    for v in getVolumes() {
      let nvramtPath = v.addPath("nvram.plist")
      if v != "/" && isInternalDevice(diskOrMtp: v) {
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
    
    /*
     Mount ESP if required
     Clover.MountEFI=Yes|diskX|GUID [default No]
     */
    if let nvdata = nvram.object(forKey: "Clover.MountEFI") as? Data {
      let value = String(decoding: nvdata, as: UTF8.self)
      print("Clover.MountEFI=\(value)")
      var disk : String? = nil
      let allEsps = getAllESPs()
      if value == "Yes" {
        // mount the boot device
        disk = findBootPartitionDevice()
      } else if value.hasPrefix("disk") {
        disk = value
      } else if value.hasPrefix("/dev/") {
        // mount desired ESP /dev/disk-rdisk
        var tmpdisk = value
        if tmpdisk.hasPrefix("/dev/disk") {
          tmpdisk = value.replacingOccurrences(of: "/dev/", with: "")
        } else if tmpdisk.hasPrefix("/dev/rdisk") {
          tmpdisk = value.replacingOccurrences(of: "/dev/r", with: "")
        }
        if tmpdisk.hasPrefix("disk") {
          disk = tmpdisk
        }
      } else if value.count == 36 && ((value .range(of: "-") != nil)) {
        // mount desired ESP by Volume UUID
        for d in allEsps {
          if let uuid = getMediaUUID(from: d) {
            if uuid == value {
              disk = d
              break
            }
          }
        }
      }
      
      if (disk != nil) {
        if (getBSDName(of: disk!) != nil) {
          if allEsps.contains(disk!) {
            print("try to mount \(disk!) as requested.")
            mount(disk: disk!, at: nil)
          } else {
            print("\(disk!) is not an EFI System Partition.")
          }
        } else {
          print("\(disk!) is not a valid disk object or is unavailable at the moment.")
        }
      } else {
        print("Nothing to mount.")
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
  
  let ProgramArguments = NSArray(object: cloverDaemonNewPath)
  
  launch.setValue(ProgramArguments, forKey: "ProgramArguments")
  
  removeCloverRCScripts()
  do {
    if !fm.fileExists(atPath: "/Library/Application Support/Clover") {
      try fm.createDirectory(atPath: "/Library/Application Support/Clover",
                             withIntermediateDirectories: false,
                             attributes: nil)
    }
    
    if fm.fileExists(atPath: launchPlistPath) {
      try fm.removeItem(atPath: launchPlistPath)
    }
    
    if fm.fileExists(atPath: cloverDaemonNewPath) {
      try fm.removeItem(atPath: cloverDaemonNewPath)
    }
    
    launch.write(toFile: launchPlistPath, atomically: true)
    
    try fm.copyItem(atPath: myPath, toPath: cloverDaemonNewPath)
    
    try fm.setAttributes(execAttr(),
                         ofItemAtPath: cloverDaemonNewPath)
    
    
    let logouthookSrc = myPath.deletingLastPath.addPath("CloverLogOut")
    if fm.fileExists(atPath: cloverLogOut) {
      try fm.removeItem(atPath: cloverLogOut)
    }
    if fm.fileExists(atPath: logouthookSrc) {
      try fm.copyItem(atPath: logouthookSrc,
                      toPath: cloverLogOut)
      try fm.setAttributes(execAttr(),
                           ofItemAtPath: cloverLogOut)
    }
    
    try fm.setAttributes(launchAttr(), ofItemAtPath: launchPlistPath)
    if fm.fileExists(atPath: launchPlistPath) {
      run(cmd: "launchctl unload \(launchPlistPath)")
    }
    run(cmd: "launchctl load \(launchPlistPath)")
    run(cmd: "launchctl start \(launchPlistPath)")
    exit(EXIT_SUCCESS)
  } catch {
    print(error)
  }
} else if CommandLine.arguments.contains("--uninstall") {
  print("uninstalling daemon...")
  do {
    if fm.fileExists(atPath: launchPlistPath) {
      run(cmd: "launchctl unload \(launchPlistPath)")
      try fm.removeItem(atPath: launchPlistPath)
    }
    
    if fm.fileExists(atPath: cloverDaemonNewPath) {
      try fm.removeItem(atPath: cloverDaemonNewPath)
    }
    
    if fm.fileExists(atPath: cloverLogOut) {
      try fm.removeItem(atPath: cloverLogOut)
    }
    
    if fm.fileExists(atPath: wrapperPath) {
      try fm.removeItem(atPath: wrapperPath)
    }
    
    if fm.fileExists(atPath: frameworksPath) {
      try fm.removeItem(atPath: frameworksPath)
    }
    exit(EXIT_SUCCESS)
  } catch {
    print(error)
  }
} else {
  main()
}

exit(EXIT_FAILURE)

