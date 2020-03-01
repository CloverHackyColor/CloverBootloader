//
//  IO.swift
//  Clover
//
//  Created by vector sigma on 12/12/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Foundation

// MARK: get IODeviceTree:/efi dictionary
fileprivate func getEFItree() -> NSMutableDictionary? {
  var ref: io_registry_entry_t
  var masterPort = mach_port_t()
  var oResult: kern_return_t
  var result: kern_return_t
  oResult = IOMasterPort(bootstrap_port, &masterPort)
  
  if oResult != KERN_SUCCESS {
    return nil
  }
  
  ref = IORegistryEntryFromPath(masterPort, "IODeviceTree:/efi")
  if ref == 0 {
    return nil
  }
  var dict : Unmanaged<CFMutableDictionary>?
  result = IORegistryEntryCreateCFProperties(ref, &dict, kCFAllocatorDefault, 0)
  if result != KERN_SUCCESS {
    IOObjectRelease(ref)
    return nil
  }
  IOObjectRelease(ref)
  return dict?.takeRetainedValue()
}

/// Get firmware-vendor string.
func getFirmawareVendor() -> String? {
  if let data = getEFItree()?.object(forKey: "firmware-vendor") as? Data {
    var cleanedData = Data()
    for i in 0..<data.count {
      if data[i] != 0x00 {
        cleanedData.append(data[i])
      }
    }
    return String(bytes: cleanedData, encoding: .utf8)
  }
  
  return nil
}

/// Get device-properties data.
func getDevicePropertiesData() -> Data? {
  if let data = getEFItree()?.object(forKey: "device-properties") as? Data {
    return data
  }
  return nil
}

/// Get Clover Settings data.
func getCloverSettingsData() -> Data? {
  if let data = getEFIPlatform()?.object(forKey: "Settings") as? Data {
    return data
  }
  return nil
}

/// Get IODeviceTree:/efi/platform Dictionary.
fileprivate func getEFIPlatform() -> NSDictionary? {
  var ref: io_registry_entry_t
  var masterPort = mach_port_t()
  var oResult: kern_return_t
  var result: kern_return_t
  oResult = IOMasterPort(bootstrap_port, &masterPort)
  
  if oResult != KERN_SUCCESS {
    return nil
  }
  
  ref = IORegistryEntryFromPath(masterPort, "IODeviceTree:/efi/platform")
  if ref == 0 {
    return nil
  }
  var dict : Unmanaged<CFMutableDictionary>?
  result = IORegistryEntryCreateCFProperties(ref, &dict, kCFAllocatorDefault, 0)
  if result != KERN_SUCCESS {
    IOObjectRelease(ref)
    return nil
  }
  IOObjectRelease(ref)
  return dict?.takeRetainedValue()
}

/// Get OEMVendor string.
func getOEMVendor() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "OEMVendor") as? Data {
    return String(bytes: data, encoding: .utf8)
  }
  
  let ockey = "4D1FDA02-38C7-4A6A-9CC6-4BCCA8B30102:oem-vendor"
  return getNVRAM(variable: ockey)
}

/// Get OEMProduct string.
func getOEMProduct() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "OEMProduct") as? Data {
    return String(bytes: data, encoding: .utf8)
  }
  
  let ockey = "4D1FDA02-38C7-4A6A-9CC6-4BCCA8B30102:oem-product"
  return getNVRAM(variable: ockey)
}

/// Get OEMBoard string.
func getOEMBoard() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "OEMBoard") as? Data {
    return String(bytes: data, encoding: .utf8)
  }
  
  let ockey = "4D1FDA02-38C7-4A6A-9CC6-4BCCA8B30102:oem-board"
  return getNVRAM(variable: ockey)
}

