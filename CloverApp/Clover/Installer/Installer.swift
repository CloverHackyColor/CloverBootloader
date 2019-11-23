//
//  Installer.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa


// MARK: Installer Window controller
class InstallerWindowController: NSWindowController, NSWindowDelegate {

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
}

let collectionItemWith : Int = 155

// MARK: ItemTextFieldCell (NSTextFieldCell sub class)
class ItemTextFieldCell: NSTextFieldCell {
  override func drawingRect(forBounds rect: NSRect) -> NSRect {
    var nr = super.drawingRect(forBounds: rect)
    let size = self.cellSize(forBounds: rect)
    let diff = nr.size.height - size.height
    if diff > 0 {
      nr.size.height -= diff
      nr.origin.y += (diff / 2)
    }
    return nr
  }
}

// MARK: ItemTextField (NSTextfield sub class)
class ItemTextField: NSTextField {
  var trackingArea: NSTrackingArea? = nil
  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    self.cell = ItemTextFieldCell()
    
    self.trackingArea = NSTrackingArea(rect: self.bounds,
                                       options: [NSTrackingArea.Options.activeAlways, NSTrackingArea.Options.mouseEnteredAndExited],
                                       owner: self, userInfo: nil)
  }
  
  
  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }
  
  override func mouseDown(with event: NSEvent) {
    self.showDescription()
  }
  
  override func updateTrackingAreas() {
    if self.trackingArea != nil {
      self.removeTrackingArea(self.trackingArea!)
    }
    
    self.trackingArea = NSTrackingArea(rect: self.bounds,
                                       options: [NSTrackingArea.Options.activeAlways, NSTrackingArea.Options.mouseEnteredAndExited],
                                       owner: self, userInfo: nil)
    
    self.addTrackingArea(self.trackingArea!)
  }
  
  private func showDescription() {
    if AppSD.isInstalling {
      return
    }
    if let info = self.cell?.representedObject as? String {
      if let ivc = self.target as? InstallerViewController {
        ivc.post(text: info, add: false, color: nil, scroll: false)
      }
    }
  }
}

// MARK: CollectionViewItem (NSCollectionViewItem sub class)
class CollectionViewItem: NSCollectionViewItem {
  var driver : EFIDriver? = nil
  var installerController : InstallerViewController? = nil
  
  public func setState(_ state: NSControl.StateValue) {
    self.driver?.state = state
    self.checkBox.state = state
  }
  
  public let checkBox: NSButton = {
    let butt = NSButton()
    butt.controlSize = .regular
    butt.imagePosition = .noImage
    butt.frame = NSRect(x: 0, y: 0, width: 18, height: 18)
    butt.setButtonType(NSButton.ButtonType.switch)
    butt.state = .off
    return butt
  } ()
  
  public let field: ItemTextField = {
    let f = ItemTextField(frame: NSRect(x: 18, y: 0, width: collectionItemWith - 18, height: 18))
    f.controlSize = .mini
    f.drawsBackground = false
    f.isEditable = false
    f.isSelectable = false
    return f
  } ()
  
  @objc func checkBoxPressed(_ sender: NSButton?) {
    if (sender != nil) {
      self.driver?.state = sender!.state
      if let sectionName = self.driver?.sectionName, let installer = self.installerController {
        let kind = self.driver!.kind
        let currDriverName = self.driver!.src.lastPath
        let sections = installer.sectionsUEFI
        // MemoryFix drivers allow only one choice
        if sectionName.hasSuffix("MemoryFix") && sections.contains(sectionName) {
          allowOnly(driver: self.driver!, kind: kind, in: sectionName, installer: installer)
        } else if sectionName.hasSuffix("FileSystem") && sections.contains(sectionName) {
          if self.driver!.state == .on {
            var exclude : [String] = [String]()
            let currLower = currDriverName.deletingFileExtension.lowercased()
            if currLower == "vboxhfs" {
              exclude = ["hfsplus"]
            } else if currLower == "hfsplus" {
              exclude = ["vboxhfs"]
            } else if currLower == "apfsdriverloader" {
              exclude = ["apfs"]
            } else if currLower == "apfs" {
              exclude = ["apfsdriverloader"]
            } else if currLower == "ntfs" {
              exclude = ["grubntfs"]
            } else if currLower == "grubntfs" {
              exclude = ["ntfs"]
            }
            uncheck(list: exclude,
                    current: currDriverName,
                    kind: kind,
                    sectionName: sectionName,
                    installer: installer)
          }
        } else if sectionName.hasSuffix("mandatory") && sections.contains(sectionName) {
          if self.driver!.state == .on {
            var exclude : [String] = [String]()
            let currLower = currDriverName.deletingFileExtension.lowercased()
            if currLower == "smchelper" {
              exclude = ["virtualsmc"]
            } else if currLower == "virtualsmc" {
              exclude = ["smchelper"]
            }
            uncheck(list: exclude,
                    current: currDriverName,
                    kind: kind,
                    sectionName: sectionName,
                    installer: installer)
          }
        } else if sectionName.hasSuffix("FileVault2") && sections.contains(sectionName) {
          /*
           AppleImageCodec.efi, AppleKeyAggregator.efi, AppleKeyMapAggregator.efi, AppleEvent.efi, AppleUITheme.efi, EnglishDxe-64.efi, FirmwareVolume.efi, HashServiceFix.efi﻿
           
           vs
           
           AppleUiSupport
          */
          if self.driver!.state == .on {
            var exclude : [String] = [String]()
            let currLower = currDriverName.deletingFileExtension.lowercased()
            if currLower == "appleimagecodec" ||
              currLower == "applekeyaggregator" ||
              currLower == "applekeymapaggregator" ||
              currLower == "appleevent" ||
              currLower == "appleuitheme" ||
              currLower == "englishdxe" ||
              currLower == "firmwarevolume" ||
              currLower == "hashservicefix" {
              exclude = ["appleuisupport"]
            } else if currLower == "appleuisupport" {
              exclude = ["appleimagecodec",
                         "applekeyaggregator",
                         "applekeymapaggregator",
                         "appleevent",
                         "appleuitheme",
                         "englishdxe",
                         "firmwarevolume",
                         "hashservicefix"]
            }
            uncheck(list: exclude,
                    current: currDriverName,
                    kind: kind,
                    sectionName: sectionName,
                    installer: installer)
          }
        }
      }
    }
  }
  
