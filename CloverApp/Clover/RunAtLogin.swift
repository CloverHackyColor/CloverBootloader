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
    UserDefaults.standard.set(success, forKey: kRunAtLogin)
    UserDefaults.standard.synchronize()
  }
  
  func removeLaunchAtStartup() {
    let success : Bool = SMLoginItemSetEnabled(gHelperID, false)
    UserDefaults.standard.set(success ? false : true, forKey: kRunAtLogin)
    UserDefaults.standard.synchronize()
  }
}
