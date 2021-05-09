/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_GUI_H_
#define _CONFIGPLISTCLASS_GUI_H_


// -------------------------------------------------- GUI class
class GUI_Class : public XmlDict
{
  using super = XmlDict;
public:

    // -------------------------------------------------- Mouse class
    class GUI_Mouse_Class : public XmlDict
    {
      using super = XmlDict;
    protected:
      XmlInt64 Speed = XmlInt64();
      XmlBool Enabled = XmlBool();
      XmlBool Mirror = XmlBool();
      XmlUInt64 DoubleClickTime = XmlUInt64();
      
      XmlDictField m_fields[4] = {
        {"Speed", Speed},
        {"Enabled", Enabled},
        {"Mirror", Mirror},
        {"DoubleClickTime", DoubleClickTime},
      };

    public:
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

      int64_t dgetPointerSpeed() const { return Speed.isDefined() ? Speed.value() : 2; };
      bool dgetPointerEnabled() const {
        if ( dgetPointerSpeed() <= 0 ) return false; // return false, whatever value Enabled has.
        if ( Enabled.isDefined() ) return Enabled.value();
        return true; // if !Enabled.isDefined(), return true because dgetPointerSpeed() > 0
      }
      bool dgetPointerMirror() const { return Mirror.isDefined() ? Mirror.value() : false; };
      uint64_t dgetDoubleClickTime() const { return DoubleClickTime.isDefined() ? DoubleClickTime.value() : 500; };

    };
    // -------------------------------------------------- Mouse class end


    // -------------------------------------------------- Scan class
    // bool or dict
    class GUI_Scan_Class : public XmlUnion
    {
      public:
        // -------------------------------------------------- Scan as dict class
        class GUI_ScanAsDict_Class : public XmlDict
        {
          using super = XmlDict;
        protected:
          XmlBool Entries = XmlBool();
          XmlBool Tool = XmlBool();
          XmlBool Linux = XmlBool();
          XmlBoolOrString Legacy = XmlBoolOrString();
          XmlBoolOrString Kernel = XmlBoolOrString();

          XmlDictField m_fields[5] = {
            {"Entries", Entries},
            {"Tool", Tool},
            {"Linux", Linux},
            {"Legacy", Legacy},
            {"Kernel", Kernel},
          };

        public:
          virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
          
