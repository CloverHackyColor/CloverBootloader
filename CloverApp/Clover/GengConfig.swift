//
//  GengConfig.swift
//  Clover
//
//  Created by vector sigma on 22/02/2020.
//  Copyright © 2020 CloverHackyColor. All rights reserved.
//


import Foundation

extension Mirror {
  var data: Data {
    var d : Data = Data()
    for child in self.children {
      d.append(child.value as! UInt8)
    }
    return d
  }
  
  var dataString: String {
    var str : String = ""
    for child in self.children {
      str += String(format: "%02x", child.value as! UInt8)
    }
    return str
  }
  
  var CHAR16String: String? {
    var data : Data = Data()
    for child in self.children {
      let u16 = child.value as! UInt16
      let u80 = UInt8(u16 >> 8)
      let u81 = UInt8(u16 & 0x00ff)
      if u80 != 0x00 {
        data.append(u80)
      }
      if u81 != 0x00 {
        data.append(u81)
      }
    }
    return String(data: data, encoding: .utf8)
  }
  
  var CHAR8String: String? {
    var chars : [Character] = [Character]()
    for child in self.children {
      if let byte = UInt8(exactly: child.value as! Int8) {
        if byte != 0x00 {
          let char = Character(UnicodeScalar(byte))
          chars.append(char)
        }
      }
    }
    return String(chars)
  }
  
  var labels : [String] {
    var l : [String] = [String]()
    for child in self.children {
      if (child.label != nil) {
        l.append(child.label!)
      }
    }
    return l
  }
}

extension EFI_GUID {
  var uuidString: String? {
    var data : Data = Data()
    data.append(Data1.data)
    data.append(Data2.data)
    data.append(Data3.data)
    data.append(Mirror(reflecting: Data4).data)
    var str = ""
    for i in 0..<data.count {
      str += String(format:"%02x", data[i])
      if str.count == 8
        || str.count == 13
        || str.count == 18
        || str.count == 23 {
        str += "-"
      }
    }
    return str
  }
}

// MARK: SETTINGS_DATA_TYPE
enum SETTINGS_DATA_TYPE: Double {
  case CHAR8String
  case CHAR16String
  case DataString
  case HexString
  case BOOLEAN
  case INTEGER
  case UUIDString
}

extension SETTINGS_DATA {
  var labels: [String] {
    return Mirror(reflecting: self).labels
  }
  
  func getDSDTFixes() -> [String : Any]? {
    var Fixes = [String : Any]()
    /*
     FixDsdt is UINT32. FixesNew array indexes reflect bit field
     */
    let FixesNew = ["AddDTGP",      "FixDarwin",  "FixShutdown",  "AddMCHC",
                    "FixHPET",      "FakeLPC",    "FixIPIC",      "FixSBUS",
                    "FixDisplay",   "FixIDE",     "FixSATA",      "FixFirewire",
                    "FixUSB",       "FixLAN",     "FixAirport",   "FixHDA",
                    "FixDarwin7",   "FixRTC",     "FixTMR",       "AddIMEI",
                    "FixIntelGfx",  "FixWAK",     "DeleteUnused", "FixADP1",
                    "AddPNLF",      "FixS3D",     "FixACST",      "AddHDMI",
                    "FixRegions",   "FixHeaders", "FixMutex"]
    
    let FixesOld = ["AddDTGP_0001",         "FixDarwin_0002",       "FixShutdown_0004",     "AddMCHC_0008",
                    "FixHPET_0010",         "FakeLPC_0020",         "FixIPIC_0040",         "FixSBUS_0080",
                    "FixDisplay_0100",      "FixIDE_0200",          "FixSATA_0400",         "FixFirewire_0800",
                    "FixUSB_1000",          "FixLAN_2000",          "FixAirport_4000",      "FixHDA_8000", "FixHDA",
                    "FixDarwin7_10000",     "FIX_RTC_20000",        "FIX_TMR_40000",        "AddIMEI_80000",
                    "FIX_INTELGFX_100000",  "FIX_WAK_200000",       "DeleteUnused_400000",  "FIX_ADP1_800000",
                    "AddPNLF_1000000",      "FIX_S3D_2000000",      "FIX_ACST_4000000",     "AddHDMI_8000000",
                    "FixRegions_10000000",  "FixHeaders_20000000",  "FixHeaders"]
    
    var found = false
    
    for child in Mirror(reflecting: self).children {
      if child.label != nil && child.label! == "FixDsdt" {
        found = true
        let FixDsdt : UINT32 = child.value as! UINT32
        if AppSD.CloverRevision >= 4006 {
          for i in 0..<FixesNew.count {
            let fix = FixesNew[i]
            Fixes[fix] = (FixDsdt & 1 << i) > 0
          }
        } else {
          for i in 0..<FixesOld.count {
            let fix = FixesOld[i]
            Fixes[fix] = (FixDsdt & 1 << i) > 0
          }
        }
        
        break
      }
    }
    
    if !found {
      print("SETTINGS_DATA: label 'FixDsdt' not found.")
      return nil
    }
    
    return Fixes
  }
  
