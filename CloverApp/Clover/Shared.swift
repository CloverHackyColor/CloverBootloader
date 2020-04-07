//
//  Shared.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

// MARK: Paths
let Cloverv2Path : String = Bundle.main.sharedSupportPath! + "/CloverV2"
let kDaemonPath = "/Library/Application Support/Clover/CloverDaemonNew"
let kLaunchPlistPath = "/Library/LaunchDaemons/com.slice.CloverDaemonNew.plist"

// MARK: Run At Login
let gHelperID : CFString = "org.slice.CloverRunAtLogin" as CFString
let kRunAtLogin = "runAtLogin"

// MARK: File Manager
let fm = FileManager.default

// MARK:  Standard users defaults
let UDs = UserDefaults.standard

// MARK: Update search interval
enum UpdateInterval: Double {
  case never = 0
  case daily = 86400
  case weekly = 604800
  case monthly = 18144000
}

// MARK: Timer interval since last update check keys
let kLastSearchUpdateDateKey = "LastSearchUpdateDate"
let kUpdateSearchInterval = "UpdateSearchInterval"
let kLastUpdateLink = "LastUpdateLink"
let kLastUpdateRevision = "LastUpdateRevision"

// MARK: find Clover Revision
func findCloverRevision(at EFIdir: String) -> String? {
  
  let bootfiles : [String] = ["/BOOT/BOOTX64.efi",
                              "/CLOVER/CLOVERX64.efi",
                              "/BOOT/BOOTXIA32.efi",
                              "/CLOVER/CLOVERIA32.efi"]
  let preMatchString = "Clover revision: "
  let terminatingCharacter = " "
  for b in bootfiles {
    if fm.fileExists(atPath: EFIdir + b) {
      do {
        var rev : NSString? = nil
        let stringToSearch : String = try String(contentsOfFile: EFIdir + b, encoding: String.Encoding.ascii)
        let scanner : Scanner = Scanner(string: stringToSearch)
        scanner.scanUpTo(preMatchString, into: nil)
        scanner.scanString(preMatchString, into: nil)
        scanner.scanUpTo(terminatingCharacter, into: &rev)
        
        if (rev != nil), let revision = String(cString: (rev?.utf8String)!,
                                               encoding: String.Encoding.utf8) {
          if revision.count == 4 {
            return revision
          }
        }
      } catch  {
        print(error.localizedDescription)
      }
    }
    
  }
  return nil
}

// MARK: find Clover gihub commit
func findCloverHashCommit(at EFIdir: String) -> String? {
  
  let bootfiles : [String] = ["/BOOT/BOOTX64.efi",
                              "/CLOVER/CLOVERX64.efi",
                              "/BOOT/BOOTXIA32.efi",
                              "/CLOVER/CLOVERIA32.efi"]
  let preMatchString = ", commit "
  let terminatingCharacter = ")"
  for b in bootfiles {
    if fm.fileExists(atPath: EFIdir + b) {
      do {
        var rev : NSString? = nil
        let stringToSearch : String = try String(contentsOfFile: EFIdir + b, encoding: String.Encoding.ascii)
        let scanner : Scanner = Scanner(string: stringToSearch)
        scanner.scanUpTo(preMatchString, into: nil)
        scanner.scanString(preMatchString, into: nil)
        scanner.scanUpTo(terminatingCharacter, into: &rev)
        
        if (rev != nil), let revision = String(cString: (rev?.utf8String)!,
                                               encoding: String.Encoding.utf8) {
          if revision.count >= 4 && revision.count <= 40 {
            return revision
          }
        }
      } catch  {
        print(error.localizedDescription)
      }
    }
    
  }
  return nil
}

// MARK: get image from CoreType.bundle
func getCoreTypeImage(named: String, isTemplate: Bool) -> NSImage? {
  var image : NSImage? = nil
  if let ctb = Bundle.init(path: "/System/Library/CoreServices/CoreTypes.bundle") {
    image = NSImage(byReferencingFile: ctb.path(forResource: named, ofType: "icns", inDirectory: nil) ?? "")
  }
  image?.isTemplate = isTemplate
  return image
}

