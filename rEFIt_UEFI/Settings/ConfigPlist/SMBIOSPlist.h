/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _SMBIOSPLISTCLASS_H_
#define _SMBIOSPLISTCLASS_H_

#include "../../include/VolumeTypes.h"
#include "../../include/OSTypes.h"
#include "../../include/OSFlags.h"
#include "../../include/BootTypes.h"
#include "../../include/Languages.h"
#include "../../include/Devices.h"
#include "../../include/QuirksCodes.h"
#include "../../include/TagTypes.h"
#include "../../include/Pci.h"
#include "../../entry_scan/loader.h" // for KERNEL_SCAN_xxx constants
#include <IndustryStandard/SmBios.h> // for Smbios memory type
#include "../../Platform/guid.h"
#include "../../include/Guid++.h"
#include "../../Platform/platformdata.h"
#include "../../Platform/smbios.h"
#include "../../Platform/VersionString.h" // for AsciiStrVersionToUint64
//#include "../cpu.h"

#include "../../cpp_lib/undefinable.h"
#include "../../cpp_lib/XmlLiteSimpleTypes.h"
#include "../../cpp_lib/XmlLiteCompositeTypes.h"
#include "../../cpp_lib/XmlLiteDictTypes.h"
#include "../../cpp_lib/XmlLiteArrayTypes.h"
#include "../../cpp_lib/XmlLiteUnionTypes.h"
#include "../../cpp_lib/XmlLiteParser.h"

#include "../../cpp_foundation/XString.h"
#include "../../cpp_foundation/XStringArray.h"
#include "../../cpp_foundation/XArray.h"
#include "../../cpp_foundation/XObjArray.h"
#include "../../Platform/Utils.h"

#include "ConfigPlistAbstract.h"


class SmbiosPlistClass : public ConfigPlistAbstractClass
{
  using super = ConfigPlistAbstractClass;
public:
//  #include "Config_SMBIOS.h"


  class SmbiosDictClass : public XmlDict
  {
    using super = XmlDict;
    public:


      class MemoryDictClass : public XmlDict
      {
        using super = XmlDict;
        public:
              
          class ModuleDictClass : public XmlDict
          {
            using super = XmlDict;
          public:
            static ModuleDictClass NullValue;
            
            class SlotIndexClass : public XmlUInt8
            {
              using super = XmlUInt8;
              virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
                bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
                if ( value() >= MAX_RAM_SLOTS ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("Slot cannot >= MAX_RAM_SLOTS. It must a number between 0 and %d at '%s:%d'", MAX_RAM_SLOTS-1, xmlPath.c_str(), keyPos.getLine()));
                return b;
              }
            };
            class TypeClass: public XmlString8AllowEmpty {
                using super = XmlString8AllowEmpty;
                virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
                  bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
                  if ( xstring8.isEqualIC("DDR") ) return b;
                  if ( xstring8.isEqualIC("DDR2") ) return b;
                  if ( xstring8.isEqualIC("DDR3") ) return b;
                  if ( xstring8.isEqualIC("DDR4") ) return b;
                  b = xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be \"DDR\", \"DDR2\", \"DDR3\" or \"DDR4\" in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                  return b;
                }
              public:
            };

            SlotIndexClass SlotIndex = SlotIndexClass();
            XmlUInt32 Size = XmlUInt32();
            XmlUInt32 Frequency = XmlUInt32();
            XmlString8AllowEmpty Vendor = XmlString8AllowEmpty();
            XmlString8AllowEmpty Part = XmlString8AllowEmpty();
            XmlString8AllowEmpty Serial = XmlString8AllowEmpty();
            TypeClass Type = TypeClass();

            XmlDictField m_fields[7] = {
              {"Slot", SlotIndex},
              {"Size", Size},
              {"Frequency", Frequency},
              {"Vendor", Vendor},
              {"Part", Part},
              {"Serial", Serial},
              {"Type", Type},
            };

            virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
            
            virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
              XBool b = true;
              if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) b = false;
              if ( !SlotIndex.isDefined() ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("Slot must be defined as a number between 0 and %d at '%s:%d'", MAX_RAM_SLOTS-1, xmlPath.c_str(), keyPos.getLine()));
              if ( !Size.isDefined() ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("Size must be defined at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
              return b;
            }

            // Currently a UInt8, so value is 0..255. Careful if you change for a bigger uint type !
            const decltype(SlotIndex)::ValueType& dgetSlotIndex() const { return SlotIndex.isDefined() ? SlotIndex.value() : SlotIndex.nullValue; };
            const decltype(Size)::ValueType& dgetModuleSize() const { return Size.isDefined() ? Size.value() : Size.nullValue; };
            const decltype(Frequency)::ValueType& dgetFrequency() const { return Frequency.isDefined() ? Frequency.value() : Frequency.nullValue; };
            const decltype(Vendor)::ValueType& dgetVendor() const { return Vendor.isDefined() ? Vendor.value() : Vendor.nullValue; };
            const decltype(Part)::ValueType& dgetPartNo() const { return Part.isDefined() ? Part.value() : Part.nullValue; };
            const decltype(Serial)::ValueType& dgetSerialNo() const { return Serial.isDefined() ? Serial.value() : Serial.nullValue; };
            UINT8 dgetType() const {
//              if ( !Type.isDefined() ) return dgetModuleSize() > 0 ? MemoryTypeDdr3: 0;
              if ( !Type.isDefined() ) return MemoryTypeDdr3;
              if (Type.value().isEqualIC("DDR2")) {
               return MemoryTypeDdr2;
              } else if (Type.value().isEqualIC("DDR3")) {
               return MemoryTypeDdr3;
              } else if (Type.value().isEqualIC("DDR4")) {
               return MemoryTypeDdr4;
              } else if (Type.value().isEqualIC("DDR")) {
               return MemoryTypeDdr;
              }
              // Cannot happen if validate has been done properly.
              panic("invalid value");
            }
//            XBool dgetInUse() const { return Size.isDefined() ? Size.value() > 0 : false; };

          };
          
          class ModuleArrayClass : public XmlArray<ModuleDictClass>
          {
              using super = XmlArray<ModuleDictClass>;
            public:

              virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
                bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
                if ( size() > UINT8_MAX ) {
                  xmlLiteParser->addWarning(generateErrors, S8Printf("You cannot declare more then 256 memory modules in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                  while ( size() > 256 ) RemoveAtIndex(size()-1);
                }

                for ( size_t i = 0 ; i < size() ; i++ ) {
                  for ( size_t j = i+1 ; j < size() ; ) {
                    if ( ElementAt(i).SlotIndex.value() >= ElementAt(j).SlotIndex.value() ) {
                      xmlLiteParser->addWarning(generateErrors, S8Printf("Ignore duplicate memory module for slot %d at '%s:%d'", ElementAt(i).SlotIndex.value(), xmlPath.c_str(), keyPos.getLine()));
                      RemoveAtIndex(j);
                    }else{
                      j++;
                    }
                  }
                }

                return b;
              }

              decltype(ModuleDictClass::SlotIndex)::ValueType dgetCalculatedSlotCount() const {
                if ( !isDefined() ) return 0;
                uint8_t max = 0;
                for ( size_t idx = 0 ; idx < size() ; ++idx ) {
                  if ( ElementAt(idx).dgetModuleSize() > 0 ) {
                    if ( ElementAt(idx).dgetSlotIndex() > max ) max = ElementAt(idx).dgetSlotIndex();
                  }
                }
                return max+1;
              };

              const ModuleDictClass& dgetSoltAtIndex(size_t slotIndex) const {
                for ( size_t idx = 0; idx < size(); ++idx ) {
                  if ( ElementAt(idx).dgetSlotIndex() == slotIndex )
                   return ElementAt(idx);
                }
                return ModuleDictClass::NullValue;
              }

          };
      
      
      
        XmlUInt8 SlotCount = XmlUInt8();
        XmlUInt8 Channels = XmlUInt8();
        ModuleArrayClass Modules = ModuleArrayClass();

        XmlDictField m_fields[3] = {
          {"SlotCount", SlotCount},
          {"Channels", Channels},
          {"Modules", Modules},
        };

        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
        
        virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
          bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
          if ( Modules.size() == 0 ) {
            // whatever if SlotCount is defined or not, and whatever value, it's ok.
            return b;
          }
          if ( !SlotCount.isDefined() ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("SlotCount is not defined in SMBIOS, but you defined memory modules. SlotCount adjusted to %d, which maybe wrong.", Modules.dgetCalculatedSlotCount())); // do not set b to false because value is auto-corrected, so it's now ok.
            SlotCount.setUInt8Value(Modules.dgetCalculatedSlotCount());
          }else{
            for ( size_t i = 0 ; i < Modules.size() ; ) {
              if ( Modules[i].SlotIndex.value() >= SlotCount.value() ) {
                xmlLiteParser->addWarning(generateErrors, S8Printf("Ignored memory module with slot >= SlotCount at '%s:%d'", xmlPath.c_str(), keyPos.getLine())); // do not set b to false because value is auto-corrected, so it's now ok.
                Modules.RemoveAtIndex(i);
              }else{
                i++;
              }
            }
            if ( SlotCount.value() < Modules.dgetCalculatedSlotCount() ) {
              log_technical_bug("SlotCount.value() < Modules.dgetCalculatedSlotCount()");
              SlotCount.setUInt8Value(Modules.dgetCalculatedSlotCount());
            }
          }
          return b;
        }

        decltype(SlotCount)::ValueType dgetSlotCount() const { return SlotCount.isDefined() ? SlotCount.value() : 0; };
        const decltype(Channels)::ValueType& dgetUserChannels() const { return Channels.isDefined() ? Channels.value() : Channels.nullValue; };
        
