//
//  bdmesg.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

/// Get Clover boot-log (or compatible)
func dumpBootlog() -> String? {
  var root: io_registry_entry_t
  var bootLog: CFTypeRef? = nil
  var log : String? = nil
  let entry = "boot-log" as CFString
  let chameleon = "IOService:/"
  let clover = "IODeviceTree:/efi/platform"
  root = IORegistryEntryFromPath(kIOMasterPortDefault, chameleon)
  
  if root != MACH_PORT_NULL {
    let attempt = IORegistryEntryCreateCFProperty(root, entry, kCFAllocatorDefault, 0)
    if (attempt != nil) {
      bootLog = attempt?.takeRetainedValue()
    }
  }
  if bootLog == nil {
    // Check for Clover boot log
    root = IORegistryEntryFromPath(kIOMasterPortDefault, clover)
    if root != MACH_PORT_NULL {
      let attempt = IORegistryEntryCreateCFProperty(root, entry, kCFAllocatorDefault, 0)
      if (attempt != nil) {
        bootLog = attempt?.takeRetainedValue()
      }
    }
  }
  IOObjectRelease(root)
  if bootLog != nil {
    let data = Data(bytes: CFDataGetBytePtr((bootLog as! CFData)), count: CFDataGetLength((bootLog as! CFData)))
    
    log = String(data: data , encoding: .utf8)
    if (log == nil) {
      log = String(data: data , encoding: .ascii)
    }
  }
  return log
}

/// Get Find the Clover Revision from the boot-log
func findCloverRevision() -> String? {
  let bdmesg = dumpBootlog()
  var rev : String? = nil
  if (bdmesg != nil) {
    for line in bdmesg!.components(separatedBy: .newlines) {
      if (line.range(of: "Starting Clover revision: ") != nil) {
        rev = line.components(separatedBy: "Starting Clover revision: ")[1]
        rev = rev!.components(separatedBy: " ")[0]
        break
      }
    }
  }
  return rev
}

/// Determine if We're booted with Clover (legay or UEFI)
func bootByClover() -> Bool {
  let bdmesg = dumpBootlog()
  if (bdmesg != nil) {
    for line in bdmesg!.components(separatedBy: .newlines) {
      if (line.range(of: "Starting Clover revision: ") != nil) {
        return true
      }
    }
  }
  return false
}

/// Find the Clover hash commit (from the boot-log)
func findCloverHash() -> String? {
  let bdmesg = dumpBootlog()
  var rev : String? = nil
  if (bdmesg != nil) {
    for line in bdmesg!.components(separatedBy: .newlines) {
      if (line.range(of: "Starting Clover revision: ") != nil
        && (line.range(of: ", commit ") != nil)) {
        rev = line.components(separatedBy: ", commit")[1]
        rev = rev!.components(separatedBy: ")")[0]
        break
      }
    }
  }
  return rev
}

/// Find the UUID of the partition boot device Clover starts from.
func findBootPartitionDevice() -> String? {
  var bsd :String? = nil
  if let bdmesg : String = dumpBootlog() {
    if (bdmesg.range(of: "SelfDevicePath") != nil) {
      var temp : String? = bdmesg
      temp = temp?.components(separatedBy: "SelfDevicePath")[1]
      
      if (temp?.range(of: "SelfDirPath") != nil) {
        temp = temp?.components(separatedBy: "SelfDirPath")[0]
        
        let comp = temp?.components(separatedBy: ",")
        var uuid : String? = nil
        for str in comp! {
          if (str.count == 36) &&
            (str.range(of: "-") != nil) &&
            str.components(separatedBy: "-").count == 5 {
            uuid = str
            break
          }
        }
        
        if (uuid != nil) {
          let dict = getAlldisks()
          for disk in dict.allKeys {
            let sub = dict.object(forKey: disk) as! NSDictionary
            if (sub.object(forKey: "UUID") != nil) {
              if (sub.object(forKey: "UUID") as! String) == uuid {
                bsd = disk as? String
                break
              }
            }
          }
        }
      }
    }
  }
  return bsd
}

/// Find the relative path of the config.plist  loaded by Clover.
func findConfigPath() -> String? {
  var path : String? = nil
  if let log : String = dumpBootlog() {
    if (log.range(of: "[ GetDefaultSettings ]") != nil) {
      var temp : String = log.components(separatedBy: "[ GetDefaultSettings ]")[1]
      if (temp.range(of: "=== [") != nil) {
        temp = temp.components(separatedBy: "=== [")[0]
        // ok now must be a line feed somewhere to create an array and split lines
        // one of them looks like "0:109  0:008  EFI\CLOVER\config.plist loaded: Success"
        let lines : [String] = temp.components(separatedBy: "\n")
        for line in lines {
          if (line.range(of: "EFI\\CLOVER\\") != nil) && (line.range(of: " loaded:") != nil) {
            temp = line.components(separatedBy: "EFI\\CLOVER\\")[1]
            temp = temp.components(separatedBy: " loaded:")[0]
            break
          }
        }
        if temp.hasSuffix(".plist") {
          path = "EFI/CLOVER/\(temp)"
        }
      }
    }
  }
  return path
}

/// Struct for Start up Sound (name, output, index).
struct SoundDevice {
  var name: String
  var output: String
  var index: Int
}

/// Return an array of SoundDevice detected by Clover
func getSoundDevices() -> [SoundDevice] {
  var devices = [SoundDevice]()
  if let bdmesg = dumpBootlog() {
    for line in bdmesg.components(separatedBy: .newlines) {
      // Found Audio Device IDT 92HD91BXX (Headphones) at index 0
      if (line.range(of: "Found Audio Device ") != nil && line.range(of: " at index ") != nil) {
        var name = line.components(separatedBy: "Found Audio Device ")[1].components(separatedBy: " at index")[0]
        let output = name.components(separatedBy: "(")[1].components(separatedBy: ")")[0]
        name = name.components(separatedBy: " (")[0]
        let index  = line.components(separatedBy: " at index ")[1]
        if let i : Int = Int(index) {
          // print("\(name) at index \(i) (\(output))")
          if i >= 0 {
            var found = false // avoid duplicates
            for d in devices {
              if d.name == name && d.output == output && d.index == i {
                found = true
                break
              }
            }
            if !found {
              let sd = SoundDevice(name: name, output: output, index: i)
              devices.append(sd)
            }
          }
        }
      }
    }
  }
  return devices
}