  private func allowOnly(driver: EFIDriver,
                         kind: EFIkind,
                         in sectionName: String,
                         installer: InstallerViewController) {
    let sections = installer.sectionsUEFI
    let sectIndex : Int = sections.firstIndex(of: sectionName)!
    let drivers = installer.driversUEFI[sectIndex]
    let driverName = driver.src.lastPath
    
    for (index, drv) in drivers.enumerated() {
      if drv.src.lastPath != driverName {
        installer.driversUEFI[sectIndex][index].state = .off
      } else {
        installer.driversUEFI[sectIndex][index].state = self.driver!.state
        self.driver = installer.driversUEFI[sectIndex][index]
      }
    }
    
    let unknownSection = kind == .uefi ? kUnknownUEFISection : kUnknownBIOSSection
    if driver.state == .on && installer.sectionsUEFI.contains(unknownSection) {
      if sectionName == "UEFI/MemoryFix" {
        uncheck(list: ["aptiomemory", "osxlowmem", "osxaptiofix"],
                current: driverName,
                kind: kind,
                sectionName: unknownSection,
                installer: installer)
      }
    }
  }
  
  private func uncheck(list: [String],
                       current: String,
                       kind: EFIkind,
                       sectionName: String,
                       installer: InstallerViewController) {
    let sections = installer.sectionsUEFI
    let sectIndex : Int = sections.firstIndex(of: sectionName)!
    let drivers = installer.driversUEFI[sectIndex]
    
    for (index, drv) in drivers.enumerated() {
      if drv.src.lastPath != current {
        for d in list {
          if drv.src.lastPath.lowercased().hasPrefix(d) {
            if installer.driversUEFI[sectIndex][index].state != .off {
              installer.driversUEFI[sectIndex][index].state = .off
            }
          }
        }
      } else {
        if installer.driversUEFI[sectIndex][index].state != self.driver!.state {
          installer.driversUEFI[sectIndex][index].state = self.driver!.state
        }
      }
    }
    
    // Since we are excluding drivers by prefix.. then looks for the same in unknown drivers
    let unknownSection = kind == .uefi ? kUnknownUEFISection : kUnknownBIOSSection
    if installer.sectionsUEFI.contains(unknownSection) {
      let usections = installer.sectionsUEFI
      let usectIndex : Int = usections.firstIndex(of: unknownSection)!
      let udrivers = installer.driversUEFI[usectIndex]
      for (index, drv) in udrivers.enumerated() {
        for d in list {
          if drv.src.lastPath.lowercased().hasPrefix(d) {
            installer.driversUEFI[usectIndex][index].state = .off
          }
        }
      }
    }
  }
  
  override func viewDidLoad() {
    super.viewDidLoad()
    self.view.wantsLayer = true
    self.view.layer?.backgroundColor = NSColor.clear.cgColor
    self.checkBox.target = self
    self.checkBox.action = #selector(self.checkBoxPressed(_:))
  }
  
  override func viewDidAppear() {
    super.viewDidAppear()
  }
  
  override func loadView() {
    self.view = NSView(frame: NSRect(x: 0, y: 0, width: collectionItemWith, height: 18))
    self.view.addSubview(self.checkBox)
    self.checkBox.target = self
    self.view.addSubview(self.field)
  }
}

