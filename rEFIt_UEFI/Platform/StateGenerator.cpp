/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo, 2012 slice
 */

#include "StateGenerator.h"


CONST UINT8 pss_ssdt_header[] =
{
  0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
  0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
  0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
  0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
  0x20, 0x03, 0x12, 0x20							/* 1.._		*/
};


UINT8 cst_ssdt_header[] =
{
  0x53, 0x53, 0x44, 0x54, 0xE7, 0x00, 0x00, 0x00, /* SSDT.... */
  0x01, 0x17, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x41, /* ..PmRefA */
  0x43, 0x70, 0x75, 0x43, 0x73, 0x74, 0x00, 0x00, /* CpuCst.. */
  0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* ....INTL */
  0x20, 0x03, 0x12, 0x20                          /* 1.._		*/
};

UINT8 resource_template_register_fixedhw[] =
{
  0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x7F,
  0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x79, 0x00
};

UINT8 resource_template_register_systemio[] =
{
  0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x01,
  0x08, 0x00, 0x00, 0x15, 0x04, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79, 0x00,
};

UINT8 plugin_type[] =
{
  0x14, 0x22, 0x5F, 0x44, 0x53, 0x4D, 0x04, 0xA0, 
  0x09, 0x93, 0x6A, 0x00, 0xA4, 0x11, 0x03, 0x01,
  0x03, 0xA4, 0x12, 0x10, 0x02, 0x0D, 0x70, 0x6C, 
  0x75, 0x67, 0x69, 0x6E, 0x2D, 0x74, 0x79, 0x70,
  0x65, 0x00,
};

