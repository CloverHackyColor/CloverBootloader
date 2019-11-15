//
//  Shared.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

// MARK: NSApplication shared delegate
let AppSD = NSApplication.shared.delegate as! AppDelegate

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

extension String {
  var locale: String {
    get {
      let preferred : [String] = Bundle.main.preferredLocalizations
      var table : String = "en"
      if preferred.count > 0 {
        table = preferred[0]
      }
      
      var result = localeBundle?.localizedString(forKey: self, value: nil, table: table)
      
      if result == self {
        result = localeBundle?.localizedString(forKey: self, value: nil, table: "en")
      }
      return (result != nil) ? result! : self
    }
  }
  
  public func locale(_ localized: Bool) -> String {
    return (localized ? self.locale : self)
  }
}

// MARK: localize view and sub views
func localize(view: NSView) {
  for o in view.subviews {
    if o is NSButton {
      let x = (o as! NSButton)
      if x.title.count > 0 {
        x.title = x.title.locale
      }
    } else if o is NSTextField {
      let x = (o as! NSTextField)
      if x.stringValue.count > 0 {
        x.stringValue = x.stringValue.locale
      }
    } else if o is NSBox {
      let x = (o as! NSBox)
      if x.title.count > 0 {
        x.title = x.title.locale
      }
    }
    
    if o.subviews.count > 0 {
      localize(view: o)
    }
  }
}

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
                                               encoding: String.Encoding.utf8)/*&& rev?.length == 4 */{
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