// MARK: HeaderView (NSView sub class)
class HeaderView: NSView {
  public let field: NSTextField = {
    let f = NSTextField()
    f.controlSize = .regular
    f.isEditable = false
    f.isBordered = false
    f.drawsBackground = false
    f.textColor = .white
    f.frame = NSRect(x: 0, y: 0, width: 250, height: 18)
    return f
  } ()
  
  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    self.wantsLayer = true
    self.layer?.backgroundColor = NSColor.gray.cgColor
    self.addSubview(self.field)
    self.field.stringValue = "header"
  }
  
  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }
}


// MARK: InstallerViewController (NSViewController sub class)
let kUEFIRelativeOptionDir : String = /* CloverV2 + */ "EFI/CLOVER/drivers/off/UEFI"
let kBIOSRelativeOptionDir : String = /* CloverV2 + */ "EFI/CLOVER/drivers/off/BIOS"

let kBIOSRelativeDir : String = /* target volume + */ "EFI/CLOVER/drivers/BIOS"
let kUEFIRelativeDir : String = /* target volume + */ "EFI/CLOVER/drivers/UEFI"

let kUnknownUEFISection = "UEFI, but not from this installer"
let kUnknownBIOSSection = "BIOS, but not from this installer"

class InstallerViewController: NSViewController {
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
  
  override func viewDidLoad() {
    super.viewDidLoad()
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
    self.boot1Field.placeholderString = "?"
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
    
    self.driversCollection.delegate = self
    self.driversCollection.dataSource = self
    self.cloverEFICheck.isEnabled = false
    
    
    /*
     let fl = NSCollectionViewFlowLayout()
     fl.itemSize = NSSize(width: Double(collectionItemWith), height: 18.0)
     fl.sectionInset = NSEdgeInsets(top: 5.0, left: 5.0, bottom: 5.0, right: 5.0)
     fl.minimumInteritemSpacing = 5.0
     fl.minimumLineSpacing = 5.0
     fl.headerReferenceSize = CGSize(width: self.driversCollection.frame.width - 30, height: 18)
     self.driversCollection.collectionViewLayout = fl*/
    
    self.driversCollection.isSelectable = true
    
    self.driversCollection.register(CollectionViewItem.self,
                                    forItemWithIdentifier: NSUserInterfaceItemIdentifier(rawValue: "CollectionViewItem"))
    
    self.driversCollection.register(HeaderView.self, forSupplementaryViewOfKind: NSCollectionView.elementKindSectionHeader, withIdentifier: NSUserInterfaceItemIdentifier(rawValue: "CollectionViewHeader"))
    
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
    
    self.driversCollection.reloadData()
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
      if isWritable(diskOrMtp: disk) {
        let fs : String = getFS(from: disk)?.lowercased() ?? kNotAvailable.locale
        let psm : String = getPartitionSchemeMap(from: disk) ?? kNotAvailable.locale
        let name : String = getVolumeName(from: disk) ?? kNotAvailable.locale
        let mp : String = getMountPoint(from: disk) ?? kNotAvailable.locale
        let parentDiskName : String = getMediaName(from: getBSDParent(of: disk) ?? "") ?? kNotAvailable.locale
        if fs == "fat32" || fs == "exfat" || fs == "hfs" {
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
    self.boot1Field.placeholderString = "?"
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
        DispatchQueue.global(qos: .background).async {
          let cmd = "diskutil mount \(disk)"
          let msg = String(format: "Clover wants to mount %@", disk)
          let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
          
          let task = Process()
          task.launchPath = "/usr/bin/osascript"
          task.arguments = ["-e", script]
          
          task.terminationHandler = { t in
            if t.terminationStatus == 0 {
              if isMountPoint(path: disk) {
                self.targetVol = getMountPoint(from: disk) ?? ""
                DispatchQueue.main.async {
                  self.populateTargets()
                  self.setPreferences(for: self.targetVol)
                }
              }
              DispatchQueue.main.async { self.view.window?.makeKeyAndOrderFront(nil) }
            } else {
              NSSound.beep()
              DispatchQueue.main.async { self.driversCollection.reloadData() }
            }
          }
          task.launch()
        }
      } else {
        self.targetVol = getMountPoint(from: disk) ?? ""
        self.populateTargets()
        self.setPreferences(for: self.targetVol)
      }
    } else {
      self.driversUEFI = [[EFIDriver]]()
      self.sectionsUEFI = [String]()
      self.driversCollection.reloadData()
    }
  }
  
