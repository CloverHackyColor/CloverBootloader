//
//  main.swift
//  CloverLogOut
//
//  Created by vector sigma on 16/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

func log(_ str: String) {
  let logUrl = URL(fileURLWithPath: "/Library/Logs/CloverEFI/clover.daemon.log")
  
  if !fm.fileExists(atPath: "/Library/Logs/CloverEFI") {
    try? fm.createDirectory(at: URL(fileURLWithPath: "/Library/Logs/CloverEFI"),
                            withIntermediateDirectories: false,
                            attributes: nil)
  }
  
  if !fm.fileExists(atPath: "/Library/Logs/CloverEFI/clover.daemon.log") {
    try? "".write(to: logUrl, atomically: false, encoding: .utf8)
  }
  
  if let fh = try? FileHandle(forUpdating: logUrl) {
    fh.seekToEndOfFile()
    fh.write("\n\(str)".data(using: .utf8)!)
    fh.closeFile()
  }
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

func saveNVRAM(nvram: NSMutableDictionary, volume: String) {
  if (nvram.object(forKey: "efi-backup-boot-device") != nil) {
    nvram.removeObject(forKey: "efi-backup-boot-device")
  }
  
  if (nvram.object(forKey: "efi-backup-boot-device-data") != nil) {
    nvram.removeObject(forKey: "efi-backup-boot-device-data")
  }
  
  if (nvram.object(forKey: "install-product-url") != nil) {
    nvram.removeObject(forKey: "install-product-url")
  }
  
  if (nvram.object(forKey: "previous-system-uuid") != nil) {
    nvram.removeObject(forKey: "previous-system-uuid")
  }
  
  if fm.fileExists(atPath: "/nvram.plist") {
    do {
      try fm.removeItem(atPath: "/nvram.plist")
    } catch {
      log("\(error)")
    }
  }
  
  let bsdname = getBSDName(of: volume) ?? ""
  let uuid = getVolumeUUID(from: bsdname) ?? kNotAvailable
  if nvram.write(toFile: volume.addPath("nvram.plist"), atomically: false) {
    if volume == "/" {
      run(cmd: "chflags hidden /nvram.plist")
    }
    log("nvram correctly saved to \(volume) with UUID: \(uuid).")
  } else {
    log("Error: nvram cannot be saved to \(volume) with UUID: \(uuid).")
  }
}

func main() {
  let df = DateFormatter()
  df.dateFormat = "yyyy-MM-dd hh:mm:ss"
  var now = df.string(from: Date())
  log("- CloverLogOut: begin at \(now)")
  if let nvram = getNVRAM() {
    // Check if we're running from CloverEFI
    if (nvram.object(forKey: "EmuVariableUefiPresent") != nil ||
      nvram.object(forKey: "TestEmuVariableUefiPresent") != nil) {
      
      var disk : String? = nil
      var espList = [String]()
      
      /* find all internal ESP in the System as we don't want
       to save the nvram in a USB pen drive (user will lost the nvram if not plugged in)
       */
      for esp in getAllESPs() {
        if isInternalDevice(diskOrMtp: esp) {
          espList.append(esp)
        }
      }
      
      // find the boot partition device and check if it is a ESP
      if let bd = findBootPartitionDevice() {
        if espList.contains(bd) {
          // boot device is a ESP :-)
          disk = bd
          log("Detected ESP \(bd) as boot device.")
        }
      }
      
      if (disk == nil) {
        // boot device not found
        if espList.count > 0 {
          // we have an internal ESP, using that
          disk = espList[0]
          log("Will use ESP \(disk!) as boot device is not found.")
        } else {
          // use root
          disk = getBSDName(of: "/")
          log("Will use / as no boot device nor an internal ESP are found.")
        }
      }
      
      if (disk != nil) {
        // get the mount point or mount it
        var mounted : Bool = false
        if let mp = getMountPoint(from: disk!) {
          mounted = true
          saveNVRAM(nvram: nvram, volume: mp)
          return
        }
        if !mounted {
          log("mount begin for \(disk!)..")
          var mountattempts : Int = 0
          repeat {
            mount(disk: disk!, at: nil) { (result) in
              mountattempts+=1
              sleep(1)
              if result == true {
                var attempts = 0
                repeat {
                  sleep(1)
                  attempts+=1
                  if let mp = getMountPoint(from: disk!) {
                    mounted = true
                    mountattempts = 5
                    saveNVRAM(nvram: nvram, volume: mp)
                    sleep(1)
                    umount(disk: disk!, force: true)
                    break
                  }
                } while (getMountPoint(from: disk!) == nil || attempts > 10)
                
              }
            }
            
          } while mountattempts < 5
          
          if !mounted {
            log("mount failed for \(disk!).")
            saveNVRAM(nvram: nvram, volume: "/")
          }
        } else {
          // CloverDaemonNew should have made the fs read-write for us.
          saveNVRAM(nvram: nvram, volume: "/")
        }
      }
    }
  }
  
  now = df.string(from: Date())
  log("- CloverLogOut: end at \(now)")
}

main()

exit(EXIT_SUCCESS)