SSDT_TABLE *generate_pss_ssdt(UINTN Number)
{
  CHAR8 name[31];
  CHAR8 name1[31];
  CHAR8 name2[31];
  CHAR8 name3[31];
  P_STATE initial, maximum, minimum, p_states[64];
  UINT8 p_states_count = 0;
  UINT8 cpu_dynamic_fsb = 0;
  UINT8 cpu_noninteger_bus_ratio = 0;
//  UINT32 i, j;
  UINT16 realMax, realMin = 6, realTurbo = 0, Apsn = 0, Aplf = 0;
  
  if (gCPUStructure.Vendor != CPU_VENDOR_INTEL) {
    MsgLog ("Not an Intel platform: P-States will not be generated !!!\n");
    return NULL;
  }
	
  if (!(gCPUStructure.Features & CPUID_FEATURE_MSR)) {
    MsgLog ("Unsupported CPU: P-States will not be generated !!!\n");
    return NULL;
  }

  /*
   APLF: Low Frequency Mode. followed Apple's standards.
   Ironlake-: there are no APLF and APSN. Sandy Bridge: only APSN
   Ivy Bridge: Mobile(U:APLF=0, M:APLF=4), Desktop(APLF=8) and APSN. Haswell+: APLF=0, APSN
   Skylake+: there are no APLF and APSN. maybe because of frequency vectors. but used it as APLF=0 in generator
   Xeon(Westmere EP-): There are no APLF and APSN. Xeon(Sandy Bridge EP): only APSN. Xeon(Ivy Bridge EP+): APLF=0, APSN
   by Sherlocks
   */
  if (gCPUStructure.Model >= CPU_MODEL_IVY_BRIDGE) {
    if (gMobile) {
      switch (gCPUStructure.Model) {
        case CPU_MODEL_IVY_BRIDGE:
          if (AsciiStrStr(gCPUStructure.BrandString, "U")) {
            Aplf = 0;
          } else if (AsciiStrStr(gCPUStructure.BrandString, "M")) {
            Aplf = 4;
          }
          break;
        default:
          Aplf = 0;
          break;
      }
    } else {
      switch (gCPUStructure.Model) {
        case CPU_MODEL_IVY_BRIDGE:
          Aplf = 8;
          break;
	    case CPU_MODEL_IVY_BRIDGE_E5:
		  Aplf = 4;
		  break;
		default:
          Aplf = 0;
          break;
      }
    }
  } else {
    gSettings.GenerateAPLF = FALSE;
  }

  if (Number > 0) {
    // Retrieving P-States, ported from code by superhai (c)
    switch (gCPUStructure.Family) {
      case 0x06:
      {
        switch (gCPUStructure.Model) {
          case CPU_MODEL_DOTHAN:       // Pentium-M
          case CPU_MODEL_CELERON:
          case CPU_MODEL_PENTIUM_M:
          case CPU_MODEL_YONAH:        // Intel Mobile Core Solo, Duo
          case CPU_MODEL_MEROM:        // Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
          case CPU_MODEL_PENRYN:       // Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
          case CPU_MODEL_ATOM:         // Intel Atom (45nm)
          {
            if ((gCPUStructure.Model >= CPU_MODEL_MEROM) && (AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 27))) {
              AsmWriteMsr64(MSR_IA32_EXT_CONFIG, (AsmReadMsr64(MSR_IA32_EXT_CONFIG) | (1 << 28)));
              gBS->Stall(10);
              cpu_dynamic_fsb = (AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 28))?1:0;
              DBG("DynamicFSB: %a\n", cpu_dynamic_fsb?"yes":"no");
            }

            cpu_noninteger_bus_ratio = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1ULL << 46)) != 0)?1:0;
            initial.Control.Control = (UINT16)AsmReadMsr64(MSR_IA32_PERF_STATUS);
            DBG("Initial control=0x%x\n", initial.Control);
						
            maximum.Control.Control = (RShiftU64(AsmReadMsr64(MSR_IA32_PERF_STATUS), 32) & 0x1F3F) | (0x4000 * cpu_noninteger_bus_ratio);
            DBG("Maximum control=0x%x\n", maximum.Control.Control);
            if (gSettings.Turbo) {
              maximum.Control.VID_FID.FID++;
              MsgLog("Turbo FID=0x%x\n", maximum.Control.VID_FID.FID);
            }
            MsgLog("UnderVoltStep=%d\n", gSettings.UnderVoltStep);
            MsgLog("PLimitDict=%d\n", gSettings.PLimitDict);
            maximum.CID = ((maximum.Control.VID_FID.FID & 0x1F) << 1) | cpu_noninteger_bus_ratio;

            minimum.Control.VID_FID.FID = (RShiftU64(AsmReadMsr64(MSR_IA32_PERF_STATUS), 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
            minimum.Control.VID_FID.VID = (RShiftU64(AsmReadMsr64(MSR_IA32_PERF_STATUS), 48) & 0x3F);
						
            if (minimum.Control.VID_FID.FID == 0) {
              minimum.Control.VID_FID.FID = 6;
              minimum.Control.VID_FID.VID = maximum.Control.VID_FID.VID;
            }
						
            minimum.CID = ((minimum.Control.VID_FID.FID & 0x1F) << 1) >> cpu_dynamic_fsb;
						
            // Sanity check
            if (maximum.CID < minimum.CID) {
                DBG("Insane FID values!\n");
                p_states_count = 0;
            } else {
              UINT8 vidstep;
              UINT8 u, invalid = 0;
              // Finalize P-States
              // Find how many P-States machine supports
              p_states_count = (UINT8)(maximum.CID - minimum.CID + 1);

              if (p_states_count > 32)
                p_states_count = 32;
                DBG("PStates count=%d\n", p_states_count);
							
                vidstep = ((maximum.Control.VID_FID.VID << 2) - (minimum.Control.VID_FID.VID << 2)) / (p_states_count - 1);
							
                for (u = 0; u < p_states_count; u++) {
                  UINT8 i = u - invalid;
								
                  p_states[i].CID = maximum.CID - u;
                  p_states[i].Control.VID_FID.FID = (UINT8)(p_states[i].CID >> 1);
								
                  if (p_states[i].Control.VID_FID.FID < 0x6) {
                    if (cpu_dynamic_fsb)
                      p_states[i].Control.VID_FID.FID = (p_states[i].Control.VID_FID.FID << 1) | 0x80;
                  } else if (cpu_noninteger_bus_ratio) {
                    p_states[i].Control.VID_FID.FID = p_states[i].Control.VID_FID.FID | (0x40 * (p_states[i].CID & 0x1));
                  }
								
                  if (i && p_states[i].Control.VID_FID.FID == p_states[i-1].Control.VID_FID.FID)
                    invalid++;
								
                  p_states[i].Control.VID_FID.VID = ((maximum.Control.VID_FID.VID << 2) - (vidstep * u)) >> 2;
                  if (u < p_states_count - 1) {
                    p_states[i].Control.VID_FID.VID -= gSettings.UnderVoltStep;
                  }
                  // Add scope so these don't have to be moved - apianti
                  {
                    UINT32 multiplier = p_states[i].Control.VID_FID.FID & 0x1f;       // = 0x08
                    UINT8 half = (p_states[i].Control.VID_FID.FID & 0x40)?1:0;        // = 0x00
                    UINT8 dfsb = (p_states[i].Control.VID_FID.FID & 0x80)?1:0;        // = 0x01
                    UINT32 fsb = (UINT32)DivU64x32(gCPUStructure.FSBFrequency, Mega); // = 200
                    UINT32 halffsb = (fsb + 1) >> 1;                                  // = 100
                    UINT32 frequency = (multiplier * fsb);                            // = 1600
                  
                    p_states[i].Frequency = (UINT32)(frequency + (half * halffsb)) >> dfsb;	// = 1600/2=800
                  }
                }
              p_states_count -= invalid;
            }
            break;
          }
          case CPU_MODEL_FIELDS:        // Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
          case CPU_MODEL_DALES:
          case CPU_MODEL_CLARKDALE:     // Intel Core i3, i5 LGA1156 (32nm)
          case CPU_MODEL_NEHALEM:       // Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
          case CPU_MODEL_NEHALEM_EX:    // Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
          case CPU_MODEL_WESTMERE:      // Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
          case CPU_MODEL_WESTMERE_EX:   // Intel Xeon E7
          case CPU_MODEL_SANDY_BRIDGE:  // Intel Core i3, i5, i7 LGA1155 (32nm)
          case CPU_MODEL_JAKETOWN:      // Intel Xeon E3
          case CPU_MODEL_ATOM_3700:
          case CPU_MODEL_IVY_BRIDGE:
          case CPU_MODEL_IVY_BRIDGE_E5:
          case CPU_MODEL_HASWELL:
          case CPU_MODEL_HASWELL_E:
          case CPU_MODEL_HASWELL_ULT:
          case CPU_MODEL_CRYSTALWELL:
          case CPU_MODEL_HASWELL_U5:    // Broadwell Mobile
          case CPU_MODEL_BROADWELL_HQ:
          case CPU_MODEL_BROADWELL_E5:
          case CPU_MODEL_BROADWELL_DE:
          case CPU_MODEL_AIRMONT:
          case CPU_MODEL_SKYLAKE_U:
          case CPU_MODEL_SKYLAKE_D:
          case CPU_MODEL_SKYLAKE_S:
          case CPU_MODEL_GOLDMONT:
          case CPU_MODEL_KABYLAKE1:
          case CPU_MODEL_KABYLAKE2:
          case CPU_MODEL_CANNONLAKE:
          {
            maximum.Control.Control = RShiftU64(AsmReadMsr64(MSR_PLATFORM_INFO), 8) & 0xff;
            if (gSettings.MaxMultiplier) {
              DBG("Using custom MaxMultiplier %d instead of automatic %d\n",
              gSettings.MaxMultiplier, maximum.Control.Control);
              maximum.Control.Control = gSettings.MaxMultiplier;
            }

            realMax = maximum.Control.Control;
            DBG("Maximum control=0x%x\n", realMax);
            if (gSettings.Turbo) {
              realTurbo = (gCPUStructure.Turbo4 > gCPUStructure.Turbo1) ?
              (gCPUStructure.Turbo4 / 10) : (gCPUStructure.Turbo1 / 10);
              maximum.Control.Control = realTurbo;
              MsgLog("Turbo control=0x%x\n", realTurbo);
            }
            Apsn = (realTurbo > realMax)?(realTurbo - realMax):0;
            realMin =  RShiftU64(AsmReadMsr64(MSR_PLATFORM_INFO), 40) & 0xff;
            if (gSettings.MinMultiplier) {
              minimum.Control.Control = gSettings.MinMultiplier;
              Aplf = (realMin > minimum.Control.Control)?(realMin - minimum.Control.Control):0;
            } else {
              minimum.Control.Control = realMin;
            }

            MsgLog("P-States: min 0x%x, max 0x%x\n", minimum.Control.Control, maximum.Control.Control);

            // Sanity check
            if (maximum.Control.Control < minimum.Control.Control) {
              DBG("Insane control values!");
              p_states_count = 0;
            } else {
              p_states_count = 0;
							
							/*
							 * Careful with downward loop, with UINT as index !
							 * This is wrong :
							 *   for (i = maximum.Control.Control; i >= minimum.Control.Control; i--) {
							 */
              for (UINTN i = maximum.Control.Control+1; i-- > minimum.Control.Control; ) {
                UINTN j = i;
                if ((gCPUStructure.Model == CPU_MODEL_SANDY_BRIDGE) ||
                    (gCPUStructure.Model == CPU_MODEL_JAKETOWN) ||
                    (gCPUStructure.Model == CPU_MODEL_ATOM_3700) ||
                    (gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE) ||
                    (gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE_E5) ||
                    (gCPUStructure.Model == CPU_MODEL_HASWELL) ||
                    (gCPUStructure.Model == CPU_MODEL_HASWELL_E) ||
                    (gCPUStructure.Model == CPU_MODEL_HASWELL_ULT) ||
                    (gCPUStructure.Model == CPU_MODEL_CRYSTALWELL) ||
                    (gCPUStructure.Model == CPU_MODEL_HASWELL_U5) ||   // Broadwell Mobile
                    (gCPUStructure.Model == CPU_MODEL_BROADWELL_HQ) ||
                    (gCPUStructure.Model == CPU_MODEL_BROADWELL_E5) ||
                    (gCPUStructure.Model == CPU_MODEL_BROADWELL_DE) ||
                    (gCPUStructure.Model == CPU_MODEL_AIRMONT) ||
                    (gCPUStructure.Model == CPU_MODEL_SKYLAKE_U) ||
                    (gCPUStructure.Model == CPU_MODEL_SKYLAKE_D) ||
                    (gCPUStructure.Model == CPU_MODEL_SKYLAKE_S) ||
                    (gCPUStructure.Model == CPU_MODEL_GOLDMONT) ||
                    (gCPUStructure.Model == CPU_MODEL_KABYLAKE1) ||
                    (gCPUStructure.Model == CPU_MODEL_KABYLAKE2) ||
                    (gCPUStructure.Model == CPU_MODEL_CANNONLAKE)) {
                    j = i << 8;
                    p_states[p_states_count].Frequency = (UINT32)(100 * i);
                } else {
                  p_states[p_states_count].Frequency = (UINT32)(DivU64x32(MultU64x32(gCPUStructure.FSBFrequency, i), Mega));
                }
                p_states[p_states_count].Control.Control = (UINT16)j;
                p_states[p_states_count].CID = j;
                
                if (!p_states_count && gSettings.DoubleFirstState) {
                  //double first state
                  p_states_count++;
                  p_states[p_states_count].Control.Control = (UINT16)j;
                  p_states[p_states_count].CID = j;
                  p_states[p_states_count].Frequency = (UINT32)(DivU64x32(MultU64x32(gCPUStructure.FSBFrequency, i), Mega)) - 1;
                }
                p_states_count++;
              }
            }
            break;
          }
          default:
            MsgLog ("Unsupported CPU (0x%X): P-States not generated !!!\n", gCPUStructure.Family);
            break;
        }
      }
    }

    // Generating SSDT
    if (p_states_count > 0) {
      INTN TDPdiv;
      SSDT_TABLE *ssdt;
      AML_CHUNK* scop;
      AML_CHUNK* method;
      AML_CHUNK* pack;
      AML_CHUNK* metPSS;
      AML_CHUNK* metPPC;
      AML_CHUNK* namePCT;
      AML_CHUNK* packPCT;
      AML_CHUNK* metPCT;
      AML_CHUNK* root = aml_create_node(NULL);
      aml_add_buffer(root, (UINT8*)&pss_ssdt_header[0], sizeof(pss_ssdt_header)); // SSDT header
      AsciiSPrint(name, 31, "%a%4a", acpi_cpu_score, acpi_cpu_name[0]);
      AsciiSPrint(name1, 31, "%a%4aPSS_", acpi_cpu_score, acpi_cpu_name[0]);
      AsciiSPrint(name2, 31, "%a%4aPCT_", acpi_cpu_score, acpi_cpu_name[0]);
      AsciiSPrint(name3, 31, "%a%4a_PPC", acpi_cpu_score, acpi_cpu_name[0]);

      scop = aml_add_scope(root, name);
      
      if (gSettings.GeneratePStates && !gSettings.HWP) {
        method = aml_add_name(scop, "PSS_");
        pack = aml_add_package(method);

        if ((gSettings.TDP != 0) && (p_states[0].Frequency != 0)) {
          TDPdiv = (gSettings.TDP * 1000) / p_states[0].Frequency;
        } else {
          TDPdiv = 8;
        }
        
        for (decltype(p_states_count) i = gSettings.PLimitDict; i < p_states_count; i++) {
          AML_CHUNK* pstt = aml_add_package(pack);

          aml_add_dword(pstt, p_states[i].Frequency);
          if (p_states[i].Control.Control < realMin) {
            aml_add_dword(pstt, 0); //zero for power
          } else {
            aml_add_dword(pstt, (UINT32)(p_states[i].Frequency * TDPdiv)); // Designed Power
          }
          aml_add_dword(pstt, 0x0000000A); // Latency
          aml_add_dword(pstt, 0x0000000A); // Latency
          aml_add_dword(pstt, p_states[i].Control.Control);
          aml_add_dword(pstt, p_states[i].Control.Control); // Status
        }
        metPSS = aml_add_method(scop, "_PSS", 0);
        aml_add_return_name(metPSS, "PSS_");
        //metPSS = aml_add_method(scop, "APSS", 0);
        //aml_add_return_name(metPSS, "PSS_");
        //metPPC = aml_add_method(scop, "_PPC", 0);
        aml_add_name(scop, "_PPC");
        aml_add_byte(scop, (UINT8)gSettings.PLimitDict);
        //aml_add_return_byte(metPPC, gSettings.PLimitDict);
        namePCT = aml_add_name(scop, "PCT_");
        packPCT = aml_add_package(namePCT);
        resource_template_register_fixedhw[8] = 0x00;
        resource_template_register_fixedhw[9] = 0x00;
        resource_template_register_fixedhw[18] = 0x00;
        aml_add_buffer(packPCT, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
        aml_add_buffer(packPCT, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
        metPCT = aml_add_method(scop, "_PCT", 0);
        aml_add_return_name(metPCT, "PCT_");
        if (gSettings.PluginType && gSettings.GeneratePluginType) {
          aml_add_buffer(scop, plugin_type, sizeof(plugin_type));
          aml_add_byte(scop, gSettings.PluginType);
        }
        if (gCPUStructure.Family >= 2) {
          if (gSettings.GenerateAPSN) {
            //APSN: High Frequency Modes (turbo)
            aml_add_name(scop, "APSN");
            aml_add_byte(scop, (UINT8)Apsn);
          }
          if (gSettings.GenerateAPLF) {
            //APLF: Low Frequency Mode
            aml_add_name(scop, "APLF");
            aml_add_byte(scop, (UINT8)Aplf);
          }
        }

        // Add CPUs
        for (decltype(Number) i = 1; i < Number; i++) {
          AsciiSPrint(name, 31, "%a%4a", acpi_cpu_score, acpi_cpu_name[i]);
          scop = aml_add_scope(root, name);
          metPSS = aml_add_method(scop, "_PSS", 0);
          aml_add_return_name(metPSS, name1);
          //metPSS = aml_add_method(scop, "APSS", 0);
          //aml_add_return_name(metPSS, name1);
          metPPC = aml_add_method(scop, "_PPC", 0);
          aml_add_return_name(metPPC, name3);
          //aml_add_return_byte(metPPC, gSettings.PLimitDict);
          metPCT = aml_add_method(scop, "_PCT", 0);
          aml_add_return_name(metPCT, name2);
        }
      } else if (gSettings.PluginType && gSettings.GeneratePluginType) {
        aml_add_buffer(scop, plugin_type, sizeof(plugin_type));
        aml_add_byte(scop, gSettings.PluginType);
      }

      aml_calculate_size(root);

      ssdt = (SSDT_TABLE *)AllocateZeroPool(root->Size);
      aml_write_node(root, (CHAR8*)ssdt, 0);
      ssdt->Length = root->Size;
      FixChecksum(ssdt);
      //ssdt->Checksum = 0;
      //ssdt->Checksum = (UINT8)(256 - Checksum8(ssdt, ssdt->Length));

      aml_destroy_node(root);

      if (gSettings.GeneratePStates && !gSettings.HWP) {
        if (gSettings.PluginType && gSettings.GeneratePluginType) {
          MsgLog ("SSDT with CPU P-States and plugin-type generated successfully\n");
        } else {
          MsgLog ("SSDT with CPU P-States generated successfully\n");
        }
      } else {
        MsgLog ("SSDT with plugin-type without P-States is generated\n");
      }

      return ssdt;
    }
  } else {
    MsgLog ("ACPI CPUs not found: P-States not generated !!!\n");
  }

  return NULL;
}

