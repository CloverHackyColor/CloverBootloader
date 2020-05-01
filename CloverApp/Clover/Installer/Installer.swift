//
//  Installer.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa


// MARK: Installer Window controller
final class InstallerWindowController: NSWindowController, NSWindowDelegate {
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
    return true
  }
  
  func windowWillClose(_ notification: Notification) {
    
  }
  /*
  class func loadFromNib() -> InstallerWindowController {
    let wc = NSStoryboard(name: "Installer",
                          bundle: nil).instantiateController(withIdentifier: "InstallerWindow") as! InstallerWindowController
    let rev = findCloverRevision(at: Cloverv2Path.addPath("EFI")) ?? "0000"
    
    var title = "\("Clover Installer".locale) r\(rev)"
    if let hash = findCloverHashCommit(at: Cloverv2Path.addPath("EFI")) {
      title = "\(title) (\(hash))"
    }
    wc.window?.title = title
    return wc
  }
  */
  class func loadFromNib() -> InstallerWindowController? {
    var topLevel: NSArray? = nil
    Bundle.main.loadNibNamed("Installer", owner: self, topLevelObjects: &topLevel)
    if (topLevel != nil) {
      var wc : InstallerWindowController? = nil
      for o in topLevel! {
        if o is InstallerWindowController {
          wc = o as? InstallerWindowController
        }
      }
      
      for o in topLevel! {
        if o is InstallerViewController {
          wc?.contentViewController = o as! InstallerViewController
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

// MARK: InstallerViewController (NSViewController sub class)
final class InstallerViewController: NSViewController {
  // MARK: variables
  var targetVol : String = ""
  
  var driversUEFI: [[EFIDriver]] = [[EFIDriver]]()
  var sectionsUEFI: [String] = [String]()
  
  @IBOutlet weak var driversCollection : NSCollectionView!
  
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
    
    // MARK: Appearance
    if #available(OSX 10.14, *) { } else {
      let appearance = NSAppearance(named: .aqua)
      self.view.window?.appearance = appearance
      self.driversCollection.enclosingScrollView?.contentView.drawsBackground = false
      self.driversCollection.enclosingScrollView?.drawsBackground = false
      //self.driversCollection.enclosingScrollView?.backgroundColor = .white
      self.driversCollection.enclosingScrollView?.contentView.appearance = appearance
      self.driversCollection.enclosingScrollView?.appearance = appearance
      self.driversCollection.appearance = appearance
      self.driversCollection.wantsLayer = true
      self.driversCollection.layer?.backgroundColor = .white
    }
    
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
    
    // self.driversCollection.delegate = self // already set in Interface Builder
    if #available(OSX 10.11, *) {
      let fl = NSCollectionViewFlowLayout()
      fl.itemSize = NSSize(width: Double(collectionItemWith), height: 18.0)
      fl.sectionInset = NSEdgeInsets(top: 5.0, left: 5.0, bottom: 5.0, right: 5.0)
      fl.minimumInteritemSpacing = 5.0
      fl.minimumLineSpacing = 5.0
      fl.headerReferenceSize = CGSize(width: self.driversCollection.frame.width - 30, height: 18)
      self.driversCollection.collectionViewLayout = fl
    }
    
    self.cloverEFICheck.isEnabled = false
    
    self.driversCollection.isSelectable = true
    
    if #available(OSX 10.11, *) {
      self.driversCollection.register(CollectionViewItem.self,
                                      forItemWithIdentifier: NSUserInterfaceItemIdentifier(rawValue: "CollectionViewItem"))
      
      self.driversCollection.register(HeaderView.self,
                                      forSupplementaryViewOfKind: NSCollectionView.elementKindSectionHeader,
                                      withIdentifier: NSUserInterfaceItemIdentifier(rawValue: "CollectionViewHeader"))
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
    
    if #available(OSX 10.11, *) {
      self.driversCollection.reloadData()
    }
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
            if t.terminationStatus == 0 {
              if isMountPoint(path: disk) {
                DispatchQueue.main.async {
                  self.targetVol = getMountPoint(from: disk) ?? ""
                  self.populateTargets()
                  self.setPreferences(for: self.targetVol)
                }
              }
              DispatchQueue.main.async {
                self.view.window?.makeKeyAndOrderFront(nil)
                self.view.window?.level = .floating
                self.view.window?.level = .normal
              }
            } else {
              NSSound.beep()
              DispatchQueue.main.async {
                if #available(OSX 10.11, *) {
                  self.driversCollection.reloadData()
                  self.view.window?.makeKeyAndOrderFront(nil)
                  self.view.window?.level = .floating
                  self.view.window?.level = .normal
                }
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
      if #available(OSX 10.11, *) {
        self.driversCollection.reloadData()
      }
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
    //let attributes = self.infoText.textStorage?.attributes(at: 0, effectiveRange: nil)
    let textColor = (color == nil) ? NSColor.controlTextColor : color!
    let attributes = [/*NSAttributedString.Key.font: font,*/ NSAttributedString.Key.foregroundColor: textColor]
    
    let astr = NSAttributedString(string: text, attributes: attributes)
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
  
  // MARK: Installation
  @IBAction func installPressed(_ sender: NSButton!) {
    if AppSD.isInstalling {
      NSSound.beep()
      return
    }
    
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
      post(text: "Error: can't install on \(filesystem.uppercased()) filesystem.",
        add: false,
        color: nil,
        scroll: false)
      return
    }
    AppSD.isInstalling = true
    self.installButton.isEnabled = false
    self.spinner.startAnimation(nil)
      
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
    self.post(text: "Checking files...\n", add: false, color: nil, scroll: false)
    if fm.fileExists(atPath: self.targetVol.addPath("EFI/CLOVER")) {
      self.post(text: "doing the backup...\n", add: false, color: nil, scroll: false)
      
      let df = DateFormatter()
      df.dateFormat = "yyyy-MM-dd_hh-mm-ss"
      let now = df.string(from: Date())
      let revIn = findCloverRevision(at: self.targetVol.addPath("EFI")) ?? "0000"
      let mediaName = getMediaName(from: getBSDParent(of: disk) ?? "") ?? "NoName"
      let backUpPath = NSHomeDirectory().addPath("Desktop/CloverBackUp/\(mediaName)/r\(revIn)_\(now)/EFI")
      if #available(OSX 10.10, *) {
        DispatchQueue.global(qos: .userInteractive).async {
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
            DispatchQueue.main.async {
              self.post(text: "The backup failed:\n", add: true, color: nil, scroll: false)
              self.post(text: error.localizedDescription, add: false, color: nil, scroll: false)
              AppSD.isInstalling = false
              self.installButton.isEnabled = true
              self.spinner.stopAnimation(nil)
            }
          }
        }
      }
      
    } else {
      if #available(OSX 10.10, *) {
        DispatchQueue.global(qos: .userInteractive).async {
          self.installClover(disk: disk, settingDict: Cloverapp)
        }
      }
    }
  }
  
  func installClover(disk: String, settingDict : NSDictionary) {
    self.post(text: "Installation begin..\n", add: true, color: nil, scroll: false)
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
      AppSD.isInstalling = true
      
      DispatchQueue.main.async {
        self.spinner.startAnimation(nil)
        self.installButton.isEnabled = false
        self.view.window?.level = .floating // just a hack to keep window in front momentarily
      }
      
      DispatchQueue.main.asyncAfter(deadline: .now() + 6.0) {
        self.view.window?.level = .normal
      }
      
      let task = Process()
      let msg = "Install Clover".locale
      let helperPath = Bundle.main.executablePath!.deletingLastPath.addPath("CloverDaemonNew")
      let script = "do shell script \"'\(helperPath)' --CLOVER\" with prompt \"\(msg)\" with administrator privileges"
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      let pipe: Pipe = Pipe()
      
      task.standardOutput = pipe
      task.standardError  = pipe
      let fh = pipe.fileHandleForReading
      fh.waitForDataInBackgroundAndNotify()
      
      var op1 : NSObjectProtocol!
      op1 = NotificationCenter.default.addObserver(forName: NSNotification.Name.NSFileHandleDataAvailable,
                                                    object: fh, queue: nil) {
                                                      notification -> Void in
                                                      let data = fh.availableData
                                                      if data.count > 0 {
                                                        let output = String(decoding: data, as: UTF8.self)
                                                        DispatchQueue.main.async {
                                                          if self.view.window?.level != .normal {
                                                            self.view.window?.level = .normal
                                                          }
                                                          
                                                          self.post(text: "\n" + output,
                                                                    add: true,
                                                                    color: nil,
                                                                    scroll: true)
                                                        }
                                                        fh.waitForDataInBackgroundAndNotify()
                                                      } else {
                                                        NotificationCenter.default.removeObserver(op1 as Any)
                                                      }
      }
      
      var op2 : NSObjectProtocol!
      op2 = NotificationCenter.default.addObserver(forName: Process.didTerminateNotification,
                                                    object: task, queue: nil) {
                                                      notification -> Void in
                                                      NotificationCenter.default.removeObserver(op2 as Any)
                                                      let success = (task.terminationStatus == 0)
                                                      DispatchQueue.main.async {
                                                        let message = success ? "Installation succeded".locale : "Installation failed".locale
                                                        self.post(text: "\n\(message).", add: true, color: nil, scroll: true)
                                                        
                                                        
                                                        NSSound(named: success ? "Glass" : "Basso")?.play()
                                                        let alert = NSAlert()
                                                        alert.messageText = message
                                                        alert.informativeText = success ? "😀" : "😱"
                                                        alert.alertStyle = success ? .informational : .critical
                                                        alert.addButton(withTitle: "Ok".locale)
                                                        
                                                        alert.beginSheetModal(for: self.view.window!) { (reponse) in
                                                          AppSD.isInstalling = false
                                                          self.installButton.isEnabled = true
                                                          self.spinner.stopAnimation(nil)
                                                          if isMountPoint(path: self.targetVol) {
                                                            self.targetVol = getMountPoint(from: disk) ?? ""
                                                          }
                                                          AppSD.reFreshDisksList()
                                                          self.setPreferences(for: self.targetVol)
                                                        }
                                                      }
      }
      
      task.launch()
      task.waitUntilExit()
      
    } else {
      DispatchQueue.main.async {
        NSSound.beep()
        self.post(text: "Can't write temporary files, installation aborted.", add: true, color: nil, scroll: false)
      }
    }
    
    
  }
}

