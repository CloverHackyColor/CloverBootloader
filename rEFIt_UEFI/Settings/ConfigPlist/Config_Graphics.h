/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_GRAPHICS_H_
#define _CONFIGPLISTCLASS_GRAPHICS_H_


class Graphics_Class : public XmlDict
{
  using super = XmlDict;
public:
  const ConfigPlistClass& configPlist;

    class Graphics_PatchVBiosBytes_Class : public XmlDict
    {
      protected:
        XmlData Find = XmlData();
        XmlData Replace = XmlData();

        XmlDictField m_fields[2] = {
          {"Find", Find},
          {"Replace", Replace},
        };

        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

      public:
        const decltype(Find)::ValueType& dgetFind() const { return Find.isDefined() ? Find.value() : Find.nullValue; };
        const decltype(Replace)::ValueType& dgetReplace() const { return Replace.isDefined() ? Replace.value() : Replace.nullValue; };

    };


    class Graphics_ATI_NVIDIA_Class : public XmlDict
    {
      using super = XmlDict;
      protected:
        XmlString8AllowEmpty Model = XmlString8AllowEmpty();
        XmlUInt32 IOPCIPrimaryMatch = XmlUInt32(); // Id
        XmlUInt32 IOPCISubDevId = XmlUInt32();     // SubId
        XmlUInt64 VRAM = XmlUInt64();              // VideoRam
        XmlUInt64 VideoPorts = decltype(VideoPorts)();        // VideoPorts
        XmlBool LoadVBios = XmlBool();             // LoadVBios

        XmlDictField m_fields[6] = {
          {"Model", Model},
          {"IOPCIPrimaryMatch", IOPCIPrimaryMatch},
          {"IOPCISubDevId", IOPCISubDevId},
          {"VRAM", VRAM},
          {"VideoPorts", VideoPorts},
          {"LoadVBios", LoadVBios},
        };

        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
      public:
        const decltype(Model)::ValueType& dgetModel() const { return Model.isDefined() ? Model.value() : Model.nullValue; };
        const decltype(IOPCIPrimaryMatch)::ValueType& dgetId() const { return IOPCIPrimaryMatch.isDefined() ? IOPCIPrimaryMatch.value() : IOPCIPrimaryMatch.nullValue; };
        const decltype(IOPCISubDevId)::ValueType& dgetSubId() const { return IOPCISubDevId.isDefined() ? IOPCISubDevId.value() : IOPCISubDevId.nullValue; };
        decltype(VRAM)::ValueType dgetVideoRam() const { return VRAM.isDefined() ? LShiftU64(VRAM.value(), 20) : VRAM.nullValue; }; //Mb -> bytes
        const decltype(VideoPorts)::ValueType& dgetVideoPorts() const { return VideoPorts.isDefined() ? VideoPorts.value() : VideoPorts.nullValue; };
        const decltype(LoadVBios)::ValueType& dgetLoadVBios() const { return LoadVBios.isDefined() ? LoadVBios.value() : LoadVBios.nullValue; };
        UINT32 dgetSignature() const { return SIGNATURE_32('C','A','R','D'); };

    };

    class Graphics_EDID_Class : public XmlDict
    {
      using super = XmlDict;
    public:
      XmlBool Inject = XmlBool();
      XmlData Custom = XmlData();
      XmlUInt16 VendorID = XmlUInt16();
      XmlUInt16 ProductID = XmlUInt16();
      XmlUInt16 HorizontalSyncPulseWidth = XmlUInt16();
      XmlUInt8 VideoInputSignal = XmlUInt8();

