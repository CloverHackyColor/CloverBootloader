//
//  Installer.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

// MARK: Installer Window controller
final class InstallerOutWindowController: NSWindowController, NSWindowDelegate {
  var viewController : NSViewController? = nil
  override var contentViewController: NSViewController? {
    get {
      self.viewController
    }
    set {
      self.viewController = newValue
    }
  }
  func windowShouldClose(_ sender: NSWindow) -> Bool {
    if AppSD.isInstalling  {
      return false
    }
    let settingVC = AppSD.settingsWC?.contentViewController as? SettingsViewController
    settingVC?.disksPopUp.isEnabled = true
    settingVC?.updateCloverButton.isEnabled = true
    settingVC?.searchESPDisks()
    AppSD.isInstallerOpen = false
    self.window = nil
    self.close()
    AppSD.installerWC = nil // remove a strong reference
    AppSD.setActivationPolicy()
    return true
  }
  
  func windowWillClose(_ notification: Notification) {
    
  }

  class func loadFromNib() -> InstallerOutWindowController? {
    var topLevel: NSArray? = nil
    Bundle.main.loadNibNamed("InstallerOutline", owner: self, topLevelObjects: &topLevel)
    if (topLevel != nil) {
      var wc : InstallerOutWindowController? = nil
      for o in topLevel! {
        if o is InstallerOutWindowController {
          wc = o as? InstallerOutWindowController
        }
      }
      
      for o in topLevel! {
        if o is InstallerOutViewController {
          wc?.contentViewController = o as! InstallerOutViewController
        }
      }
      
      let rev = findCloverRevision(at: Cloverv2Path.addPath("EFI")) ?? "0000"
      
      var title = "\("Clover Installer".locale) r\(rev)"
      if let hash = findCloverHashCommit(at: Cloverv2Path.addPath("EFI")) {
        title = "\(title) (\(hash))"
      }
      wc?.window?.title = title
      return wc
    }
    return nil
  }
}

final class InstallerOutViewController: NSViewController {
  // MARK: variables
  var targetVol : String = ""
  
  var driversUEFI: [[EFIDriver]] = [[EFIDriver]]()
  var sectionsUEFI: [String] = [String]()
  
  @IBOutlet weak var driversOutline : NSOutlineView!
  
  @IBOutlet var infoText : NSTextView!
  
  @IBOutlet var cloverEFICheck : NSButton!
  @IBOutlet var bootSectCheck : NSButton!
  @IBOutlet var boot0Pop : NSPopUpButton!
  @IBOutlet var boot1Field : NSTextField!
  @IBOutlet var boot2Pop : NSPopUpButton!
  @IBOutlet var altBootCheck : NSButton!
  @IBOutlet var targetPop : FWPopUpButton!
  
  @IBOutlet var spinner : NSProgressIndicator!
  @IBOutlet var installButton : NSButton!
  
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

  override func viewDidLoad() {
    if #available(OSX 10.10, *) {
      super.viewDidLoad()
    }
    AppSD.isInstallerOpen = true
    let settingVC = AppSD.settingsWC?.contentViewController as? SettingsViewController
    settingVC?.disksPopUp.isEnabled = false
    settingVC?.updateCloverButton.isEnabled = false
    settingVC?.unmountButton.isEnabled = false
    
    // MARK: localize view and subviews
    localize(view: self.view)
    