// MARK: InstallerViewController extension with NSCollectionViewDataSource
extension InstallerViewController: NSCollectionViewDataSource {
  
  func collectionView(_ collectionView: NSCollectionView,
                      itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
    if #available(OSX 10.11, *) {
      let item = collectionView.makeItem(withIdentifier:
        NSUserInterfaceItemIdentifier(rawValue: "CollectionViewItem"), for: indexPath)
      if let ci : CollectionViewItem = item as? CollectionViewItem {
        ci.installerController = self
        let drv = self.driversUEFI[indexPath.section][indexPath.item]
        ci.field.target = self
        ci.field.stringValue = drv.src.lastPath
        ci.field.cell?.representedObject = drv.src.lastPath.locale
        ci.driver = drv
        ci.checkBox.state = drv.state
        drv.itemView = ci
      }
      return item
    }
    // that will not happen as this class is only used in 10.11+
    return CollectionViewItem()
  }
  
  func numberOfSections(in collectionView: NSCollectionView) -> Int {
    return self.sectionsUEFI.count
  }
  
  func collectionView(_ collectionView: NSCollectionView,
                      numberOfItemsInSection section: Int) -> Int {
    return self.driversUEFI[section].count
  }
  
  func collectionView(_ collectionView: NSCollectionView,
                      viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind,
                      at indexPath: IndexPath) -> NSView {
    var h : NSView? = nil
    if #available(OSX 10.11, *) {
      h = collectionView.makeSupplementaryView(ofKind: kind,
                                                   withIdentifier: NSUserInterfaceItemIdentifier(rawValue: "CollectionViewHeader"),
                                                   for: indexPath)
      
      if let hv : HeaderView = h as? HeaderView {
        let str : String = self.sectionsUEFI[indexPath.section]
        hv.field.stringValue = str.locale
        if str.hasPrefix("BIOS") {
          hv.layer?.backgroundColor = NSColor.gray.cgColor
        } else {
          hv.layer?.backgroundColor = NSColor.darkGray.cgColor
        }
      }
    }
    return h!
  }
}