SSDT_TABLE *generate_cst_ssdt(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt, UINTN Number)
{
  BOOLEAN c2_enabled = gSettings.EnableC2;
  BOOLEAN c3_enabled;
  BOOLEAN c4_enabled = gSettings.EnableC4;
//  BOOLEAN c6_enabled = gSettings.EnableC6;
  BOOLEAN cst_using_systemio = gSettings.EnableISS;
  UINT8   p_blk_lo, p_blk_hi;
  UINT8   cstates_count;
  UINT32  acpi_cpu_p_blk;
  CHAR8 name2[31];
  CHAR8 name0[31];
  CHAR8 name1[31];
  AML_CHUNK* root;
  AML_CHUNK* scop;
  AML_CHUNK* name;
  AML_CHUNK* pack;
  AML_CHUNK* tmpl;
  AML_CHUNK* met;
//  AML_CHUNK* ret;
  UINTN i;
  SSDT_TABLE *ssdt;
  
  if (!fadt) {
    return NULL;
  }
  
  acpi_cpu_p_blk = fadt->Pm1aEvtBlk + 0x10;
  c2_enabled = c2_enabled || (fadt->PLvl2Lat < 100);
  c3_enabled = (fadt->PLvl3Lat < 1000);
  cstates_count = 1 + (c2_enabled ? 1 : 0) + ((c3_enabled || c4_enabled)? 1 : 0)
                  + (gSettings.EnableC6 ? 1 : 0) + (gSettings.EnableC7 ? 1 : 0);
  
  root = aml_create_node(NULL);
  aml_add_buffer(root, cst_ssdt_header, sizeof(cst_ssdt_header)); // SSDT header
  AsciiSPrint(name0, 31, "%a%4a", acpi_cpu_score, acpi_cpu_name[0]);
  AsciiSPrint(name1, 31, "%a%4aCST_",  acpi_cpu_score, acpi_cpu_name[0]);
  scop = aml_add_scope(root, name0);
  name = aml_add_name(scop, "CST_");
  pack = aml_add_package(name);
  aml_add_byte(pack, cstates_count);
  
  tmpl = aml_add_package(pack);
  if (cst_using_systemio) {    // C1
    resource_template_register_fixedhw[8] = 0x00;
    resource_template_register_fixedhw[9] = 0x00;
    resource_template_register_fixedhw[0x12] = 0x00;
    aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
    aml_add_byte(tmpl, 0x01); // C1
    aml_add_word(tmpl, 0x0001); // Latency
    aml_add_dword(tmpl, 0x000003e8); // Power
    
    if (c2_enabled)  {        // C2
      p_blk_lo = (UINT8)(acpi_cpu_p_blk + 4);
      p_blk_hi = (UINT8)((acpi_cpu_p_blk + 4) >> 8);
      
      tmpl = aml_add_package(pack);
      resource_template_register_systemio[11] = p_blk_lo; // C2
      resource_template_register_systemio[12] = p_blk_hi; // C2
      aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
      aml_add_byte(tmpl, 0x02); // C2
      aml_add_word(tmpl, 0x0040); // Latency
      aml_add_dword(tmpl, 0x000001f4); // Power
    }
    
    if (c4_enabled) {         // C4
      p_blk_lo = (acpi_cpu_p_blk + 6) & 0xff;
      p_blk_hi = (UINT8)((acpi_cpu_p_blk + 6) >> 8);
      
      tmpl = aml_add_package(pack);
      resource_template_register_systemio[11] = p_blk_lo; // C4
      resource_template_register_systemio[12] = p_blk_hi; // C4
      aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
      aml_add_byte(tmpl, 0x04); // C4
      aml_add_word(tmpl, 0x0080); // Latency
      aml_add_dword(tmpl, 0x000000C8); // Power
    }
    else if (c3_enabled) {      // C3
      p_blk_lo = (UINT8)(acpi_cpu_p_blk + 5);
      p_blk_hi = (UINT8)((acpi_cpu_p_blk + 5) >> 8);
      
      tmpl = aml_add_package(pack);
      resource_template_register_systemio[11] = p_blk_lo; // C3
      resource_template_register_systemio[12] = p_blk_hi; // C3
      aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
      aml_add_byte(tmpl, 0x03);			// C3
      aml_add_word(tmpl, gSettings.C3Latency);			// Latency
      aml_add_dword(tmpl, 0x000001F4);	// Power
    }
    if (gSettings.EnableC6) {       // C6
      p_blk_lo = (UINT8)(acpi_cpu_p_blk + 5);
      p_blk_hi = (UINT8)((acpi_cpu_p_blk + 5) >> 8);
      
      tmpl = aml_add_package(pack);
      resource_template_register_systemio[11] = p_blk_lo; // C6
      resource_template_register_systemio[12] = p_blk_hi; // C6
      aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
      aml_add_byte(tmpl, 0x06);			// C6
      aml_add_word(tmpl, gSettings.C3Latency + 3);			// Latency
      aml_add_dword(tmpl, 0x0000015E);	// Power
    }
    if (gSettings.EnableC7) {       //C7
      p_blk_lo = (acpi_cpu_p_blk + 6) & 0xff;
      p_blk_hi = (UINT8)((acpi_cpu_p_blk + 6) >> 8);
      
      tmpl = aml_add_package(pack);
      resource_template_register_systemio[11] = p_blk_lo; // C4 or C7
      resource_template_register_systemio[12] = p_blk_hi; 
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x07);			// C7
      aml_add_word(tmpl, 0xF5);			// Latency as in iMac14,1
      aml_add_dword(tmpl, 0xC8);	// Power
    }
  } else {
    // C1
    resource_template_register_fixedhw[8] = 0x01;
    resource_template_register_fixedhw[9] = 0x02;
//    resource_template_register_fixedhw[18] = 0x01;
    resource_template_register_fixedhw[10] = 0x01;
    
    resource_template_register_fixedhw[11] = 0x00; // C1
    
    aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
    aml_add_byte(tmpl, 0x01);			// C1
    aml_add_word(tmpl, 0x0001);			// Latency
    aml_add_dword(tmpl, 0x000003e8);	// Power
    
//    resource_template_register_fixedhw[18] = 0x03;
    resource_template_register_fixedhw[10] = 0x03;
    
    if (c2_enabled) {         // C2
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x10; // C2
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x02);			// C2
      aml_add_word(tmpl, 0x0040);			// Latency
      aml_add_dword(tmpl, 0x000001f4);	// Power
    }
    
    if (c4_enabled) {         // C4
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x30; // C4
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x04);			// C4
      aml_add_word(tmpl, 0x0080);			// Latency
      aml_add_dword(tmpl, 0x000000C8);	// Power
    }
    else if (c3_enabled) {
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x20; // C3
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x03);			// C3
      aml_add_word(tmpl, gSettings.C3Latency);			// Latency as in MacPro6,1 = 0x0043
      aml_add_dword(tmpl, 0x000001F4);	// Power
    }
    if (gSettings.EnableC6) {     // C6
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x20; // C6
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x06);			// C6
      aml_add_word(tmpl, gSettings.C3Latency + 3);			// Latency as in MacPro6,1 = 0x0046
      aml_add_dword(tmpl, 0x0000015E);	// Power
    }
    if (gSettings.EnableC7) {
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x30; // C4 or C7
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x07);			// C7
      aml_add_word(tmpl, 0xF5);			// Latency as in iMac14,1 
      aml_add_dword(tmpl, 0xC8);	// Power      
    }
  }
  met = aml_add_method(scop, "_CST", 0);
  aml_add_return_name(met, "CST_");
//  met = aml_add_method(scop, "ACST", 0);
//  ret = aml_add_return_name(met, "CST_");

  // Aliases
  for (i = 1; i < Number; i++) {
    AsciiSPrint(name2, 31, "%a%4a",  acpi_cpu_score, acpi_cpu_name[i]);
    
    scop = aml_add_scope(root, name2);
    met = aml_add_method(scop, "_CST", 0);
    aml_add_return_name(met, name1);
//    met = aml_add_method(scop, "ACST", 0);
//    ret = aml_add_return_name(met, name1);

  }
  
  aml_calculate_size(root);
  
  ssdt = (SSDT_TABLE *)AllocateZeroPool(root->Size);
  
  aml_write_node(root, (CHAR8*)ssdt, 0);
  
  ssdt->Length = root->Size;
  FixChecksum(ssdt);
//  ssdt->Checksum = 0;
//  ssdt->Checksum = (UINT8)(256 - Checksum8((VOID*)ssdt, ssdt->Length));
  
  aml_destroy_node(root);
  
  //dumpPhysAddr("C-States SSDT content: ", ssdt, ssdt->Length);
  
  MsgLog ("SSDT with CPU C-States generated successfully\n");
  
  return ssdt;
}
