/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo, 2012 slice
 */

#include "StateGenerator.h"


//CHAR8* acpi_cpu_name[] = {"CPU0", "CPU1", "CPU2", "CPU3", "CPU4", "CPU5", "CPU6", "CPU7", "CPU8", "CPU9"};

CONST UINT8 pss_ssdt_header[] =
{
  0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
  0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
  0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
  0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
  0x20, 0x03, 0x12, 0x20							/* 1.._		*/
};


CHAR8 cst_ssdt_header[] =
{
  0x53, 0x53, 0x44, 0x54, 0xE7, 0x00, 0x00, 0x00, /* SSDT.... */
  0x01, 0x17, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x41, /* ..PmRefA */
  0x43, 0x70, 0x75, 0x43, 0x73, 0x74, 0x00, 0x00, /* CpuCst.. */
  0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* ....INTL */
  0x20, 0x03, 0x12, 0x20                          /* 1.._		*/
};

CHAR8 resource_template_register_fixedhw[] =
{
  0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x7F,
  0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x79, 0x00
};

CHAR8 resource_template_register_systemio[] =
{
  0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x01,
  0x08, 0x00, 0x00, 0x15, 0x04, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x79, 0x00,
};



SSDT_TABLE *generate_pss_ssdt(UINT8 FirstID, UINTN Number)
{
  CHAR8 name[9];
  CHAR8 name1[13];
  CHAR8 name2[13];
  P_STATE initial, maximum, minimum, p_states[64];
  UINT8 p_states_count = 0;
  UINT8 cpu_dynamic_fsb = 0;
  
//  acpi_cpu_count = (UINT8)Number; //gCPUStructure.Cores;
  
	if (gCPUStructure.Vendor != CPU_VENDOR_INTEL) {
		MsgLog ("Not an Intel platform: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (!(gCPUStructure.Features & CPUID_FEATURE_MSR)) {
		MsgLog ("Unsupported CPU: P-States will not be generated !!!\n");
		return NULL;
	}
  
	if (acpi_cpu_count > 0)
	{
    UINT8 cpu_noninteger_bus_ratio = 0;
		// Retrieving P-States, ported from code by superhai (c)
		switch (gCPUStructure.Family) {
			case 0x06:
			{
				switch (gCPUStructure.Model)
				{
					case CPU_MODEL_DOTHAN:				// Pentium-M
					case CPU_MODEL_YONAH:	// Intel Mobile Core Solo, Duo
					case CPU_MODEL_MEROM:	// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPU_MODEL_PENRYN:	// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPU_MODEL_ATOM:	// Intel Atom (45nm)
					{
						
						if (AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 27))
						{
							AsmWriteMsr64(MSR_IA32_EXT_CONFIG, (AsmReadMsr64(MSR_IA32_EXT_CONFIG) | (1 << 28)));
							gBS->Stall(10);
							cpu_dynamic_fsb = (AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 28))?1:0;
              DBG("DynamicFSB=%x\n", cpu_dynamic_fsb);
						}
						
						cpu_noninteger_bus_ratio = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1ULL << 46)) != 0)?1:0;
						
						initial.Control.Control = (UINT16)AsmReadMsr64(MSR_IA32_PERF_STATUS);
            DBG("Initial control=%x\n", initial.Control);
						
						maximum.Control.Control = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 32) & 0x1F3F) | (0x4000 * cpu_noninteger_bus_ratio);
            DBG("Maximum control=%x\n", maximum.Control.Control);
            if (gSettings.Turbo) {
              maximum.Control.VID_FID.FID++;
              MsgLog("Turbo FID=%x\n", maximum.Control.VID_FID.FID);
            }
            MsgLog("UnderVoltStep=%d\n", gSettings.UnderVoltStep);
            MsgLog("PLimitDict=%d\n", gSettings.PLimitDict);
						maximum.CID = ((maximum.Control.VID_FID.FID & 0x1F) << 1) | cpu_noninteger_bus_ratio;
						
						minimum.Control.VID_FID.FID = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
						minimum.Control.VID_FID.VID = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x3F);
						
						if (minimum.Control.VID_FID.FID == 0)
						{
              minimum.Control.VID_FID.FID = 6;
              minimum.Control.VID_FID.VID = maximum.Control.VID_FID.VID;
            }
						
						minimum.CID = ((minimum.Control.VID_FID.FID & 0x1F) << 1) >> cpu_dynamic_fsb;
						
						// Sanity check
						if (maximum.CID < minimum.CID)
						{
							DBG("Insane FID values!");
							p_states_count = 0;
						}
						else
						{
              UINT8 vidstep;
							UINT8 i = 0, u, invalid = 0;
							// Finalize P-States
							// Find how many P-States machine supports
							p_states_count = (UINT8)(maximum.CID - minimum.CID + 1);
							
							if (p_states_count > 32)
								p_states_count = 32;
							DBG("PStates count=%x\n", p_states_count);
							
							vidstep = ((maximum.Control.VID_FID.VID << 2) - (minimum.Control.VID_FID.VID << 2)) / (p_states_count - 1);
							
							for (u = 0; u < p_states_count; u++)
							{
								i = u - invalid;
								
								p_states[i].CID = maximum.CID - u;
								p_states[i].Control.VID_FID.FID = (UINT8)(p_states[i].CID >> 1);
								
								if (p_states[i].Control.VID_FID.FID < 0x6)
								{
									if (cpu_dynamic_fsb)
										p_states[i].Control.VID_FID.FID = (p_states[i].Control.VID_FID.FID << 1) | 0x80;
								}
								else if (cpu_noninteger_bus_ratio)
								{
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
                  UINT32 multiplier = p_states[i].Control.VID_FID.FID & 0x1f;		// = 0x08
                  UINT8 half = (p_states[i].Control.VID_FID.FID & 0x40)?1:0;					// = 0x00
                  UINT8 dfsb = (p_states[i].Control.VID_FID.FID & 0x80)?1:0;					// = 0x01
                  UINT64 fsb = DivU64x32(gCPUStructure.FSBFrequency, Mega); // = 200
                  UINT64 halffsb = (fsb + 1) >> 1;					// = 100
                  UINT64 frequency = (multiplier * fsb);			// = 1600
                  
                  p_states[i].Frequency = (UINT32)(frequency + (half * halffsb)) >> dfsb;	// = 1600/2=800
                }
							}
							
							p_states_count -= invalid;
						}
						
						break;
					}
					case CPU_MODEL_FIELDS:		// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
					case CPU_MODEL_DALES:
					case CPU_MODEL_CLARKDALE:	// Intel Core i3, i5 LGA1156 (32nm)
					case CPU_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPU_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
					case CPU_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPU_MODEL_WESTMERE_EX:	// Intel Xeon E7
          case CPU_MODEL_SANDY_BRIDGE:		// Intel Core i3, i5, i7 LGA1155 (32nm)
          case CPU_MODEL_JAKETOWN:	// Intel Xeon E3
          case CPU_MODEL_IVY_BRIDGE:
					{
            if ((gCPUStructure.Model == CPU_MODEL_SANDY_BRIDGE) ||
                (gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE) ||
                (gCPUStructure.Model == CPU_MODEL_JAKETOWN))
            {
              maximum.Control.Control = (AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 8) & 0xff;
            } else {
              maximum.Control.Control = AsmReadMsr64(MSR_IA32_PERF_STATUS) & 0xff;
            }
            DBG("Maximum control=%x\n", maximum.Control.Control);
            if (gSettings.Turbo) {
              maximum.Control.Control++;
              MsgLog("Turbo control=%x\n", maximum.Control.Control);
            }
						
						minimum.Control.Control = (AsmReadMsr64(MSR_PLATFORM_INFO) >> 40) & 0xff;
						
						MsgLog("P-States: min 0x%x, max 0x%x\n", minimum.Control.Control, maximum.Control.Control);
						
						// Sanity check
						if (maximum.Control.Control < minimum.Control.Control)
						{
							DBG("Insane control values!");
							p_states_count = 0;
						}
						else
						{
							UINT32 i, j;
							p_states_count = 0;
							
							for (i = maximum.Control.Control; i >= minimum.Control.Control; i--)
							{
                j = i;
                if ((gCPUStructure.Model == CPU_MODEL_SANDY_BRIDGE) ||
                    (gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE) ||
                    (gCPUStructure.Model == CPU_MODEL_JAKETOWN))
                {
                  j = i << 8;
                }
								p_states[p_states_count].Control.Control = (UINT16)j;
								p_states[p_states_count].CID = j;
								p_states[p_states_count].Frequency = (UINT32)(DivU64x32(gCPUStructure.FSBFrequency, Mega) * i);
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
		if (p_states_count > 0)
		{
			INTN i;
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
      aml_add_buffer(root, (CONST CHAR8*)&pss_ssdt_header[0], sizeof(pss_ssdt_header)); // SSDT header
      /*
       AsciiSPrint(name, 9, "_PR_CPU%1d", FirstID);
       AsciiSPrint(name1, 13, "_PR_CPU%1dPSS_", FirstID);
       AsciiSPrint(name2, 13, "_PR_CPU%1dPCT_", FirstID);
       */
      AsciiSPrint(name, 9, "_PR_%4a", acpi_cpu_name[0]);
      AsciiSPrint(name1, 13, "_PR_%4aPSS_", acpi_cpu_name[0]);
      AsciiSPrint(name2, 13, "_PR_%4aPCT_", acpi_cpu_name[0]);
      
      scop = aml_add_scope(root, name);
      method = aml_add_name(scop, "PSS_");
      pack = aml_add_package(method);
			
      for (i = gSettings.PLimitDict; i < p_states_count; i++)
      {
        AML_CHUNK* pstt = aml_add_package(pack);
        
        aml_add_dword(pstt, p_states[i].Frequency);
        aml_add_dword(pstt, p_states[i].Frequency<<3); // Power
        aml_add_dword(pstt, 0x0000000A); // Latency
        aml_add_dword(pstt, 0x0000000A); // Latency
        aml_add_dword(pstt, p_states[i].Control.Control);
        aml_add_dword(pstt, p_states[i].Control.Control); // Status
      }
      metPSS = aml_add_method(scop, "_PSS", 0);
      aml_add_return_name(metPSS, "PSS_");
      metPPC = aml_add_method(scop, "_PPC", 0);
      aml_add_return_byte(metPPC, gSettings.PLimitDict);
      namePCT = aml_add_name(scop, "PCT_");
      packPCT = aml_add_package(namePCT);
      resource_template_register_fixedhw[8] = 0x00;
      resource_template_register_fixedhw[9] = 0x00;
      resource_template_register_fixedhw[18] = 0x00;
      aml_add_buffer(packPCT, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_buffer(packPCT, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      metPCT = aml_add_method(scop, "_PCT", 0);
      aml_add_return_name(metPCT, "PCT_");
      
      
      
			// Add CPUs
			for (i = 1; i < acpi_cpu_count; i++)
			{
        
				AsciiSPrint(name, 9, "_PR_%4a", acpi_cpu_name[i]);
				
				scop = aml_add_scope(root, name);
        metPSS = aml_add_method(scop, "_PSS", 0);
        aml_add_return_name(metPSS, name1);
        metPPC = aml_add_method(scop, "_PPC", 0);
        aml_add_return_byte(metPPC, gSettings.PLimitDict);
        metPCT = aml_add_method(scop, "_PCT", 0);
        aml_add_return_name(metPCT, name2);
        
			}
			
			aml_calculate_size(root);
			
			ssdt = (SSDT_TABLE *)AllocateZeroPool(root->Size);
			
			aml_write_node(root, (VOID*)ssdt, 0);
			
			ssdt->Length = root->Size;
			ssdt->Checksum = 0;
			ssdt->Checksum = (UINT8)(256 - Checksum8(ssdt, ssdt->Length));
			
			aml_destroy_node(root);
			
			//dumpPhysAddr("P-States SSDT content: ", ssdt, ssdt->Length);
			
			MsgLog ("SSDT with CPU P-States generated successfully\n");
			
			return ssdt;
		}
	}
	else
	{
		MsgLog ("ACPI CPUs not found: P-States not generated !!!\n");
	}
	
	return NULL;
}

SSDT_TABLE *generate_cst_ssdt(EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt, UINT8 FirstID, UINTN Number)
{
  BOOLEAN c2_enabled = gSettings.EnableC2;
  BOOLEAN c3_enabled;
  BOOLEAN c4_enabled = gSettings.EnableC4;
  BOOLEAN cst_using_systemio = gSettings.EnableISS;
  UINT8   p_blk_lo, p_blk_hi;
//  UINT8   acpi_cpu_count = (UINT8)Number; //gCPUStructure.Cores;
  UINT8   cstates_count;
  UINT32  acpi_cpu_p_blk;
  CHAR8 name2[9];
  CHAR8 name0[9];
  CHAR8 name1[13];
  AML_CHUNK* root;
  AML_CHUNK* scop;
  AML_CHUNK* name;
  AML_CHUNK* pack;
  AML_CHUNK* tmpl;
  AML_CHUNK* met;
  AML_CHUNK* ret;
  INTN i;
  SSDT_TABLE *ssdt;
  
  if (!fadt) {
    return NULL;
  }
  
  acpi_cpu_p_blk = fadt->Pm1aEvtBlk + 0x10;
  c2_enabled = c2_enabled || (fadt->PLvl2Lat < 100);
  c3_enabled = (fadt->PLvl3Lat < 1000);
  cstates_count = 1 + (c2_enabled ? 1 : 0) + ((c3_enabled || c4_enabled)? 1 : 0);
  
  root = aml_create_node(NULL);
  aml_add_buffer(root, cst_ssdt_header, sizeof(cst_ssdt_header)); // SSDT header
  AsciiSPrint(name0, 9, "_PR_%4a", acpi_cpu_name[0]);
  AsciiSPrint(name1, 13, "_PR_%4aCST_", acpi_cpu_name[0]);
  scop = aml_add_scope(root, name0);
  name = aml_add_name(scop, "CST_");
  pack = aml_add_package(name);
  aml_add_byte(pack, cstates_count);
  
  tmpl = aml_add_package(pack);
  if (cst_using_systemio)
  {
    // C1
    resource_template_register_fixedhw[8] = 0x00;
    resource_template_register_fixedhw[9] = 0x00;
    resource_template_register_fixedhw[0x12] = 0x00;
    aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
    aml_add_byte(tmpl, 0x01); // C1
    aml_add_word(tmpl, 0x0001); // Latency
    aml_add_dword(tmpl, 0x000003e8); // Power
    
    if (c2_enabled) // C2
    {
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
    
    if (c4_enabled) // C4
    {
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
    else if (c3_enabled) // C3
    {
      p_blk_lo = (UINT8)(acpi_cpu_p_blk + 5);
      p_blk_hi = (UINT8)((acpi_cpu_p_blk + 5) >> 8);
      
      tmpl = aml_add_package(pack);
      resource_template_register_systemio[11] = p_blk_lo; // C3
      resource_template_register_systemio[12] = p_blk_hi; // C3
      aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
      aml_add_byte(tmpl, 0x03);			// C3
      aml_add_word(tmpl, 0x0060);			// Latency
      aml_add_dword(tmpl, 0x0000015e);	// Power
    }
  }
  else
  {
    // C1
    resource_template_register_fixedhw[8] = 0x01;
    resource_template_register_fixedhw[9] = 0x02;
    resource_template_register_fixedhw[18] = 0x01;
    
    resource_template_register_fixedhw[11] = 0x00; // C1
    
    aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
    aml_add_byte(tmpl, 0x01);			// C1
    aml_add_word(tmpl, 0x0001);			// Latency
    aml_add_dword(tmpl, 0x000003e8);	// Power
    
    resource_template_register_fixedhw[18] = 0x03;
    
    if (c2_enabled) // C2
    {
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x10; // C2
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x02);			// C2
      aml_add_word(tmpl, 0x0040);			// Latency
      aml_add_dword(tmpl, 0x000001f4);	// Power
    }
    
    if (c4_enabled) // C4
    {
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x30; // C4
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x04);			// C4
      aml_add_word(tmpl, 0x0080);			// Latency
      aml_add_dword(tmpl, 0x000000C8);	// Power
    }
    else if (c3_enabled)
    {
      tmpl = aml_add_package(pack);
      resource_template_register_fixedhw[11] = 0x20; // C3
      aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
      aml_add_byte(tmpl, 0x03);			// C3
      aml_add_word(tmpl, 0x0060);			// Latency
      aml_add_dword(tmpl, 0x0000015e);	// Power
    }
  }
  met = aml_add_method(scop, "_CST", 0);
  ret = aml_add_return_name(met, "CST_");
  
  // Aliases
  for (i = 1; i < acpi_cpu_count; i++)
  {
    AsciiSPrint(name2, 9, "_PR_%4a", acpi_cpu_name[i]);
    
    scop = aml_add_scope(root, name2);
    met = aml_add_method(scop, "_CST", 0);
    ret = aml_add_return_name(met, name1);
    
  }
  
  aml_calculate_size(root);
  
  ssdt = (SSDT_TABLE *)AllocateZeroPool(root->Size);
  
  aml_write_node(root, (VOID*)ssdt, 0);
  
  ssdt->Length = root->Size;
  ssdt->Checksum = 0;
  ssdt->Checksum = (UINT8)(256 - Checksum8((VOID*)ssdt, ssdt->Length));
  
  aml_destroy_node(root);
  
  //dumpPhysAddr("C-States SSDT content: ", ssdt, ssdt->Length);
  
  MsgLog ("SSDT with CPU C-States generated successfully\n");
  
  return ssdt;
}