/*
// MARK: InstallerViewController extension with NSCollectionViewDelegateFlowLayout
extension InstallerViewController: NSCollectionViewDelegateFlowLayout {
  func collectionView(_ collectionView: NSCollectionView,
                      layout collectionViewLayout: NSCollectionViewLayout,
                      sizeForItemAt indexPath: IndexPath) -> NSSize {
    return NSSize(width: Double(collectionItemWith), height: 18.0)
  }
  
  func collectionView(_ collectionView: NSCollectionView,
                      layout collectionViewLayout: NSCollectionViewLayout,
                      referenceSizeForHeaderInSection section: Int) -> NSSize {
    return CGSize(width: self.driversCollection.frame.width - 30, height: 18)
  }
  
  func collectionView(_ collectionView: NSCollectionView,
                      layout collectionViewLayout: NSCollectionViewLayout,
                      insetForSectionAt section: Int) -> NSEdgeInsets {
    return NSEdgeInsets(top: 5.0, left: 5.0, bottom: 5.0, right: 5.0)
  }
  
  func collectionView(_ collectionView: NSCollectionView,
                      layout collectionViewLayout: NSCollectionViewLayout,
                      minimumInteritemSpacingForSectionAt section: Int) -> CGFloat {
    return 5.0
  }
  
  func collectionView(_ collectionView: NSCollectionView,
                      layout collectionViewLayout: NSCollectionViewLayout,
                      minimumLineSpacingForSectionAt section: Int) -> CGFloat {
    return 5.0
  }
}
*/