          bool dgetDisableEntryScan() const { return Entries.isDefined() && Entries.value() == false; };
          bool dgetDisableToolScan() const { return Tool.isDefined() && Tool.value() == false; };
          bool dgetLinux() const { return !(Linux.isDefined() && Linux.value() == false); };
          bool dgetNoLegacy() const {
            if ( !Legacy.isDefined() ) return false;
            if ( Legacy.xmlBool.isDefined() && Legacy.xmlBool.value() == false ) return true;
            return false;
          }
          bool dgetLegacyFirst() const { return Legacy.isDefined() && Legacy.xmlString8.isDefined() && Legacy.xmlString8.value().startWithIC("F"); };
          UINT8 dgetKernel() const {
            if ( !Kernel.isDefined() ) return 0;
            if ( Kernel.xmlBool.isDefined() ) {
              if ( Kernel.xmlBool.value() == false ) return KERNEL_SCAN_NONE;
              return 0;
            }
            if ( !Kernel.xmlString8.isDefined() || Kernel.xmlString8.value().isEmpty() ) return 0;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("ne") ) return KERNEL_SCAN_NEWEST;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("n") ) return KERNEL_SCAN_NONE;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("o") ) return KERNEL_SCAN_OLDEST;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("f") ) return KERNEL_SCAN_FIRST;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("l") ) return KERNEL_SCAN_LAST;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("m") ) return KERNEL_SCAN_MOSTRECENT;
            if ( Kernel.xmlString8.value().startWithOrEqualToIC("e") ) return KERNEL_SCAN_EARLIEST;
            return 0;
          }
        };
        // -------------------------------------------------- Scan as dict class end

        const GUI_Class& parent;
        
      public:
        XmlBool ScanAsBool = XmlBool();
        GUI_ScanAsDict_Class ScanAsAsDict = GUI_ScanAsDict_Class();
        
        GUI_Scan_Class(const GUI_Class& _parent) : parent(_parent) {}

        XmlUnionField m_fields[2] = { ScanAsBool, ScanAsAsDict};
        virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
        
        bool dgetDisableEntryScan() const {
          if ( ScanAsBool.isDefined() && ScanAsBool.value() == false ) return true;
          return ScanAsAsDict.dgetDisableEntryScan();
        }
        bool dgetDisableToolScan() const {
          if ( ScanAsBool.isDefined() && ScanAsBool.value() == false ) return true;
          return ScanAsAsDict.dgetDisableToolScan();
        }
        bool dgetLinuxScan() const {
          if ( !ScanAsAsDict.isDefined() ) return parent.isDefined(); // TODO: different default value if section is not defined
          return ScanAsAsDict.dgetLinux();
        }
        bool dgetNoLegacy() const {
          if ( ScanAsBool.isDefined() && ScanAsBool.value() == false ) return true;
          return ScanAsAsDict.dgetNoLegacy();
        }
        bool dgetLegacyFirst() const {
          if ( !ScanAsAsDict.isDefined() ) return false;
          return ScanAsAsDict.dgetLegacyFirst();
        }
        UINT8 dgetKernelScan() const {
          if ( !ScanAsAsDict.isDefined() ) return 0;
          return ScanAsAsDict.dgetKernel();
        }
    };
      // -------------------------------------------------- Scan class end




    // -------------------------------------------------- Custom_Class class end
    class GUI_Custom_Class : public XmlDict
    {
      using super = XmlDict;
    public:

        class GUI_Custom_Entry_VolumeType_Class : public XmlUnion
        {
          using super = XmlDict;
        protected:
          XmlString8AllowEmpty VolumeTypeAsString = XmlString8AllowEmpty();
          XmlArray<XmlString8AllowEmpty> VolumeTypeAsArray = XmlArray<XmlString8AllowEmpty>();

          XmlUnionField m_fields[2] = { VolumeTypeAsString, VolumeTypeAsArray};
          virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
            
          static UINT8 CheckVolumeType(UINT8 VolumeType, const XString8& VolumeTypeAsString)
          {
            UINT8 VolumeTypeTmp = VolumeType;
            if (VolumeTypeAsString.isEqualIC("Internal")) {
              VolumeTypeTmp |= VOLTYPE_INTERNAL;
            } else if (VolumeTypeAsString.isEqualIC("External")) {
              VolumeTypeTmp |= VOLTYPE_EXTERNAL;
            } else if (VolumeTypeAsString.isEqualIC("Optical")) {
              VolumeTypeTmp |= VOLTYPE_OPTICAL;
            } else if (VolumeTypeAsString.isEqualIC("FireWire")) {
              VolumeTypeTmp |= VOLTYPE_FIREWIRE;
            }
            return VolumeTypeTmp;
          }

        public:
          UINT8 dgetVolumeType() const {
            if ( VolumeTypeAsString.isDefined() ) return CheckVolumeType(0, VolumeTypeAsString.value());
            if ( VolumeTypeAsArray.isDefined() ) {
              UINT8 VolumeType = 0;
              for (size_t idx = 0; idx < VolumeTypeAsArray.size() ; ++idx) {
                VolumeType = CheckVolumeType(VolumeType, VolumeTypeAsArray[idx].value());
              }
              return VolumeType;
            }
            return 0;
          }
        };


          class GUI_Custom_SubEntry_Class;
  //
  //
  //class CUSTOM_LOADER_ENTRY_SETTINGS;
  //class GUI_Custom_Entry_Class;
  //template<class C> class XmlArray;
  //void CompareCustomEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_ENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_Entry_Class>& newCustomEntries);

          class GUI_Custom_Entry_Class : public XmlDict
          {
            using super = XmlDict;
          public:
//              const GUI_Custom_Entry_Class* Parent = NULL;

            class HiddenClass: public XmlUnion
            {
              using super = XmlUnion;
            public:
              XmlBool xmlBool = XmlBool();
              XmlString8AllowEmpty xmlString8 = XmlString8AllowEmpty();
              XmlUnionField m_fields[2] = { xmlBool, xmlString8};
              virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
              virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
                RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
                if ( !xmlString8.isDefined() ) return true;
                if ( xmlString8.value().isEqualIC("Always") ) return true;
                xmlLiteParser->addWarning(generateErrors, S8Printf("Expecting a boolean or \"Always\" for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
                return false; // parsing can continue.
              }
            };

          protected:
            XmlBool Disabled = XmlBool();
            XmlStringW Volume = XmlStringW();
            XmlStringW Path = XmlStringW();
            XmlStringW Settings = XmlStringW();
            XmlBool CommonSettings = XmlBool();
            XmlString8AllowEmpty AddArguments = XmlString8AllowEmpty();
            XmlString8AllowEmpty Arguments = XmlString8AllowEmpty();
            XmlString8AllowEmpty Title = XmlString8AllowEmpty();
            XmlString8AllowEmpty FullTitle = XmlString8AllowEmpty();
            XmlStringW ImagePath = XmlStringW();
            XmlData    ImageData = XmlData();
            XmlStringW DriveImagePath = XmlStringW();
            XmlData    DriveImageData = XmlData();
            XmlString8AllowEmpty Hotkey = XmlString8AllowEmpty(); // todo check it's only one UTF16 char (or maybe we should switch to char32_t)
            XmlBoolOrStringOrData CustomLogo = XmlBoolOrStringOrData(); // Todo validate
            XmlString8AllowEmpty BootBgColor = XmlString8AllowEmpty();
            HiddenClass Hidden = HiddenClass();
          // Type
            XmlString8AllowEmpty Type = XmlString8AllowEmpty();
            GUI_Custom_Entry_VolumeType_Class VolumeType = GUI_Custom_Entry_VolumeType_Class();
            XmlBoolOrString InjectKexts = XmlBoolOrString();
            XmlBool NoCaches = XmlBool();
            XmlString8AllowEmpty Kernel = XmlString8AllowEmpty();

            XmlBool ForceTextMode = XmlBool(); // 2021-04-22

          public:
            XmlArray<GUI_Custom_SubEntry_Class> SubEntries = XmlArray<GUI_Custom_SubEntry_Class>();

          protected:
            XmlDictField m_fields[24] = {
              {"Disabled", Disabled},
              {"Volume", Volume},
              {"Path", Path},
              {"Settings", Settings},
              {"CommonSettings", CommonSettings},
              {"AddArguments", AddArguments},
              {"Arguments", Arguments},
              {"Title", Title},
              {"FullTitle", FullTitle},
              {"Image", ImagePath},
              {"ImageData", ImageData},
              {"DriveImage", DriveImagePath},
              {"DriveImageData", DriveImageData},
              {"Hotkey", Hotkey},
              {"CustomLogo", CustomLogo},
              {"BootBgColor", BootBgColor},
              {"Hidden", Hidden},
              {"Type", Type},
              {"VolumeType", VolumeType},
              {"InjectKexts", InjectKexts},
              {"NoCaches", NoCaches},
              {"Kernel", Kernel},
              {"SubEntries", SubEntries},
              {"ForceTextMode", ForceTextMode},
            };

          public:
            virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
            virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override;

            bool dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : false; };
            const XStringW& dgetVolume() const { return Volume.isDefined() ? Volume.value() : NullXStringW; };
            const XStringW& dgetPath() const { return Path.isDefined() ? Path.value() : NullXStringW; };
            const XStringW& dgetSettings() const { return Settings.isDefined() ? Settings.value() : NullXStringW; };
            BOOLEAN dgetCommonSettings() const { return CommonSettings.isDefined() ? CommonSettings.value() : false; };
            const XString8& dgetAddArguments() const { return AddArguments.isDefined() ? AddArguments.value() : NullXString8; };
            const undefinable_XString8 dgetArguments() const { return Arguments.isDefined() ? undefinable_XString8(Arguments.value()) : undefinable_XString8(); };
            const XString8& dgetm_Title() const { return Title.isDefined() ? Title.value() : NullXString8; };
            const XString8& dgetFullTitle() const { return FullTitle.isDefined() ? FullTitle.value() : NullXString8; };
            const XStringW& dgetm_ImagePath() const { return ImagePath.isDefined() ? ImagePath.value() : NullXStringW; };
            const XBuffer<UINT8>& dgetImageData() const { return ImageData.isDefined() ? ImageData.value() : XBuffer<UINT8>::NullXBuffer; };
            const XStringW& dgetm_DriveImagePath() const { return DriveImagePath.isDefined() ? DriveImagePath.value() : NullXStringW; };
            const XBuffer<UINT8>& dgetDriveImageData() const { return DriveImageData.isDefined() ? DriveImageData.value() : XBuffer<UINT8>::NullXBuffer; };
            char32_t dgetHotkey() const { return Hotkey.isDefined() && Hotkey.value().notEmpty() ? Hotkey.value()[0] : 0; };

            const XString8& dgetCustomLogoAsXString8() const  { return CustomLogo.xmlString8.isDefined() ? CustomLogo.xmlString8.value() : NullXString8; };
            const XBuffer<UINT8>& dgetCustomLogoAsData() const  { return CustomLogo.xmlData.isDefined() ? CustomLogo.xmlData.value() :  XBuffer<UINT8>::NullXBuffer; };
            UINT8 dgetCustomLogoTypeSettings() const {
              if ( CustomLogo.xmlBool.isDefined() && CustomLogo.xmlBool.value() ) return CUSTOM_BOOT_APPLE;
              if ( CustomLogo.xmlString8.isDefined() ) {
                if ( CustomLogo.xmlString8.value() == "Apple"_XS8 ) return CUSTOM_BOOT_APPLE;
                if ( CustomLogo.xmlString8.value() == "Alternate"_XS8 ) return CUSTOM_BOOT_ALT_APPLE;
                if ( CustomLogo.xmlString8.value() == "Theme"_XS8 ) return CUSTOM_BOOT_THEME;
                return CUSTOM_BOOT_USER;
              }
              if ( CustomLogo.xmlData.isDefined() ) {
                return CUSTOM_BOOT_USER;
              }
              return CUSTOM_BOOT_DISABLED;
            }

            EFI_GRAPHICS_OUTPUT_BLT_PIXEL dgetBootBgColor() const {
              if ( !BootBgColor.isDefined() ) return EFI_GRAPHICS_OUTPUT_BLT_PIXEL({0,0,0,0});
              UINTN Color = AsciiStrHexToUintn(BootBgColor.value());
              return EFI_GRAPHICS_OUTPUT_BLT_PIXEL({uint8_t((Color >> 8) & 0xFF),uint8_t((Color >> 16) & 0xFF),uint8_t((Color >> 24) & 0xFF),uint8_t((Color >> 0) & 0xFF)});
            }
            bool dgetHidden() const { return Hidden.isDefined() && Hidden.xmlBool.isDefined() ? Hidden.xmlBool.value() : false; };
            bool dgetAlwaysHidden() const { return Hidden.isDefined() && Hidden.xmlString8.isDefined() ? Hidden.xmlString8.value().isEqualIC("Always") : false; };
            UINT8 dgetType() const {
              if ( Type.isDefined() ) {
                if ((Type.value().isEqualIC("OSX")) ||
                    (Type.value() .isEqualIC("macOS"))) {
                  return OSTYPE_OSX;
                } else if (Type.value() .isEqualIC("OSXInstaller")) {
                  return OSTYPE_OSX_INSTALLER;
                } else if (Type.value() .isEqualIC("OSXRecovery")) {
                  return OSTYPE_RECOVERY;
                } else if (Type.value() .isEqualIC("Windows")) {
                  return OSTYPE_WINEFI;
                } else if (Type.value() .isEqualIC("Linux")) {
                  return OSTYPE_LIN;
                } else if (Type.value() .isEqualIC("LinuxKernel")) {
                  return OSTYPE_LINEFI;
                } else {
                  return OSTYPE_OTHER;
                }
              } else {
                if ( dgetPath().notEmpty() ) {
                  // Try to set Entry->type from Entry->Path
                  return GetOSTypeFromPath(dgetPath());
                }else{
                  return 0;
                }
              }
            }

            UINT8 dgetVolumeType() const { return VolumeType.isDefined() ? VolumeType.dgetVolumeType() : /*Parent ? Parent->dgetVolumeType() :*/ 0; } // VolumeType is duplicated in DuplicateCustomEntry(), but unconditionnally assigned in FillinCustomEntry(). So no "inheritance" from parent.
            INT8 dgetInjectKexts() const {
              if ( OSTYPE_IS_OSX(dgetType()) || OSTYPE_IS_OSX_RECOVERY(dgetType()) || OSTYPE_IS_OSX_INSTALLER(dgetType()) ) {
                if ( InjectKexts.xmlBool.isDefined() ) return InjectKexts.xmlBool.value();
                if ( InjectKexts.xmlString8.isDefined() && InjectKexts.xmlString8.value().isEqualIC("Detect") ) return 2;
              }
              return -1;
            }
            undefinable_bool dgetNoCaches() const { return NoCaches.isDefined() && NoCaches.value() ? undefinable_bool(true) : undefinable_bool(); };
            UINT8 dgetKernelScan() const {
              if ( !Kernel.isDefined() ) return KERNEL_SCAN_ALL;
              if ((Kernel.value()[0] == 'N') || (Kernel.value()[0] == 'n')) {
                return KERNEL_SCAN_NEWEST;
              } else if ((Kernel.value()[0] == 'O') || (Kernel.value()[0] == 'o')) {
                return KERNEL_SCAN_OLDEST;
              } else if ((Kernel.value()[0] == 'F') || (Kernel.value()[0] == 'f')) {
                return KERNEL_SCAN_FIRST;
              } else if ((Kernel.value()[0] == 'L') || (Kernel.value()[0] == 'l')) {
                return KERNEL_SCAN_LAST;
              } else if ((Kernel.value()[0] == 'M') || (Kernel.value()[0] == 'm')) {
                return KERNEL_SCAN_MOSTRECENT;
              } else if ((Kernel.value()[0] == 'E') || (Kernel.value()[0] == 'e')) {
                return KERNEL_SCAN_EARLIEST;
              }
              return KERNEL_SCAN_ALL;
            }
            
            // 2021-04-22
            decltype(ForceTextMode)::ValueType dgetForceTextMode() const { return ForceTextMode.isDefined() ? ForceTextMode.value() : false; };

//              /* calculated values */
//              UINT8 getFlags(bool NoCachesDefault) const {
//                UINT8 Flags = 0;
//                if ( Arguments.isDefined() ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTARGS);
//                if ( dgetAlwaysHidden() ) Flags = OSFLAG_SET(Flags, OSFLAG_DISABLED);
//                if ( dgetType() == OSTYPE_LIN ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTARGS);
//                if (OSTYPE_IS_OSX(dgetType()) || OSTYPE_IS_OSX_RECOVERY(dgetType()) || OSTYPE_IS_OSX_INSTALLER(dgetType())) {
//                  Flags = OSFLAG_UNSET(Flags, OSFLAG_NOCACHES);
//                }
//                if ( NoCaches.isDefined() ) {
//                  if ( NoCaches ) Flags = OSFLAG_SET(Flags, OSFLAG_NOCACHES);
//                }else{
//                  if (NoCachesDefault) {
//                    Flags = OSFLAG_SET(Flags, OSFLAG_NOCACHES);
//                  }
//                }
//                if ( SubEntries.notEmpty() ) Flags = OSFLAG_SET(Flags, OSFLAG_NODEFAULTMENU);
//                return Flags;
//              }

          };

    //class CUSTOM_LOADER_SUBENTRY_SETTINGS;
    //class GUI_Custom_SubEntry_Class;
    //template<class C> class XmlArray;
    //void CompareCustomSubEntries(const XString8& label, const XObjArray<CUSTOM_LOADER_SUBENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_SubEntry_Class>& newCustomEntries);

            class GUI_Custom_SubEntry_Class : public XmlDict
            {
              using super = XmlDict;
            public:
              const GUI_Custom_Entry_Class* Parent = NULL;
            protected:
              XmlBool Disabled = XmlBool();
              XmlString8AllowEmpty AddArguments = XmlString8AllowEmpty();
              XmlString8AllowEmpty Arguments = XmlString8AllowEmpty();
              XmlString8AllowEmpty Title = XmlString8AllowEmpty();
              XmlString8AllowEmpty FullTitle = XmlString8AllowEmpty();
              XmlBool NoCaches = XmlBool();
            public:

            protected:
              XmlDictField m_fields[6] = {
                {"Disabled", Disabled},
                {"AddArguments", AddArguments},
                {"Arguments", Arguments},
                {"Title", Title},
                {"FullTitle", FullTitle},
                {"NoCaches", NoCaches},
              };

            public:
              virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
              virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
                if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
                return true;
              }

              bool dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : false; };
              const XString8& dget_AddArguments() const { return AddArguments.isDefined() ? AddArguments.value() : NullXString8; };
              const undefinable_XString8 dget_Arguments() const { return Arguments.isDefined() ? undefinable_XString8(Arguments.value()) : undefinable_XString8(); };
              undefinable_XString8 dget_Title() const { return Title.isDefined() ? undefinable_XString8(Title.value()) : undefinable_XString8(); };
              undefinable_XString8 dget_FullTitle() const { return FullTitle.isDefined() ? undefinable_XString8(FullTitle.value()) : undefinable_XString8(); };
              undefinable_bool dget_NoCaches() const { return NoCaches.isDefined() && NoCaches.value() ? undefinable_bool(true) : undefinable_bool(); };
