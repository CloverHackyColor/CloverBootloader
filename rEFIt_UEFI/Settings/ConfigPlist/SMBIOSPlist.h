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
#include "../../Platform/platformdata.h"
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
            
            class SlotClass : public XmlUInt64
            {
              using super = XmlUInt64;
              virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
                if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
                if ( !isDefined() ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Slot must be defined as a number between 0 and 15 inclusive at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                if ( value() < 0 ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Slot cannot be negative. It must a number between 0 and 15 inclusive at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                if ( value() > 15 ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Slot cannot > 15. It must a number between 0 and 15 inclusive at '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                return true;
              }
            };

            XmlUInt64 Slot = XmlUInt64();
            XmlUInt32 Size = XmlUInt32();
            XmlUInt32 Frequency = XmlUInt32();
            XmlString8AllowEmpty Vendor = XmlString8AllowEmpty();
            XmlString8AllowEmpty Part = XmlString8AllowEmpty();
            XmlString8AllowEmpty Serial = XmlString8AllowEmpty();
            class TypeClass: public XmlString8AllowEmpty {
                using super = XmlString8AllowEmpty;
                virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
                  if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
                  if ( isDefined() ) {
                    if ( xstring8.isEqualIC("DDR") ) return true;
                    if ( xstring8.isEqualIC("DDR2") ) return true;
                    if ( xstring8.isEqualIC("DDR3") ) return true;
                    if ( xstring8.isEqualIC("DDR4") ) return true;
                  }
                  return xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be \"DDR\", \"DDR2\", \"DDR3\" or \"DDR4\" in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
                }
              public:
            } Type = TypeClass();

            XmlDictField m_fields[7] = {
              {"Slot", Slot},
              {"Size", Size},
              {"Frequency", Frequency},
              {"Vendor", Vendor},
              {"Part", Part},
              {"Serial", Serial},
              {"Type", Type},
            };

            virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
            
            virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
              if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
              if ( !Slot.isDefined() ) return false;
              return true;
            }

            const decltype(Slot)::ValueType& dgetSlotNo() const { return Slot.isDefined() ? Slot.value() : Slot.nullValue; };
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
            bool dgetInUse() const { return Size.isDefined() ? Size.value() > 0 : false; };

          };
          
          class ModuleArrayClass : public XmlArray<ModuleDictClass>
          {
            public:
              
              const ModuleDictClass& dgetSolt(size_t slotNo) const {
                for ( size_t idx = 0; idx < size(); ++idx ) {
                  if ( ElementAt(idx).dgetSlotNo() == slotNo )
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
        
        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
          bool b = true;
          return b;
        }

        decltype(SlotCount)::ValueType dgetSlotCountSetting() const { return SlotCount.isDefined() ? SlotCount.value() : 0; };
        const decltype(Channels)::ValueType& dgetUserChannels() const { return Channels.isDefined() ? Channels.value() : Channels.nullValue; };
        
        decltype(SlotCount)::ValueType dgetSlotMax() const {
          if ( !isDefined() || !Modules.isDefined() || Modules.size() == 0 ) return 0;
          uint8_t max = 0;
          for ( size_t idx = 0 ; idx < Modules.size() ; ++idx ) {
            if ( Modules[idx].dgetModuleSize() > 0 ) {
              if ( Modules[idx].dgetSlotNo() > UINT8_MAX ) {
                log_technical_bug("Modules[idx].dgetSlotNo() > UINT8_MAX");
              }else{
                if ( Modules[idx].dgetSlotNo() > max ) max = (uint8_t)Modules[idx].dgetSlotNo(); // safe cast Modules[idx].dgetSlotNo() is <= UINT8_MAX
              }
            }
          }
          return max+1;
        };
        
        decltype(SlotCount)::ValueType dgetSlotCounts() const { return dgetSlotCountSetting() > dgetSlotMax() ? dgetSlotCountSetting() : dgetSlotMax(); };

      };

      class SlotDeviceDictClass : public XmlDict
      {
        using super = XmlDict;
      public:
        static SlotDeviceDictClass NullValue;
        
        class DeviceClass: public XmlString8AllowEmpty {
          using super = XmlString8AllowEmpty;
          virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
            if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
            if ( isDefined() ) {
              if ( xstring8.isEqualIC("ATI") ) return true;
              if ( xstring8.isEqualIC("NVidia") ) return true;
              if ( xstring8.isEqualIC("IntelGFX") ) return true;
              if ( xstring8.isEqualIC("LAN") ) return true;
              if ( xstring8.isEqualIC("WIFI") ) return true;
              if ( xstring8.isEqualIC("Firewire") ) return true;
              if ( xstring8.isEqualIC("HDMI") ) return true;
              if ( xstring8.isEqualIC("USB") ) return true;
              if ( xstring8.isEqualIC("NVME") ) return true;
            }
            return xmlLiteParser->addWarning(generateErrors, S8Printf("Type must be \"ATI\", \"NVidia\", \"IntelGFX\", \"LAN\", \"WIFI\", \"Firewire\", \"HDMI\", \"USB\" or \"NVME\" in dict '%s:%d'", xmlPath.c_str(), keyPos.getLine()));
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
        
        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
          bool b = true;
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
          virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
            if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
            if ( !isDefined() ) return true;
            if ( !IsValidGuidString(xstring8) ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Invalid SmUUID '%s' - should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
            return true;
          }
      };

      class ProductNameClass : public XmlString8Trimed
      {
          using super = XmlString8Trimed;
        public:
          ProductNameClass() : super(true) {};
          virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
            if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
            if ( !isDefined() ) {
              xmlLiteParser->addError(generateErrors, S8Printf("You must define ProductName in SMBIOS dict, line %d", keyPos.getLine()));
              return true;
            }
            MACHINE_TYPES Model;
            Model = GetModelFromString(xstring8);
            if ( Model == MaxMachineType ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Invalid ProductName '%s' in dict '%s:%d'", xstring8.c_str(), xmlPath.c_str(), keyPos.getLine()));
            return true;
          }
      };

  protected:
    XmlString8Trimed BiosVendor = XmlString8Trimed(true); // true = allow empty
    XmlString8Trimed BiosVersion = XmlString8Trimed(true); // RomVersion
    XmlString8AllowEmpty EfiVersion = XmlString8AllowEmpty();
    XmlString8Trimed BiosReleaseDate = XmlString8Trimed(true);

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
  public:
    MemoryDictClass Memory = MemoryDictClass();
    SlotDeviceArrayClass Slots = SlotDeviceArrayClass();

    XmlDictField m_fields[29] = {
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
    
    
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      if ( !ProductName.isDefined() ) {
        return xmlLiteParser->addWarning(generateErrors, S8Printf("ProductName is not defined, the whole SMBIOS dict is ignored at line %d.", keyPos.getLine()));
      }
      if ( BiosVersion.isDefined() ) {
        if ( !BiosVersion.value().contains(".") ) {
          xmlLiteParser->addWarning(generateErrors, S8Printf("BiosVersion '%s' doesn't contains a dot in dict '%s:%d'.", BiosVersion.value().c_str(), xmlPath.c_str(), keyPos.getLine()));
          BiosVersion.reset();
        }else{
          size_t rindex = BiosVersion.value().rindexOf(".");
          if ( BiosVersion.value().length() - rindex < 7 ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("Last part of BiosVersion '%s' must be at least 6 chars in dict '%s:%d'.", BiosVersion.value().c_str(), xmlPath.c_str(), keyPos.getLine()));
            BiosVersion.reset();
          }else{
            // Should we check the format of these 6 last chars ?
            if ( hasModel() ) {
              if ( !is2ndBiosVersionGreaterThan1st(ApplePlatformData[getModel()].firmwareVersion, BiosVersion.value()) ) {
                xmlLiteParser->addWarning(generateErrors, S8Printf("BiosVersion '%s' is before than default ('%s') -> ignored. Dict '%s:%d'.", BiosVersion.value().c_str(), ApplePlatformData[getModel()].firmwareVersion.c_str(), xmlPath.c_str(), keyPos.getLine()));
                BiosVersion.reset();
              }
            }else{
              xmlLiteParser->addWarning(generateErrors, S8Printf("Cannot check validity of BiosVersion because ProductName is not set. Dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
            }
          }
        }
      }
      return true; // we don't want to invalidate the whole dict
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


    /*
     * DO NOT call this if !ProductName.isDefined()
     */
    MACHINE_TYPES getModel() const
    {
      if ( !ProductName.isDefined() ) {
        log_technical_bug("%s : !ProductName.isDefined()", __PRETTY_FUNCTION__);
        return iMac132;
      }
      return GetModelFromString(ProductName.value()); // ProductName has been validated, so Model CANNOT be MaxMachineType
    }
    bool hasModel() const { return ProductName.isDefined(); }

    MACHINE_TYPES dgetModel() const
    {
      if ( !hasModel() ) return MaxMachineType;
      return getModel();
    }

    decltype(BiosVendor)::ValueType dgetBiosVendor() const {
      if ( BiosVendor.isDefined() ) return BiosVendor.value();
      return AppleBiosVendor;
    };

    decltype(BiosVersion)::ValueType dgetBiosVersion() const {
      if ( BiosVersion.isDefined() ) return BiosVersion.value();
      return ApplePlatformData[dgetModel()].firmwareVersion;
    };

    decltype(EfiVersion)::ValueType dgetEfiVersion() const {
      if ( !EfiVersion.isDefined() ) return ApplePlatformData[dgetModel()].efiversion;
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
      return ApplePlatformData[dgetModel()].systemVersion;
    };

    const decltype(SerialNumber)::ValueType& dgetSerialNr() const {
      if ( SerialNumber.isDefined() ) return SerialNumber.value();
      return ApplePlatformData[dgetModel()].serialNumber;
    };

    decltype(SmUUID)::ValueType dgetSmUUID() const { return SmUUID.isDefined() ? SmUUID.value() : nullGuidAsString; };

    decltype(Family)::ValueType dgetFamilyName() const {
      if ( Family.isDefined() ) return Family.value();
      return ApplePlatformData[dgetModel()].productFamily;
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
      return ApplePlatformData[dgetModel()].boardID;
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
      MACHINE_TYPES Model = dgetModel();
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
      return ApplePlatformData[dgetModel()].chassisAsset;
    };

    decltype(SmbiosVersion)::ValueType dgetSmbiosVersion() const { return SmbiosVersion.isDefined() ? SmbiosVersion.value() : 0x204; };
    decltype(MemoryRank)::ValueType dgetAttribute() const { return MemoryRank.isDefined() ? MemoryRank.value() : -1; };
    decltype(Trust)::ValueType dgetTrustSMBIOS() const { return Trust.isDefined() ? Trust.value() : true; };
    bool dgetInjectMemoryTables() const { return Memory.Modules.isDefined() && Memory.Modules.size() > 0 ? Memory.dgetSlotCounts() > 0 : false; };
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

};


//extern const ConfigPlist& configPlist;
//extern const ConfigPlist& getConfigPlist();

#endif /* _SMBIOSPLISTCLASS_H_ */