      XmlDictField m_fields[6] = {
        {"Inject", Inject},
        {"Custom", Custom},
        {"VendorID", VendorID},
        {"ProductID", ProductID},
        {"HorizontalSyncPulseWidth", HorizontalSyncPulseWidth},
        {"VideoInputSignal", VideoInputSignal},
      };

      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        RETURN_IF_FALSE( super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) );
        if ( Inject.isDefined() ) {
          if ( Inject.value() ) {
            if ( !Custom.isDefined() ) {
              xmlLiteParser->addWarning(generateErrors, S8Printf("Custom has to be defined if Inject is defined in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
//              return false; // Allow this to be compatible with old way of reading settings. Can be put back when definitive switch is made
            }else{
              if ( Custom.value().size() % 128 != 0 ) {
                xmlLiteParser->addWarning(generateErrors, S8Printf("Custom length must be modulo 128 in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
                Custom.reset();
                return true; // that's what Clover is currently doing.
              }
            }
          }else{
            if ( Custom.isDefined() ) {
              xmlLiteParser->addWarning(generateErrors, S8Printf("Custom is defined and will be ignored because Inject is false in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
              return false;
            }
          }
        }else{
          if ( Custom.isDefined() ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("Custom is defined and will be ignored because Inject isn't defined '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
            return false;
          }
        }
        return true;
      
      };
      const decltype(Inject)::ValueType& dgetInjectEDID() const { return Inject.isDefined() ? Inject.value() : Inject.nullValue; };
      const decltype(Custom)::ValueType& dgetCustomEDID() const { return dgetInjectEDID() && Custom.isDefined() ? Custom.value() : Custom.nullValue; };
      const decltype(VendorID)::ValueType& dgetVendorEDID() const { return dgetInjectEDID() && VendorID.isDefined() ? VendorID.value() : VendorID.nullValue; };
      const decltype(ProductID)::ValueType& dgetProductEDID() const { return dgetInjectEDID() && ProductID.isDefined() ? ProductID.value() : ProductID.nullValue; };
      const decltype(HorizontalSyncPulseWidth)::ValueType& dgetEdidFixHorizontalSyncPulseWidth() const { return dgetInjectEDID() && HorizontalSyncPulseWidth.isDefined() ? HorizontalSyncPulseWidth.value() : HorizontalSyncPulseWidth.nullValue; };
      const decltype(VideoInputSignal)::ValueType& dgetEdidFixVideoInputSignal() const { return dgetInjectEDID() && VideoInputSignal.isDefined() ? VideoInputSignal.value() : VideoInputSignal.nullValue; };

    };
    class XmlInjectUnion;
    class Graphics_Inject_Class : public XmlDict
    {
      using super = XmlDict;
    protected:
      XmlBool Intel = XmlBool();
      XmlBool ATI = XmlBool();
      XmlBool NVidia = XmlBool();

      XmlDictField m_fields[3] = {
        {"Intel", Intel},
        {"ATI", ATI},
        {"NVidia", NVidia},
      };

      virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    protected:
      const decltype(Intel)::ValueType& dgetIntel() const { return Intel.isDefined() ? Intel.value() : Intel.nullValue; };
      const decltype(ATI)::ValueType& dgetATI() const { return ATI.isDefined() ? ATI.value() : ATI.nullValue; };
      const decltype(NVidia)::ValueType& dgetNVidia() const { return NVidia.isDefined() ? NVidia.value() : NVidia.nullValue; };
      friend class XmlInjectUnion;
      bool isInjectIntelDefined() const { return Intel.isDefined(); };
      bool isInjectATIDefined() const { return ATI.isDefined(); };
      bool isInjectNVidiaDefined() const { return NVidia.isDefined(); };
    };

    class XmlInjectUnion: public XmlUnion
    {
      protected:
        XmlBool xmlBool = XmlBool();
        Graphics_Inject_Class xmlDict = Graphics_Inject_Class();
        XmlUnionField m_fields[2] = { xmlBool, xmlDict};
        virtual void getFields(XmlUnionField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
      public:
        bool dgetGraphicsInjector() const {
          if ( xmlBool.isDefined() ) return xmlBool.value();
          return false;
        }
      const decltype(Graphics_Inject_Class::Intel)::ValueType& dgetInjectIntel() const { return xmlBool.isDefined() ? xmlBool.value() : xmlDict.isDefined() ? xmlDict.dgetIntel() : decltype(Graphics_Inject_Class::Intel)::nullValue; };
      const decltype(Graphics_Inject_Class::ATI)::ValueType& dgetInjectATI() const { return xmlBool.isDefined() ? xmlBool.value() : xmlDict.isDefined() ? xmlDict.dgetATI() : decltype(Graphics_Inject_Class::ATI)::nullValue; };
      const decltype(Graphics_Inject_Class::NVidia)::ValueType& dgetInjectNVidia() const { return xmlBool.isDefined() ? xmlBool.value() : xmlDict.isDefined() ? xmlDict.dgetNVidia() : decltype(Graphics_Inject_Class::NVidia)::nullValue; };

      bool isInjectIntelDefined() const { return xmlBool.isDefined() || xmlDict.isInjectIntelDefined(); };
      bool isInjectATIDefined() const { return xmlBool.isDefined() || xmlDict.isInjectATIDefined(); };
      bool isInjectNVidiaDefined() const { return xmlBool.isDefined() || xmlDict.isInjectNVidiaDefined(); };
    };
    
protected:
  XmlBool PatchVBios = XmlBool();
  XmlBool RadeonDeInit = XmlBool();
  XmlUInt64 VRAM = XmlUInt64();
  XmlUInt32 RefCLK = XmlUInt32();
  XmlBool LoadVBios = XmlBool();
  XmlUInt16 VideoPorts = XmlUInt16();
  XmlInt8 BootDisplay = XmlInt8();
  XmlString8AllowEmpty FBName = XmlString8AllowEmpty();
  XmlString8AllowEmpty NVCAP = XmlString8AllowEmpty(); // It's currently an hex sequence without 0x. TODO validation. Only hex char, space and comma (see hex2bin), length < 20
  XmlString8AllowEmpty displayCfg = XmlString8AllowEmpty(); // It's currently an hex sequence without 0x. TODO validation. Only hex char, space and comma, length < 8
  XmlUInt32 DualLink = XmlUInt32();
  XmlBool NvidiaGeneric = XmlBool();
  XmlBool NvidiaNoEFI = XmlBool();
  XmlBool NvidiaSingle = XmlBool();
  XmlUInt32 igPlatformId = XmlUInt32();
  XmlUInt32 snbPlatformId = XmlUInt32();

public:
  XmlInjectUnion Inject = XmlInjectUnion();
  Graphics_EDID_Class EDID = Graphics_EDID_Class();
  XmlArray<Graphics_PatchVBiosBytes_Class> PatchVBiosBytesArray = XmlArray<Graphics_PatchVBiosBytes_Class>();
  XmlArray<Graphics_ATI_NVIDIA_Class> ATI = XmlArray<Graphics_ATI_NVIDIA_Class>();
  XmlArray<Graphics_ATI_NVIDIA_Class> NVIDIA = XmlArray<Graphics_ATI_NVIDIA_Class>();

  XmlDictField m_fields[21] = {
    {"PatchVBios", PatchVBios},
    {"RadeonDeInit", RadeonDeInit},
    {"VRAM", VRAM},
    {"RefCLK", RefCLK},
    {"LoadVBios", LoadVBios},
    {"VideoPorts", VideoPorts},
    {"BootDisplay", BootDisplay},
    {"FBName", FBName},
    {"NVCAP", NVCAP},
    {"display-cfg", displayCfg},
    {"DualLink", DualLink},
    {"NvidiaGeneric", NvidiaGeneric},
    {"NvidiaNoEFI", NvidiaNoEFI},
    {"NvidiaSingle", NvidiaSingle},
    {"ig-platform-id", igPlatformId},
    {"snb-platform-id", snbPlatformId}, // TODO: validate : shpouldn't have both.
    {"Inject", Inject},
    {"EDID", EDID},
    {"PatchVBiosBytes", PatchVBiosBytesArray},
    {"ATI", ATI},
    {"NVIDIA", NVIDIA},
  };

  Graphics_Class(const ConfigPlistClass& _configPlist) : configPlist(_configPlist) {}

  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

  const decltype(PatchVBios)::ValueType& dgetPatchVBios() const { return PatchVBios.isDefined() ? PatchVBios.value() : PatchVBios.nullValue; };
  const decltype(PatchVBiosBytesArray)::ValueType& dgetPatchVBiosBytes() const { return PatchVBiosBytesArray.isDefined() ? PatchVBiosBytesArray.value() : PatchVBiosBytesArray.nullValue; };
  const decltype(RadeonDeInit)::ValueType& dgetRadeonDeInit() const { return RadeonDeInit.isDefined() ? RadeonDeInit.value() : RadeonDeInit.nullValue; };
  const decltype(VRAM)::ValueType& dgetVRAM() const { return VRAM.isDefined() ? VRAM.value() : VRAM.nullValue; };
  const decltype(RefCLK)::ValueType& dgetRefCLK() const { return RefCLK.isDefined() ? RefCLK.value() : RefCLK.nullValue; };
  const decltype(LoadVBios)::ValueType& dgetLoadVBios() const { return LoadVBios.isDefined() ? LoadVBios.value() : LoadVBios.nullValue; };
  const decltype(VideoPorts)::ValueType& dgetVideoPorts() const { return VideoPorts.isDefined() ? VideoPorts.value() : VideoPorts.nullValue; };
  int8_t dgetBootDisplay() const { return isDefined() ? BootDisplay.isDefined() ? BootDisplay.value() : -1 : 0; }; // TODO: different default value if section is not defined
  const decltype(FBName)::ValueType& dgetFBName() const { return FBName.isDefined() ? FBName.value() : FBName.nullValue; };
//  const decltype(displayCfg)::ValueType& dgetdisplayCfg() const { return displayCfg.isDefined() ? displayCfg.value() : displayCfg.nullValue; };
  decltype(DualLink)::ValueType dgetDualLink() const { return DualLink.isDefined() ? DualLink.value() : 0xA; };
  const decltype(NvidiaGeneric)::ValueType& dgetNvidiaGeneric() const { return NvidiaGeneric.isDefined() ? NvidiaGeneric.value() : NvidiaGeneric.nullValue; };
  const decltype(NvidiaNoEFI)::ValueType& dgetNvidiaNoEFI() const { return NvidiaNoEFI.isDefined() ? NvidiaNoEFI.value() : NvidiaNoEFI.nullValue; };
  const decltype(NvidiaSingle)::ValueType& dgetNvidiaSingle() const { return NvidiaSingle.isDefined() ? NvidiaSingle.value() : NvidiaSingle.nullValue; };
  decltype(igPlatformId)::ValueType dget_IgPlatform() const {
    for ( size_t idx = 0 ; idx < configPlist.Devices.Arbitrary.size() ; ++idx ) {
      const ConfigPlistClass::DevicesClass::Devices_Arbitrary_Class& arbitraryProperty = configPlist.Devices.Arbitrary[idx];
      for ( size_t jdx = 0 ; jdx < arbitraryProperty.CustomProperties.size() ; ++jdx ) {
        const ConfigPlistClass::DevicesClass::SimplePropertyClass_Class& customProperty = arbitraryProperty.CustomProperties[jdx];
        if ( customProperty.key.isDefined() && customProperty.key.value().contains("-platform-id") ) {
          XBuffer<uint8_t> buf = customProperty.dgetValue();
          if ( buf.size() == 0 ) return 0;
          decltype(igPlatformId)::ValueType v = 0;
          memcpy(&v, buf.data(), MIN(sizeof(decltype(igPlatformId)::ValueType), buf.size()));
          return v;
        }
      }
    }
    if ( snbPlatformId.isDefined() ) return snbPlatformId.value();
    return igPlatformId.isDefined() ? igPlatformId.value() : igPlatformId.nullValue;
  };
//  const decltype(snbPlatformId)::ValueType& dgetsnbPlatformId() const { return snbPlatformId.isDefined() ? snbPlatformId.value() : snbPlatformId.nullValue; };
  
//  undefinable_bool dgetInjectAsBool() const { return Inject.isDefined() && Inject.xmlBool.isDefined() ? undefinable_bool(Inject.xmlBool.value()) : undefinable_bool(); };
  const XArray<uint8_t> dgetDcfg() const {
    XArray<uint8_t> xbuffer;
    xbuffer.Add(0, 8);
    if ( displayCfg.isDefined() ) {
      hex2bin(displayCfg.value().c_str(), displayCfg.value().length(), xbuffer.data(), 8);
    }else{
      memcpy(xbuffer.data(), default_dcfg_0, 4);
      memcpy(&xbuffer[4], default_dcfg_1, 4);
    }
    return xbuffer;
  };
  const XArray<uint8_t> dgetNVCAP() const {
    XArray<uint8_t> xbuffer;
    xbuffer.Add(0, 20);
    if ( NVCAP.isDefined() ) {
      hex2bin(NVCAP.value().c_str(), NVCAP.value().length(), xbuffer.data(), 20);
    }else{
      memcpy(xbuffer.data(), default_NVCAP, 20);
    }
    return xbuffer;
  };

  const decltype(NVIDIA)::ValueType& dgetNVIDIA() const { return NVIDIA.isDefined() ? NVIDIA.value() : NVIDIA.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_GRAPHICS_H_ */