//              XString8 dget_Title() const {
//                if ( Title.isDefined() && Title.xstring8.notEmpty() ) return Title;
//                if ( OSTYPE_IS_OSX_RECOVERY(Parent->dgetType()) ) {
//                  return "Recovery"_XS8; // TODO use a shared static
//                } else if ( OSTYPE_IS_OSX_INSTALLER(Parent->dgetType()) ) {
//                  return "Install macOS"_XS8; // TODO use a shared static
//                }
//                return NullXStringW;
//
//                if ( Title.isDefined() ) return Title.xstring8;
//                if ( FullTitle.isDefined() ) return NullXString8;
//                if ( Parent ) return Parent->dgetTitle();
//                return NullXString8;
//              };
//              const XString8& dget_FullTitle() const {
//                if ( FullTitle.isDefined() ) return FullTitle.xstring8;
//                if ( Title.isDefined() ) return NullXString8;
//                if ( Parent ) return Parent->dgetFullTitle();
//                return NullXString8;
//              };

              
    //          class CUSTOM_LOADER_SUBENTRY_SETTINGS;
    //          friend void ::CompareCustomSubEntries(const XString8& label, const XObjArray<::CUSTOM_LOADER_SUBENTRY_SETTINGS>& olDCustomEntries, const XmlArray<GUI_Custom_SubEntry_Class>& newCustomEntries);

            };

            class GUI_Custom_Legacy_Class : public XmlDict
            {
              using super = XmlDict;
            protected:
              XmlBool Disabled = XmlBool();
              XmlString8AllowEmpty Volume = XmlString8AllowEmpty();
              XmlString8AllowEmpty Title = XmlString8AllowEmpty();
              XmlString8AllowEmpty FullTitle = XmlString8AllowEmpty();
              XmlString8AllowEmpty ImagePath = XmlString8AllowEmpty();
              XmlData    ImageData = XmlData();
              XmlString8AllowEmpty DriveImagePath = XmlString8AllowEmpty();
              XmlData    DriveImageData = XmlData();
              class HotKeyClass : public XmlString8AllowEmpty {
                using super = XmlString8AllowEmpty;
                virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
                  RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
                  if ( !isDefined() ) return true;
                  if ( xstring8.length() != 1 ) {
                    xmlLiteParser->addError(generateErrors, S8Printf("HotKey must a string of only one UTF-16 char at line %d.", keyPos.getLine()));
                    return false;
                  }
                  if ( xstring8[0] >= __WCHAR_MAX__ ) {
                    xmlLiteParser->addError(generateErrors, S8Printf("HotKey must be a UTF-16 char at line %d.", keyPos.getLine()));
                    return false;
                  }
                  return true;
                }
              } Hotkey = HotKeyClass();
              XmlBoolOrString Hidden = XmlBoolOrString();
              XmlString8AllowEmpty Type = XmlString8AllowEmpty();
              GUI_Custom_Entry_VolumeType_Class VolumeType = GUI_Custom_Entry_VolumeType_Class();

              XmlDictField m_fields[12] = {
                {"Disabled", Disabled},
                {"Volume", Volume},
                {"Title", Title},
                {"FullTitle", FullTitle},
                {"Image", ImagePath},
                {"ImageData", ImageData},
                {"DriveImage", DriveImagePath},
                {"DriveImageData", DriveImageData},
                {"Hotkey", Hotkey},
                {"Hidden", Hidden},
                {"Type", Type},
                {"VolumeType", VolumeType},
              };
              
            public:
              virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
              
              bool dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : false; };
              const XString8& dgetVolume() const { return Volume.isDefined() ? Volume.value() : NullXString8; };
              const XString8& dgetTitle() const { return Title.isDefined() ? Title.value() : NullXString8; };
              const XString8& dgetFullTitle() const { return FullTitle.isDefined() ? FullTitle.value() : NullXString8; };
              const XString8& dgetImagePath() const { return ImagePath.isDefined() ? ImagePath.value() : NullXString8; };
              const XBuffer<UINT8>& dgetImageData() const { return ImageData.isDefined() ? ImageData.value() : XBuffer<UINT8>::NullXBuffer; };
              const XString8& dgetDriveImagePath() const { return DriveImagePath.isDefined() ? DriveImagePath.value() : NullXString8; };
              const XBuffer<UINT8>& dgetDriveImageData() const { return DriveImageData.isDefined() ? DriveImageData.value() : XBuffer<UINT8>::NullXBuffer; };
              char32_t dgetHotkey() const { return Hotkey.isDefined() && Hotkey.value().notEmpty() ? Hotkey.value()[0] : 0; };
              bool dgetHidden() const { return Hidden.isDefined() && Hidden.xmlBool.isDefined() ? Hidden.xmlBool.value() : false; };
              bool dgetAlwaysHidden() const { return Hidden.isDefined() && Hidden.xmlString8.isDefined() ? Hidden.xmlString8.value().isEqualIC("Always") : false; };
              UINT8 dgetType() const {
                if ( Type.isDefined() ) {
                  if (Type.value().isEqualIC("Windows")) {
                    return OSTYPE_WIN;
                  } else if (Type.value().isEqualIC("Linux")) {
                    return OSTYPE_LIN;
                  } else {
                    return OSTYPE_OTHER;
                  }
                }
                return 0;
              }

              UINT8 dgetVolumeType() const { return VolumeType.isDefined() ? VolumeType.dgetVolumeType() : /*Parent ? Parent->dgetVolumeType() :*/ 0; } // VolumeType is duplicated in DuplicateCustomEntry(), but unconditionnally assigned in FillinCustomEntry(). So no "inheritance" from parent.
            };


        class GUI_Custom_Tool_Class : public XmlDict
        {
          using super = XmlDict;
        protected:
          XmlBool Disabled = XmlBool();
          XmlString8AllowEmpty Volume = XmlString8AllowEmpty();
          XmlString8AllowEmpty Path = XmlString8AllowEmpty();
          XmlString8AllowEmpty Arguments = XmlString8AllowEmpty();
          XmlString8AllowEmpty Title = XmlString8AllowEmpty();
          XmlString8AllowEmpty FullTitle = XmlString8AllowEmpty();
          XmlString8AllowEmpty ImagePath = XmlString8AllowEmpty();
          XmlData    ImageData = XmlData();
          XmlString8AllowEmpty Hotkey = XmlString8AllowEmpty();
          XmlBoolOrString Hidden = XmlBoolOrString();
          GUI_Custom_Entry_VolumeType_Class VolumeType = GUI_Custom_Entry_VolumeType_Class();

          XmlDictField m_fields[11] = {
            {"Disabled", Disabled},
            {"Volume", Volume},
            {"Path", Path},
            {"Arguments", Arguments},
            {"Title", Title},
            {"FullTitle", FullTitle},
            {"Image", ImagePath},
            {"ImageData", ImageData},
            {"Hotkey", Hotkey},
            {"Hidden", Hidden},
            {"VolumeType", VolumeType},
          };
          
        public:
          virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

          bool dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : false; };
          const XString8& dgetVolume() const { return Volume.isDefined() ? Volume.value() : NullXString8; };
          const XString8& dgetPath() const { return Path.isDefined() ? Path.value() : NullXString8; };
          const XString8& dgetArguments() const { return Arguments.isDefined() ? Arguments.value() : NullXString8; };
          const XString8& dgetTitle() const { return Title.isDefined() ? Title.value() : NullXString8; };
          const XString8& dgetFullTitle() const { return FullTitle.isDefined() ? FullTitle.value() : NullXString8; };
          const XString8& dgetImagePath() const { return ImagePath.isDefined() ? ImagePath.value() : NullXString8; };
          const XBuffer<UINT8>& dgetImageData() const { return ImageData.isDefined() ? ImageData.value() : XBuffer<UINT8>::NullXBuffer; };
          char32_t dgetHotkey() const { return Hotkey.isDefined() && Hotkey.value().notEmpty() ? Hotkey.value()[0] : 0; };
          bool dgetHidden() const { return Hidden.isDefined() && Hidden.xmlBool.isDefined() ? Hidden.xmlBool.value() : false; };
          bool dgetAlwaysHidden() const { return Hidden.isDefined() && Hidden.xmlString8.isDefined() ? Hidden.xmlString8.value().isEqualIC("Always") : false; };

          UINT8 dgetVolumeType() const { return VolumeType.isDefined() ? VolumeType.dgetVolumeType() : /*Parent ? Parent->dgetVolumeType() :*/ 0; } // VolumeType is duplicated in DuplicateCustomEntry(), but unconditionnally assigned in FillinCustomEntry(). So no "inheritance" from parent.
        };


    public:
      XmlArray<GUI_Custom_Entry_Class> Entries = XmlArray<GUI_Custom_Entry_Class>();
      XmlArray<GUI_Custom_Legacy_Class> Legacy = XmlArray<GUI_Custom_Legacy_Class>();
      XmlArray<GUI_Custom_Tool_Class> Tool = XmlArray<GUI_Custom_Tool_Class>();

    protected:
      XmlDictField m_fields[3] = {
        {"Entries", Entries},
        {"Legacy", Legacy},
        {"Tool", Tool},
      };
      
    public:
      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    };