/// Get OEMVendor Short string.
func getOEMVendorShort() -> String? {
  if let vendor = getOEMVendor() {
    switch vendor {
    case "ASRock": fallthrough
    case "Alienware": fallthrough
    case "ECS": fallthrough
    case "EVGA": fallthrough
    case "FUJITSU": fallthrough
    case "IBM": fallthrough
    case "Intel": fallthrough
    case "Shuttle": fallthrough
    case "TOSHIBA": fallthrough
    case "XFX":
      return vendor
    case "Apple Inc.":
      return "Apple"
    case "ASUSTeK Computer INC.": fallthrough
    case "ASUSTeK COMPUTER INC.":
      return "ASUS"
    case "Dell Inc.":
      return "Dell"
    case "DFI": fallthrough
    case "DFI Inc.":
      return "DFI"
    case "EPoX COMPUTER CO., LTD":
      return "EPoX"
    case "First International Computer, Inc.":
      return "FIC"
    case "FUJITSU SIEMENS":
      return "FUJITSU"
    case "Gigabyte Technology Co., Ltd.":
      return "Gigabyte"
    case "Hewlett-Packard":
      return "HP"
    case "Intel Corp.": fallthrough
    case "Intel Corporation": fallthrough
    case "INTEL Corporation":
      return "Intel"
    case "Lenovo": fallthrough
    case "LENOVO":
      return "Lenovo"
    case "Micro-Star International": fallthrough
    case "Micro-Star International Co., Ltd.": fallthrough
    case "MICRO-STAR INTERNATIONAL CO., LTD": fallthrough
    case "MICRO-STAR INTERNATIONAL CO.,LTD": fallthrough
    case "MSI":
      return "MSI"
      case "To be filled by O.E.M.": break
    default:
      return vendor
    }
  }
  return nil
}

/// Get SystemSerialNumber string.
func getSystemSerialNumber() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "SystemSerialNumber") as? Data {
    var cleanedData = Data()
    for i in 0..<data.count {
      if data[i] != 0x00 {
        cleanedData.append(data[i])
      }
    }
    return String(bytes: cleanedData, encoding: .utf8)
  }
  return nil
}

/// Get motherboard Model string.
func getEFIModel() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "Model") as? Data {
    var cleanedData = Data()
    for i in 0..<data.count {
      if data[i] != 0x00 {
        cleanedData.append(data[i])
      }
    }
    return String(bytes: cleanedData, encoding: .utf8)
  }
  return nil
}

/// Get macOS board-id string.
func getEFIBoardID() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "board-id") as? Data {
    var cleanedData = Data()
    for i in 0..<data.count {
      if data[i] != 0x00 {
        cleanedData.append(data[i])
      }
    }
    return String(bytes: cleanedData, encoding: .utf8)
  }
  return nil
}

/// Determine if the bootloader is a (known) legacy firmware
func isLegacyFirmware() -> Bool {
  var isUEFI = true
  let fwname = getFirmawareVendor()?.lowercased()
  
  if fwname!.lowercased().hasPrefix("edk")
    || fwname!.lowercased().hasPrefix("chameleon")
    || fwname!.lowercased().hasPrefix("enoch")
    || fwname!.lowercased().hasPrefix("duet")
    || fwname!.lowercased().hasPrefix("clover") {
    isUEFI = false
  }
  
  return !isUEFI
}

/// Get the  property value from the given IOService dictionary. You’re responsible for releasing the result (if not transferred to Swift objects).
func IOServiceGetPropertyFrom(matching name: String, property: String) -> Any? {
  var obj : Any? = nil
  var iter : io_iterator_t = 0
  var rl : uint32 = 0
  
  var result : kern_return_t = IORegistryCreateIterator(kIOMasterPortDefault,
                                                        kIOServicePlane,
                                                        0,
                                                        &iter)
  
  if result == KERN_SUCCESS && iter != 0 {
    var entry : io_object_t
    repeat {
      
      entry = IOIteratorNext(iter)
      if entry != IO_OBJECT_NULL {
        if entry.name() == name {
          let ref = IORegistryEntryCreateCFProperty(entry,
                                                    property as CFString,
                                                    kCFAllocatorDefault,
                                                    0)
          if ref != nil {
            obj = ref!.takeRetainedValue()
            IOObjectRelease(entry)
            break
          }
        }
        
        rl += 1
        result = IORegistryIteratorEnterEntry(iter)
      } else {
        if rl == 0 {
          IOObjectRelease(entry)
          break
        }
        result = IORegistryIteratorExitEntry(iter)
        rl -= 1
      }
    } while (true)
    IOObjectRelease(iter)
  }
  
  return obj
}
