//
//  DriversCollection.swift
//  Clover
//
//  Created by vector sigma on 27/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

let collectionItemWith : Int = 155

let kUEFIRelativeOptionDir : String = /* CloverV2 + */ "EFI/CLOVER/drivers/off/UEFI"
let kBIOSRelativeOptionDir : String = /* CloverV2 + */ "EFI/CLOVER/drivers/off/BIOS"

let kBIOSRelativeDir : String = /* target volume + */ "EFI/CLOVER/drivers/BIOS"
let kUEFIRelativeDir : String = /* target volume + */ "EFI/CLOVER/drivers/UEFI"

let kUnknownUEFISection = "UEFI, but not from this installer"
let kUnknownBIOSSection = "BIOS, but not from this installer"

// MARK: ItemTextFieldCell (NSTextFieldCell sub class)
final class ItemTextFieldCell: NSTextFieldCell {
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
final class ItemTextField: NSTextField {
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
final class CollectionViewItem: NSCollectionViewItem {
  var driver : EFIDriver? = nil
  var installerController : InstallerViewController? = nil
  var installerOutController : InstallerOutViewController? = nil
  
  public func setState(_ state: NSControl.StateValue) {
    self.driver?.state = state
    self.checkBox.state = state
  }
  
  public let checkBox: NSButton = {
    let butt = NSButton()
    if #available(OSX 10.10, *) {
      butt.controlSize = .regular
    }
    butt.imagePosition = .noImage
    butt.frame = NSRect(x: 0, y: 0, width: 18, height: 18)
    butt.setButtonType(NSButton.ButtonType.switch)
    butt.state = .off
    return butt
  } ()
  
  public let field: ItemTextField = {
    let f = ItemTextField(frame: NSRect(x: 18, y: 0, width: collectionItemWith - 18, height: 18))
    if #available(OSX 10.10, *) {
      f.controlSize = .mini
    }
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
      } else if let sectionName = self.driver?.sectionName, let installer = self.installerOutController {
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
                         installer: NSViewController) {
    if let vc = installer as? InstallerViewController {
      let sections = vc.sectionsUEFI
      let sectIndex : Int = sections.firstIndex(of: sectionName)!
      let drivers = vc.driversUEFI[sectIndex]
      let driverName = driver.src.lastPath
      
      for (index, drv) in drivers.enumerated() {
        if drv.src.lastPath != driverName {
          vc.driversUEFI[sectIndex][index].state = .off
        } else {
          vc.driversUEFI[sectIndex][index].state = self.driver!.state
          self.driver = vc.driversUEFI[sectIndex][index]
        }
      }
      
      let unknownSection = kind == .uefi ? kUnknownUEFISection : kUnknownBIOSSection
      if driver.state == .on && vc.sectionsUEFI.contains(unknownSection) {
        if sectionName == "UEFI/MemoryFix" {
          uncheck(list: ["aptiomemory", "osxlowmem", "osxaptiofix", "ocquirks", "openruntime"],
                  current: driverName,
                  kind: kind,
                  sectionName: unknownSection,
                  installer: installer)
        }
      }
      
      // OcQuirks exception
      if sectionName == "UEFI/MemoryFix" &&
        kind == .uefi &&
        (driverName.lowercased() == "ocquirks.efi" || driverName.lowercased() == "openruntime.efi") &&
        driver.state == .on {
        check(driver: "OcQuirks.efi", kind: kind, sectionName: sectionName, installer: vc)
        check(driver: "OpenRuntime.efi", kind: kind, sectionName: sectionName, installer: vc)
      }
    } else if let vc = installer as? InstallerOutViewController {
      let sections = vc.sectionsUEFI
      let sectIndex : Int = sections.firstIndex(of: sectionName)!
      let drivers = vc.driversUEFI[sectIndex]
      let driverName = driver.src.lastPath
      
      for (index, drv) in drivers.enumerated() {
        if drv.src.lastPath != driverName {
          vc.driversUEFI[sectIndex][index].state = .off
        } else {
          vc.driversUEFI[sectIndex][index].state = self.driver!.state
          self.driver = vc.driversUEFI[sectIndex][index]
        }
      }
      
      let unknownSection = kind == .uefi ? kUnknownUEFISection : kUnknownBIOSSection
      if driver.state == .on && vc.sectionsUEFI.contains(unknownSection) {
        if sectionName == "UEFI/MemoryFix" {
          uncheck(list: ["aptiomemory", "osxlowmem", "osxaptiofix", "ocquirks", "openruntime"],
                  current: driverName,
                  kind: kind,
                  sectionName: unknownSection,
                  installer: installer)
        }
      }
      
      // OcQuirks exception
      if sectionName == "UEFI/MemoryFix" &&
        kind == .uefi &&
        (driverName.lowercased() == "ocquirks.efi" || driverName.lowercased() == "openruntime.efi") &&
        driver.state == .on {
        check(driver: "OcQuirks.efi", kind: kind, sectionName: sectionName, installer: vc)
        check(driver: "OpenRuntime.efi", kind: kind, sectionName: sectionName, installer: vc)
      }
    }
  }
  
