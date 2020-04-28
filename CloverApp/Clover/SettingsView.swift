//
//  SettingsView.swift
//  Clover
//
//  Created by vector sigma on 30/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

final class LITabView: NSTabView {
  private var drawBack : Bool = false
  var tabIndex: Int = 0
  var lastTabIndex: Int {
    get {
      return self.tabIndex
    } set {
      self.tabIndex = newValue
    }
  }
  
  override var allowsVibrancy: Bool {
    return false
  }
  
  override var drawsBackground: Bool {
    get {
      return false
    } set {
      self.drawBack = false
    }
  }
}

final class SoundSlider : NSSlider {
  var field : NSTextField?
}

final class SettingsViewController:
  NSViewController, NSTextFieldDelegate, NSComboBoxDelegate, NSComboBoxDataSource, URLSessionDownloadDelegate {
  // MARK: Variables
  @IBOutlet var tabViewInfo : LITabView!
  // tab 0
  @IBOutlet var currentRevField : NSTextField!
  @IBOutlet var bootDeviceField : NSTextField!
  @IBOutlet var configPathField : FWTextField!
  // tab 1
  @IBOutlet var snField : NSTextField!
  @IBOutlet var modelField : NSTextField!
  @IBOutlet var boardIdField : NSTextField!
  // tab 2
  @IBOutlet var oemVendorField : NSTextField!
  @IBOutlet var oemProductField : NSTextField!
  @IBOutlet var oemBoardIdField : NSTextField!
  // tab 3
  @IBOutlet var nativeNVRAMField : NSTextField!
  @IBOutlet var bootTypeField : NSTextField!
  @IBOutlet var firmwareVendorfield : NSTextField!
  
  @IBOutlet var tabViewFunc : LITabView!
  @IBOutlet var tabViewFuncSelector : NSSegmentedControl!
  // tab 0
  @IBOutlet var disksPopUp : NSPopUpButton!
  @IBOutlet var autoMountButton : NSButton!
  @IBOutlet var unmountButton : NSButton!
  // tab 1
  @IBOutlet var themeBox : NSComboBox!
  @IBOutlet var openThemeButton : NSButton!
  @IBOutlet var themeUserCBox : NSComboBox!
  @IBOutlet var themeRepoField : NSTextField!
  // tab 2
  @IBOutlet var soundDevicePopUp : NSPopUpButton!
  @IBOutlet var soundVolumeSlider : SoundSlider!
  @IBOutlet var soundVolumeField : NSTextField!
  // tab 3
  @IBOutlet var autoSaveButton : NSButton!
  @IBOutlet var newPlistButton : NSButton!
  @IBOutlet var openDocumentButton : NSButton!
  @IBOutlet var disksPEPopUp : NSPopUpButton!
  @IBOutlet var plistsPopUp : NSPopUpButton!
  
  @IBOutlet var disbaleSleepProxyButton : NSButton!
  @IBOutlet var makeRootRWButton : NSButton!
  @IBOutlet var installDaemonButton : NSButton!
  @IBOutlet var unInstallDaemonButton : NSButton!
  @IBOutlet var timeIntervalPopUp : NSPopUpButton!
  @IBOutlet var checkNowButton : NSButton!
  @IBOutlet var lastUpdateCheckField : NSTextField!
  @IBOutlet var runAtLoginButton : NSButton!
  
  @IBOutlet var updateCloverButton : NSButton!
  @IBOutlet var installCloverButton : NSButton!
  
  @IBOutlet var progressBar : NSProgressIndicator!
  
  @IBOutlet var appVersionField : NSTextField!
  
  @IBOutlet var infoButton : NSButton!
  @IBOutlet var closeButton : NSButton!
  
  
  var lastReleaseRev : String? = nil
  var lastReleaseLink : String? = nil
  var currentRev : String = findCloverRevision() ?? kNotAvailable.locale
  var bootDevice : String? = findBootPartitionDevice()
  
  var timerUpdate : Timer? = nil
  var timeUpdateInterval : TimeInterval = UpdateInterval.never.rawValue
  
  var downloadTask : URLSessionDownloadTask? = nil
  var downloading : Bool = false
  
  var loaded : Bool = false
  
  override func awakeFromNib() {
    super.awakeFromNib()
    if !self.loaded {
      if #available(OSX 10.10, *) {} else {
        self.viewDidLoad()
      }
      self.loaded = true
    }
  }
  
  // MARK: View customization
  override func viewDidLoad() {
    if #available(OSX 10.10, *) {
      super.viewDidLoad()
    }

    let appVersion = Bundle.main.infoDictionary?["CFBundleShortVersionString"] as! String
    self.appVersionField.stringValue = "v\(appVersion)"
    localize(view: self.view)
    
    self.view.wantsLayer = true
    self.view.layer?.backgroundColor = NSColor.clear.cgColor
   
    self.tabViewFuncSelector.setImage(getCoreTypeImage(named: "SidebarInternalDisk", isTemplate: true), forSegment: 0)
    self.tabViewFuncSelector.setImage(getCoreTypeImage(named: "SidebarMoviesFolder", isTemplate: true), forSegment: 1)
    self.tabViewFuncSelector.setImage(getCoreTypeImage(named: "SidebarMusicFolder", isTemplate: true), forSegment: 2)
    self.tabViewFuncSelector.setImage(getCoreTypeImage(named: "SidebarDocumentsFolder", isTemplate: true), forSegment: 3)
    // sync
    self.tabViewFuncSelector.selectSegment(withTag: 0)
    self.tabViewFunc.selectTabViewItem(at: 0)

    AppSD.themeUser = UDs.string(forKey: kThemeUserKey) ?? kDefaultThemeUser
    AppSD.themeRepo = UDs.string(forKey: kThemeRepoKey) ?? kDefaultThemeRepo
    
    self.themeUserCBox.stringValue = AppSD.themeUser
    self.themeRepoField.stringValue = AppSD.themeRepo
    
    if #available(OSX 10.10, *) {
      self.themeUserCBox.placeholderString = kDefaultThemeUser
      self.themeRepoField.placeholderString = kDefaultThemeRepo
    }
    
    
    let authors = ["CloverHackyColor", "badruzeus", "LAbyOne"]
    self.themeUserCBox.removeAllItems()
    self.themeUserCBox.addItems(withObjectValues: authors)
    self.themeUserCBox.numberOfVisibleItems = authors.count
    self.themeUserCBox.completes = true
    
    let themeManagerIndexDir = NSHomeDirectory().addPath("Library/Application Support/CloverApp/Themeindex/\(AppSD.themeUser)_\(AppSD.themeRepo)")
    
    let tm = ThemeManager(user: AppSD.themeUser,
                          repo: AppSD.themeRepo,
                          basePath: themeManagerIndexDir,
                          indexDir: themeManagerIndexDir,
                          delegate: nil)

    let indexedThemes = tm.getIndexedThemesForAllRepositories()
    AppSD.themes = indexedThemes.sorted()
    
    if !AppSD.themes.contains("embedded") {
      AppSD.themes.append("embedded")
    }
    
    if !AppSD.themes.contains("random") {
      AppSD.themes.append("random")
    }

    self.themeBox.delegate = self
    self.themeBox.dataSource = self
    self.themeBox.usesDataSource = true
    self.themeBox.completes = true
    self.themeBox.reloadData()
    
    self.soundVolumeSlider.field = self.soundVolumeField
    self.soundVolumeField.stringValue = kNotAvailable.locale
    self.soundDevicePopUp.removeAllItems()
    self.soundDevicePopUp.addItem(withTitle: "...")
  
    let soundDevices = getSoundDevices()
    if soundDevices.count > 0 {
      for sd in soundDevices {
        self.soundDevicePopUp.addItem(withTitle: "\(sd.name) (\(sd.output.locale))")
        self.soundDevicePopUp.lastItem?.representedObject = sd
      }
    }
    self.soundVolumeSlider.doubleValue = 0
    
    self.progressBar.isHidden = true
    
    self.runAtLoginButton.state = UDs.bool(forKey: kRunAtLogin) ? .on : .off
    self.unmountButton.isEnabled = false
    self.autoMountButton.isEnabled = false
    self.autoMountButton.isHidden = true
    
    
    // tab 1
    self.snField.stringValue = getSystemSerialNumber() ?? kNotAvailable.locale
    self.modelField.stringValue = getEFIModel() ?? kNotAvailable.locale
    self.boardIdField.stringValue = getEFIBoardID() ?? kNotAvailable.locale
    // tab 2
    self.oemVendorField.stringValue = getOEMVendor() ?? kNotAvailable.locale
    self.oemProductField.stringValue = getOEMProduct() ?? kNotAvailable.locale
    self.oemBoardIdField.stringValue = getOEMBoard() ?? kNotAvailable.locale
    // tab 3
    self.plistsPopUp.removeAllItems()
    if #available(OSX 10.10, *) {
      self.autoSaveButton.state = UDs.bool(forKey: kAutoSavePlistsKey) ? .on : .off
    } else {
      self.autoSaveButton.isEnabled = false
      self.autoSaveButton.isHidden = true
      self.newPlistButton.isEnabled = false
      self.newPlistButton.isHidden = true
    }

    self.setUpInfo()
    self.setUpdateButton()
    
    if !fm.fileExists(atPath: Bundle.main.sharedSupportPath!.addPath("CloverV2/EFI")) {
      self.searchUpdate()
    }
  }
  
  func setUpInfo() {
    var osminorVersion : Int = 9
    if #available(OSX 10.10, *) {
      osminorVersion = ProcessInfo().operatingSystemVersion.minorVersion
    }
    
    self.setUpdateInformations()
    
    let nvram = getNVRAM()
    var nvdata = nvram?.object(forKey: "Clover.Theme") as? Data
    
    if #available(OSX 10.10, *) {
      self.themeBox.placeholderString = "NVRAM"
    }
    self.themeBox.stringValue = (nvdata != nil) ? String(decoding: nvdata!, as: UTF8.self) : ""
    self.themeBox.cell?.representedObject = self.themeBox.stringValue
    
    nvdata = nvram?.object(forKey: "Clover.RootRW") as? Data
    var value : String = String(decoding: nvdata ?? Data(), as: UTF8.self)
    self.makeRootRWButton.state = (value == "true") ? .on : .off
    
    nvdata = nvram?.object(forKey: "Clover.SoundIndex") as? Data
    if (nvdata != nil) {
      let soundIndex = nvdata!.reversed().reduce(0) { $0 << 8 + UInt64($1) }
      if soundIndex >= 0 && soundIndex <= 20 {
        for item in self.soundDevicePopUp.itemArray {
          if let sd = item.representedObject as? SoundDevice {
            if sd.index == Int(soundIndex) {
              self.soundDevicePopUp.select(item)
              break
            }
          }
        }
      }
    }

    nvdata = nvram?.object(forKey: "Clover.SoundVolume") as? Data
    if (nvdata != nil) {
      let volume = nvdata!.reversed().reduce(0) { $0 << 8 + UInt64($1) }
      if volume >= 0 && volume <= 100 {
        self.soundVolumeSlider.field?.stringValue = "\(volume)%"
        self.soundVolumeSlider.doubleValue = Double(volume)
      }
    }
    
    // copy the Swift Framenworks for oldest OSes
    if osminorVersion < 15 {
      self.makeRootRWButton.isEnabled = false
      self.makeRootRWButton.isHidden = true
    }
    
    nvdata = nvram?.object(forKey: "Clover.DisableSleepProxyClient") as? Data
    value = String(decoding: nvdata ?? Data(), as: UTF8.self)
    self.disbaleSleepProxyButton.state = (value == "true") ? .on : .off
    
    
    // tab 3
    let fwname = getFirmawareVendor()
    self.firmwareVendorfield.stringValue = "Firmware: \(fwname ?? kNotAvailable.locale)"
    
    let emuvarPresent = nvram?.object(forKey: "EmuVariableUefiPresent") != nil
    var nvramIsNative : String? = nil
    let isUEFI : String = isLegacyFirmware() ? "No".locale.lowercased() : "Yes".locale.lowercased()
    
    
    if isUEFI == "Yes".locale.lowercased() {
      nvramIsNative = emuvarPresent ? "No".locale.lowercased() : "Yes".locale.lowercased()
    }
    
    self.nativeNVRAMField.stringValue =  "\("NVRAM is native:".locale) " + (nvramIsNative ?? "unknown".locale)
    
    self.bootTypeField.stringValue = "UEFI: \(isUEFI)"
    
    
    let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
    self.unInstallDaemonButton.isEnabled = daemonExist
    
    self.searchDisks()
    self.searchESPDisks()
    
    if self.disksPEPopUp.indexOfSelectedItem == 0 {
      for item in self.disksPEPopUp.itemArray {
        if let disk = item.representedObject as? String {
          if self.bootDevice != nil && disk == self.bootDevice {
            self.disksPEPopUp.select(item)
            self.volumePopUpPressed(self.disksPEPopUp)
            break
          }
        }
      }
    }
    
    let itervals = ["never", "daily", "weekly", "monthly"]
    self.timeIntervalPopUp.removeAllItems()
    for i in itervals {
      self.timeIntervalPopUp.addItem(withTitle: i.locale)
      self.timeIntervalPopUp.lastItem?.representedObject = i
    }
    
    if (UDs.object(forKey: kUpdateSearchInterval) == nil) {
      // search update the first time..
      UDs.set("monthly", forKey: kUpdateSearchInterval)
      UDs.synchronize()
    }
    
    let def = UDs.string(forKey: kUpdateSearchInterval)!
    for i in self.timeIntervalPopUp.itemArray {
      let c = i.representedObject as! String
      if def == c {
        self.timeIntervalPopUp.select(i)
        break
      }
    }
    
    if let date : Date = UDs.object(forKey: kLastSearchUpdateDateKey) as? Date {
      let now = DateFormatter.localizedString(from: date, dateStyle: .short, timeStyle: .short)
      self.lastUpdateCheckField.stringValue = "\("last checked:".locale) \(now)"
    } else {
      self.lastUpdateCheckField.stringValue = "\("last checked:".locale) \("never".locale)"
    }
    
    self.timerUpdate = Timer.scheduledTimer(timeInterval: 60 * 60,
                                            target: self,
                                            selector: #selector(self.setUpdateTimer),
                                            userInfo: nil,
                                            repeats: true)
    
    if #available(OSX 10.10, *) {
      let clickVersion = NSClickGestureRecognizer(target: self, action: #selector(goToWebSite))
      self.appVersionField.addGestureRecognizer(clickVersion)
    }
  }
  
  func setUpdateInformations() {
    var cloverAvailable = false
    var title = "Download".locale
    if fm.fileExists(atPath: Bundle.main.sharedSupportPath!.addPath("CloverV2/EFI")) {
      cloverAvailable = true
      let rev = findCloverRevision(at: Bundle.main.sharedSupportPath!.addPath("CloverV2/EFI")) ?? "0000"
      title = "\("Install Clover".locale) \(rev)"
    }
    self.installCloverButton.title = title
    
    self.bootDevice = findBootPartitionDevice()
    if (self.bootDevice != nil) {
      let bootDeviceUUID = getMediaUUID(from: self.bootDevice ?? "")
      title = "\(self.bootDevice!)"
      if (bootDeviceUUID != nil) {
        title += " \(bootDeviceUUID!)"
      }
    } else {
      title = kNotAvailable.locale
    }
    
    self.bootDeviceField.stringValue = title
    
    self.configPathField.stringValue = findConfigPath() ?? kNotAvailable.locale
    title = "\("Current Clover revision".locale): \(self.currentRev)"
    self.currentRevField.stringValue = title
    
    let last : String = (self.lastReleaseRev == nil) ? kNotAvailable.locale : "r\(self.lastReleaseRev!)"
    title = "\("Update Clover available".locale): \(last)"
    
    self.installCloverButton.isEnabled = cloverAvailable
  }
  
  override var representedObject: Any? {
    didSet {
      // Update the view, if already loaded.
    }
  }
  
  // MARK: Go to Website
  @objc func goToWebSite() {
    let link = "https://github.com/CloverHackyColor/CloverBootloader"
    NSWorkspace.shared.open(URL(string: link)!)
  }
  
  @IBAction func goToTopic(_ sender: NSButton?) {
    let link = "https://www.insanelymac.com/forum/topic/341047-cloverapp-testing/"
    NSWorkspace.shared.open(URL(string: link)!)
  }
  
  // MARK: Disks
  func searchDisks() {
    let selected = self.disksPEPopUp.selectedItem?.representedObject as? String
    self.disksPEPopUp.removeAllItems()
    self.disksPEPopUp.addItem(withTitle: "Select a disk..".locale)
    
    for v in getVolumes() {
      if !isWritable(diskOrMtp: v) {
        continue
      }
      if !fm.fileExists(atPath: v.addPath("EFI/CLOVER")) {
        continue
      }
      if let disk = getBSDName(of: v) {
        if kBannedMedia.contains(getVolumeName(from: disk) ?? "") {
          continue
        }
        let parentDiskName : String = getMediaName(from: getBSDParent(of: disk) ?? "") ?? kNotAvailable.locale
        let fs = getFS(from: v) ?? kNotAvailable.locale
        let title : String = "\(disk), \(fs), \("mount point".locale): \(v), \(parentDiskName)"
        self.disksPEPopUp.addItem(withTitle: title)
        self.disksPEPopUp.lastItem?.representedObject = disk
        if disk == self.bootDevice {
          let image : NSImage = NSImage(named: "NSApplicationIcon")!.copy() as! NSImage
          image.size = NSMakeSize(16, 16)
          self.disksPEPopUp.lastItem?.image = image
        } else if let image : NSImage = getIconFor(volume: disk) {
          image.size = NSMakeSize(16, 16)
          self.disksPEPopUp.lastItem?.image = image
        }
      }
    }
    
    if (selected != nil) {
      for item in self.disksPEPopUp.itemArray {
        if let d = item.representedObject as? String {
          if d == selected {
            self.disksPEPopUp.select(item)
            break
          }
        }
      }
    }
  }
  
  func searchESPDisks() {
    self.unmountButton.isEnabled = false
    let selected = self.disksPopUp.selectedItem?.representedObject as? String
    
    self.disksPopUp.removeAllItems()
    self.disksPopUp.addItem(withTitle: "Select a disk..".locale)
    for e in getAllESPs() {
      if isWritable(diskOrMtp: e) {
        let fs = getFS(from: e) ?? kNotAvailable.locale
        let mp = getMountPoint(from: e) ?? kNotAvailable.locale
        let parentDiskName : String = getMediaName(from: getBSDParent(of: e) ?? "") ?? kNotAvailable.locale
        let title : String = "\(e), \(fs), \("mount point".locale): \(mp), \(parentDiskName)"
        self.disksPopUp.addItem(withTitle: title)
        self.disksPopUp.lastItem?.representedObject = e
        if e == self.bootDevice {
          let image : NSImage = NSImage(named: "NSApplicationIcon")!.copy() as! NSImage
          image.size = NSMakeSize(16, 16)
          self.disksPopUp.lastItem?.image = image
        } else if let image : NSImage = getIconFor(volume: e) {
          image.size = NSMakeSize(16, 16)
          self.disksPopUp.lastItem?.image = image
        }
      }
    }
    
    if (selected != nil) {
      for item in self.disksPopUp.itemArray {
        if let d = item.representedObject as? String {
          if d == selected {
            self.disksPopUp.select(item)
            if isMountPoint(path: d) {
              self.unmountButton.isEnabled = true
            }
            break
          }
        }
      }
    }
  }
  
  // MARK: Mount ESPs
  @IBAction func mountESP(_ sender: NSPopUpButton!) {
    self.unmountButton.isEnabled = false
    self.autoMountButton.isEnabled = false
    self.autoMountButton.animator().isHidden = true
    if let disk = sender.selectedItem?.representedObject as? String {
      if !isMountPoint(path: disk) {
        //DispatchQueue.global(qos: .background).async {
          let task = Process()
          let cmd = "diskutil mount \(disk)"
          let msg = String(format: "Clover wants to mount %@", disk)
          let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
          
          if #available(OSX 10.12, *) {
            task.launchPath = "/usr/bin/osascript"
            task.arguments = ["-e", script]
          } else {
            task.launchPath = "/usr/sbin/diskutil"
            task.arguments = ["mount", disk]
          }
        
          task.terminationHandler = { t in
            if t.terminationStatus == 0 {
              DispatchQueue.main.async {
                self.unmountButton.isEnabled = true
              }
              NSWorkspace.shared.openFile(getMountPoint(from: disk) ?? "")
            } else {
              NSSound.beep()
            }
            DispatchQueue.main.async {
              self.autoMountButton.isEnabled = true
              self.autoMountButton.animator().isHidden = false
              self.autoMount(nil)
            }
          }
          task.launch()
        //}
      } else {
        self.unmountButton.isEnabled = true
        self.autoMountButton.isEnabled = true
        self.autoMountButton.animator().isHidden = false
        self.autoMount(nil)
        NSWorkspace.shared.openFile(getMountPoint(from: disk) ?? "")
      }
    }
  }
  
  // MARK: Auto mount
  @IBAction func autoMount(_ sender: NSButton?) {
    if self.disksPopUp.indexOfSelectedItem > 0 {
      let key = "Clover.MountEFI"
      if let disk = self.disksPopUp.selectedItem?.representedObject as? String {
        // find the uuid
        let uuid = getMediaUUID(from: disk)
        var deadline : DispatchTime = .now() + 4.0
        if (sender != nil) {
          if sender!.state == .on {
            if (uuid != nil) {
              setNVRAM(key: key, stringValue: uuid!)
            }
          } else {
            deleteNVRAM(key: key)
          }
        } else {
          deadline = .now() + 0.2
        }
        
        DispatchQueue.main.asyncAfter(deadline: deadline) {
          var value = ""
          if let nvram = getNVRAM() {
            let nvdata = nvram.object(forKey: key) as? Data
            value = String(decoding: nvdata ?? Data(), as: UTF8.self)
          }
         
          var enabled = (value == uuid)
          if !enabled {
            enabled = (value == "Yes" && disk == findBootPartitionDevice())
          }
          if !enabled {
            enabled = (value == disk || value == "/dev/\(disk)" || value == "/dev/r\(disk)" )
          }
          self.autoMountButton.state = enabled ? .on : .off
        }
        
      }
    }
    
  }
  
  // MARK: Umount
  @IBAction func umount(_ sender: NSButton!) {
    if let disk = self.disksPopUp.selectedItem?.representedObject as? String {
      if isMountPoint(path: disk) {
        //DispatchQueue.global(qos: .background).async {
          let cmd = "diskutil umount \(disk)"
          let msg = String(format: "Clover wants to umount %@", disk)
          let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
          
          let task = Process()
        
          if #available(OSX 10.12, *) {
            task.launchPath = "/usr/bin/osascript"
            task.arguments = ["-e", script]
          } else {
            task.launchPath = "/usr/sbin/diskutil"
            task.arguments = ["umount", "force", disk]
          }
          
          task.terminationHandler = { t in
            if t.terminationStatus != 0 {
              NSSound.beep()
            }
            DispatchQueue.main.async {
              self.searchDisks()
              self.searchESPDisks()
            }
          }
          task.launch()
        //}
      }
    }
  }

  // MARK: Controls actions
  @IBAction func openThemeManager(_ sender: NSButton!) {
    DispatchQueue.main.async {
      if #available(OSX 10.11, *) {
        if (AppSD.themeManagerWC == nil) {
          AppSD.themeManagerWC = ThemeManagerWC.loadFromNib()
        }
        
        AppSD.themeManagerWC?.showWindow(self)
      } else {
        if (AppSD.themeManagerWC == nil) {
          AppSD.themeManagerWC = ThemeManagerWC.loadFromNib()
        }
        
        AppSD.themeManagerWC?.showWindow(self)
      }
      NSApp.activate(ignoringOtherApps: true)
    }
  }
  
  func numberOfItems(in comboBox: NSComboBox) -> Int {
    return AppSD.themes.count
  }

  func comboBox(_ comboBox: NSComboBox, indexOfItemWithStringValue string: String) -> Int {
    return AppSD.themes.firstIndex(of: string) ?? -1
  }
  func comboBox(_ comboBox: NSComboBox, objectValueForItemAt index: Int) -> Any? {
    return AppSD.themes[index]
  }
  
  func comboBox(_ comboBox: NSComboBox, completedString string: String) -> String? {
    for theme in AppSD.themes {
      if theme.lowercased().hasPrefix(string.lowercased()) {
        return theme
      }
    }
    return nil
  }
  
  func comboBoxSelectionDidChange(_ notification: Notification) {
    if (notification.object as? NSComboBox) == self.themeBox {
      let themeName = self.themeBox.itemObjectValue(at: self.themeBox.indexOfSelectedItem) as! String
      self.set(theme: themeName, sender: self.themeBox)
    } else if (notification.object as? NSComboBox) == self.themeUserCBox {
      let val = self.themeUserCBox.itemObjectValue(at: self.themeUserCBox.indexOfSelectedItem) as! String
      UDs.set(val, forKey: kThemeUserKey)
      UDs.synchronize()
      AppSD.themeUser = val
    }
  }
  
  func controlTextDidEndEditing(_ obj: Notification) {
    if (obj.object as? NSComboBox) == self.themeBox {
      let themeName = self.themeBox.stringValue
      self.set(theme: themeName, sender: self.themeBox)
      return
    }
    
    if let cbox = obj.object as? NSComboBox {
      if cbox == self.themeUserCBox {
        if cbox.stringValue.count > 0 {
          UDs.set(cbox.stringValue, forKey: kThemeUserKey)
        } else {
          UDs.set(kDefaultThemeUser, forKey: kThemeUserKey)
        }
        AppSD.themeUser = UDs.string(forKey: kThemeUserKey) ?? kDefaultThemeUser
      }
    } else if let field = obj.object as? NSTextField {
      if field == self.themeRepoField {
        if field.stringValue.count > 0 {
          UDs.set(field.stringValue, forKey: kThemeRepoKey)
        } else {
          UDs.set(kDefaultThemeRepo, forKey: kThemeRepoKey)
        }
        UDs.synchronize()
        AppSD.themeRepo = UDs.string(forKey: kThemeRepoKey) ?? kDefaultThemeRepo
      }
    }
  }
  
  @IBAction func generateConfig(_ sender: NSButton!) {
    DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
      if var conf = CloverConfig().generateCloverConfig() {
        let appVersion = Bundle.main.infoDictionary?["CFBundleShortVersionString"] as! String
        conf["GenTool"] = "Clover r\(AppSD.CloverRevision) by Clover.app v\(appVersion)"
        let configName : String = (conf["ConfigName"] as? String ?? "config")
        let dir = NSHomeDirectory().addPath("Desktop/Clover_config")
        let fullPath = dir.addPath("\(configName).plist")
        var isDir : ObjCBool = false
        if fm.fileExists(atPath: dir, isDirectory: &isDir) {
          if !isDir.boolValue {
            try? fm.removeItem(atPath: dir)
          }
        }
        
        if !fm.fileExists(atPath: dir) {
          try? fm.createDirectory(atPath: dir,
                                  withIntermediateDirectories: false,
                                  attributes: nil)
        }
        if NSDictionary(dictionary: conf).write(toFile: fullPath,
                                                atomically: false) {
          loadPlist(at: fullPath)
        } else {
          NSSound.beep()
        }
      } else {
        NSSound.beep()
      }
    }
  }
  
  @IBAction func createNewPlist(_ sender: NSButton!) {
    DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
      NSDocumentController.shared.newDocument(self)
    }
  }
  
  @IBAction func autosaveDocuments(_ sender: NSButton!) {
    UDs.set(sender.state == .on, forKey: kAutoSavePlistsKey)
    UDs.synchronize()
  }
  
  @IBAction func searchPanelForPlist(_ sender: NSButton!) {
    DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
      let op = NSOpenPanel()
      op.allowsMultipleSelection = false
      op.canChooseDirectories = false
      op.canCreateDirectories = false
      op.canChooseFiles = true
      op.allowedFileTypes = ["plist"]
      
      // make the app regular
      NSApp.setActivationPolicy(.regular)
      
      op.begin { (result) in
        if result == .OK {
          if let path = op.url?.path {
            loadPlist(at: path) // this will make the app regular again in 10.11+
          }
        } else {
          // check if a document is opened some where
          if NSDocumentController.shared.documents.count == 0 {
            NSApp.setActivationPolicy(.accessory)
          }
        }
      }
    }
  }
  
  @IBAction func volumePopUpPressed(_ sender: NSPopUpButton!) {
    DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
      self.plistsPopUp.removeAllItems()
      self.plistsPopUp.addItem(withTitle: "...")
      if let disk = sender.selectedItem?.representedObject as? String {
        if let mp = getMountPoint(from: disk) {
          let clover = mp.addPath("EFI/CLOVER")
          if fm.fileExists(atPath: clover) {
            if fm.fileExists(atPath: clover.addPath("config.plist")) {
              self.plistsPopUp.addItem(withTitle: "config.plist")
              self.plistsPopUp.lastItem?.representedObject = clover.addPath("config.plist")
            }
            let enumerator = fm.enumerator(atPath: clover)
            while let file = enumerator?.nextObject() as? String {
              if file.fileExtension == "plist" &&
                (file.range(of: "kexts/") == nil) &&
                !file.hasPrefix("themes/") &&
                file != "pref.plist" &&
                file != "config.plist"{
                self.plistsPopUp.addItem(withTitle: file)
                self.plistsPopUp.lastItem?.representedObject = clover.addPath(file)
              }
            }
          }
        } else {
          NSSound.beep()
        }
      }
    }
  }
  
  @IBAction func openCloverPlist(_ sender: NSPopUpButton!) {
    if let path = sender.selectedItem?.representedObject as? String {
      if !fm.fileExists(atPath: path) {
        NSSound.beep()
        return
      }
      loadPlist(at: path)
    }
  }
  
  @IBAction func installClover(_ sender: NSButton!) {
    
    let myPath = Bundle.main.bundlePath.lowercased()
    var isXcode = false
    var showAlert = false
    if (myPath.range(of: "/deriveddata/") != nil) {
      showAlert = true
      isXcode = true
    } else if (myPath.range(of: "/apptranslocation/") != nil) {
      showAlert = true
    }
    
    if showAlert {
      let alert = NSAlert()
      alert.alertStyle = .critical
      let path = isXcode ? "Xcode" : "App Trans Location"
      alert.messageText = "Running from \(path)"
      
      if isXcode {
        alert.informativeText = "The Installer should not run from \(path) because it can reuse old resources like old built dependencies (in the DerivedData directory) instead ones from the app bundle in which you may had made changes.\nThis appear to be a bug in the Xcode build system, so please move Clover.app somewhere else for real installations, unless you are a Developer and you're just testing."
      } else {
        alert.informativeText = "The Installer cannot run from the \(path) and surely it will fail the installation.\nPlease move Clover.app somewhere else or use\n\nsudo spctl --master-disable\n\nThanks.\n\nP.S. Clover.app is not code signed because this require a paid Apple Developer certificate We cannot effort. If you have doubs, officiale releases are here:\n\n https://github.com/CloverHackyColor/CloverBootloader/releases\n\n..and you can build the app by your self if you prefear as this project is completely open source!"
      }
      
      if isXcode {
        alert.addButton(withTitle: "OK".locale)
        alert.runModal()
      } else {
        alert.addButton(withTitle: "Close".locale)
        alert.runModal()
        return
      }
      
    }
    DispatchQueue.main.async {
      if #available(OSX 10.11, *) {
        if (AppSD.installerWC == nil) {
          AppSD.installerWC = InstallerWindowController.loadFromNib()
        }
        
        AppSD.installerWC?.showWindow(self)
      } else {
        if (AppSD.installerOutWC == nil) {
          AppSD.installerOutWC = InstallerOutWindowController.loadFromNib()
        }
        
        AppSD.installerOutWC?.showWindow(self)
      }
      //NSApp.activate(ignoringOtherApps: true)
    }
  }
  
  @IBAction func installDaemon(_ sender: NSButton!) {
    let daemonInstallerPath = Bundle.main.executablePath!.deletingLastPath.addPath("daemonInstaller")
  
    if #available(OSX 10.11, *) {
      let task = Process()
      let msg = "Install CloverDaemonNew".locale
      let script = "do shell script \"'\(daemonInstallerPath)'\" with prompt \"\(msg)\" with administrator privileges"
      
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      
      task.terminationHandler = { t in
        if t.terminationStatus != 0 {
          print(t.terminationReason)
          NSSound.beep()
        }
        
        DispatchQueue.main.async {
          let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
          self.unInstallDaemonButton.isEnabled = daemonExist
        }
      }
      task.launch()
    } else {
      let script = "do shell script \"'\(daemonInstallerPath)'\" with administrator privileges"
      var err : NSDictionary? = nil
      let result : NSAppleEventDescriptor = NSAppleScript(source: script)!.executeAndReturnError(&err)
      if (err != nil) {
        NSSound.beep()
        print(result.stringValue ?? "")
      }
      let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
      self.unInstallDaemonButton.isEnabled = daemonExist
    }
  }
  
  @IBAction func unInstallDaemon(_ sender: NSButton!) {
    let daemonPath = Bundle.main.executablePath!.deletingLastPath.addPath("CloverDaemonNew")
    if #available(OSX 10.11, *) {
      let task = Process()
      let msg = "Uninstall CloverDaemonNew".locale
      
      let script = "do shell script \"'\(daemonPath)' --uninstall\" with prompt \"\(msg)\" with administrator privileges"
      
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      
      task.terminationHandler = { t in
        if t.terminationStatus != 0 {
          print(t.terminationReason)
          NSSound.beep()
        }
        
        DispatchQueue.main.async {
          let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
          self.unInstallDaemonButton.isEnabled = daemonExist
        }
      }
      task.launch()
    } else {
      let script = "do shell script \"'\(daemonPath)' --uninstall\" with administrator privileges"
      var err : NSDictionary? = nil
      let result : NSAppleEventDescriptor = NSAppleScript(source: script)!.executeAndReturnError(&err)
      if (err != nil) {
        NSSound.beep()
        print(result.stringValue ?? "")
      }
      let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
      self.unInstallDaemonButton.isEnabled = daemonExist
    }
  }
  
  @IBAction func setUpdateInterval(_ sender: NSPopUpButton!) {
    if let selected = sender.selectedItem?.representedObject as? String {
      UDs.set(selected, forKey: kUpdateSearchInterval)
      self.setUpdateTimer()
    }
  }
  
  @IBAction func checkNow(_ sender: NSButton!) {
    if self.downloading {
      NSSound.beep()
      return
    }
    self.searchUpdate()
  }
  
  // MARK: Run At Login
  @IBAction func runAtLogin(_ sender: NSButton!) {
    if sender.state == .on {
      AppSD.setLaunchAtStartup()
    } else {
      AppSD.removeLaunchAtStartup()
    }
    
    // check the result
    sender.state = UDs.bool(forKey: kRunAtLogin) ? .on : .off
  }
  
  // MARK: NVRAM editing
  func themeBoxSelected(_ sender: NSComboBox!) {
    let themeName = sender.itemObjectValue(at: sender.indexOfSelectedItem) as! String
    self.set(theme: themeName, sender: sender)
  }
  
  private func set(theme name: String, sender: NSComboBox) {
    let key = "Clover.Theme"
    var nvramValue : String = ""
    if let data = getNVRAM()?.object(forKey: key) as? Data {
      nvramValue = String(decoding: data, as: UTF8.self)
    }
    
    if nvramValue != name {
      if name.count == 0 {
        deleteNVRAM(key: key.nvramString)
      } else {
        setNVRAM(key: key, stringValue: name.nvramString)
      }
    } else {
      return
    }
    
    DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
      var value = ""
      if let nvram = getNVRAM() {
        let nvdata = nvram.object(forKey: key) as? Data
        value = String(decoding: nvdata ?? Data(), as: UTF8.self)
      }
      sender.stringValue = value
    }
  }
  
  
  @IBAction func soundSliderDidMove(_ sender: SoundSlider!) {
    sender.field?.stringValue = "\(Int(sender.doubleValue))%"
    
    let key = "Clover.SoundVolume"
    var num = Int(sender.doubleValue)
    let value = String(format: "%%%02x", UInt8(num))
    setNVRAM(key: key, stringValue: value)
    num = 0
    DispatchQueue.main.asyncAfter(deadline: .now() + 3.0) {
      if let nvram = getNVRAM() {
        if let nvdata = nvram.object(forKey: key) as? Data {
          let volume = nvdata.reversed().reduce(0) { $0 << 8 + UInt64($1) }
          num = (volume >= 0 && volume <= 100) ? Int(num) : 0
        }
      }
      sender.field?.stringValue = "\(num)%"
      sender.doubleValue = Double(num)
    }
  }
  
  @IBAction func soundDeviceSelected(_ sender: NSPopUpButton!) {
    let key = "Clover.SoundIndex"
    if let sd = sender.selectedItem?.representedObject as? SoundDevice {
      let value = String(format: "%%%02x", UInt8(sd.index))
      setNVRAM(key: key, stringValue: value)
    }
    
    DispatchQueue.main.asyncAfter(deadline: .now() + 3.0) {
      var index : Int = -1
      if let nvram = getNVRAM() {
        if let nvdata = nvram.object(forKey: key) as? Data {
          let val = nvdata.reversed().reduce(0) { $0 << 8 + UInt64($1) }
          index = (val >= 0 && val < self.soundDevicePopUp.numberOfItems) ? Int(val) : -1
        }
      }
      
      if index >= 0 {
        var found = false
        for item in self.soundDevicePopUp.itemArray {
          if let sd = item.representedObject as? SoundDevice {
            if sd.index == index {
              found = true
              self.soundDevicePopUp.select(item)
              break
            }
          }
        }
        if !found {
          self.soundDevicePopUp.selectItem(at: 0)
        }
      } else {
        self.soundDevicePopUp.selectItem(at: 0)
      }
    }
  }
  
  @IBAction func disableSleepProxy(_ sender: NSButton!) {
    let key = "Clover.DisableSleepProxyClient"
    if sender.state == .on {
      setNVRAM(key: key, stringValue: "true"/*, error: &error*/)
    } else {
      deleteNVRAM(key: key)
    }
    
    DispatchQueue.main.asyncAfter(deadline: .now() + 4.0) {
      var value = ""
      if let nvram = getNVRAM() {
        let nvdata = nvram.object(forKey: key) as? Data
        value = String(decoding: nvdata ?? Data(), as: UTF8.self)
      }
      self.disbaleSleepProxyButton.state = (value == "true") ? .on : .off
    }
  }
  
  @IBAction func makeRootRW(_ sender: NSButton!) {
    let key = "Clover.RootRW"
    if sender.state == .on {
      setNVRAM(key: key, stringValue: "true")
    } else {
      deleteNVRAM(key: key)
    }
    
    DispatchQueue.main.asyncAfter(deadline: .now() + 4.0) {
      var value = ""
      if let nvram = getNVRAM() {
        let nvdata = nvram.object(forKey: key) as? Data
        value = String(decoding: nvdata ?? Data(), as: UTF8.self)
      }
      self.makeRootRWButton.state = (value == "true") ? .on : .off
    }
  }
  
  @IBAction func readDaemonLog(_ sender: NSButton!) {
    let cmd = "cat /Library/Logs/CloverEFI/clover.daemon.log > /tmp/clover.daemon.log && open /tmp/clover.daemon.log"
    if #available(OSX 10.11, *) {
      let task = Process()
      
      let msg = "CloverDaemon log"
      let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
      
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      
      task.terminationHandler = { t in
        if t.terminationStatus == 0 {
          NSWorkspace.shared.openFile("/tmp/clover.daemon.log")
        } else {
          NSSound.beep()
        }
      }
      task.launch()
    } else {
      let script = "do shell script \"\(cmd)\" with administrator privileges"
      var err : NSDictionary? = nil
      let _ : NSAppleEventDescriptor = NSAppleScript(source: script)!.executeAndReturnError(&err)
      if (err != nil) {
        NSSound.beep()
      } else {
        NSWorkspace.shared.openFile("/tmp/clover.daemon.log")
      }
    }
  }
  
  @IBAction func readbdmesg(_ sender: NSButton!) {
    if let bdmesg = dumpBootlog() {
      do {
        try bdmesg.write(toFile: "/tmp/bdmesg.log", atomically: true, encoding: .utf8)
        NSWorkspace.shared.openFile("/tmp/bdmesg.log")
      } catch {
        NSSound.beep()
      }
    } else {
      NSSound.beep()
    }
  }
  
  
  // MARK: Search update
  @objc func setUpdateTimer() {
    // check last date update was checked
    var ti : TimeInterval = UpdateInterval.never.rawValue
    let def = (UDs.value(forKey: kUpdateSearchInterval) as? String) ?? "never"
    switch def {
    case "never":
      ti = UpdateInterval.never.rawValue
    case "daily":
      ti = UpdateInterval.daily.rawValue
    case "weekly":
      ti = UpdateInterval.weekly.rawValue
    case "monthly":
      ti = UpdateInterval.monthly.rawValue
    default:
      break
    }
    
    
    // Time interval is what user defines less time elapsed
    if ti > 0 {
      let lastCheckDate : Date = (UDs.object(forKey: kLastSearchUpdateDateKey) as? Date) ?? Date()
      let secElapsed = Date().timeIntervalSinceReferenceDate - lastCheckDate.timeIntervalSinceReferenceDate
      
      if secElapsed >= ti {
        self.searchUpdate()
      }
    }
  }
  
  func setUpdateButton() {
    if (self.lastReleaseRev != nil && self.lastReleaseLink != nil) {
      AppSD.statusItem.title = self.lastReleaseRev
      self.updateCloverButton.title = String(format: "Update to %@".locale, self.lastReleaseRev!)
      self.updateCloverButton.isEnabled = true
    } else {
      AppSD.statusItem.title = ""
      self.updateCloverButton.title = kNotAvailable.locale
      self.updateCloverButton.isEnabled = false
    }
  }
  
  @objc func searchUpdate() {
    getLatestReleases { (bootlink, bootvers, applink, appvers) in
      // Clover Bootloader
      self.lastReleaseLink = bootlink
      self.lastReleaseRev = bootvers
      let currRevNum : Int = Int(self.currentRev) ?? 0
      let lastRevNum : Int = Int(self.lastReleaseRev ?? "0") ?? 0
      let installerRev = findCloverRevision(at: Bundle.main.sharedSupportPath!.addPath("CloverV2/EFI"))
      let installerRevNum = Int(installerRev ?? "0") ?? 0
    
      if (self.lastReleaseLink != nil && self.lastReleaseRev != nil) {
        if !fm.fileExists(atPath: Bundle.main.sharedSupportPath!.addPath("CloverV2/EFI")) {
          DispatchQueue.main.async {
            self.updateClover(self.updateCloverButton)
          }
        }
      }
      
      if (self.lastReleaseLink != nil && self.lastReleaseRev != nil)
        && lastRevNum > 0
        && (lastRevNum > currRevNum) {
        UDs.set(self.lastReleaseLink!, forKey: kLastUpdateLink)
        UDs.set(self.lastReleaseRev!, forKey: kLastUpdateRevision)
        
        
        DispatchQueue.main.async {
          if #available(OSX 10.10, *) {
            AppSD.statusItem.button?.title = "\(lastRevNum)"
            AppSD.statusItem.button?.imagePosition = .imageLeft
          } else {
            AppSD.statusItem.title = "\(lastRevNum)"
          }
          
          if installerRevNum < lastRevNum {
            self.updateCloverButton.isEnabled = true
            self.updateCloverButton.title = String(format: "Update to r%d".locale, lastRevNum)
          } else {
            self.updateCloverButton.isEnabled = false
            self.updateCloverButton.title = kNotAvailable.locale
          }
        }
      } else {
        DispatchQueue.main.async {
          let ll = UDs.string(forKey: kLastUpdateLink)
          let lr = UDs.string(forKey: kLastUpdateRevision)
          let lrnum : Int = Int(lr ?? "0") ?? 0
          if ((ll != nil && lr != nil) && lrnum > currRevNum) {
            self.lastReleaseLink = ll
            self.lastReleaseRev = lr
            if #available(OSX 10.10, *) {
              AppSD.statusItem.button?.title = "\(lastRevNum)"
              AppSD.statusItem.button?.imagePosition = .imageLeft
            } else {
              AppSD.statusItem.title = "\(lastRevNum)"
            }
            self.updateCloverButton.isEnabled = true
            self.updateCloverButton.title = String(format: "Update to r%d".locale, lastRevNum)
          } else {
            if #available(OSX 10.10, *) {
              AppSD.statusItem.button?.title = ""
              AppSD.statusItem.button?.imagePosition = .imageOnly
            } else {
              AppSD.statusItem.title = ""
            }
            
            self.updateCloverButton.isEnabled = false
            self.updateCloverButton.title = kNotAvailable.locale
          }
        }
      }
      
      let date = Date()
      let now = DateFormatter.localizedString(from: date, dateStyle: .short, timeStyle: .short)
      DispatchQueue.main.async {
        self.lastUpdateCheckField.stringValue = "\("last checked:".locale) \(now)"
      }
      UDs.set(date, forKey: kLastSearchUpdateDateKey)
      UDs.synchronize()
      
      // Clover.app
      if appvers != nil && applink != nil && !self.downloading {
        let currVerS = Bundle.main.infoDictionary!["CFBundleShortVersionString"] as! String
        
        if appvers!.count > 0 && applink!.count > 0 {
          let currVer : Int = Int(currVerS.replacingOccurrences(of: ".", with: "")) ?? 0
          let newVer : Int = Int(appvers!.replacingOccurrences(of: ".", with: "")) ?? 0
          
          if newVer > currVer {
            if let url = URL(string: applink!) {
              // setup an alert
              DispatchQueue.main.async {
                let alert = NSAlert()
                alert.messageText = "Clover.app v\(appvers!)"
                alert.informativeText = "\(currVerS) => \(appvers!)"
                alert.alertStyle = .informational
                alert.addButton(withTitle: "Download".locale)
                alert.addButton(withTitle: "Close".locale)
                if alert.runModal() == .alertFirstButtonReturn {
                  self.updateCloverApp(at: url)
                }
              }
            }
          }
        }
      }
    }
  }
  
  // MARK: Download
  @IBAction func updateClover(_ sender: NSButton!) {
    if AppSD.isInstalling ||
      AppSD.isInstallerOpen ||
      self.lastReleaseLink == nil ||
      self.lastReleaseRev == nil ||
      self.downloading {
      NSSound.beep()
      return
    }
    
    self.progressBar.isHidden = false
    self.progressBar.doubleValue = 0.0
    let url = URL(string: self.lastReleaseLink!)
    
    let b = URLSessionConfiguration.default
    let session = Foundation.URLSession(configuration: b, delegate: self, delegateQueue: nil)
    
    if (url != nil) {
      self.downloading = true
      self.installCloverButton.isEnabled = false
      self.downloadTask = session.downloadTask(with: url!)
      self.downloadTask?.resume()
    }
  }
  
  func updateCloverApp(at url: URL) {
    if AppSD.isInstalling || AppSD.isInstallerOpen || self.downloading {
      NSSound.beep()
      return
    }
    self.progressBar.isHidden = false
    self.progressBar.doubleValue = 0.0
    
    let b = URLSessionConfiguration.default
    let session = Foundation.URLSession(configuration: b, delegate: self, delegateQueue: nil)
    
    self.downloading = true
    self.installCloverButton.isEnabled = false
    self.downloadTask = session.downloadTask(with: url)
    self.downloadTask?.resume()
  }
  
  func cleanUpdateDirectory() {
    let tempDir = "/tmp/CloverXXXXX\(NSUserName())Update"
    if fm.fileExists(atPath: tempDir) {
      try? fm.removeItem(atPath: tempDir)
    }
  }
  
  func urlSession(_ session: URLSession,
                  downloadTask: URLSessionDownloadTask,
                  didFinishDownloadingTo location: URL) {
   self.downloading = false
    DispatchQueue.main.async {
      self.progressBar.doubleValue = 100
      self.progressBar.isHidden = true
    }
    
    do {
      
      let lastPath = downloadTask.originalRequest!.url!.lastPathComponent
      let data = try Data(contentsOf: location)

      if (lastPath.fileExtension == "zip" && lastPath.hasPrefix("CloverV2")) ||
        (lastPath.hasPrefix("Clover.app") && lastPath.fileExtension == "pkg") {
        let isApp = lastPath.hasPrefix("Clover.app")

        // Decompress the zip archive
        // NSUserName() ensure the user have read write permissions
        let tempDir = "/tmp/CloverXXXXX\(NSUserName())Update"
        if fm.fileExists(atPath: tempDir) {
          try fm.removeItem(atPath: tempDir)
          
        }
        try fm.createDirectory(atPath: tempDir,
                               withIntermediateDirectories: false,
                               attributes: nil)
        
        let file = tempDir.addPath(lastPath)
        try data.write(to: URL(fileURLWithPath: file))
        
        if isApp {
          self.moveCloverApp(at: file)
        } else {
          DispatchQueue.main.async {
            let task : Process = Process()
            task.environment = ProcessInfo().environment
            let bash = "/bin/bash"
            // unzip -d output_dir/ zipfiles.zip
            let cmd = "/usr/bin/unzip -qq -d \(tempDir) \(file)"
            if #available(OSX 10.13, *) {
              task.executableURL = URL(fileURLWithPath: bash)
            } else {
              task.launchPath = bash
            }
            
            task.arguments = ["-c", cmd]
            task.terminationHandler = { t in
              if t.terminationStatus == 0 {
                self.replaceCloverV2(with: tempDir.addPath("CloverV2"))
              } else {
                self.cleanUpdateDirectory()
              }
            }
            
            task.launch()
          }
        }
        
      }
      
    } catch {
      print(error)
    }
  }
  
  func urlSession(_ session: URLSession,
                  task: URLSessionTask,
                  didCompleteWithError error: Error?) {
    self.downloading = false
    DispatchQueue.main.async {
      self.installCloverButton.isEnabled = true
    }
    if (error != nil) {
      print(error!.localizedDescription)
      self.cleanUpdateDirectory()
    }
  }
  
  func urlSession(_ session: URLSession,
                  downloadTask: URLSessionDownloadTask,
                  didWriteData bytesWritten: Int64,
                  totalBytesWritten: Int64,
                  totalBytesExpectedToWrite: Int64) {
    
    if totalBytesExpectedToWrite == NSURLSessionTransferSizeUnknown {
      DispatchQueue.main.async {
        self.progressBar.isIndeterminate = true
        self.progressBar.startAnimation(nil)
      }
    } else {
      let percentage : Double  = Double(totalBytesWritten) / Double(totalBytesExpectedToWrite) * 100

      DispatchQueue.main.async {
        self.progressBar?.isIndeterminate = false
        self.progressBar?.doubleValue = percentage
      }
    }
  }
  
  private func replaceCloverV2(with newOne: String) {
    var isDir : ObjCBool = false
    if fm.fileExists(atPath: newOne, isDirectory: &isDir) {
      if isDir.boolValue {
        // clean some unused stuff
        if fm.fileExists(atPath: newOne.addPath("rcScripts")) {
          try? fm.removeItem(atPath: newOne.addPath("rcScripts"))
        }
        if fm.fileExists(atPath: newOne.addPath("ThirdParty")) {
          try? fm.removeItem(atPath: newOne.addPath("ThirdParty"))
        }
        // let only one theme (Clovy) as we have a themes manager
        if fm.fileExists(atPath: newOne.addPath("themespkg/Clovy")) {
          if let themes = try? fm.contentsOfDirectory(atPath: newOne.addPath("themespkg")) {
            for file in themes {
              if file != "Clovy" {
                try? fm.removeItem(atPath: newOne.addPath("themespkg").addPath(file))
              }
            }
          }
        }
        
        do {
          if fm.fileExists(atPath: Cloverv2Path) {
            try fm.removeItem(atPath: Cloverv2Path)
          }
          try fm.copyItem(atPath: newOne, toPath: Cloverv2Path)
          self.cleanUpdateDirectory()
          
          DispatchQueue.main.async {
            self.lastReleaseRev = nil
            self.lastReleaseLink = nil
            self.setUpdateInformations()
            self.setUpdateButton()
          }
        } catch {
          print(error)
          NSSound.beep()
        }
      }
    }
  }
  
  private func moveCloverApp(at path: String) {
    // move to ~/Desktop/Clover_app_new
    let new = NSHomeDirectory().addPath("Desktop/Clover_app_download")
    if fm.fileExists(atPath: new) {
      try? fm.removeItem(atPath: new)
    }
    
    do {
      if fm.fileExists(atPath: new) {
        try fm.removeItem(atPath: new)
      }
      try fm.createDirectory(atPath: new, withIntermediateDirectories: false, attributes: nil)
      try fm.copyItem(atPath: path, toPath: new.addPath(path.lastPath))
      self.cleanUpdateDirectory()
      DispatchQueue.main.async {
        self.lastReleaseRev = nil
        self.lastReleaseLink = nil
        self.setUpdateInformations()
        self.setUpdateButton()
      }
      self.removeAttributes(at: new.addPath(path.lastPath))
      NSWorkspace.shared.openFile(new)
    } catch {
      print(error)
      NSSound.beep()
    }
  }
  
  private func removeAttributes(at path: String) {
    let task : Process = Process()
    task.environment = ProcessInfo().environment
    let bash = "/bin/bash"
    let cmd = "/usr/bin/xattr -rc \(path)"
    if #available(OSX 10.13, *) {
      task.executableURL = URL(fileURLWithPath: bash)
    } else {
      task.launchPath = bash
    }
    
    task.arguments = ["-c", cmd]
    task.terminationHandler = { t in
      if t.terminationStatus != 0 {
        NSSound.beep()
      }
    }
    
    task.launch()
  }
  
  // MARK: Close
  @IBAction func closeApp(_ sender: NSButton?) {
    NSApp.terminate(nil)
  }
  
}