  private func setPreferences(for volume: String) {
    self.installButton.isEnabled = true
    self.cloverEFICheck.isEnabled = true
    self.cloverEFICheck.state = .off
    self.bootSectCheck.isEnabled = false
    self.bootSectCheck.state = .off
    self.boot0Pop.isEnabled = false
    self.boot2Pop.isEnabled = false
    self.altBootCheck.isEnabled = false
    self.altBootCheck.state = .off
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
    self.boot1Field.placeholderString = self.boot1Field.stringValue
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
    DispatchQueue.global(qos: .background).async {
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
    }
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
    */
    
    let Cloverapp = NSMutableDictionary()
    let toDelete = NSMutableArray()
    let UEFI = NSMutableArray()
    let BIOS = NSMutableArray()
    
    // minimum required arguments
    Cloverapp.setValue(self.targetVol, forKey: "targetVol")
    
    let  disk = getBSDName(of: self.targetVol) ?? ""
    Cloverapp.setValue(disk, forKey: "disk")
    let shemeMap = getPartitionSchemeMap(from: disk) ?? kNotAvailable
    Cloverapp.setValue(shemeMap, forKey: "shemeMap")
    Cloverapp.setValue(Cloverv2Path, forKey: "CloverV2")
    let filesystem = (getFS(from: disk)?.lowercased()) ?? kNotAvailable
    Cloverapp.setValue(filesystem, forKey: "filesystem")
 
    
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
    if fm.fileExists(atPath: self.targetVol.addPath("EFI/CLOVER")) {
      let df = DateFormatter()
      df.dateFormat = "yyyy-MM-dd hh:mm:ss"
      let now = df.string(from: Date())
      let revIn = findCloverRevision(at: self.targetVol.addPath("EFI")) ?? "0000"
      let backUpPath = NSHomeDirectory().addPath("Desktop/CloverBackUp/EFI_r\(revIn)_\(now)")
      do {
        if !fm.fileExists(atPath: backUpPath.deletingLastPath) {
          try fm.createDirectory(atPath: backUpPath.deletingLastPath,
                                 withIntermediateDirectories: false,
                                 attributes: nil)
        }
        try fm.copyItem(atPath: self.targetVol.addPath("EFI"),
                        toPath: backUpPath)
        
        self.installClover(disk: disk, settingDict: Cloverapp)
      } catch {
        post(text: error.localizedDescription, add: false, color: nil, scroll: false)
      }
    } else {
      self.installClover(disk: disk, settingDict: Cloverapp)
    }
  }
  
  func installClover(disk: String, settingDict : NSDictionary) {
    self.post(text: "Installation begin..", add: false, color: nil, scroll: false)
    if !isMountPoint(path: self.targetVol) {
      NSSound.beep()
      self.post(text: "Can't find target volume, installation aborted.", add: true, color: nil, scroll: false)
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
      self.installButton.isEnabled = false
      AppSD.isInstalling = true
      self.spinner.startAnimation(nil)
      
      self.view.window?.level = .floating // just a hack to keep window in front momentarily
      DispatchQueue.main.asyncAfter(deadline: .now() + 6.0) {
        self.view.window?.level = .normal
      }
      DispatchQueue.global(qos: .background).async {
        let task = Process()
        let msg = "Install Clover".locale
        let helperPath = Bundle.main.executablePath!.deletingLastPath.addPath("Cloverhelper")
        
        let script = "do shell script \"\" & quoted form of \"\(helperPath)\" with prompt \"\(msg)\" with administrator privileges"
        
        task.launchPath = "/usr/bin/osascript"
        task.arguments = ["-e", script]
        let pipe: Pipe = Pipe()
        
        let stdOutHandler =  { (file: FileHandle!) -> Void in
          let data = file.availableData
          //file.closeFile()
          let output = String(decoding: data, as: UTF8.self)
          DispatchQueue.main.async {
            self.view.window?.level = .normal // restore window level to normal
            self.post(text: "\n" + output, add: true, color: nil, scroll: true)
          }
        }
        
        task.standardOutput = pipe
        task.standardError  = pipe
        pipe.fileHandleForReading.readabilityHandler = stdOutHandler
        
        task.terminationHandler = { t in
          
          if t.terminationStatus == 0 {
            DispatchQueue.main.async {
              self.post(text: "\nInstallation succeded.", add: true, color: nil, scroll: true)
            }
          } else {
            NSSound.beep()
            DispatchQueue.main.async {
              self.post(text: "\nInstallation failed.", add: true, color: nil, scroll: true)
            }
          }
          
          DispatchQueue.main.async {
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
        task.launch()
        //task.waitUntilExit()
      }
      
    } else {
      NSSound.beep()
      self.post(text: "Can't write temporary files, installation aborted.", add: true, color: nil, scroll: false)
    }
  }
}

// MARK: InstallerViewController extension with NSCollectionViewDataSource
extension InstallerViewController: NSCollectionViewDataSource {
  
  func collectionView(_ collectionView: NSCollectionView,
                      itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
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
    let h = collectionView.makeSupplementaryView(ofKind: kind,
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
    return h
  }
}

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