  private func check(driver name: String,
                     kind: EFIkind,
                     sectionName: String,
                     installer: NSViewController) {
    if let vc = installer as? InstallerViewController {
      let sections = vc.sectionsUEFI
      let sectIndex : Int = sections.firstIndex(of: sectionName)!
      let drivers = vc.driversUEFI[sectIndex]
      
      for (index, drv) in drivers.enumerated() {
        if drv.src.lastPath == name {
          if vc.driversUEFI[sectIndex][index].state == .off {
            vc.driversUEFI[sectIndex][index].state = .on
          }
        }
      }
    } else if let vc = installer as? InstallerOutViewController {
      let sections = vc.sectionsUEFI
      let sectIndex : Int = sections.firstIndex(of: sectionName)!
      let drivers = vc.driversUEFI[sectIndex]
      
      for (index, drv) in drivers.enumerated() {
        if drv.src.lastPath == name {
          if vc.driversUEFI[sectIndex][index].state == .off {
            vc.driversUEFI[sectIndex][index].state = .on
          }
        }
      }
    }
  }
  
  private func uncheck(list: [String],
                       current: String,
                       kind: EFIkind,
                       sectionName: String,
                       installer: NSViewController) {
    if let vc = installer as? InstallerViewController {
      let sections = vc.sectionsUEFI
      let sectIndex : Int = sections.firstIndex(of: sectionName)!
      let drivers = vc.driversUEFI[sectIndex]
      
      for (index, drv) in drivers.enumerated() {
        if drv.src.lastPath != current {
          for d in list {
            if drv.src.lastPath.lowercased().hasPrefix(d) {
              if vc.driversUEFI[sectIndex][index].state != .off {
                vc.driversUEFI[sectIndex][index].state = .off
              }
            }
          }
        } else {
          if vc.driversUEFI[sectIndex][index].state != self.driver!.state {
            vc.driversUEFI[sectIndex][index].state = self.driver!.state
          }
        }
      }
      
      // Since we are excluding drivers by prefix.. then looks for the same in unknown drivers
      let unknownSection = kind == .uefi ? kUnknownUEFISection : kUnknownBIOSSection
      if vc.sectionsUEFI.contains(unknownSection) {
        let usections = vc.sectionsUEFI
        let usectIndex : Int = usections.firstIndex(of: unknownSection)!
        let udrivers = vc.driversUEFI[usectIndex]
        for (index, drv) in udrivers.enumerated() {
          for d in list {
            if drv.src.lastPath.lowercased().hasPrefix(d) {
              vc.driversUEFI[usectIndex][index].state = .off
            }
          }
        }
      }
    } else if let vc = installer as? InstallerOutViewController {
      let sections = vc.sectionsUEFI
      let sectIndex : Int = sections.firstIndex(of: sectionName)!
      let drivers = vc.driversUEFI[sectIndex]
      
      for (index, drv) in drivers.enumerated() {
        if drv.src.lastPath != current {
          for d in list {
            if drv.src.lastPath.lowercased().hasPrefix(d) {
              if vc.driversUEFI[sectIndex][index].state != .off {
                vc.driversUEFI[sectIndex][index].state = .off
              }
            }
          }
        } else {
          if vc.driversUEFI[sectIndex][index].state != self.driver!.state {
            vc.driversUEFI[sectIndex][index].state = self.driver!.state
          }
        }
      }
      
      // Since we are excluding drivers by prefix.. then looks for the same in unknown drivers
      let unknownSection = kind == .uefi ? kUnknownUEFISection : kUnknownBIOSSection
      if vc.sectionsUEFI.contains(unknownSection) {
        let usections = vc.sectionsUEFI
        let usectIndex : Int = usections.firstIndex(of: unknownSection)!
        let udrivers = vc.driversUEFI[usectIndex]
        for (index, drv) in udrivers.enumerated() {
          for d in list {
            if drv.src.lastPath.lowercased().hasPrefix(d) {
              vc.driversUEFI[usectIndex][index].state = .off
            }
          }
        }
      }
    }
    
  }
  
  override func viewDidLoad() {
    if #available(OSX 10.10, *) {
      super.viewDidLoad()
    }
    self.view.wantsLayer = true
    self.view.layer?.backgroundColor = NSColor.clear.cgColor
    self.checkBox.target = self
    self.checkBox.action = #selector(self.checkBoxPressed(_:))
  }
  
  override func loadView() {
    self.view = NSView(frame: NSRect(x: 0, y: 0, width: collectionItemWith, height: 18))
    self.view.addSubview(self.checkBox)
    self.checkBox.target = self
    self.view.addSubview(self.field)
    
    if #available(OSX 10.10, *) {} else {
      self.viewDidLoad()
    }
  }
}

// MARK: HeaderView (NSView sub class)
final class HeaderView: NSView {
  public let field: NSTextField = {
    let f = NSTextField()
    if #available(OSX 10.10, *) {
      f.controlSize = .regular
    }
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

