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
    cleanedData.append(0x00)
    return String(bytes: cleanedData, encoding: .utf8)
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
  return nil
}

/// Get OEMProduct string.
func getOEMProduct() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "OEMProduct") as? Data {
    return String(bytes: data, encoding: .utf8)
  }
  return nil
}

/// Get OEMBoard string.
func getOEMBoard() -> String? {
  if let data = getEFIPlatform()?.object(forKey: "OEMBoard") as? Data {
    return String(bytes: data, encoding: .utf8)
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
    cleanedData.append(0x00)
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
    cleanedData.append(0x00)
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
    cleanedData.append(0x00)
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
