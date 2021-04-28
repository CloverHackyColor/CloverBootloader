/*
 * ConfigPlist.h
 *
 *  Created on: Oct 9, 2020
 *      Author: jief
 */

#ifndef _CONFIGPLISTCLASS_KERNELANDKEXTPATCHES_H_
#define _CONFIGPLISTCLASS_KERNELANDKEXTPATCHES_H_



class KernelAndKextPatches_Class : public XmlDict
{
  using super = XmlDict;
public:


    class KernelAndKextPatches_AbstractPatch_Class : public XmlDict
    {
        using super = XmlDict;
      public:
        XmlBool Disabled = XmlBool();
        XmlString8AllowEmpty Comment = XmlString8AllowEmpty();
        XmlData Find = XmlData(); // ABSTRACT_PATCH.Data
        XmlData Replace = XmlData(); // ABSTRACT_PATCH.Patch
        XmlData MaskFind = XmlData();
        XmlData MaskReplace = XmlData();
        XmlData StartPattern = XmlData();
        XmlData MaskStart = XmlData(); // ABSTRACT_PATCH.StartMask
        XmlInt64 RangeFind = XmlInt64();
        XmlInt64 Skip = XmlInt64();
        XmlInt64 Count = XmlInt64();
        XmlString8AllowEmpty MatchOS = XmlString8AllowEmpty(); // validation ?
        XmlString8AllowEmpty MatchBuild = XmlString8AllowEmpty(); // validation ?

      //  XmlDictField m_fields[13] = {
      //    {"Comment", Comment},
      //    {"Disabled", Disabled},
      //    {"RangeFind", RangeFind},
      //    {"Skip", Skip},
      //    {"StartPattern", StartPattern},
      //    {"MaskStart", MaskStart},
      //    {"Find", Find},
      //    {"Replace", Replace},
      //    {"MaskFind", MaskFind},
      //    {"MaskReplace", MaskReplace},
      //    {"Count", Count},
      //    {"MatchOS", MatchOS},
      //    {"MatchBuild", MatchBuild},
      //  };