//        decltype(SlotCount)::ValueType dgetSlotMax() const {
//          if ( !isDefined() || !Modules.isDefined() || Modules.size() == 0 ) return 0;
//          uint8_t max = 0;
//          for ( size_t idx = 0 ; idx < Modules.size() ; ++idx ) {
//            if ( Modules[idx].dgetModuleSize() > 0 ) {
//              if ( Modules[idx].dgetSlotIndex() > UINT8_MAX ) {
//                log_technical_bug("Modules[idx].dgetSlotNo() > UINT8_MAX");
//              }else{
//                if ( Modules[idx].dgetSlotIndex() > max ) max = (uint8_t)Modules[idx].dgetSlotIndex(); // safe cast Modules[idx].dgetSlotNo() is <= UINT8_MAX
//              }
//            }
//          }
//          return max+1;
//        };
//
//        decltype(SlotCount)::ValueType dgetSlotCounts() const { return dgetSlotCountSetting() > dgetSlotMax() ? dgetSlotCountSetting() : dgetSlotMax(); };

      };

      class SlotDeviceDictClass : public XmlDict
      {
        using super = XmlDict;
      public:
        static SlotDeviceDictClass NullValue;
        
        class DeviceClass: public XmlString8AllowEmpty {
          using super = XmlString8AllowEmpty;
          virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
            bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
            if ( xstring8.isEqualIC("ATI") ) return b;
            if ( xstring8.isEqualIC("NVidia") ) return b;
            if ( xstring8.isEqualIC("IntelGFX") ) return b;
            if ( xstring8.isEqualIC("LAN") ) return b;
            if ( xstring8.isEqualIC("WIFI") ) return b;
            if ( xstring8.isEqualIC("Firewire") ) return b;
            if ( xstring8.isEqualIC("HDMI") ) return b;
            if ( xstring8.isEqualIC("USB") ) return b;
            if ( xstring8.isEqualIC("NVME") ) return b;
            b = xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be \"ATI\", \"NVidia\", \"IntelGFX\", \"LAN\", \"WIFI\", \"Firewire\", \"HDMI\", \"USB\" or \"NVME\" in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
            return b;
          }
        } Device = DeviceClass();
        XmlUInt8 ID = XmlUInt8();
        XmlUInt8 Type = XmlUInt8();
        XmlString8AllowEmpty Name = XmlString8AllowEmpty();

        XmlDictField m_fields[4] = {
          {"Device", Device},
          {"ID", ID},
          {"Type", Type},
          {"Name", Name},
        };

        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
        
        virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
          bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
          return b;
        }
        const decltype(Device)::ValueType& dgetDevice() const { return Device.isDefined() ? Device.value() : Device.nullValue; };
        uint8_t dgetDeviceN() const {
  //        if ( !Device.isDefined() ) panic("%s: invalid value. Check validate method.", __PRETTY_FUNCTION__);
          if ( !Device.isDefined() ) return 0;
          if (Device.value().isEqualIC("ATI")) {
            return 0;
          } else if (Device.value().isEqualIC("NVidia")) {
            return 1;
          } else if (Device.value().isEqualIC("IntelGFX")) {
            return 2;
          } else if (Device.value().isEqualIC("LAN")) {
            return 5;
          } else if (Device.value().isEqualIC("WIFI")) {
            return 6;
          } else if (Device.value().isEqualIC("Firewire")) {
            return 12;
          } else if (Device.value().isEqualIC("HDMI")) {
            return 4;
          } else if (Device.value().isEqualIC("USB")) {
            return 11;
          } else if (Device.value().isEqualIC("NVME")) {
            return 13;
          } else {
            panic("%s: invalid value. Check validate method.", __PRETTY_FUNCTION__);
          }
        }

        decltype(ID)::ValueType dgetSlotID() const { return ID.isDefined() ? ID.value() : dgetDeviceN(); };
        MISC_SLOT_TYPE dgetSlotType() const {
          if ( !Type.isDefined() ) return Device.isDefined() ? SlotTypePci : MISC_SLOT_TYPE(0);
          switch ( Type.value() ) {
            case 0:
              return SlotTypePci;
            case 1:
              return SlotTypePciExpressX1;
            case 2:
              return SlotTypePciExpressX2;
            case 4:
              return SlotTypePciExpressX4;
            case 8:
              return SlotTypePciExpressX8;
            case 16:
              return SlotTypePciExpressX16;
            default:
              return SlotTypePciExpress;
            }
          }

        decltype(Name)::ValueType dgetSlotName() const { return Name.isDefined() ? Name.value() : Device.isDefined() ? S8Printf("PCI Slot %hhd", dgetDeviceN()) : NullXString8; };

      };
      
      
      class SlotDeviceArrayClass : public XmlArray<SlotDeviceDictClass>
      {
        public:
          const SlotDeviceDictClass& dgetSoltDevice(INTN slotNo) const {
            for ( size_t idx = 0; idx < size(); ++idx ) {
              if ( ElementAt(idx).dgetDeviceN() == slotNo )
               return ElementAt(idx);
            }
            return SlotDeviceDictClass::NullValue;
          }
      };

      class SmUUIDClass : public XmlString8AllowEmpty
      {
          using super = XmlString8AllowEmpty;
        protected:
          virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
            bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
            if ( !EFI_GUID::IsValidGuidString(xstring8) ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("Invalid SmUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
            return b;
          }
      };

      class ProductNameClass : public XmlString8Trimed
      {
          using super = XmlString8Trimed;
        public:
          ProductNameClass() : super(true) {};
          virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
            bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
            MacModel Model;
            Model = GetModelFromString(xstring8);
            if ( Model == MaxMacModel ) b = xmlLiteParser->addWarning(generateErrors, S8Printf("Invalid ProductName '%s' in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
            return b;
          }
      };

      class BiosVersionClass : public XmlString8Trimed
      {
          using super = XmlString8Trimed;
        public:
          BiosVersionClass() : super(true) {};
          virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
            bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
            if ( !value().contains(".") ) {
              b = xmlLiteParser->addWarning(generateErrors, S8Printf("BiosVersion '%s' doesn't contains a dot in dict '%s:%d'.", value().c_str(), xmlPath.c_str(), keyPos.getLine()));
            }else{
              size_t rindex = value().rindexOf(".");
              if ( value().length() - rindex < 7 ) {
                b = xmlLiteParser->addWarning(generateErrors, S8Printf("Last part of BiosVersion '%s' must be at least 6 chars in dict '%s:%d'.", value().c_str(), xmlPath.c_str(), keyPos.getLine()));
              }
              // Should we check the format of these 6 last chars ?
            }
            return b;
          }
      };

      class BiosReleaseDateClass : public XmlString8Trimed
      {
          using super = XmlString8Trimed;
        public:
          BiosReleaseDateClass() : super(true) {};
          virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
            bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
            if ( value().length() != 8  &&  value().length() != 10 ) {
              b = xmlLiteParser->addWarning(generateErrors, S8Printf("BiosReleaseDate '%s' must 8 or 10 chars in dict '%s:%d'.", value().c_str(), xmlPath.c_str(), keyPos.getLine()));
            }
            // Should we check the format of these 8 or 10 last chars ?
            return b;
          }
      };

  protected:
    XmlString8Trimed BiosVendor = XmlString8Trimed(true); // true = allow empty
    BiosVersionClass BiosVersion = BiosVersionClass(); // RomVersion
    XmlString8AllowEmpty EfiVersion = XmlString8AllowEmpty();
    BiosReleaseDateClass BiosReleaseDate = BiosReleaseDateClass();

    XmlString8Trimed Manufacturer = XmlString8Trimed(true);
    ProductNameClass ProductName = ProductNameClass();
    XmlString8Trimed Version = XmlString8Trimed(true);
    XmlString8Trimed SerialNumber = XmlString8Trimed(true);
    SmUUIDClass SmUUID = SmUUIDClass();
    XmlString8Trimed Family = XmlString8Trimed(true);

    XmlString8Trimed BoardManufacturer = XmlString8Trimed();
    XmlString8Trimed BoardSerialNumber = XmlString8Trimed();
    XmlString8Trimed BoardID = XmlString8Trimed(true);
    XmlString8Trimed LocationInChassis = XmlString8Trimed(true);
    XmlString8Trimed BoardVersion = XmlString8Trimed(true);
    XmlUInt8 BoardType = XmlUInt8();

    XmlBool Mobile = XmlBool();
    XmlUInt8 ChassisType = XmlUInt8();
    XmlString8Trimed ChassisManufacturer = XmlString8Trimed(true);
    XmlString8Trimed ChassisAssetTag = XmlString8Trimed(true);

    XmlUInt16 SmbiosVersion = XmlUInt16();
    XmlInt8 MemoryRank = XmlInt8(); // Attribute

    XmlBool Trust = XmlBool();
    XmlUInt64 PlatformFeature = XmlUInt64();
    XmlBool NoRomInfo = XmlBool();

    XmlUInt32 FirmwareFeatures = XmlUInt32(); // gFwFeatures
    XmlUInt32 FirmwareFeaturesMask = XmlUInt32();
    XmlUInt64 ExtendedFirmwareFeatures = XmlUInt64(); // gFwFeatures
    XmlUInt64 ExtendedFirmwareFeaturesMask = XmlUInt64();
  public:
    MacModel defaultMacModel = MaxMacModel;

    MemoryDictClass Memory = MemoryDictClass();
    SlotDeviceArrayClass Slots = SlotDeviceArrayClass();

    XmlDictField m_fields[31] = {  //31
      {"Trust", Trust},
      {"MemoryRank", MemoryRank},
      {"Memory", Memory},
      {"Slots", Slots},
      {"ProductName", ProductName},
      {"SmbiosVersion", SmbiosVersion},
      {"BiosVersion", BiosVersion},
      {"BiosReleaseDate", BiosReleaseDate},
      {"EfiVersion", EfiVersion},
      {"FirmwareFeatures", FirmwareFeatures},
      {"FirmwareFeaturesMask", FirmwareFeaturesMask},
      {"ExtendedFirmwareFeatures", ExtendedFirmwareFeatures},
      {"ExtendedFirmwareFeaturesMask", ExtendedFirmwareFeaturesMask},
      {"PlatformFeature", PlatformFeature},
      {"BiosVendor", BiosVendor},
      {"Manufacturer", Manufacturer},
      {"Version", Version},
      {"Family", Family},
      {"SerialNumber", SerialNumber},
      {"SmUUID", SmUUID},
      {"BoardManufacturer", BoardManufacturer},
      {"BoardSerialNumber", BoardSerialNumber},
      {"Board-ID", BoardID},
      {"BoardVersion", BoardVersion},
      {"BoardType", BoardType},
      {"Mobile", Mobile},
      {"LocationInChassis", LocationInChassis},
      {"ChassisManufacturer", ChassisManufacturer},
      {"ChassisAssetTag", ChassisAssetTag},
      {"ChassisType", ChassisType},
      {"NoRomInfo", NoRomInfo},
    };

    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    
    
    virtual XBool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, XBool generateErrors) override {
      bool b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
      if ( !ProductName.isDefined() ) {
//        return xmlLiteParser->addWarning(generateErrors, S8Printf("ProductName is not defined, the whole SMBIOS dict is ignored at line %d.", keyPos.getLine()));
//        if ( defaultMacModel < MaxMacModel ) {
//          ProductName.setStringValue(MachineModelName[defaultMacModel]);
//        }
      }
      if ( dgetModel() < MaxMacModel ) {
        if ( BiosVersion.isDefined() ) {
          if ( !is2ndBiosVersionGreaterThan1st(ApplePlatformDataArray[dgetModel()].firmwareVersion, BiosVersion.value()) ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("BiosVersion '%s' is before than default ('%s') -> ignored. Dict '%s:%d'.", BiosVersion.value().c_str(), ApplePlatformDataArray[dgetModel()].firmwareVersion.c_str(), xmlPath.c_str(), keyPos.getLine())); // Do not set b to false : we don't want to invalidate the whole dict
            xmlLiteParser->productNameNeeded = !getProductName().isDefined();
            BiosVersion.reset();
          }else
          if ( is2ndBiosVersionEqual(ApplePlatformDataArray[dgetModel()].firmwareVersion, BiosVersion.value()) ) {
            xmlLiteParser->addInfo(generateErrors, S8Printf("BiosVersion '%s' is the same as default. Dict '%s:%d'.", BiosVersion.value().c_str(), xmlPath.c_str(), keyPos.getLine())); // Do not set b to false : we don't want to invalidate the whole dict
            xmlLiteParser->productNameNeeded = !getProductName().isDefined();
            BiosVersion.reset();
          }
        }
        if ( BiosReleaseDate.isDefined() ) {
          int compareReleaseDateResult = compareReleaseDate(GetReleaseDate(dgetModel()), BiosReleaseDate.value());
          if ( compareReleaseDateResult == 1 ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("BiosReleaseDate '%s' is older than default ('%s') -> ignored. Dict '%s:%d'.", BiosReleaseDate.value().c_str(), GetReleaseDate(dgetModel()).c_str(), xmlPath.c_str(), keyPos.getLine())); // Do not set b to false : we don't want to invalidate the whole dict
            xmlLiteParser->productNameNeeded = !getProductName().isDefined();
            BiosReleaseDate.reset();
          }else
          if ( compareReleaseDateResult == 0 ) {
            // This is just 'info'. It's useless but fine to define the same as default.
            xmlLiteParser->addInfo(generateErrors, S8Printf("BiosReleaseDate '%s' is the same as default. Dict '%s:%d'.", BiosReleaseDate.value().c_str(), xmlPath.c_str(), keyPos.getLine())); // Do not set b to false : we don't want to invalidate the whole dict
            xmlLiteParser->productNameNeeded = !getProductName().isDefined();
            BiosReleaseDate.reset();
          }
        }
        if ( EfiVersion.isDefined() ) {
          if ( AsciiStrVersionToUint64(ApplePlatformDataArray[dgetModel()].efiversion, 4, 5) > AsciiStrVersionToUint64(EfiVersion.value(), 4, 5)) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("EfiVersion '%s' is older than default ('%s') -> ignored. Dict '%s:%d'.", EfiVersion.value().c_str(), ApplePlatformDataArray[dgetModel()].efiversion.c_str(), xmlPath.c_str(), keyPos.getLine())); // Do not set b to false : we don't want to invalidate the whole dict
            xmlLiteParser->productNameNeeded = !getProductName().isDefined();
            EfiVersion.reset();
          } else if (AsciiStrVersionToUint64(ApplePlatformDataArray[dgetModel()].efiversion, 4, 5) == AsciiStrVersionToUint64(EfiVersion.value(), 4, 5)) {
            xmlLiteParser->addInfo(generateErrors, S8Printf("EfiVersion '%s' is the same as default. Dict '%s:%d'.", EfiVersion.value().c_str(), xmlPath.c_str(), keyPos.getLine())); // Do not set b to false : we don't want to invalidate the whole dict
            xmlLiteParser->productNameNeeded = !getProductName().isDefined();
            EfiVersion.reset();
          }
        }
      }else{
        // This is supposed to never happen within Clover, because Clover initialise defaultMacModel.
        xmlLiteParser->addInfo(generateErrors, S8Printf("Cannot check validity of BiosVersion, BiosReleaseDate and EfiVersion because ProductName is not set in dict '%s:%d'. Define ProductName or run this tool with --productname=[your product name].", xmlPath.c_str(), keyPos.getLine()));
      }
      return b;
    }



    const decltype(Trust)& getTrust() const { return Trust; }
    const decltype(MemoryRank)& getMemoryRank() const { return MemoryRank; }
    const decltype(Memory)& getMemory() const { return Memory; }
    const decltype(Slots)& getSlots() const { return Slots; }
    const decltype(ProductName)& getProductName() const { return ProductName; }
    const decltype(SmbiosVersion)& getSmbiosVersion() const { return SmbiosVersion; }
    const decltype(BiosVersion)& getBiosVersion() const { return BiosVersion; }
    const decltype(BiosReleaseDate)& getBiosReleaseDate() const { return BiosReleaseDate; }
    const decltype(EfiVersion)& getEfiVersion() const { return EfiVersion; }
    const decltype(FirmwareFeatures)& getFirmwareFeatures() const { return FirmwareFeatures; }
    const decltype(FirmwareFeaturesMask)& getFirmwareFeaturesMask() const { return FirmwareFeaturesMask; }
    const decltype(ExtendedFirmwareFeatures)& getExtendedFirmwareFeatures() const { return ExtendedFirmwareFeatures; }
    const decltype(ExtendedFirmwareFeaturesMask)& getExtendedFirmwareFeaturesMask() const { return ExtendedFirmwareFeaturesMask; }
    const decltype(PlatformFeature)& getPlatformFeature() const { return PlatformFeature; }
    const decltype(BiosVendor)& getBiosVendor() const { return BiosVendor; }
    const decltype(Manufacturer)& getManufacturer() const { return Manufacturer; }
    const decltype(Version)& getVersion() const { return Version; }
    const decltype(Family)& getFamily() const { return Family; }
    const decltype(SerialNumber)& getSerialNumber() const { return SerialNumber; }
    const decltype(SmUUID)& getSmUUID() const { return SmUUID; }
    const decltype(BoardManufacturer)& getBoardManufacturer() const { return BoardManufacturer; }
    const decltype(BoardSerialNumber)& getBoardSerialNumber() const { return BoardSerialNumber; }
    const decltype(BoardID)& getBoardID() const { return BoardID; }
    const decltype(BoardVersion)& getBoardVersion() const { return BoardVersion; }
    const decltype(BoardType)& getBoardType() const { return BoardType; }
    const decltype(Mobile)& getMobile() const { return Mobile; }
    const decltype(LocationInChassis)& getLocationInChassis() const { return LocationInChassis; }
    const decltype(ChassisManufacturer)& getChassisManufacturer() const { return ChassisManufacturer; }
    const decltype(ChassisAssetTag)& getChassisAssetTag() const { return ChassisAssetTag; }
    const decltype(ChassisType)& getChassisType() const { return ChassisType; }
    const decltype(NoRomInfo)& getNoRomInfo() const { return NoRomInfo; }


