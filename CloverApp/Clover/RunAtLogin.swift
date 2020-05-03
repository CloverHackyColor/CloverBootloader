//
//  RunAtLogin.swift
//  HWMonitorSMC
//
//  Created by vector sigma on 18/03/18.
//  Copyright © 2018 HWSensor. All rights reserved.
//

import Cocoa
import ServiceManagement

extension AppDelegate {
  //MARK: new login item methods (but buggie)
  func setLaunchAtStartup() {
    let success : Bool = SMLoginItemSetEnabled(gHelperID, true)
    UDs.set(success, forKey: kRunAtLogin)
    UDs.synchronize()
  }
  
  func removeLaunchAtStartup() {
    let success : Bool = SMLoginItemSetEnabled(gHelperID, false)
    UDs.set(success ? false : true, forKey: kRunAtLogin)
    UDs.synchronize()
  }
  
  //MARK: old login item methods (but working)
  func getLoginItemURL(for item: LSSharedFileListItem) -> URL? {
    var url : URL? = nil
    if #available(OSX 10.10, *) {
      url = LSSharedFileListItemCopyResolvedURL(item, 0, nil)?.takeRetainedValue() as URL?
    } else {
      var ItemURL : Unmanaged<CFURL>? = nil
      let flags : UInt32  = UInt32(kLSSharedFileListNoUserInteraction | kLSSharedFileListDoNotMountVolumes)
      LSSharedFileListItemResolve(item, flags, &ItemURL, nil)
      url = ItemURL?.takeRetainedValue() as URL?
    }
    return url
  }

  func addAsLoginItem() -> Bool {
    var found : Bool = false
    let currentUrl = Bundle.main.bundleURL
    if let sharedFileList = LSSharedFileListCreate(nil,
                                                kLSSharedFileListSessionLoginItems.takeRetainedValue(),
                                                nil)?.takeRetainedValue() {
      if let snapshot = LSSharedFileListCopySnapshot(sharedFileList,
                                                     nil).takeRetainedValue() as?  [LSSharedFileListItem] {
        for item in snapshot {
          
          
          if let itemUrl = self.getLoginItemURL(for: item) {
            guard let info = NSDictionary(contentsOfFile: itemUrl.path.addPath("Contents/Info.plist")) as? [String: Any] else { continue }
            let bi = info[kCFBundleIdentifierKey as String] as? String
            if bi == Bundle.main.bundleIdentifier {
              // is Clover.app, but is the current one?
            
              if itemUrl == currentUrl {
                found = true
              } else {
                LSSharedFileListItemRemove(sharedFileList, item)
              }
            }
          }
        }
      }
      
      if !found {
        LSSharedFileListInsertItemURL(sharedFileList,
                                      kLSSharedFileListItemBeforeFirst.takeRetainedValue(),
                                      nil,
                                      nil,
                                      currentUrl as CFURL,
                                      nil,
                                      nil)
        found = true
      }
    }
    
    return found
  }
  
  func removeAsLoginItem() -> Bool {
    // remove any Clover.app logged in
    self.removeLaunchAtStartup() // call new method too (just in case store login item somewhere..)
    print("removeAsLoginItem()")
    let sharedFileList = LSSharedFileListCreate(nil,
                                                kLSSharedFileListSessionLoginItems.takeRetainedValue(),
                                                nil).takeRetainedValue()
    if let snapshot = LSSharedFileListCopySnapshot(sharedFileList, nil).takeRetainedValue() as?  [LSSharedFileListItem] {
      for item in snapshot {
        if let url = self.getLoginItemURL(for: item) {
          guard let info = NSDictionary(contentsOfFile: url.path.addPath("Contents/Info.plist")) as? [String: Any] else { continue }
          let bi = info[kCFBundleIdentifierKey as String] as? String
          if bi == Bundle.main.bundleIdentifier {
            let status = LSSharedFileListItemRemove(sharedFileList, item)
            print(status)
          }
        }
      }
    }
    return true
  }
  
  func amILoginItem() -> Bool {
    let sharedFileList = LSSharedFileListCreate(nil,
                                                kLSSharedFileListSessionLoginItems.takeRetainedValue(),
                                                nil).takeRetainedValue()
    if let snapshot = LSSharedFileListCopySnapshot(sharedFileList, nil).takeRetainedValue() as?  [LSSharedFileListItem] {
      for item in snapshot {
        if let url = self.getLoginItemURL(for: item) {
          guard let info = NSDictionary(contentsOfFile: url.path.addPath("Contents/Info.plist")) as? [String: Any] else { continue }
          let bi = info[kCFBundleIdentifierKey as String] as? String
          if bi == Bundle.main.bundleIdentifier && url == Bundle.main.bundleURL {
            return true
          }
        }
      }
    }
    return false
  }
}
