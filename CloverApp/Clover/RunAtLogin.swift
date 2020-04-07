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
}
