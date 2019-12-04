//
//  bdmesg.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

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