  func getDropOEM_DSM() -> [String : Any]? {
    var dict = [String : Any]()
    /*
     DropOEM_DSM is UINT16. dsms array indexes reflect bit field
     */
    let dsms = ["ATI",
                "NVidia",
                "IntelGFX",
                "HDA",
                "HDMI",
                "LAN",
                "WIFI",
                "SATA",
                "IDE",
                "LPC",
                "SmBUS",
                "USB",
                "Firewire"]
    
    var found = false
    
    for child in Mirror(reflecting: self).children {
      if child.label != nil && child.label! == "DropOEM_DSM" {
        found = true
        let DropOEM_DSM : UINT16 = child.value as! UINT16
        for i in 0..<dsms.count {
          let fix = dsms[i]
          dict[fix] = (DropOEM_DSM & 1 << i) > 0
        }
        
        break
      }
    }
    
    if !found {
      print("SETTINGS_DATA: label 'DropOEM_DSM' not found.")
      return nil
    }
    
    return dict
  }
  
  func kpValue(for label: String, type: SETTINGS_DATA_TYPE) -> Any? {
    var found = false
    var value : Any? = nil
    if self.labels.contains("KernelAndKextPatches") {
      for child in Mirror(reflecting: self.KernelAndKextPatches).children {
        if child.label != nil && child.label! == label {
          //print(child.label!)
          switch type {
          case .CHAR8String:
            value = Mirror(reflecting: child.value).CHAR8String
          case .CHAR16String:
            value = Mirror(reflecting: child.value).CHAR16String
          case .DataString:
            value = Mirror(reflecting: child.value).dataString
          case .UUIDString:
            if let eg = child.value as? EFI_GUID {
              value = eg.uuidString
            }
          case .HexString:
            value = NSNumber(value: Int("\(child.value)")!).hexString
          case .BOOLEAN:
            value = NSNumber(value: child.value as! BOOLEAN).boolValue
          case .INTEGER:
            value = Int("\(child.value)")
          }
          found = true
          break
        }
      }
    }
    
    if !found {
      print("KernelAndKextPatches: label '\(label)' not found.")
      return nil
    }
    
    if value == nil {
      print("KernelAndKextPatches: value for label '\(label)' is nil.")
      return nil
    }
    return value
  }
  
  func value(for label: String, type: SETTINGS_DATA_TYPE) -> Any? {
    var found = false
    var value : Any? = nil
    for child in Mirror(reflecting: self).children {
      if child.label != nil && child.label! == label {
        //print(child.label!)
        switch type {
        case .CHAR8String:
          value = Mirror(reflecting: child.value).CHAR8String
        case .CHAR16String:
          value = Mirror(reflecting: child.value).CHAR16String
        case .DataString:
          value = Mirror(reflecting: child.value).dataString
        case .UUIDString:
          if let eg = child.value as? EFI_GUID {
            value = eg.uuidString
          }
        case .HexString:
          value = NSNumber(value: Int("\(child.value)")!).hexString
        case .BOOLEAN:
          value = NSNumber(value: child.value as! BOOLEAN).boolValue
        case .INTEGER:
          if child.label! == "flagstate" {
            value = NSNumber(value: self.flagstate.0).intValue
          } else {
            value = Int("\(child.value)")
          }
        }
        found = true
        break
      }
    }
    
    if !found {
      print("SETTINGS_DATA: label '\(label)' not found.")
      return nil
    }
    
    if value == nil {
      print("SETTINGS_DATA: value for label '\(label)' is nil.")
      return nil
    }
    
    return value
  }
}