// MARK: - Tab animation
extension SettingsViewController: NSTabViewDelegate {
  @IBAction func selectTabInfo(_ sender: NSSegmentedControl!) {
    let count = self.tabViewInfo.tabViewItems.count
    
    let index = self.tabViewInfo.indexOfTabViewItem(self.tabViewInfo.selectedTabViewItem!)
    
    if sender.selectedSegment == 1 {
      if index >= (count - 1) {
        NSSound.beep()
        return
      }
      self.tabViewInfo.selectNextTabViewItem(nil)
    } else {
      if index <= 0 {
        NSSound.beep()
        return
      }
      self.tabViewInfo.selectPreviousTabViewItem(nil)
    }
  }
  
  @IBAction func selectFuncTab(_ sender: NSSegmentedControl!) {
    self.tabViewFunc.animator().selectTabViewItem(at: sender.indexOfSelectedItem)
  }
  
  func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
    if (tabViewItem != nil) {
      if let t = tabView as? LITabView {
        let position = CABasicAnimation(keyPath: "position")
        
        if t.lastTabIndex > t.indexOfTabViewItem(tabViewItem!) {
          position.fromValue = NSValue(point: CGPoint(x: CGFloat(tabViewItem!.view!.frame.origin.x - 520), y: CGFloat(tabViewItem!.view!.frame.origin.y)))
        } else {
          position.fromValue = NSValue(point: CGPoint(x: CGFloat(tabViewItem!.view!.frame.origin.x + 520), y: CGFloat(tabViewItem!.view!.frame.origin.y)))
        }
        position.toValue = NSValue(point: CGPoint(x: CGFloat(tabViewItem!.view!.frame.origin.x), y: CGFloat(tabViewItem!.view!.frame.origin.y)))
        tabViewItem?.view?.layer?.add(position, forKey: "controlViewPosition")
        tabViewItem?.view?.animations = [
          "frameOrigin" : position
        ]
        
        tabViewItem?.view?.animator().frame.origin = CGPoint(x: CGFloat(tabViewItem!.view!.frame.origin.x), y: CGFloat(tabViewItem!.view!.frame.origin.y))
        t.lastTabIndex = t.indexOfTabViewItem(tabViewItem!)
      }
      
    }
  }
}

// MARK: Settings Window controller
final class SettingsWindowController: NSWindowController, NSWindowDelegate {
  var viewController : NSViewController? = nil
  override var contentViewController: NSViewController? {
    get {
      self.viewController
    }
    set {
      self.viewController = newValue
    }
  }

  class func loadFromNib() -> SettingsWindowController? {
    var topLevel: NSArray? = nil
    Bundle.main.loadNibNamed("Settings", owner: self, topLevelObjects: &topLevel)
    if (topLevel != nil) {
      var wc : SettingsWindowController? = nil
      for o in topLevel! {
        if o is SettingsWindowController {
          wc = o as? SettingsWindowController
        }
      }
      
      for o in topLevel! {
        if o is SettingsViewController {
          wc?.contentViewController = o as! SettingsViewController
        }
      }
      return wc
    }
    return nil
  }
}