    // MARK: controls setup
    self.boot0Pop.removeAllItems()
    self.boot2Pop.removeAllItems()
    self.boot1Field.stringValue = ""
    if #available(OSX 10.10, *) {
      self.boot1Field.placeholderString = "?"
    }
    self.installButton.isEnabled = false
    self.spinner.stopAnimation(nil)
    
    var path : String = Cloverv2Path.addPath("Bootloaders/x64")
    var files = getFiles(at: path).sorted()
    var isDir : ObjCBool = false
    for f in files {
      
      let fp = path.addPath(f)
      if fm.fileExists(atPath: fp, isDirectory: &isDir) {
        if !isDir.boolValue && f.hasPrefix("boot") {
          self.boot2Pop.addItem(withTitle: f)
        }
      }
    }
    
    path = Cloverv2Path.addPath("BootSectors")
    files = getFiles(at: path).sorted()
    
    for f in files {
      let fp = path.addPath(f)
      if fm.fileExists(atPath: fp, isDirectory: &isDir) {
        if !isDir.boolValue {
          if (f == "boot0af" || f == "boot0ss") {
            self.boot0Pop.addItem(withTitle: f)
          }
        }
      }
    }
    
    self.cloverEFICheck.state = .off
    self.cloverEFIPressed(nil)
    
    // don't enable CloverEFI if boot sectors, nor bootloaders exists
    if self.boot0Pop.itemArray.count == 0 || self.boot2Pop.itemArray.count == 0 {
      self.cloverEFICheck.isEnabled = false
    }
    
    self.targetPop.removeAllItems()
    self.populateTargets()
  }
  
  override var representedObject: Any? {
    didSet {
      // Update the view, if already loaded.
    }
  }
  
  // MARK: file scanner
  func getFiles(at path: String) -> [String] {
    var isDir : ObjCBool = false
    var files : [String] = [String]()
    
    if fm.fileExists(atPath: path, isDirectory: &isDir) {
      if isDir.boolValue {
        do { files = try fm.contentsOfDirectory(atPath: path) } catch { }
      }
      for i in  0..<files.count {
        if fm.fileExists(atPath: path.addPath(files[i]), isDirectory: &isDir) {
          if isDir.boolValue {
            files.remove(at: i)
          }
        }
      }
    }
    return files.sorted()
  }
  
  // MARK: directories scanner
  func getDirs(at path: String) -> [String] {
    var isDir : ObjCBool = false
    var dirs : [String] = [String]()
    
    if fm.fileExists(atPath: path, isDirectory: &isDir) {
      if isDir.boolValue {
        do { dirs = try fm.contentsOfDirectory(atPath: path) } catch { }
      }
      for i in  0..<dirs.count {
        if fm.fileExists(atPath: path.addPath(dirs[i]), isDirectory: &isDir) {
          if !isDir.boolValue {
            dirs.remove(at: i)
          }
        }
      }
    }
    return dirs.sorted()
  }
  
  // MARK: populate drivers
  func populateDrivers() {
    self.driversUEFI = [[EFIDriver]]()
    self.sectionsUEFI = [String]()
    var driversList = [String]()
    // MARK: get UEFI default drivers
    var path = Cloverv2Path.addPath(kUEFIRelativeDir)
    var drivers : [EFIDriver] = [EFIDriver]()
    var files : [String] = self.getFiles(at: path)
    var destDir : String = self.targetVol.addPath(kUEFIRelativeDir)
    let isFresh : Bool = !fm.fileExists(atPath: self.targetVol.addPath("EFI/CLOVER"))
    let isLegacy = self.cloverEFICheck.state == .on
    
    var sectionName : String = "UEFI mandatory"
    var driver : EFIDriver? = nil
    for file in files {
      if file.fileExtension == "efi" {
        let fullPath = destDir.addPath(file)
        driver = EFIDriver(dest: destDir,
                           src: path.addPath(file),
                           kind: .uefi,
                           sectionName: sectionName,
                           state: (isFresh || fm.fileExists(atPath: fullPath)) ? .on : .off,
                           isFromClover: true)
        driversList.append(file)
        drivers.append(driver!)
      }
    }
    
    if drivers.count > 0 {
      self.driversUEFI.append(drivers)
      self.sectionsUEFI.append(sectionName)
    }
    
    // MARK: get UEFI optional drivers
    var HFSPlus : Bool = false
    var ApfsDriverLoader : Bool = false
    
    path = Cloverv2Path.addPath(kUEFIRelativeOptionDir)
    for dir in self.getDirs(at: path) {
      files = self.getFiles(at: path.addPath(dir))
      sectionName = "UEFI/\(dir)"
      drivers = [EFIDriver]()
      for file in files {
        if file.fileExtension == "efi" {
          let fullPath = destDir.addPath(file)
          let selected : Bool = fm.fileExists(atPath: fullPath)
          
          if file == "HFSPlus.efi" {
            HFSPlus = true
          } else if file == "ApfsDriverLoader.efi" {
            ApfsDriverLoader = true
          }
          
          driver = EFIDriver(dest: destDir,
                             src: path.addPath(dir).addPath(file),
                             kind: .uefi,
                             sectionName: sectionName,
                             state: selected ? .on : .off,
                             isFromClover: true)
          drivers.append(driver!)
          driversList.append(file)
        }
      }
      
      if drivers.count > 0 {
        if isFresh {
          var toActivate = HFSPlus ? "HFSPlus.efi" : "VBoxHfs.efi"
          
          for d in drivers {
            if d.src.lastPath == toActivate {
              d.state = .on
              break
            }
          }
          
          toActivate  = ApfsDriverLoader ? "ApfsDriverLoader.efi" : "apfs.efi"
          for d in drivers {
            if d.src.lastPath == toActivate {
              d.state = .on
              break
            }
          }
        }
        self.driversUEFI.append(drivers)
        self.sectionsUEFI.append(sectionName)
      }
    }
    
    
    // MARK: check users UEFI drivers
    sectionName = "UEFI, but not from this installer"
    path = self.targetVol.addPath(kUEFIRelativeDir)
    files = self.getFiles(at: path)
    drivers = [EFIDriver]()
    for file in files {
      if file.fileExtension == "efi" && !driversList.contains(file) {
        driver = EFIDriver(dest: destDir,
                           src: path.addPath(file),
                           kind: .uefi,
                           sectionName: sectionName,
                           state: .on,
                           isFromClover: false)
        drivers.append(driver!)
      }
    }
    
    if drivers.count > 0 {
      self.driversUEFI.append(drivers)
      self.sectionsUEFI.append(sectionName)
    }
    
    driversList = [String]()
    // MARK: get BIOS default drivers
    path = Cloverv2Path.addPath(kBIOSRelativeDir)
    destDir = self.targetVol.addPath(kBIOSRelativeDir)
    drivers = [EFIDriver]()
    files = self.getFiles(at: path)
    sectionName = "BIOS mandatory"
    
    for file in files {
      if file.fileExtension == "efi" {
        let fullPath = destDir.addPath(file)
        let selected : Bool = (isFresh && isLegacy) || fm.fileExists(atPath: fullPath)
        driver = EFIDriver(dest: destDir,
                           src: path.addPath(file),
                           kind: .bios,
                           sectionName: sectionName,
                           state: selected ? .on : .off,
                           isFromClover: true)
        drivers.append(driver!)
        driversList.append(file)
      }
    }
    
    if drivers.count > 0 {
      self.driversUEFI.append(drivers)
      self.sectionsUEFI.append(sectionName)
    }
    
    // MARK: get BIOS optional drivers
    path = Cloverv2Path.addPath(kBIOSRelativeOptionDir)
    HFSPlus = false
    ApfsDriverLoader = false
    for dir in self.getDirs(at: path) {
      sectionName = "BIOS/\(dir)"
      files = self.getFiles(at: path.addPath(dir))
      drivers = [EFIDriver]()
      for file in files {
        if file.fileExtension == "efi" {
          let fullPath = destDir.addPath(file)
          let selected : Bool = fm.fileExists(atPath: fullPath)
          
          if file == "HFSPlus.efi" {
            HFSPlus = true
          } else if file == "ApfsDriverLoader.efi" {
            ApfsDriverLoader = true
          }
          
          driver = EFIDriver(dest: destDir,
                             src: path.addPath(dir).addPath(file),
                             kind: .bios,
                             sectionName: sectionName,
                             state: selected ? .on : .off,
                             isFromClover: true)
          drivers.append(driver!)
          driversList.append(file)
        }
      }
      
      if drivers.count > 0 {
        if isFresh && isLegacy {
          var toActivate = HFSPlus ? "HFSPlus.efi" : "VBoxHfs.efi"
          for d in drivers {
            if d.src.lastPath == toActivate {
              d.state = .on
            }
          }
          
          toActivate  = ApfsDriverLoader ? "ApfsDriverLoader.efi" : "apfs.efi"
          for d in drivers {
            if d.src.lastPath == toActivate {
              d.state = .on
            }
          }
        }
        self.driversUEFI.append(drivers)
        self.sectionsUEFI.append(sectionName)
      }
    }
    
    // MARK: check users BIOS drivers
    path = self.targetVol.addPath(kBIOSRelativeDir)
    files = self.getFiles(at: path)
    drivers = [EFIDriver]()
    sectionName = "BIOS, but not from this installer"
    
    for file in files {
      if file.fileExtension == "efi" && !driversList.contains(file) {
        driver = EFIDriver(dest: destDir,
                           src: path.addPath(file),
                           kind: .bios,
                           sectionName: sectionName,
                           state: .on,
                           isFromClover: false)
        drivers.append(driver!)
      }
    }
    
    if drivers.count > 0 {
      self.driversUEFI.append(drivers)
      self.sectionsUEFI.append(sectionName)
    }
    
    self.driversOutline.reloadData()
    self.expandAllSections()
  }
  
  // MARK: find suitable disks
  func populateTargets() {
    if AppSD.isInstalling {
      return
    }
    let selected : String? = self.targetPop.selectedItem?.representedObject as? String
    self.targetPop.removeAllItems()
    self.targetPop.addItem(withTitle: "Select a disk..".locale)
    let disks : NSDictionary = getAlldisks()
    let bootPartition = findBootPartitionDevice()
    let diskSorted : [String] = (disks.allKeys as! [String]).sorted()
    for d in diskSorted {
      let disk : String = d
      if isWritable(diskOrMtp: disk) && !kBannedMedia.contains(getVolumeName(from: disk) ?? "") {
        let fs : String = getFS(from: disk)?.lowercased() ?? kNotAvailable.locale
        let psm : String = getPartitionSchemeMap(from: disk) ?? kNotAvailable.locale
        let name : String = getVolumeName(from: disk) ?? kNotAvailable.locale
        let mp : String = getMountPoint(from: disk) ?? kNotAvailable.locale
        let parentDiskName : String = getMediaName(from: getBSDParent(of: disk) ?? "") ?? kNotAvailable.locale
        
        let supportedFS = ["msdos", "fat16", "fat32", "exfat", "hfs"]
     
        if supportedFS.contains(fs) {
          self.targetPop.addItem(withTitle: "\(disk)\t\(name), \("mount point".locale): \(mp), \(fs.uppercased()), \(psm): (\(parentDiskName))")
          self.targetPop.invalidateIntrinsicContentSize()
          // get the image
          if disk == bootPartition {
            let image : NSImage = NSImage(named: "NSApplicationIcon")!.copy() as! NSImage
            image.size = NSMakeSize(16, 16)
            self.targetPop.lastItem?.image = image
          } else if let image : NSImage = getIconFor(volume: disk) {
            image.size = NSMakeSize(16, 16)
            self.targetPop.lastItem?.image = image
          }
          self.targetPop.lastItem?.representedObject = disk
        }
      }
    }
    
    if (selected != nil) {
      for item in self.targetPop.itemArray {
        if let d = item.representedObject as? String {
          if selected == d {
            self.targetPop.select(item)
            break
          }
        }
      }
    }
  }
  
  // MARK: actions
  @IBAction func targetSelected(_ sender: FWPopUpButton?) {
    self.targetVol = ""
    self.installButton.isEnabled = false
    self.driversUEFI = [[EFIDriver]]()
    self.sectionsUEFI = [String]()
    self.boot1Field.stringValue = ""
    if #available(OSX 10.10, *) {
      self.boot1Field.placeholderString = "?"
    }
    self.cloverEFICheck.isEnabled = false
    self.cloverEFICheck.state = .off
    self.bootSectCheck.isEnabled = false
    self.bootSectCheck.state = .off
    self.boot0Pop.isEnabled = false
    self.boot2Pop.isEnabled = false
    self.altBootCheck.isEnabled = false
    self.altBootCheck.state = .off
    if let disk = sender?.selectedItem?.representedObject as? String {
      if !isMountPoint(path: disk) {
        DispatchQueue.global(priority: .background).async(execute: { () -> Void in
          let cmd = "diskutil mount \(disk)"
          let msg = String(format: "Clover wants to mount %@", disk)
          let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
          
          let task = Process()
          
          if #available(OSX 10.12, *) {
            task.launchPath = "/usr/bin/osascript"
            task.arguments = ["-e", script]
          } else {
            task.launchPath = "/usr/sbin/diskutil"
            task.arguments = ["mount", disk]
          }
          
          task.terminationHandler = { t in
            DispatchQueue.main.async {
              self.view.window?.level = .floating
              self.view.window?.makeKeyAndOrderFront(nil)
              self.view.window?.level = .normal
            }
            
            if t.terminationStatus == 0 {
              if isMountPoint(path: disk) {
                DispatchQueue.main.async {
                  self.targetVol = getMountPoint(from: disk) ?? ""
                  self.populateTargets()
                  self.setPreferences(for: self.targetVol)
                }
              }
            } else {
              NSSound.beep()
              DispatchQueue.main.async {
                self.driversOutline.reloadData()
                self.expandAllSections()
              }
            }
          }
          task.launch()
        })
      } else {
        self.targetVol = getMountPoint(from: disk) ?? ""
        self.populateTargets()
        self.setPreferences(for: self.targetVol)
      }
    } else {
      self.driversUEFI = [[EFIDriver]]()
      self.sectionsUEFI = [String]()
      self.driversOutline.reloadData()
      self.expandAllSections()
    }
  }
  
  private func setPreferences(for volume: String) {
    let fs = getFS(from: volume)?.lowercased()
    let legacyAllowed : Bool = (fs != "fat16")
    self.installButton.isEnabled = true
    self.cloverEFICheck.isEnabled = legacyAllowed
    self.cloverEFICheck.state = .off
    self.bootSectCheck.isEnabled = legacyAllowed
    self.bootSectCheck.state = .off
    self.boot0Pop.isEnabled = false
    self.boot2Pop.isEnabled = false
    self.altBootCheck.isEnabled = false
    self.altBootCheck.state = .off
    
    if !legacyAllowed {
      self.cloverEFIPressed(nil)
      self.populateDrivers()
      return
    }
    /*
     boot0    String
     boot2    String
     boot2Alt Bool
     
     if boot0 and boot2 are present than CloverEFI must be turned on and
     the same for other legacy options
     */
    
    let prefPath = volume.addPath("EFI/CLOVER/pref.plist")
    if let prefDict = NSDictionary(contentsOfFile: prefPath) {
      let boot0 = prefDict.object(forKey: "boot0") as? String
      let boot2 = prefDict.object(forKey: "boot2") as? String
      let boot2Alt = prefDict.object(forKey: "boot2Alt") as? NSNumber

      if boot2 != nil && self.boot2Pop.itemTitles.contains(boot2!) {
        self.cloverEFICheck.state = .on
        self.boot2Pop.selectItem(withTitle: boot2!)
        
        if boot0 != nil && self.boot0Pop.itemTitles.contains(boot0!) {
          self.bootSectCheck.state = .on
          self.bootSectCheck.isEnabled = true
          self.boot0Pop.selectItem(withTitle: boot0!)
          self.boot0Pop.isEnabled = true
        }
        
        if (boot2Alt != nil) {
          self.altBootCheck.state = .on
          self.altBootCheck.isEnabled = true
        }
      }
    }
    
    self.cloverEFIPressed(nil)
    self.populateDrivers()
  }
  
  @IBAction func cloverEFIPressed(_ sender: NSButton?) {
    self.bootSectCheck.isEnabled = self.cloverEFICheck.state == .on
    self.boot2Pop.isEnabled = self.cloverEFICheck.state == .on
    self.altBootCheck.isEnabled = self.cloverEFICheck.state == .on
    self.bootSectPressed(nil)

    if (sender != nil) {
      if sender!.state == .on {
        post(text: "Clover legacy BIOS boot sectors".locale,
             add: false,
             color: nil,
             scroll: false)
      } else {
        post(text: "UEFI only".locale,
             add: false,
             color: nil,
             scroll: false)
      }
    }
    if fm.fileExists(atPath: self.targetVol) {
      self.populateDrivers()
    }
  }
  
  @IBAction func bootSectPressed(_ sender: NSButton?) {
    self.boot0Pop.isEnabled = self.bootSectCheck.state == .on
    self.boot1Field.isEnabled = self.bootSectCheck.state == .on
    self.altBootPressed(nil)
    if (sender != nil) {
      if sender!.state == .on {
        post(text: "Clover legacy BIOS boot sectors".locale,
             add: false,
             color: nil,
             scroll: false)
      } else {
        post(text: "Don't install any bootloader (boot0X, boot1X)".locale,
             add: false,
             color: nil,
             scroll: false)
      }
    }
  }
  
  @IBAction func altBootPressed(_ sender: NSButton?) {
    self.boot1Field.stringValue = getBoot1() ?? "?"
    if #available(OSX 10.10, *) {
      self.boot1Field.placeholderString = self.boot1Field.stringValue
    }
    if self.cloverEFICheck.state == .off || self.bootSectCheck.state == .off {
      self.boot1Field.stringValue = ""
    }
    
    if (sender != nil && sender!.state == .on) {
      post(text: "Install alternative booting PBR".locale,
           add: false,
           color: nil,
           scroll: false)
    }
  }
  
  @IBAction func boot0Selected(_ sender: NSPopUpButton) {
    post(text: sender.titleOfSelectedItem!.locale, add: false, color: nil, scroll: false)
  }
  
  @IBAction func boot2Selected(_ sender: NSPopUpButton) {
    post(text: sender.titleOfSelectedItem!.locale, add: false, color: nil, scroll: false)
  }
  
  private func getBoot1() -> String? {
    let fs : String = getFS(from: self.targetVol) ?? ""
    var boot1 : String? = nil
    let alt : Bool = self.altBootCheck.state == .on
    switch fs.lowercased() {
    case "hfs":
      boot1 =  alt ? "boot1h2" : "boot1h"
    case "exfat":
      boot1 =  alt ? "boot1xalt" : "boot1x"
    case "fat32":
      boot1 =  alt ? "boot1f32alt" : "boot1f32"
    default:
      break
    }
    return boot1
  }
  
  // MARK: Post text
  func post(text: String, add: Bool, color: NSColor?, scroll: Bool) {
    let textColor = (color == nil) ? NSColor.controlTextColor : color!
    let font = NSFont.userFixedPitchFont(ofSize: 11)
    let attributes = [NSAttributedString.Key.font: font, NSAttributedString.Key.foregroundColor: textColor]
    
    let astr = NSAttributedString(string: text, attributes: attributes as [NSAttributedString.Key : Any])
    //DispatchQueue.global(qos: .background).async {
      DispatchQueue.main.async {
        if add {
          self.infoText.textStorage?.append(astr)
        } else {
          self.infoText.string = ""
          self.infoText.textStorage?.append(astr)
        }
        
        if scroll {
          let loc = self.infoText.string.lengthOfBytes(using: String.Encoding.utf8)
          let range = NSRange(location: loc, length: 0)
          self.infoText.scrollRangeToVisible(range)
        } else {
          self.infoText.scroll(NSPoint.zero)
        }
      }
    //}
  }
  
  // MARK: Installation
  @IBAction func installPressed(_ sender: NSButton!) {
    
    /*
     NSString *targetVol             = [CloverappDict objectForKey:@"targetVol"];
     NSString *disk                  = [CloverappDict objectForKey:@"disk"];
     NSString *filesystem            = [CloverappDict objectForKey:@"filesystem"];
     NSString *shemeMap              = [CloverappDict objectForKey:@"shemeMap"];
     NSString *boot0                 = [CloverappDict objectForKey:@"boot0"];
     NSString *boot1                 = [CloverappDict objectForKey:@"boot1"];
     NSString *boot2                 = [CloverappDict objectForKey:@"boot2"];
     NSString *cloverv2              = [CloverappDict objectForKey:@"CloverV2"];
     NSString *boot1installPath      = [CloverappDict objectForKey:@"boot1install"];
     NSString *bootSectorsInstallSrc = [CloverappDict objectForKey:@"bootsectors-install"];
     NSString *backUpPath            = [CloverappDict objectForKey:@"BackUpPath"];
     NSString *version               = [CloverappDict objectForKey:@"version"];
     BOOL isESP                      = [[CloverappDict objectForKey:@"isESP"] boolValue];
    */
    
    let Cloverapp = NSMutableDictionary()
    let toDelete = NSMutableArray()
    let UEFI = NSMutableArray()
    let BIOS = NSMutableArray()
    
    // minimum required arguments
    Cloverapp.setValue(self.targetVol, forKey: "targetVol")
    Cloverapp.setValue(self.view.window!.title, forKey: "version")
    
    let  disk = getBSDName(of: self.targetVol) ?? ""
    Cloverapp.setValue(disk, forKey: "disk")
    let shemeMap = getPartitionSchemeMap(from: disk) ?? kNotAvailable
    Cloverapp.setValue(shemeMap, forKey: "shemeMap")
    Cloverapp.setValue(Cloverv2Path, forKey: "CloverV2")
    let filesystem = (getFS(from: disk)?.lowercased()) ?? kNotAvailable
    Cloverapp.setValue(filesystem, forKey: "filesystem")
    Cloverapp.setValue(getAllESPs().contains(disk), forKey: "isESP")
 
    let supportedFS = ["exfat", "fat16", "fat32", "hfs"]
    if !supportedFS.contains(filesystem) {
      NSSound.beep()
      post(text: "Error: can't install on \(filesystem.uppercased()) filesystem.", add: false, color: nil, scroll: false)
      return
    }
      
    // drivers
    for sect in self.driversUEFI {
      for driver in sect {
        let fullDest = driver.dest.addPath(driver.src.lastPath)
        if driver.state == .off {
          toDelete.add(fullDest as NSString)
        } else if driver.state == .on {
          if driver.isFromClover {
            if driver.kind == .uefi {
              UEFI.add(driver.src as NSString)
            } else {
              BIOS.add(driver.src as NSString)
            }
          }
        }
      }
    }
    
    Cloverapp.setValue(toDelete, forKey: "toDelete")
    Cloverapp.setValue(UEFI, forKey: "UEFI")
    Cloverapp.setValue(BIOS, forKey: "BIOS")

    // optional arguments
    if self.cloverEFICheck.state == .on && self.cloverEFICheck.isEnabled {
      Cloverapp.setValue(self.boot2Pop.titleOfSelectedItem!, forKey: "boot2")
      
      if self.bootSectCheck.state == .on && self.bootSectCheck.isEnabled {
        let boot1install = Bundle.main.executablePath!.deletingLastPath.addPath("boot1-install")
        Cloverapp.setValue(boot1install, forKey: "boot1install")
        
        Cloverapp.setValue(self.boot0Pop.titleOfSelectedItem!, forKey: "boot0")
        Cloverapp.setValue(self.boot1Field.stringValue, forKey: "boot1")
        
        let bsinstallpath = Bundle.main.sharedSupportPath!.addPath("bootsectors-install")
        Cloverapp.setValue(bsinstallpath, forKey: "bootsectors-install")
      }
      
      if self.altBootCheck.state == .on && self.altBootCheck.isEnabled {
        Cloverapp.setValue(NSNumber(value: true), forKey: "alt")
      }
    }
    
    // backup in ~/Desktop/EFI_Backup_date
    post(text: "Checking files...\n", add: false, color: nil, scroll: false)
    if fm.fileExists(atPath: self.targetVol.addPath("EFI/CLOVER")) {
      let df = DateFormatter()
      df.dateFormat = "yyyy-MM-dd_hh-mm-ss"
      let now = df.string(from: Date())
      let revIn = findCloverRevision(at: self.targetVol.addPath("EFI")) ?? "0000"
      let mediaName = getMediaName(from: getBSDParent(of: disk) ?? "") ?? "NoName"
      let backUpPath = NSHomeDirectory().addPath("Desktop/CloverBackUp/\(mediaName)/r\(revIn)_\(now)/EFI")
      
      DispatchQueue.global(priority: .background).async(execute: { () -> Void in
        do {
          if !fm.fileExists(atPath: backUpPath.deletingLastPath) {
            try fm.createDirectory(atPath: backUpPath.deletingLastPath,
                                   withIntermediateDirectories: true,
                                   attributes: nil)
          }
          try fm.copyItem(atPath: self.targetVol.addPath("EFI"),
                          toPath: backUpPath)
          //post(text: "backup made at '\(backUpPath)'.\n", add: true, color: nil, scroll: false)
          Cloverapp.setValue(backUpPath, forKey: "BackUpPath")
          self.installClover(disk: disk, settingDict: Cloverapp)
        } catch {
          self.post(text: "The backup failed:\n", add: true, color: nil, scroll: false)
          self.post(text: error.localizedDescription, add: false, color: nil, scroll: false)
        }
      })
    } else {
      DispatchQueue.global(priority: .background).async(execute: { () -> Void in
        self.installClover(disk: disk, settingDict: Cloverapp)
      })
    }
  }
  
  func installClover(disk: String, settingDict : NSDictionary) {
    run(cmd: "LC_ALL=C /usr/sbin/diskutil list > /tmp/diskutil.List")
    DispatchQueue.main.async {
      self.post(text: "Installation begin..\n", add: true, color: nil, scroll: false)
    }
    if !isMountPoint(path: self.targetVol) {
      DispatchQueue.main.async {
        NSSound.beep()
        self.post(text: "Can't find target volume, installation aborted.", add: true, color: nil, scroll: false)
      }
      return
    }
    
    /*
     let's roll!.... but one problem: AuthorizationExecuteWithPrivileges is deprecated and
     will be removed soon, so creating an helper tool will kill the ability for users to easily
     compile the app because will require a lot and complex work (code sign certificate + set up of the app
     and the helper). For this reason We're using Process() + osascript (or at least NSAppleScript)
     with administrator privileges. One of the problem with AppleScript is to escape paths...
     .. so just give a secure path, i.e. "/tmp/cloverhelper"
     */
    
    try? fm.removeItem(atPath: "/tmp/Cloverapp")
    
    if settingDict.write(toFile: "/tmp/Cloverapp", atomically: false) {
      DispatchQueue.main.async {
        self.installButton.isEnabled = false
        AppSD.isInstalling = true
        self.spinner.startAnimation(nil)
        
        let helperPath = Bundle.main.executablePath!.deletingLastPath.addPath("CloverDaemonNew")
        
        let script = "do shell script \"'\(helperPath)' --CLOVER\" with administrator privileges"
        var err : NSDictionary? = nil
        let result : NSAppleEventDescriptor = NSAppleScript(source: script)!.executeAndReturnError(&err)
        
        self.post(text: result.stringValue ?? "", add: false, color: nil, scroll: true)
        self.spinner.stopAnimation(nil)
        let message = (err == nil) ? "Installation succeded".locale : "Installation failed".locale
        self.post(text: "\n\(message).", add: true, color: nil, scroll: true)
        
        let alert = NSAlert()
        alert.messageText = message
        if #available(OSX 10.10, *) {
          alert.informativeText = (err == nil) ? "😀" : "😱"
        }
        alert.alertStyle = (err == nil) ? .informational : .critical
        alert.addButton(withTitle: "Close".locale)
        alert.beginSheetModal(for: self.view.window!) { (reponse) in
          
        }
        AppSD.isInstalling = false
        self.installButton.isEnabled = true
        self.spinner.stopAnimation(nil)
        if isMountPoint(path: self.targetVol) {
          self.targetVol = getMountPoint(from: disk) ?? ""
        }
        AppSD.reFreshDisksList()
        self.setPreferences(for: self.targetVol)
      }
      
    } else {
      DispatchQueue.main.async {
        NSSound.beep()
        self.post(text: "Can't write temporary files, installation aborted.", add: true, color: nil, scroll: false)
      }
    }
  }
}