// -------------------------------------------------- GUI class
protected:
// Timezone
  XmlInt32 Timezone = XmlInt32(); // TODO : checkeck Timezone validity
// Theme
  XmlString8AllowEmpty Theme = XmlString8AllowEmpty();
// EmbeddedThemeType
  class EmbeddedThemeTypeClass: public XmlString8AllowEmpty {
    using super = XmlString8AllowEmpty;
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
      if ( xstring8.isEqualIC("Dark") ) return true;
      if ( xstring8.isEqualIC("Light") ) return true;
      if ( xstring8.isEqualIC("Daytime") ) return true;
      xmlLiteParser->addError(generateErrors, S8Printf("EmbeddedThemeType must \"Dark\" \"Light\" or \"Daytime\" at line %d.", keyPos.getLine()));
      return false;
    }
  } EmbeddedThemeType = EmbeddedThemeTypeClass();
// PlayAsync
  XmlBool PlayAsync = XmlBool();
// CustomIcons
  XmlBool CustomIcons = XmlBool();
// TextOnly
  XmlBool TextOnly = XmlBool();
// ShowOptimus
  XmlBool ShowOptimus = XmlBool();
// ScreenResolution
  class ScreenResolutionClass: public XmlString8AllowEmpty {
    using super = XmlString8AllowEmpty;
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
      // TODO Check that resolution is Integer x Integer
      return true;
      //xmlLiteParser->addError(generateErrors, S8Printf("ScreenResolution must be {width}x{height} at line %d.", keyPos.getLine()));
      //return false;
    }
  } ScreenResolution = ScreenResolutionClass();
