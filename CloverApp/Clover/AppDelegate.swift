//
//  AppDelegate.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

// MARK: NSApplication shared delegate
let AppSD = NSApplication.shared.delegate as! AppDelegate
let localeBundle = Bundle(path: Bundle.main.sharedSupportPath! + "/Lang.bundle")

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate, NSPopoverDelegate {
  var isInstalling : Bool = false
  var isInstallerOpen : Bool = false
  var popover : NSPopover?
  
  let statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
  
  var settingsWC: SettingsWindowController? = nil
  var installerWC : InstallerWindowController? = nil
  
  var daSession : DASession? = nil
  var daContext : UnsafeMutablePointer<Int> = UnsafeMutablePointer<Int>.allocate(capacity: 1)
  
  deinit {
    if (self.daSession != nil) {
      DASessionUnscheduleFromRunLoop(self.daSession!,
                                     CFRunLoopGetCurrent(),
                                     CFRunLoopMode.defaultMode.rawValue)
    }
    
    self.daContext.deallocate()
  }

  func applicationWillFinishLaunching(_ notification: Notification) {
    /*
     do not activate the following code: is for debug only
     .. it delete stored preferences in the User Default dictionary
     
    if let domain = Bundle.main.bundleIdentifier {
      UDs.removePersistentDomain(forName: domain)
      UDs.synchronize()
    }
     the following is to dump info about all disk in the System
     let diskLog = "- getAlldisks() -----------------\n\(getAlldisks())\n- getAllESPs() ------------------\n\(getAllESPs())\n---------------------------------"
     try? diskLog.write(toFile: NSHomeDirectory().addPath("Desktop/diskLog.txt"), atomically: true, encoding: .utf8)
    */
   
    let pi = NSRunningApplication.current.processIdentifier
    let runnings =
      NSRunningApplication.runningApplications(withBundleIdentifier: Bundle.main.bundleIdentifier!)
    
    for app in runnings {
      if app.processIdentifier != pi {
        print("App terminated because another instance is already running, which is newer is unknown:")
        print("ensure the old version (may be the running one?) is not marked as login item and quit it.")
        NSApp.terminate(self)
      }
    }
  }
  
  func applicationDidFinishLaunching(_ aNotification: Notification) {
    let appImage = NSImage(named: "NSApplicationIcon")
    let size = self.statusItem.button!.frame.height - 3
    appImage?.size = NSMakeSize(size, size)
    self.statusItem.button?.image = appImage
    
    if let button = self.statusItem.button {
      button.target = self
      button.action = #selector(self.showPopover(_:))
      button.sendAction(on: [.leftMouseUp, .rightMouseUp])
      button.imagePosition = .imageLeft
    }
    
    self.settingsWC = SettingsWindowController.loadFromNib()
    
    NotificationCenter.default.addObserver(self,
                                           selector: #selector(self.reFreshDisksList),
                                           name: Notification.Name("DiskDisappeared"),
                                           object: nil)
    NSWorkspace.shared.notificationCenter.addObserver(self,
                                                      selector: #selector(self.reFreshDisksList),
                                                      name: NSWorkspace.didMountNotification,
                                                      object: nil)
    
    
    NSWorkspace.shared.notificationCenter.addObserver(self,
                                                      selector: #selector(self.reFreshDisksList),
                                                      name: NSWorkspace.didUnmountNotification,
                                                      object: nil)
    
    NSWorkspace.shared.notificationCenter.addObserver(self,
                                                      selector: #selector(self.reFreshDisksList),
                                                      name: NSWorkspace.didRenameVolumeNotification,
                                                      object: nil)
    
    
    self.daSession  = DASessionCreate(kCFAllocatorDefault)
    if (self.daSession != nil) {
      self.daContext.initialize(repeating: 0, count: 1)
      
      DASessionScheduleWithRunLoop(self.daSession!,
                                   CFRunLoopGetCurrent(),
                                   CFRunLoopMode.defaultMode.rawValue)
      
      DARegisterDiskDisappearedCallback(self.daSession!, nil, {
        (dis, ctx) -> Void in
        if (ctx != nil) {
          NotificationCenter.default.post(name: Notification.Name("DiskDisappeared"), object: dis)
        }
      }, self.daContext)
    }
    CFRunLoopRun()
  }

  @objc func showPopover(_ sender: NSStatusBarButton?) {
    if (self.popover == nil) {
      self.popover = NSPopover()
      self.popover?.animates = true
      self.popover?.contentViewController = self.settingsWC?.contentViewController
      self.popover?.behavior = .transient
      self.popover?.delegate = self
    }
    
    self.popover?.show(relativeTo: (sender?.bounds)!, of: sender!, preferredEdge: NSRectEdge.maxY)
    NSApp.activate(ignoringOtherApps: true)
  }
  
  @objc func reFreshDisksList() {
    (self.settingsWC?.contentViewController as? SettingsViewController)?.searchESPDisks()
    (self.installerWC?.contentViewController as? InstallerViewController)?.populateTargets()
  }
  
  
  func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
    return self.isInstalling ? .terminateLater : .terminateNow
  }

  func applicationWillTerminate(_ aNotification: Notification) {
    
  }


}