// MARK: InstallerOutViewController extension with Delegate and Data source
extension InstallerOutViewController: NSOutlineViewDelegate, NSOutlineViewDataSource {
  
  func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
    if (item != nil) {
      if let array = item as? [EFIDriver] {
        return array.count
      }
    } else {
      return self.sectionsUEFI.count
    }
    
    return 0
  }
  
  func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any {
    if let group = item as? [EFIDriver] {
      return group[index]
    } else {
      return self.driversUEFI[index]
    }
  }
  
  func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
    return item is [EFIDriver]
  }
  
  func outlineViewSelectionDidChange(_ notification: Notification) {
    if AppSD.isInstalling {
      return
    }
    guard let outlineView = notification.object as? NSOutlineView else {
      return
    }
    
    let row = outlineView.selectedRow
    if row >= 0 {
      if let driver = outlineView.item(atRow: row) as? EFIDriver {
        post(text: driver.src.lastPath.locale, add: false, color: nil, scroll: false)
      } else if let group = outlineView.item(atRow: row) as? [EFIDriver] {
        if group.count > 0 {
          post(text: group[0].sectionName.locale, add: false, color: nil, scroll: false)
        } else {
          post(text: "", add: false, color: nil, scroll: false)
        }
      } else {
        post(text: "", add: false, color: nil, scroll: false)
      }
    }
  }
  
  func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {
    var view : NSView? = nil
    if item is EFIDriver {
      let ci = CollectionViewItem()
      let drv : EFIDriver = item as! EFIDriver
      ci.field.target = self
      ci.field.stringValue = drv.src.lastPath
      ci.field.cell?.representedObject = drv.src.lastPath.locale
      ci.driver = drv
      ci.checkBox.state = drv.state
      ci.installerOutController = self
      drv.itemView = ci
      view = ci.view
    } else if let array = item as? [EFIDriver] {
      // take the first object and find the section name
      let field = NSTextField()
      field.isEditable = false
      field.drawsBackground = false
      field.isBordered = false
      if array.count > 0 {
        field.stringValue = array[0].sectionName.locale
      } else {
        field.stringValue = "Bug: empty section"
      }
      view = field
    }
    
    return view
  }
  
  func expandAllSections() {
    for i in self.driversUEFI {
      self.driversOutline.expandItem(i)
    }
  }
}