// ProvideConsoleGop
  XmlBool ProvideConsoleGop = XmlBool();
// ConsoleMode
  class ConsoleModeClass : public XmlInt64OrString {
      using super = XmlInt64OrString;
    public:
//      ConsoleMode_Class() : XmlInt64OrString(false) {};
      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
        if ( !xmlString8.isDefined() ) return true;
        if ( xmlString8.value().isEqualIC("Max") ) return true;
        if ( xmlString8.value().isEqualIC("Min") ) return true;
        xmlLiteParser->addWarning(generateErrors, S8Printf("Expecting an integer or \"Min\" or \"Max\" for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        return false;
      }
  } ConsoleMode = ConsoleModeClass();
  // Language
  class LanguageClass: public XmlString8AllowEmpty {
    using super = XmlString8AllowEmpty;
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
      if ( xstring8.containsIC("en") ) return true;
      if ( xstring8.containsIC("ru") ) return true;
      if ( xstring8.containsIC("ua") ) return true;
      if ( xstring8.containsIC("fr") ) return true;
      if ( xstring8.containsIC("it") ) return true;
      if ( xstring8.containsIC("es") ) return true;
      if ( xstring8.containsIC("pt") ) return true;
      if ( xstring8.containsIC("br") ) return true;
      if ( xstring8.containsIC("de") ) return true;
      if ( xstring8.containsIC("nl") ) return true;
      if ( xstring8.containsIC("pl") ) return true;
      if ( xstring8.containsIC("cz") ) return true;
      if ( xstring8.containsIC("hr") ) return true;
      if ( xstring8.containsIC("id") ) return true;
      if ( xstring8.containsIC("zh_CN") ) return true;
      if ( xstring8.containsIC("ro") ) return true;
      if ( xstring8.containsIC("ko") ) return true;
      xmlLiteParser->addError(generateErrors, S8Printf("Language '%s' is unknown at line %d. Ignored.", xstring8.c_str(), keyPos.getLine()));
      return false;
    }
  } Language = LanguageClass();