      public:
      //  virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
        virtual void getFields(XmlDictField** fields, size_t* nb) override { panic("BUG: This cannot be called"); };

        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
          bool b = true;
          // TODO after switch to new parser : name.isEmpty()
          if ( !Find.isDefined() || Find.value().size() == 0 ) return xmlLiteParser->addWarning(generateErrors, S8Printf("Find has to be defined in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
          if ( !Replace.isDefined() || Replace.value().size() == 0 ) return xmlLiteParser->addWarning(generateErrors, S8Printf("One of Find or Replace has to be defined in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
          if ( Replace.value().size() > Find.value().size() ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("Replace is longer (%zu) than Find (%zu) and will be truncated in dict '%s:%d'.", Replace.value().size(), Find.value().size(), xmlPath.c_str(), keyPos.getLine()));
            Replace.modifiableValue().setSize(Find.value().size(), 0);
          }
          return b;
        }
        
        bool dgetDisabled() const { return Disabled.isDefined() ? Disabled.value() : false; };
        const XBuffer<UINT8>& dgetFind() const { return Find.isDefined() ? Find.value() : XBuffer<UINT8>::NullXBuffer; };
        const XBuffer<UINT8>& dgetReplace() const { return Replace.isDefined() ? Replace.value() : XBuffer<UINT8>::NullXBuffer; };
        XBuffer<UINT8> dgetMaskFind() const
        {
          if ( !Find.isDefined() || Find.value().size() == 0 ) return MaskReplace.nullValue;
          if ( !MaskFind.isDefined() || MaskFind.value().size() == 0 ) return MaskReplace.nullValue;
          XBuffer<UINT8> returnValue;
          size_t findSize = dgetFind().size();
          returnValue.ncpy(MaskFind.value().data(), MIN(findSize, MaskFind.value().size()));
          returnValue.setSize(findSize, 0xFF);
          return returnValue;
        };

        XBuffer<UINT8> dgetMaskReplace() const
        {
          if ( !Find.isDefined() || Find.value().size() == 0 ) return MaskReplace.nullValue;
          if ( !Replace.isDefined() || Replace.value().size() == 0 ) return MaskReplace.nullValue;
          if ( !MaskReplace.isDefined() || MaskReplace.value().size() == 0 ) return MaskReplace.nullValue;
          XBuffer<UINT8> returnValue;
          size_t findSize = dgetFind().size();
          returnValue.ncpy(MaskReplace.value().data(), MIN(findSize, MaskReplace.value().size()));
          returnValue.setSize(findSize, 0);
          return returnValue;
        };
        
        const XBuffer<UINT8>& dgetStartPattern() const { return StartPattern.isDefined() ? StartPattern.value() : XBuffer<UINT8>::NullXBuffer; };
        XBuffer<UINT8> dgetStartMask() const
        {
          if ( !StartPattern.isDefined() || StartPattern.value().size() == 0 ) return MaskReplace.nullValue;
//          if ( !MaskStart.isDefined() || MaskStart.value().size() == 0 ) return MaskReplace.nullValue;
          XBuffer<UINT8> returnValue;
          size_t findSize = StartPattern.value().size();
          if ( MaskStart.isDefined() ) {
            returnValue.ncpy(MaskStart.value().data(), MIN(findSize, MaskStart.value().size()));
          }
          returnValue.setSize(findSize, 0xFF);
          return returnValue;
        };
        INTN dgetSearchLen() const { return RangeFind.isDefined() ? RangeFind.value() : false; };
        virtual INTN dgetCount() const { return Count.isDefined() ? Count.value() : 0; };
        INTN dgetSkip() const { return Skip.isDefined() ? Skip.value() : false; };
        const XString8& dgetMatchOS() const { return MatchOS.isDefined() ? MatchOS.value() : NullXString8; };
        const XString8& dgetMatchBuild() const { return MatchBuild.isDefined() ? MatchBuild.value() : NullXString8; };

        virtual XString8 dgetLabel() const = 0;

    };

    class ABSTRACT_KEXT_OR_KERNEL_PATCH : public KernelAndKextPatches_AbstractPatch_Class
    {
      public:
        XmlString8AllowEmpty Procedure = XmlString8AllowEmpty();
        const XString8& dgetProcedureName() const { return Procedure.isDefined() ? Procedure.value() : NullXString8; };
    };

    class KernelAndKextPatches_KextsToPatch_Class : public ABSTRACT_KEXT_OR_KERNEL_PATCH
    {
        using super = KernelAndKextPatches_AbstractPatch_Class;
      public:
        XmlString8AllowEmpty Name = XmlString8AllowEmpty();
        XmlBool InfoPlistPatch = XmlBool();

        XmlDictField m_fields[16] = {
          {"Name", Name},
          {"Comment", Comment},
          {"Disabled", Disabled},
          {"RangeFind", RangeFind},
          {"Skip", Skip},
          {"StartPattern", StartPattern},
          {"MaskStart", MaskStart},
          {"Find", Find},
          {"Replace", Replace},
          {"Procedure", Procedure},
          {"MaskFind", MaskFind},
          {"MaskReplace", MaskReplace},
          {"Count", Count},
          {"MatchOS", MatchOS},
          {"MatchBuild", MatchBuild},
          {"InfoPlistPatch", InfoPlistPatch},
        };

      public:
        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
          bool b = true;
          b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
          if ( !Name.isDefined() ) {
            b = xmlLiteParser->addWarning(generateErrors, S8Printf("Kernel patch is ignored because 'Name' is not defined in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
          }
          // TODO after switch to new parser : name.isEmpty()
          return b;
        }
        
        const XString8& dgetName() const { return Name.isDefined() ? Name.value() : NullXString8; };
        XString8 dgetLabel() const override { return Comment.isDefined() ? S8Printf("%s (%s)", Name.value().c_str(), Comment.value().c_str()) : S8Printf("%s (NoLabel)", Name.value().c_str()); };
        bool dgetIsPlistPatch() const { return InfoPlistPatch.isDefined() ? InfoPlistPatch.value() : false; };

        // override because of different defaultvalue
        virtual INTN dgetCount() const override { return Count.isDefined() ? Count.value() : 1; };

    };

    class KernelAndKextPatches_KernelToPatch_Class : public ABSTRACT_KEXT_OR_KERNEL_PATCH
    {
        using super = KernelAndKextPatches_AbstractPatch_Class;
      public:

        XmlDictField m_fields[14] = {
          {"Comment", Comment},
          {"Disabled", Disabled},
          {"RangeFind", RangeFind},
          {"Skip", Skip},
          {"StartPattern", StartPattern},
          {"MaskStart", MaskStart},
          {"Find", Find},
          {"Replace", Replace},
          {"Procedure", Procedure},
          {"MaskFind", MaskFind},
          {"MaskReplace", MaskReplace},
          {"Count", Count},
          {"MatchOS", MatchOS},
          {"MatchBuild", MatchBuild},
        };

      public:
        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          bool b = true;
          b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
          return b;
        }
        XString8 dgetName() const { return "kernel"_XS8; };
        XString8 dgetLabel() const override { return Comment.isDefined() ? Comment.value() : "NoLabel"_XS8; };
        const XString8& dgetProcedureName() const { return Procedure.isDefined() ? Procedure.value() : NullXString8; };
    };

    class KernelAndKextPatches_BootPatch_Class : public KernelAndKextPatches_AbstractPatch_Class
    {
        using super = KernelAndKextPatches_AbstractPatch_Class;
      public:

        XmlDictField m_fields[13] = {
          {"Comment", Comment},
          {"Disabled", Disabled},
          {"RangeFind", RangeFind},
          {"Skip", Skip},
          {"StartPattern", StartPattern},
          {"MaskStart", MaskStart},
          {"Find", Find},
          {"Replace", Replace},
          {"MaskFind", MaskFind},
          {"MaskReplace", MaskReplace},
          {"Count", Count},
          {"MatchOS", MatchOS},
          {"MatchBuild", MatchBuild},
        };

      public:
        virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };

        virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
          bool b = true;
          b = super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors);
          return b;
        }
        XString8 dgetName() const { return "boot.efi"_XS8; };
        XString8 dgetLabel() const override { return Comment.isDefined() ? Comment.value() : "NoLabel"_XS8; };
    };
    
    
    
    class ForceKextsToLoadClass : public XmlStringWArray
    {
      using super = XmlStringWArray;
      virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
        if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
        for ( size_t idx = 0 ; idx < value().size() ; ) {
          if ( value()[idx] == "\\"_XS8 ) {
            xmlLiteParser->addWarning(generateErrors, S8Printf("String cannot be '\\' for tag '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
            value().removeAtPos(idx);
          }else{
            ++idx;
          }
        }
        return true;
      }
    };
    
    XmlBool Debug = XmlBool();
    XmlBool KernelLapic = XmlBool();
    XmlBool KernelXCPM = XmlBool();
    XmlBool KernelPm = XmlBool();
    XmlBool PanicNoKextDump = XmlBool();
    XmlBool AppleIntelCPUPM = XmlBool();
    XmlBool AppleRTC = XmlBool();
    XmlBool EightApple = XmlBool();
    XmlBool DellSMBIOSPatch = XmlBool();
    XmlUInt32 FakeCPUID = XmlUInt32();
    XmlString8AllowEmpty ATIConnectorsController = XmlString8AllowEmpty();
    XmlData ATIConnectorsData = XmlData();
    XmlData ATIConnectorsPatch = XmlData();
    ForceKextsToLoadClass ForceKextsToLoad = ForceKextsToLoadClass();
    XmlArray<KernelAndKextPatches_KextsToPatch_Class> KextsToPatch = XmlArray<KernelAndKextPatches_KextsToPatch_Class>();
    XmlArray<KernelAndKextPatches_KernelToPatch_Class> KernelToPatch = XmlArray<KernelAndKextPatches_KernelToPatch_Class>();
    XmlArray<KernelAndKextPatches_BootPatch_Class> BootPatches = XmlArray<KernelAndKextPatches_BootPatch_Class>();

    XmlDictField m_fields[17] = {
      {"Debug", Debug},
      {"KernelLapic", KernelLapic},
      {"KernelXCPM", KernelXCPM},
      {"KernelPm", KernelPm},
      {"PanicNoKextDump", PanicNoKextDump},
      {"AppleIntelCPUPM", AppleIntelCPUPM},
      {"AppleRTC", AppleRTC},
      {"EightApple", EightApple},
      {"DellSMBIOSPatch", DellSMBIOSPatch},
      {"FakeCPUID", FakeCPUID},
      {"ATIConnectorsController", ATIConnectorsController},
      {"ATIConnectorsData", ATIConnectorsData},
      {"ATIConnectorsPatch", ATIConnectorsPatch},
      {"ForceKextsToLoad", ForceKextsToLoad},
      {"KextsToPatch", KextsToPatch},
      {"KernelToPatch", KernelToPatch},
      {"BootPatches", BootPatches},
    };

    virtual void getFields(XmlDictField** fields, size_t* nb) override { *fields = m_fields; *nb = sizeof(m_fields)/sizeof(m_fields[0]); };
    
    virtual bool validate(XmlLiteParser* xmlLiteParser, const XString8& xmlPath, const XmlParserPosition& keyPos, bool generateErrors) override {
      if ( !super::validate(xmlLiteParser, xmlPath, keyPos, generateErrors) ) return false;
      bool b = true;
      if ( !ATIConnectorsController.isDefined() ) {
        if ( ATIConnectorsData.isDefined() ) {
          b = xmlLiteParser->addWarning(generateErrors, S8Printf("ATIConnectorsData is ignored because ATIConnectorsController is not defined in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        }
        if ( ATIConnectorsPatch.isDefined() ) {
          b = xmlLiteParser->addWarning(generateErrors, S8Printf("ATIConnectorsPatch is ignored because ATIConnectorsController is not defined in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        }
      }else{
        // ATIConnectorsController is defined
        if ( !ATIConnectorsPatch.isDefined() || ATIConnectorsPatch.value().isEmpty() ) {
          b = xmlLiteParser->addWarning(generateErrors, S8Printf("ATIConnectorsController is ignored because ATIConnectorsPatch is not defined or empty in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        }
        if ( !ATIConnectorsData.isDefined() || ATIConnectorsData.value().isEmpty() ) {
          b = xmlLiteParser->addWarning(generateErrors, S8Printf("ATIConnectorsController is ignored because ATIConnectorsData is not defined or empty in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        }
        if ( b  &&  ATIConnectorsPatch.value().size() != ATIConnectorsData.value().size() ) {
          b = xmlLiteParser->addWarning(generateErrors, S8Printf("ATIConnectorsPatch length != ATIConnectorsData length in dict '%s:%d'.", xmlPath.c_str(), keyPos.getLine()));
        }
      }
      if ( !b ) {
        ATIConnectorsController.reset();
        ATIConnectorsData.reset();
        ATIConnectorsPatch.reset();
      }
      return true;
    }

    bool dgetKPDebug() const { return Debug.isDefined() ? Debug.value() : false; };
    bool dgetKPKernelLapic() const { return KernelLapic.isDefined() ? KernelLapic.value() : false; };
    bool dgetKPKernelXCPM() const { return KernelXCPM.isDefined() ? KernelXCPM.value() : false; };
    bool dget_KPKernelPm() const { return KernelPm.isDefined() ? KernelPm.value() : false; };
    bool dgetKPPanicNoKextDump() const { return PanicNoKextDump.isDefined() ? PanicNoKextDump.value() : false; };
    bool dget_KPAppleIntelCPUPM() const { return AppleIntelCPUPM.isDefined() ? AppleIntelCPUPM.value() : false; };
    bool dgetKPAppleRTC() const { return AppleRTC.isDefined() ? AppleRTC.value() : true; };
    bool dgetEightApple() const { return EightApple.isDefined() ? EightApple.value() : false; };
    bool dgetKPDELLSMBIOS() const { return DellSMBIOSPatch.isDefined() ? DellSMBIOSPatch.value() : false; };
    uint32_t dgetFakeCPUID() const { return FakeCPUID.isDefined() ? FakeCPUID.value() : false; };
    const XString8& dgetKPATIConnectorsController() const { return ATIConnectorsController.isDefined() ? ATIConnectorsController.value() : NullXString8; };
    const XBuffer<UINT8>& dgetKPATIConnectorsData() const { return ATIConnectorsData.isDefined() ? ATIConnectorsData.value() : XBuffer<UINT8>::NullXBuffer; };
    const XBuffer<UINT8>& dgetKPATIConnectorsPatch() const { return ATIConnectorsPatch.isDefined() ? ATIConnectorsPatch.value() : XBuffer<UINT8>::NullXBuffer; };
    const decltype(ForceKextsToLoad)::ValueType& dgetForceKextsToLoad() const { return ForceKextsToLoad.isDefined() ? ForceKextsToLoad.value() : ForceKextsToLoad.nullValue; };

};


#endif /* _CONFIGPLISTCLASS_KERNELANDKEXTPATCHES_H_ */