final class CloverConfig: NSObject {
  private var config = [String: Any]()
  func generateCloverConfig() -> [String: Any]? {
    
    if AppSD.CloverRevision <= 3250 {
      print("Clover gen Config: Clover Revision too old or just not Clover.")
      return nil
    }
    if let data = getCloverSettingsData() {
      
      var s : SETTINGS_DATA = SETTINGS_DATA()
      
      withUnsafeMutablePointer(to: &s) { pointer in
        let bound = pointer.withMemoryRebound(to: UInt8.self, capacity: data.count) { $0 }
        data.enumerated().forEach { (bound + $0.offset).pointee = $0.element }
      }

      self.config["ConfigName"] = s.value(for: "ConfigName", type: .CHAR16String)
      
      // MARK: ACPI
      var ACPI = [String : Any]()
      ACPI["ResetAddress"] =  s.value(for: "ResetAddr", type: .HexString)
      ACPI["ResetValue"] =    s.value(for: "ResetVal", type: .HexString)
      ACPI["HaltEnabler"] =   s.value(for: "SlpSmiEnable", type: .BOOLEAN)
      ACPI["PatchAPIC"] =     s.value(for: "PatchNMI", type: .BOOLEAN)
      ACPI["smartUPS"] =      s.value(for: "smartUPS", type: .BOOLEAN)
      ACPI["AutoMerge"] =     s.value(for: "AutoMerge", type: .BOOLEAN)
      ACPI["DisableASPM"] =   s.value(for: "NoASPM", type: .BOOLEAN)
      ACPI["FixHeaders"] =    s.value(for: "FixHeaders", type: .BOOLEAN)
      ACPI["FixMCFG"] =       s.value(for: "FixMCFG", type: .BOOLEAN)
      
      // MARK: ACPI->DSDT
      var DSDT = [String : Any]()
      
      DSDT["Name"] =            s.value(for: "DsdtName", type: .CHAR16String)
      DSDT["Debug"] =           s.value(for: "DebugDSDT", type: .BOOLEAN)
      DSDT["ReuseFFFF"] =       s.value(for: "ReuseFFFF", type: .BOOLEAN)
      DSDT["SuspendOverride"] = s.value(for: "SuspendOverride", type: .BOOLEAN)
      DSDT["Rtc8Allowed"] =     s.value(for: "Rtc8Allowed", type: .BOOLEAN)
      DSDT["#Patches count"] =  s.value(for: "PatchDsdtNum", type: .INTEGER)
      
      // MARK: ACPI->DSDT->Fixes
      DSDT["Fixes"] = s.getDSDTFixes()
      
      // MARK: ACPI->DSDT->Patches
      var Patches = [Any]()
      var PatchesDict1 = [String : Any]()
      PatchesDict1["Comment"] = "This is for sample"
      PatchesDict1["Disabled"] = true
      PatchesDict1["Find"] = "_NOT_SHOWN_"
      PatchesDict1["Replace"] = "_NOT_SHOWN_"
      Patches.append(PatchesDict1)
      DSDT["Patches"] = Patches
      
      // MARK: ACPI->DSDT->DropOEM_DSM
      DSDT["DropOEM_DSM"] = s.getDropOEM_DSM()
      
      ACPI["DSDT"] = DSDT
      
      // MARK: ACPI->SSDT
      var SSDT = [String : Any]()
      SSDT["DropOem"] =           s.value(for: "DropSSDT", type: .BOOLEAN)
      SSDT["#DoubleFirstState"] = s.value(for: "DoubleFirstState", type: .BOOLEAN)
      SSDT["#MinMultiplier"] =    s.value(for: "MinMultiplier", type: .INTEGER)
      SSDT["#MaxMultiplier"] =    s.value(for: "MaxMultiplier", type: .INTEGER)
      SSDT["#PLimitDict"] =       s.value(for: "PLimitDict", type: .INTEGER)
      SSDT["#UnderVoltStep"] =    s.value(for: "UnderVoltStep", type: .INTEGER)
      SSDT["#PluginType"] =       s.value(for: "PluginType", type: .INTEGER)
      SSDT["#UseSystemIO"] =      s.value(for: "EnableISS", type: .BOOLEAN)
      SSDT["#EnableC2"] =         s.value(for: "EnableC2", type: .BOOLEAN)
      SSDT["#EnableC4"] =         s.value(for: "EnableC4", type: .BOOLEAN)
      SSDT["#EnableC6"] =         s.value(for: "EnableC6", type: .BOOLEAN)
      SSDT["#EnableC7"] =         s.value(for: "EnableC7", type: .BOOLEAN)
      SSDT["#C3Latency"] =        s.value(for: "C3Latency", type: .INTEGER)
      SSDT["NoDynamicExtract"] =  s.value(for: "NoDynamicExtract", type: .BOOLEAN)
      
      // MARK: ACPI->SSDT->Generate
      var Generate = [String : Any]()
      Generate["PStates"] =     s.value(for: "GeneratePStates", type: .BOOLEAN)
      Generate["CStates"] =     s.value(for: "GenerateCStates", type: .BOOLEAN)
      Generate["APSN"] =        s.value(for: "GenerateAPSN", type: .BOOLEAN)
      Generate["APLF"] =        s.value(for: "GenerateAPLF", type: .BOOLEAN)
      Generate["PluginType"] =  s.value(for: "GeneratePluginType", type: .BOOLEAN)

      if Generate.keys.count > 0 {
        SSDT["Generate"] = Generate
      }
      
      ACPI["SSDT"] = SSDT
      
      // MARK: ACPI->DropTables
      var DropTables = [Any]()
      var DropTablesDict1 = [String : Any]()
      DropTablesDict1["#Signature"] = "_NOT_SHOWN_"
      DropTablesDict1["#TableId"] = "_NOT_SHOWN_"
      DropTablesDict1["#Length"] = 0
      DropTables.append(DropTablesDict1)
      ACPI["DropTables"] = DropTables
      
      // MARK: ACPI->SortedOrder
      var SortedOrder = [Any]()
      SortedOrder.append("SSDT-1.aml")
      ACPI["#Sorted ACPI tables Count"] = s.value(for: "SortedACPICount", type: .INTEGER)
      ACPI["#SortedOrder"] = SortedOrder
      
      // MARK: ACPI->RenameDevices
      if AppSD.CloverRevision >= 4468 {
        var RenameDevices = [String : Any]()
        RenameDevices["#_SB.PCI0.RP01.PXSX"] = "ARPT"
        RenameDevices["_SB.PCI0.RP02.PXSX"] = "XHC2"
        ACPI["#RenameDevices"] = RenameDevices
      }
      
      self.config["ACPI"] = ACPI
      
      // MARK: Boot
      var Boot = [String: Any]()
      Boot["Arguments"]  =              s.value(for: "BootArgs", type: .CHAR8String)
      Boot["Legacy"]  =                 s.value(for: "LegacyBoot", type: .CHAR16String)
      Boot["XMPDetection"] =            s.value(for: "XMPDetection", type: .INTEGER)
      Boot["Debug"] =                   s.value(for: "Debug", type: .BOOLEAN)
      Boot["#Timeout"] = "_NOT_SHOWN_"
      Boot["Fast"] = false
      Boot["#CustomLogo"] = "_NOT_SHOWN_"
      Boot["#NeverHibernate"] = false
      Boot["#StrictHibernate"] = false
      Boot["#RtcHibernateAware"] = false
      Boot["NeverDoRecovery"] =         s.value(for: "NeverDoRecovery", type: .BOOLEAN)
      Boot["SkipHibernateTimeout"] =    s.value(for: "SkipHibernateTimeout", type: .BOOLEAN)
      Boot["DisableCloverHotkeys"] =    s.value(for: "DisableCloverHotkeys", type: .BOOLEAN)
      Boot["LegacyBiosDefaultEntry"] =  s.value(for: "LegacyBiosDefaultEntry", type: .INTEGER)
      
      if Boot.keys.count > 0 {
        self.config["Boot"] = Boot
      }
      
      // MARK: BootGraphics
      var BootGraphics = [String: Any]()
      BootGraphics["DefaultBackgroundColor"] =  s.value(for: "DefaultBackgroundColor", type: .HexString)
      BootGraphics["UIScale"] =                 s.value(for: "UIScale", type: .INTEGER)
      BootGraphics["EFILoginHiDPI"] =           s.value(for: "EFILoginHiDPI", type: .INTEGER)
      BootGraphics["flagstate"] =               s.value(for: "flagstate", type: .INTEGER)
      
      if BootGraphics.keys.count > 0 {
        self.config["BootGraphics"] = BootGraphics
      }
      
      // MARK: CPU
      var CPU = [String: Any]()
      CPU["Type"] =             s.value(for: "CpuType", type: .HexString)
      CPU["FrequencyMHz"] =     s.value(for: "CpuFreqMHz", type: .INTEGER)
      CPU["#BusSpeedkHz"] =     s.value(for: "BusSpeed", type: .INTEGER)
      CPU["QPI"] =              s.value(for: "QPI", type: .INTEGER)
      CPU["SavingMode"] =       s.value(for: "SavingMode", type: .INTEGER)
      CPU["#UseARTFrequency"] = s.value(for: "UseARTFreq", type: .BOOLEAN)
      CPU["#TurboDisable"] =    s.value(for: "Turbo", type: .BOOLEAN)
      CPU["#HWPEnable"] =       s.value(for: "HWP", type: .BOOLEAN)
      CPU["#HWPValue"] =        s.value(for: "HWPValue", type: .INTEGER)
      CPU["EnabledCores"] =     s.value(for: "EnabledCores", type: .INTEGER)
      CPU["#TDP"] =             s.value(for: "TDP", type: .BOOLEAN)
      CPU["#QEMU"] =            s.value(for: "QEMU", type: .BOOLEAN)
      
      if CPU.keys.count > 0 {
        self.config["CPU"] = CPU
      }
      
      // MARK: Devices
      var Devices = [String: Any]()
      Devices["#Inject"] =                  s.value(for: "StringInjector", type: .BOOLEAN)
      Devices["#Properties"] = "_NOT_SHOWN_"
      Devices["#NoDefaultProperties"] =     s.value(for: "NoDefaultProperties", type: .BOOLEAN)
      Devices["UseIntelHDMI"] =             s.value(for: "UseIntelHDMI", type: .BOOLEAN)
      Devices["ForceHPET"] =                s.value(for: "ForceHPET", type: .BOOLEAN)
      Devices["#SetIntelBacklight"] =       s.value(for: "IntelBacklight", type: .BOOLEAN)
      Devices["#SetIntelMaxBacklight"] =    s.value(for: "IntelMaxBacklight", type: .BOOLEAN)
      Devices["#IntelMaxValue"] =           s.value(for: "IntelMaxValue", type: .INTEGER)
     
      
      // MARK: Devices->AddProperties
      var AddProperties = [Any]()
      var AddPropertiesDict1 = [String: Any]()
      
      AddPropertiesDict1["#Device"] = "XXX"
      AddPropertiesDict1["#Disabled"] = true
      AddPropertiesDict1["#Key"] = "AAPL,XXX"
      AddPropertiesDict1["#Value"] = NSNumber(value: 0xFFFF).hexString
      AddProperties.append(AddPropertiesDict1)
      Devices["AddProperties"] = AddProperties
      
      // MARK: Devices->Properties
      if AppSD.CloverRevision >= 4466 {
        if let data = getDevicePropertiesData()?.toPointer() {
          var Properties = [String : Any]()
          let gfx : GFX_HEADER? = parse_binary(data).pointee
          var block = gfx?.blocks
          
          repeat {
            var entry = block?.pointee.entries
            if let dpath = ConvertDevicePathToAscii(block!.pointee.devpath, 1, 1) {
              let blockKey = String(cString: dpath)
              var blockDict = [String: Any]()
              repeat {
                if entry != nil {
                  let key = String(cString: entry!.pointee.key)
                  let count = Int(entry!.pointee.val_len)
                  switch entry!.pointee.val_type {
                  case DATA_INT8:  fallthrough
                  case DATA_INT16: fallthrough
                  case DATA_INT32: fallthrough
                  case DATA_BINARY:
                    let data = Data(bytes: entry!.pointee.val, count: count)
                    blockDict[key] = data
                  case DATA_STRING:
                    let str = String(cString: entry!.pointee.val)
                    blockDict[key] = str
                  default:
                    // we should not be here as all the types are enumerated
                    break
                  }
                  entry = entry!.pointee.next
                }
              } while (entry != nil)
              
              if blockDict.keys.count > 0 {
                Properties[blockKey] = blockDict
              }
            }
            
            block = block?.pointee.next
          } while (block != nil)
          Devices["Properties"] = Properties
        }
      }
      
      
      // MARK: Devices->FakeID
      var FakeID = [String : Any]()
      FakeID["ATI"] =       s.value(for: "FakeATI", type: .HexString)
      FakeID["NVidia"] =    s.value(for: "FakeNVidia", type: .HexString)
      FakeID["IntelGFX"] =  s.value(for: "FakeIntel", type: .HexString)
      FakeID["LAN"] =       s.value(for: "FakeLAN", type: .HexString)
      FakeID["WIFI"] =      s.value(for: "FakeWIFI", type: .HexString)
      FakeID["SATA"] =      s.value(for: "FakeSATA", type: .HexString)
      FakeID["XHCI"] =      s.value(for: "FakeXHCI", type: .HexString)
      FakeID["IMEI"] =      s.value(for: "FakeIMEI", type: .HexString)
      
      if FakeID.keys.count > 0 {
        Devices["FakeID"] = FakeID
      }
      
      // MARK: Devices->Audio
      var Audio = [String : Any]()
      if s.labels.contains("HDAInjection") && NSNumber(value: s.HDAInjection).boolValue {
        Audio["#Inject"] = s.value(for: "HDALayoutId", type: .INTEGER)
      } else {
        Audio["#Inject"] = s.value(for: "HDAInjection", type: .BOOLEAN)
      }
      Audio["#ResetHDA"] = s.value(for: "ResetHDA", type: .BOOLEAN)
      
      if Audio.keys.count > 0 {
        Devices["Audio"] = Audio
      }
      
      // MARK: Devices->USB
      var USB = [String : Any]()
      USB["Inject"] =       s.value(for: "USBInjection", type: .BOOLEAN)
      USB["FixOwnership"] = s.value(for: "USBFixOwnership", type: .BOOLEAN)
      USB["AddClockID"] =   s.value(for: "InjectClockID", type: .BOOLEAN)
      USB["HighCurrent"] =  s.value(for: "HighCurrent", type: .BOOLEAN)
      
      if USB.keys.count > 0 {
        Devices["USB"] = USB
      }
      
      self.config["Devices"] = Devices
      
      // MARK: Graphics
      var Graphics = [String : Any]()
      Graphics["LoadVBios"] =   s.value(for: "LoadVBios", type: .BOOLEAN)
      Graphics["PatchVBios"] =  s.value(for: "PatchVBios", type: .BOOLEAN)
      Graphics["VideoPorts"] =  s.value(for: "VideoPorts", type: .INTEGER)
      Graphics["VRAM"] =        s.value(for: "VRAM", type: .INTEGER)
      Graphics["DualLink"] =    s.value(for: "DualLink", type: .INTEGER)
      
      // MARK: Graphics (ATI specific)
      Graphics["FBName"] =        s.value(for: "FBName", type: .CHAR16String)
      Graphics["RadeonDeInit"] =  s.value(for: "DeInit", type: .BOOLEAN)
      
      // MARK: Graphics (NVIDIA specific)
      Graphics["display-cfg"] =   s.value(for: "Dcfg", type: .DataString)
      Graphics["NVCAP"] =         s.value(for: "NVCAP", type: .DataString)
      Graphics["NvidiaGeneric"] = s.value(for: "NvidiaGeneric", type: .BOOLEAN)
      Graphics["NvidiaNoEFI"] =   s.value(for: "NvidiaNoEFI", type: .BOOLEAN)
      Graphics["NvidiaSingle"] =  s.value(for: "NvidiaSingle", type: .BOOLEAN)
      
      // MARK: Graphics (NVIDIA Intel)
      Graphics["ig-platform-id"] =          s.value(for: "IgPlatform", type: .HexString)
      Graphics["#PatchVBiosBytes Count"] =  s.value(for: "PatchVBiosBytesCount", type: .INTEGER)
      
      // MARK: Graphics->Inject
      var Inject = [String : Any]()
      Inject["ATI"] =     s.value(for: "InjectATI", type: .BOOLEAN)
      Inject["NVidia"] =  s.value(for: "InjectNVidia", type: .BOOLEAN)
      Inject["Intel"] =   s.value(for: "InjectIntel", type: .BOOLEAN)
      
      if Inject.keys.count > 0 {
        Graphics["Inject"] = Inject
      }
      
      // MARK: Graphics->PatchVBiosBytes
      var PatchVBiosBytes = [Any]()
      Graphics["#PatchVBiosBytes Count"] = s.value(for: "PatchVBiosBytesCount", type: .INTEGER)
      var PatchVBiosBytesDict1 = [String : Any]()
      PatchVBiosBytesDict1["#Find"] = "_NOT_SHOWN_"
      PatchVBiosBytesDict1["#Replace"] = "_NOT_SHOWN_"
      PatchVBiosBytes.append(PatchVBiosBytesDict1)
      Graphics["#PatchVBiosBytes"] = PatchVBiosBytes
      
      // MARK: Graphics->EDID
      if AppSD.CloverRevision >= 3737 {
        var EDID = [String : Any]()
        if AppSD.CloverRevision >= 4058 {
          EDID["Inject"] =      s.value(for: "InjectEDID", type: .INTEGER)
          EDID["#VendorID"] =   s.value(for: "VendorEDID", type: .HexString)
          EDID["#ProductID"] =  s.value(for: "ProductEDID", type: .HexString)
          EDID["#Custom"] = "_NOT_SHOWN_"
          
        } else {
          EDID["InjectEDID"] = s.value(for: "InjectEDID", type: .BOOLEAN)
          EDID["#CustomEDID"] = "_NOT_SHOWN_"
        }
        
        if EDID.keys.count > 0 {
          Graphics["EDID"] = EDID
        }
      }
      if Graphics.keys.count > 0 {
        self.config["Graphics"] = Graphics
      }
      
      // MARK: GUI
      var GUI = [String: Any]()
      GUI["#Language"]  = s.value(for: "Language", type: .CHAR8String)
      GUI["#Theme"]  = "embedded"
      GUI["TextOnly"]  = false
      GUI["CustomIcons"]  = false
      // MARK: GUI->Mouse
      var Mouse = [String: Any]()
      Mouse["Enabled"] =  s.value(for: "PointerEnabled", type: .BOOLEAN)
      Mouse["Speed"] =    s.value(for: "PointerSpeed", type: .INTEGER)
      Mouse["Mirror"] =   s.value(for: "PointerMirror", type: .BOOLEAN)
      if Mouse.keys.count > 0 {
        GUI["Mouse"] = Mouse
      }
      // MARK: GUI->Hide
      var Hide = [String]()
      Hide.append("VolumeName_NOT_SHOWN")
      Hide.append("VolumeUUID_NOT_SHOWN")
      Hide.append("EntryPath_NOT_SHOWN")
      GUI["Hide"] = Hide
      // MARK: GUI->Scan
      var Scan = [String: Any]()
      Scan["Comment"] = "These values wrong, they present for sample"
      Scan["#Entries"] = true
      Scan["#Tool"] = true
      Scan["#Legacy"] = true
      GUI["Scan"] = Scan
      
      // MARK: GUI->Custom
      var Custom = [String: Any]()
      Custom["Comment"] = "These values wrong, they present for sample"
      // MARK: GUI->Custom->Entries
      var Entries = [Any]()
      // MARK: GUI->Custom->Entries->example dict
      var EntriesDict1 = [String: Any]()
      EntriesDict1["Comment"] = "These values wrong, they present for sample"
      EntriesDict1["#Volume"] = "VolumeUUID_NOT_SHOWN"
      EntriesDict1["#Path"] = "_NOT_SHOWN_"
      EntriesDict1["#Type"] = "_NOT_SHOWN_"
      EntriesDict1["#Arguments"] = "_NOT_SHOWN_"
      EntriesDict1["#AddArguments"] = "-v"
      EntriesDict1["#Title"] = "_NOT_SHOWN_"
      EntriesDict1["#FullTitle"] = "_NOT_SHOWN_"
      EntriesDict1["#Image"] = "_NOT_SHOWN_"
      EntriesDict1["#Hotkey"] = "_NOT_SHOWN_"
      EntriesDict1["#Disabled"] = true
      EntriesDict1["#InjectKexts"] = true
      // EntriesDict1["#NoCaches"] = false // how to boot without cache???
      EntriesDict1["#Hidden"] = true
      // MARK: GUI->Custom->Entries->example dict->SubEntries
      var SubEntries = [Any]()
      var SubEntriesDict1 = [String: Any]()
      SubEntriesDict1["#Title"] = "_NOT_SHOWN_"
      SubEntriesDict1["#AddArguments"] = "_NOT_SHOWN_"
      
      SubEntries.append(SubEntriesDict1)
      EntriesDict1["SubEntries"] = SubEntries
      Entries.append(EntriesDict1)
      Custom["Entries"] = Entries
      
      // MARK: GUI->Custom->Legacy
      var Legacy = [Any]()
      var LegacyDict1 = [String: Any]()
      LegacyDict1["#Volume"] = "VolumeUUID_NOT_SHOWN"
      LegacyDict1["#Type"] = "_NOT_SHOWN_"
      LegacyDict1["#Title"] = "_NOT_SHOWN_"
      LegacyDict1["#Hotkey"] = "_NOT_SHOWN_"
      LegacyDict1["#Disabled"] = true
      LegacyDict1["#Hidden"] = true
      Legacy.append(LegacyDict1)
      Custom["Legacy"] = Legacy
      
      // MARK: GUI->Custom->Tool
      var Tool = [Any]()
      var ToolDict1 = [String: Any]()
      ToolDict1["#Volume"] = "VolumeUUID_NOT_SHOWN"
      ToolDict1["#Path"] = "_NOT_SHOWN_"
      ToolDict1["#Type"] = "_NOT_SHOWN_"
      ToolDict1["#Title"] = "_NOT_SHOWN_"
      ToolDict1["#Arguments"] = "_NOT_SHOWN_"
      ToolDict1["#Hotkey"] = "_NOT_SHOWN_"
      ToolDict1["#Disabled"] = true
      ToolDict1["#Hidden"] = true
      Tool.append(ToolDict1)
      Custom["Tool"] = Tool
      
      GUI["Custom"] = Custom
      self.config["GUI"] = GUI
      
      // MARK: KernelAndKextPatches
      var KernelAndKextPatches = [String : Any]()
     
      KernelAndKextPatches["#Debug"] =                      s.kpValue(for: "KPDebug", type: .BOOLEAN)
      KernelAndKextPatches["KernelCpu"] =                   s.kpValue(for: "KPKernelCpu", type: .BOOLEAN)
      KernelAndKextPatches["KernelLapic"] =                 s.kpValue(for: "KPKernelLapic", type: .BOOLEAN)
      if AppSD.CloverRevision >= 4250 {
        KernelAndKextPatches["KernelXCPM"] =                s.kpValue(for: "KPKernelXCPM", type: .BOOLEAN)
      } else {
        KernelAndKextPatches["KernelIvyXCPM﻿"] =             s.kpValue(for: "KernelIvyXCPM﻿", type: .BOOLEAN)
      }
      KernelAndKextPatches["KernelPm"] =                    s.kpValue(for: "KPKernelPm", type: .BOOLEAN)
      if AppSD.CloverRevision >= 4152 {
        KernelAndKextPatches["AppleIntelCPUPM"] =           s.kpValue(for: "KPAppleIntelCPUPM", type: .BOOLEAN)
      } else {
        KernelAndKextPatches["AsusAICPUPM"] =               s.kpValue(for: "KPAsusAICPUPM", type: .BOOLEAN)
      }
      
      KernelAndKextPatches["AppleRTC"] =                    s.kpValue(for: "KPAppleRTC", type: .BOOLEAN)
      KernelAndKextPatches["DellSMBIOSPatch"] =             s.kpValue(for: "KPDELLSMBIOS", type: .BOOLEAN)
      KernelAndKextPatches["#Number of KextsToPatch"] =     s.kpValue(for: "NrKexts", type: .INTEGER)
      KernelAndKextPatches["#Number of Patchs To Kernel"] = s.kpValue(for: "NrKernels", type: .INTEGER)
      KernelAndKextPatches["#FakeCPUID"] =                  s.kpValue(for: "FakeCPUID", type: .HexString)
      
      // MARK: KernelAndKextPatches->KextsToPatch
      var KextsToPatch = [Any]()
      var KextsToPatchDict1 = [String : Any]()
      KextsToPatchDict1["Comment"] = "this is a sample"
      KextsToPatchDict1["#Name"] = "AppleUSBXHCIPCI"
      KextsToPatchDict1["#Find"] = "_NOT_SHOWN_"
      KextsToPatchDict1["#Replace"] = "_NOT_SHOWN_"
      
      if AppSD.CloverRevision >= 3327 {
        KextsToPatchDict1["#Disabled"] = true
      }
      if AppSD.CloverRevision >= 3580 {
        KextsToPatchDict1["#MatchOS"] = "10.11.6,10.12.x"
      }
      if AppSD.CloverRevision >= 3920 {
        KextsToPatchDict1["#MatchBuild"] = "16D1111"
      }
      KextsToPatch.append(KextsToPatchDict1)
      KernelAndKextPatches["#KextsToPatch"] = KextsToPatch
      
      self.config["KernelAndKextPatches"] = KernelAndKextPatches
      
      // MARK: RtVariables
      if AppSD.CloverRevision >= 3250 {
        var RtVariables = [String : Any]()
        RtVariables["#ROM"] = "UseMacAddr0"
        RtVariables["#MLB"] =             s.value(for: "BoardSerialNumber", type: .CHAR8String)
        RtVariables["CsrActiveConfig"] =  s.value(for: "CsrActiveConfig", type: .HexString)
        RtVariables["BooterConfig"] =     s.value(for: "BooterConfig", type: .HexString)
        
        self.config["RtVariables"] = RtVariables
      }
      
      // MARK: SMBIOS
      var SMBIOS = [String: Any]()
      // SMBIOS TYPE0
      SMBIOS["BiosVendor"] =          s.value(for: "VendorName", type: .CHAR8String)
      SMBIOS["BiosVersion"] =         s.value(for: "RomVersion", type: .CHAR8String)
      SMBIOS["BiosReleaseDate"] =     s.value(for: "ReleaseDate", type: .CHAR8String)
      // SMBIOS TYPE1
      SMBIOS["Manufacturer"] =        s.value(for: "ManufactureName", type: .CHAR8String)
      SMBIOS["ProductName"] =         s.value(for: "ProductName", type: .CHAR8String)
      SMBIOS["Version"] =             s.value(for: "VersionNr", type: .CHAR8String)
      SMBIOS["SerialNumber"] =        s.value(for: "SerialNr", type: .CHAR8String)
      SMBIOS["SmUUID"] =              s.value(for: "SmUUID", type: .UUIDString)
      SMBIOS["Family"] =              s.value(for: "FamilyName", type: .CHAR8String)
      // SMBIOS TYPE2
      SMBIOS["BoardManufacturer"] =   s.value(for: "BoardManufactureName", type: .CHAR8String)
      SMBIOS["BoardSerialNumber"] =   s.value(for: "BoardSerialNumber", type: .CHAR8String)
      SMBIOS["Board-ID"] =            s.value(for: "BoardNumber", type: .CHAR8String)
      SMBIOS["BoardVersion"] =        s.value(for: "BoardVersion", type: .CHAR8String)
      SMBIOS["BoardType"] =           s.value(for: "BoardType", type: .INTEGER)
      SMBIOS["LocationInChassis"] =   s.value(for: "LocationInChassis", type: .CHAR8String)
      SMBIOS["ChassisManufacturer"] = s.value(for: "ChassisManufacturer", type: .CHAR8String)
      SMBIOS["ChassisAssetTag"] =     s.value(for: "ChassisAssetTag", type: .CHAR8String)
      SMBIOS["ChassisType"] =         s.value(for: "ChassisType", type: .HexString)
      SMBIOS["Mobile"] =              s.value(for: "Mobile", type: .BOOLEAN)
      // SMBIOS TYPE17
      SMBIOS["Trust"] =               s.value(for: "TrustSMBIOS", type: .BOOLEAN)
      SMBIOS["OEMProduct"] =          s.value(for: "OEMProduct", type: .CHAR8String)
      SMBIOS["OEMVendor"] =           s.value(for: "OEMVendor", type: .CHAR8String)
      SMBIOS["OEMBoard"] =            s.value(for: "OEMBoard", type: .CHAR8String)
      
      if AppSD.CloverRevision >= 3368 {
        if (s.PlatformFeature != 0xFFFF) {
          SMBIOS["PlatformFeature"] = s.value(for: "PlatformFeature", type: .HexString)
        }
      }
      
      if s.labels.contains("InjectMemoryTables") && NSNumber(value: s.InjectMemoryTables).boolValue {
        // MARK: SMBIOS->Memory
        var Memory = [String: Any]()
        Memory["Comment"] = "there are no real data here"
        Memory["#SlotCount"] = 0
        Memory["#Channels"] = 0
        // MARK: SMBIOS->Memory->Modules
        var Modules = [Any]()
        var ModulesDict1 = [String: Any]()
        ModulesDict1["#Slot"] = 0
        ModulesDict1["#Size"] = 0
        ModulesDict1["#Vendor"] =     s.value(for: "MemoryManufacturer", type: .CHAR8String)
        ModulesDict1["#Serial"] =     s.value(for: "MemorySerialNumber", type: .CHAR8String)
        ModulesDict1["#Part"] =       s.value(for: "MemoryPartNumber", type: .CHAR8String)
        ModulesDict1["#Frequency"] =  s.value(for: "MemorySpeed", type: .CHAR8String)
        ModulesDict1["#Type"] = "DDRx"
        Modules.append(ModulesDict1)
        Memory["Modules"] = Modules
        SMBIOS["Memory"] = Memory
      }
      // MARK: SMBIOS->Slots
      var Slots = [Any]()
      var SlotsDict1 = [String: Any]()
      SlotsDict1["Comment"] = "there is a sample"
      SlotsDict1["Device"] = "WIFI"
      SlotsDict1["ID"] = 5
      SlotsDict1["Type"] = 1
      SlotsDict1["Name"] = "Airport"
      Slots.append(SlotsDict1)
      SMBIOS["Slots"] = Slots
      
      self.config["SMBIOS"] = SMBIOS
      
      
      // MARK: SystemParameters
      var SystemParameters = [String: Any]()
      SystemParameters["CustomUUID"] =      s.value(for: "CustomUuid", type: .CHAR16String)
      SystemParameters["InjectSystemID"] =  s.value(for: "InjectSystemID", type: .BOOLEAN)
      SystemParameters["BacklightLevel"] =  s.value(for: "BacklightLevel", type: .HexString)
      SystemParameters["NvidiaWeb"] =       s.value(for: "NvidiaWeb", type: .BOOLEAN)
      SystemParameters["#InjectKexts"] = "Detect"
      self.config["SystemParameters"] = SystemParameters
      
      return self.config
    }
    
    return nil
  }
}