//    /*
//     * DO NOT call this if !ProductName.isDefined()
//     */
//    MacModel getModel() const
//    {
//      if ( !ProductName.isDefined() ) {
//        // This must not happen in Clover because Clover set a defaultMacModel
//        // This must not happen in ccpv because ccpv doesn't call dget... methods
//        log_technical_bug("%s : !ProductName.isDefined()", __PRETTY_FUNCTION__);
//        return iMac132; // cannot return GetDefaultModel() because we don't want to link runtime configuration to the xml reading layer.
//      }
//      return GetModelFromString(ProductName.value()); // ProductName has been validated, so Model CANNOT be MaxMacModel
//    }
//    XBool hasModel() const { return ProductName.isDefined(); }

    MacModel dgetModel() const
    {
      if ( ProductName.isDefined() ) return GetModelFromString(ProductName.value());
      if ( defaultMacModel < MaxMacModel ) return defaultMacModel;
      return MaxMacModel;
    }

    decltype(BiosVendor)::ValueType dgetBiosVendor() const {
      if ( BiosVendor.isDefined() ) return BiosVendor.value();
      return AppleBiosVendor;
    };

    decltype(BiosVersion)::ValueType dgetBiosVersion() const {
      if ( BiosVersion.isDefined() ) return BiosVersion.value();
      return ApplePlatformDataArray[dgetModel()].firmwareVersion;
    };

    decltype(EfiVersion)::ValueType dgetEfiVersion() const {
      if ( !EfiVersion.isDefined() ) return ApplePlatformDataArray[dgetModel()].efiversion;
      XString8 returnValue;
      returnValue = EfiVersion.value();
      returnValue.trim();
      return returnValue;
    }

    decltype(BiosReleaseDate)::ValueType dgetBiosReleaseDate() const
    {
      if ( BiosReleaseDate.isDefined() ) return BiosReleaseDate.value();
      XString8 dBiosVersion = dgetBiosVersion();
      XString8 returnValue;
      if ( dBiosVersion.notEmpty() ) {
        if ( !dBiosVersion.contains(".") ) {
          log_technical_bug("!dBiosVersion.contains(\".\")"); // Must not happen, BiosVersion has been validated to have a '.' followed by 6 chars.
          return NullXString8;
        }
        const char* i = dBiosVersion.c_str();
        i += AsciiStrLen(i);
        while ( *i != '.' ) i--;
        if ( strlen(i) < 7 ) {
          log_technical_bug("strlen(i) < 7"); // Must not happen, BiosVersion has been validated to have a '.' followed by 6 chars.
          return NullXString8;
        }
        if ( isReleaseDateWithYear20(dgetModel()) ) {
          returnValue.S8Printf("%c%c/%c%c/20%c%c", i[3], i[4], i[5], i[6], i[1], i[2]);
        }else{
          returnValue.S8Printf("%c%c/%c%c/%c%c", i[3], i[4], i[5], i[6], i[1], i[2]);
        }
      }
      return returnValue;
    }

    decltype(Manufacturer)::ValueType dgetManufactureName() const {
      if ( Manufacturer.isDefined() ) return Manufacturer.value();
      return AppleBiosVendor;
    };

    const decltype(ProductName)::ValueType& dgetProductName() const { return ProductName.isDefined() ? ProductName.value() : ProductName.nullValue; };

    decltype(Version)::ValueType dgetSystemVersion() const {
      if ( Version.isDefined() ) return Version.value();
      return ApplePlatformDataArray[dgetModel()].systemVersion;
    };

    const decltype(SerialNumber)::ValueType dgetSerialNr() const {
      if ( SerialNumber.isDefined() ) return SerialNumber.value();
      return ApplePlatformDataArray[dgetModel()].serialNumber;
    };

    EFI_GUID dgetSmUUID() const {
      if ( !SmUUID.isDefined() ) return nullGuid;
      EFI_GUID g;
      g.takeValueFromBE(SmUUID.value());
      if ( g.isNull() ) panic("SmUUID is not valid. This could not happen because SmUUID is checked to be valid. Did you comment out the validation ?");
      return g;
    }

    decltype(Family)::ValueType dgetFamilyName() const {
      if ( Family.isDefined() ) return Family.value();
      return ApplePlatformDataArray[dgetModel()].productFamily;
    };

    decltype(BoardManufacturer)::ValueType dgetBoardManufactureName() const {
      if ( BoardManufacturer.isDefined() ) return BoardManufacturer.value();
      return dgetBiosVendor();

    };

    decltype(BoardSerialNumber)::ValueType dgetBoardSerialNumber() const {
      if ( BoardSerialNumber.isDefined() ) return BoardSerialNumber.value();
      return AppleBoardSN;
    };

    decltype(BoardID)::ValueType dgetBoardNumber() const {
      if ( BoardID.isDefined() ) return BoardID.value();
      return ApplePlatformDataArray[dgetModel()].boardID;
    };

    decltype(LocationInChassis)::ValueType dgetLocationInChassis() const {
      if ( LocationInChassis.isDefined() ) return LocationInChassis.value();
      return AppleBoardLocation;
    };

    decltype(BoardVersion)::ValueType dgetBoardVersion() const {
      return BoardVersion.isDefined() ? BoardVersion.value() : dgetProductName();
    };

    decltype(BoardType)::ValueType dgetBoardType() const {
      if ( BoardType.isDefined() ) return BoardType.value();
      MacModel Model = dgetModel();
      if ((Model > MacPro31) && (Model < MacPro71)) {
        return BaseBoardTypeProcessorMemoryModule; //0xB;
      } else {
        return BaseBoardTypeMotherBoard; //0xA;
      }
    };

    decltype(Mobile)::ValueType dgetMobile() const {
      if ( Mobile.isDefined() ) return Mobile.value();
      return GetMobile(dgetModel());
    };

    decltype(ChassisType)::ValueType dgetChassisType() const {
      if ( ChassisType.isDefined() ) return ChassisType.value();
      return GetChassisTypeFromModel(dgetModel());
    };

    decltype(ChassisManufacturer)::ValueType dgetChassisManufacturer() const {
      if ( ChassisManufacturer.isDefined() ) return ChassisManufacturer.value();
      return dgetBiosVendor();
    };

    decltype(ChassisAssetTag)::ValueType dgetChassisAssetTag() const {
      if ( ChassisAssetTag.isDefined() ) return ChassisAssetTag.value();
      return ApplePlatformDataArray[dgetModel()].chassisAsset;
    };

    decltype(SmbiosVersion)::ValueType dgetSmbiosVersion() const { return SmbiosVersion.isDefined() ? SmbiosVersion.value() : 0x204; };
    decltype(MemoryRank)::ValueType dgetAttribute() const { return MemoryRank.isDefined() ? MemoryRank.value() : -1; };
    decltype(Trust)::ValueType dgetTrustSMBIOS() const { return Trust.isDefined() ? Trust.value() : XBool(true); };
    XBool dgetInjectMemoryTables() const { return Memory.Modules.isDefined() && Memory.Modules.size() > 0 ? Memory.dgetSlotCount() > 0 : false; };
    decltype(PlatformFeature)::ValueType dgetgPlatformFeature() const {
      if ( PlatformFeature.isDefined() ) return PlatformFeature.value();
      return GetPlatformFeature(dgetModel());
    };
    const decltype(NoRomInfo)::ValueType& dgetNoRomInfo() const { return NoRomInfo.isDefined() ? NoRomInfo.value() : NoRomInfo.nullValue; };
    decltype(FirmwareFeatures)::ValueType dgetFirmwareFeatures() const {
      if ( FirmwareFeatures.isDefined() ) return FirmwareFeatures.value();
      return GetFwFeatures(dgetModel());
    };
    decltype(FirmwareFeaturesMask)::ValueType dgetFirmwareFeaturesMask() const {
      if ( FirmwareFeaturesMask.isDefined() ) return FirmwareFeaturesMask.value();
      return GetFwFeaturesMaskFromModel(dgetModel());
    };
    decltype(ExtendedFirmwareFeatures)::ValueType dgetExtendedFirmwareFeatures() const {
      if ( ExtendedFirmwareFeatures.isDefined() ) return ExtendedFirmwareFeatures.value();
      return GetExtFwFeatures(dgetModel());
    };
    decltype(ExtendedFirmwareFeaturesMask)::ValueType dgetExtendedFirmwareFeaturesMask() const {
      if ( ExtendedFirmwareFeaturesMask.isDefined() ) return ExtendedFirmwareFeaturesMask.value();
      return GetExtFwFeaturesMask(dgetModel());
    }

  };

  SmbiosDictClass SMBIOS = SmbiosDictClass();
  
  XmlDictField m_fields[1] = {
    {"SMBIOS", SMBIOS},
  };

public:
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

public:
  SmbiosPlistClass() {};

  const decltype(SMBIOS)& getSMBIOS() const { return SMBIOS; };


};


//extern const ConfigPlist& configPlist;
//extern const ConfigPlist& getConfigPlist();

#endif /* _SMBIOSPLISTCLASS_H_ */