// KbdPrevLang
  XmlBool KbdPrevLang = XmlBool();
public:
  // Mouse
  GUI_Mouse_Class Mouse = GUI_Mouse_Class();
// Hide
protected:
  XmlString8Array Hide = XmlString8Array();
public:
  // Scan
  GUI_Scan_Class Scan;
  // Custom
  GUI_Custom_Class Custom = GUI_Custom_Class();

protected:

  XmlDictField m_fields[16] {
    {"Timezone", Timezone},
    {"Theme", Theme},
    {"EmbeddedThemeType", EmbeddedThemeType},
    {"PlayAsync", PlayAsync},
    {"CustomIcons", CustomIcons},
    {"TextOnly", TextOnly},
    {"ShowOptimus", ShowOptimus},
    {"ScreenResolution", ScreenResolution},
    {"ProvideConsoleGop", ProvideConsoleGop},
    {"ConsoleMode", ConsoleMode},
    {"Language", Language},
    {"KbdPrevLang", KbdPrevLang},
    {"Mouse", Mouse},
    {"Hide", Hide},
    {"Scan", Scan},
    {"Custom", Custom},
  };
  
public:

  GUI_Class() : Scan(*this) {}
  
  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
//  virtual bool validate(XmlLiteParser* xmlLiteParser, const char* name, XmlAbstractType* xmlTyp, const XString8& xmlPath, const XmlParserPosition& pos, bool generateErrors) override;

  int32_t dgetTimezone() const { return Timezone.isDefined() ? Timezone.value() : 0xFF; };
  const XString8& dgetTheme() const { return Theme.isDefined() ? Theme.value() : NullXString8; };
  const XString8& dgetEmbeddedThemeType() const { return EmbeddedThemeType.isDefined() ? EmbeddedThemeType.value() : NullXString8; };
  bool dgetPlayAsync() const { return PlayAsync.isDefined() ? PlayAsync.value() : false; };
  bool dgetCustomIcons() const { return CustomIcons.isDefined() ? CustomIcons.value() : false; };
  bool dgetTextOnly() const { return TextOnly.isDefined() ? TextOnly.value() : false; };
  bool dgetShowOptimus() const { return ShowOptimus.isDefined() ? ShowOptimus.value() : false; };
  const XString8& dgetScreenResolution() const { return ScreenResolution.isDefined() ? ScreenResolution.value() : NullXString8; };
  bool dgetProvideConsoleGop() const { return isDefined() ? ProvideConsoleGop.isDefined() ? ProvideConsoleGop.value() : true : false; }; // TODO: different default value if section is not defined
  int64_t dgetConsoleMode() const {
    if ( ConsoleMode.xmlInt64.isDefined() ) {
      return ConsoleMode.xmlInt64.value();
    } else if ( ConsoleMode.xmlString8.isDefined() ) {
      if ( ConsoleMode.xmlString8.value().contains("Max") ) {
        return -1;
      } else if ( ConsoleMode.xmlString8.value().contains("Min") ) {
        return -2;
      } else {
        return (INT32)AsciiStrDecimalToUintn(ConsoleMode.xmlString8.value());
      }
    }
    return 0;
  }
  const decltype(Language)::ValueType& dgetLanguage() const { return Language.isDefined() ? Language.value() : Language.nullValue; };
  LanguageCode dgetlanguageCode() const {
    if ( !Language.isDefined() ) return english;
    if ( Language.value().contains("en") ) {
      return english;
    } else if ( Language.value().contains("ru")) {
      return russian;
    } else if ( Language.value().contains("ua")) {
      return ukrainian;
    } else if ( Language.value().contains("fr")) {
      return french; //default is extended latin
    } else if ( Language.value().contains("it")) {
      return italian;
    } else if ( Language.value().contains("es")) {
      return spanish;
    } else if ( Language.value().contains("pt")) {
      return portuguese;
    } else if ( Language.value().contains("br")) {
      return brasil;
    } else if ( Language.value().contains("de")) {
      return german;
    } else if ( Language.value().contains("nl")) {
      return dutch;
    } else if ( Language.value().contains("pl")) {
      return polish;
    } else if ( Language.value().contains("cz")) {
      return czech;
    } else if ( Language.value().contains("hr")) {
      return croatian;
    } else if ( Language.value().contains("id")) {
      return indonesian;
    } else if ( Language.value().contains("zh_CN")) {
      return chinese;
    } else if ( Language.value().contains("ro")) {
      return romanian;
    } else if ( Language.value().contains("ko")) {
      return korean;
    }
    return english;
  }


    
  bool dgetKbdPrevLang() const { return KbdPrevLang.isDefined() ? KbdPrevLang.value() : false; };

  /* calculated value */

  bool getDarkEmbedded(bool isDaylight) const {
    if ( !EmbeddedThemeType.isDefined() ) return false;
    if ( EmbeddedThemeType.value().isEqualIC("Dark") ) return true;
    if ( EmbeddedThemeType.value().isEqualIC("Daytime") ) return !isDaylight;
    return false;
  }
  
  const decltype(Hide)::ValueType& dgetHVHideStrings() const { return Hide.isDefined() ? Hide.value() : Hide.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_GUI_H_ */
