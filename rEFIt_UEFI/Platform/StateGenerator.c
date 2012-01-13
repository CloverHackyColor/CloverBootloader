/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo, slice
 */

#include "Platform.h"

#ifndef DEBUG_ACPI
#define DEBUG_ACPI 0
#endif

#if DEBUG_ACPI==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_ACPI==3
#define DBG(x...)  MsgLog(x)
#elif DEBUG_ACPI==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif


#define INTEL_VENDOR 0x756E6547

// TODO Migrate
#pragma pack(1)
struct acpi_2_ssdt {
	CHAR8           Signature[4];
	UINT32          Length;
	UINT8           Revision;
	UINT8           Checksum;
	CHAR8            OEMID[6];
	CHAR8            OEMTableId[8];
	UINT32        OEMRevision;
	UINT32        CreatorId;
	UINT32        CreatorRevision;
} __attribute__((packed));
#pragma pack()

UINT8	acpi_cpu_count = 0;
CHAR8** acpi_cpu_name = {"CPU0", "CPU1", "CPU2", "CPU3", "CPU4", "CPU5", "CPU6", "CPU7", "CPU8", "CPU9"};
UINT32 acpi_cpu_p_blk = 0;

struct acpi_2_ssdt *generate_pss_ssdt()
{	
	UINT8 ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
		0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
		0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
		0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
		0x31, 0x03, 0x10, 0x20,							/* 1.._		*/
	};
//	cpuid_update_generic_info();
	if (gCPUStructure.Vendor != INTEL_VENDOR) {
		MsgLog ("Not an Intel platform: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (!(gCPUStructure.Features & CPU_FEATURE_MSR)) {
		MsgLog ("Unsupported CPU: P-States will not be generated !!!\n");
		return NULL;
	}
	
	acpi_cpu_count = gCPUStructure.Cores;
		
	if (acpi_cpu_count > 0) 
	{
		struct p_state initial, maximum, minimum, p_states[32];
		UINT8 p_states_count = 0;		
		
		// Retrieving P-States, ported from code by superhai (c)
		switch (gCPUStructure.Family) {
			case 0x06: 
			{
				switch (gCPUStructure.Model) 
				{
					case CPU_MODEL_PENTIUM_M:				// ???
					case CPU_MODEL_YONAH:	// Intel Mobile Core Solo, Duo
					case CPU_MODEL_MEROM:	// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPU_MODEL_PENRYN:	// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPU_MODEL_ATOM:	// Intel Atom (45nm)
					{
						BOOLEAN cpu_dynamic_fsb = false;
						
						if (AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 27)) 
						{
							AsmWriteMsr64(MSR_IA32_EXT_CONFIG, (AsmReadMsr64(MSR_IA32_EXT_CONFIG) | (1 << 28))); 
							delay(1);
							cpu_dynamic_fsb = AsmReadMsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
						}
						
						BOOLEAN cpu_noninteger_bus_ratio = (AsmReadMsr64(MSR_IA32_PERF_STATUS) & (1ULL << 46));
						
						initial.Control = AsmReadMsr64(MSR_IA32_PERF_STATUS);
						
						maximum.Control = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 32) & 0x1F3F) | (0x4000 * cpu_noninteger_bus_ratio);
						maximum.CID = ((maximum.FID & 0x1F) << 1) | cpu_noninteger_bus_ratio;
						
						minimum.FID = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
						minimum.VID = ((AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x3F);
						
						if (minimum.FID == 0) 
						{
							UINT64 msr;
							UINT8 i;
							// Probe for lowest fid
							for (i = maximum.FID; i >= 0x6; i--) 
							{
								msr = AsmReadMsr64(MSR_IA32_PERF_CONTROL);
								AsmWriteMsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (i << 8) | minimum.VID);
								intel_waitforsts();
								minimum.FID = (AsmReadMsr64(MSR_IA32_PERF_STATUS) >> 8) & 0x1F; 
								delay(1);
							}
							
							msr = AsmReadMsr64(MSR_IA32_PERF_CONTROL);
							AsmWriteMsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
							intel_waitforsts();
						}
						
						if (minimum.VID == maximum.VID) 
						{	
							UINT64 msr;
							UINT8 i;
							// Probe for lowest vid
							for (i = maximum.VID; i > 0xA; i--) 
							{
								msr = AsmReadMsr64(MSR_IA32_PERF_CONTROL);
								AsmWriteMsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (minimum.FID << 8) | i);
								intel_waitforsts();
								minimum.VID = AsmReadMsr64(MSR_IA32_PERF_STATUS) & 0x3F; 
								delay(1);
							}
							
							msr = AsmReadMsr64(MSR_IA32_PERF_CONTROL);
							AsmWriteMsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
							intel_waitforsts();
						}
						
						minimum.CID = ((minimum.FID & 0x1F) << 1) >> cpu_dynamic_fsb;
						
						// Sanity check
						if (maximum.CID < minimum.CID) 
						{
							DBG("Insane FID values!");
							p_states_count = 0;
						}
						else
						{
							// Finalize P-States
							// Find how many P-States machine supports
							p_states_count = maximum.CID - minimum.CID + 1;
							
							if (p_states_count > 32) 
								p_states_count = 32;
							
							UINT8 vidstep;
							UINT8 i = 0, u, invalid = 0;
							
							vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);
							
							for (u = 0; u < p_states_count; u++) 
							{
								i = u - invalid;
								
								p_states[i].CID = maximum.CID - u;
								p_states[i].FID = (p_states[i].CID >> 1);
								
								if (p_states[i].FID < 0x6) 
								{
									if (cpu_dynamic_fsb) 
										p_states[i].FID = (p_states[i].FID << 1) | 0x80;
								} 
								else if (cpu_noninteger_bus_ratio) 
								{
									p_states[i].FID = p_states[i].FID | (0x40 * (p_states[i].CID & 0x1));
								}
								
								if (i && p_states[i].FID == p_states[i-1].FID)
									invalid++;
								
								p_states[i].VID = ((maximum.VID << 2) - (vidstep * u)) >> 2;
								
								UINT32 multiplier = p_states[i].FID & 0x1f;		// = 0x08
								BOOLEAN half = p_states[i].FID & 0x40;					// = 0x01
								BOOLEAN dfsb = p_states[i].FID & 0x80;					// = 0x00
								UINT32 fsb = gCPUStructure.FSBFrequency / 1000000; // = 400
								UINT32 halffsb = (fsb + 1) >> 1;					// = 200
								UINT32 frequency = (multiplier * fsb);			// = 3200
								
								p_states[i].Frequency = (frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
							}
							
							p_states_count -= invalid;
						}
						
						break;
					} 
					case CPU_MODEL_FIELDS:		// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
					case CPU_MODEL_DALES:		
					case CPU_MODEL_DALES_32NM:	// Intel Core i3, i5 LGA1156 (32nm)
					case CPU_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPU_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
					case CPU_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPU_MODEL_WESTMERE_EX:	// Intel Xeon E7
          case CPU_MODEL_SANDY:		// Intel Core i3, i5, i7 LGA1155 (32nm)
          case CPU_MODEL_SANDY_XEON:	// Intel Xeon E3
					{
						maximum.Control = AsmReadMsr64(MSR_IA32_PERF_STATUS) & 0xff; // Seems it always contains maximum multiplier value (with turbo, that's we need)...
						minimum.Control = (AsmReadMsr64(MSR_PLATFORM_INFO) >> 40) & 0xff;
						
						MsgLog("P-States: min 0x%x, max 0x%x\n", minimum.Control, maximum.Control);			
						
						// Sanity check
						if (maximum.Control < minimum.Control) 
						{
							DBG("Insane control values!");
							p_states_count = 0;
						}
						else
						{
							UINT8 i;
							p_states_count = 0;
							
							for (i = maximum.Control; i >= minimum.Control; i--) 
							{
								p_states[p_states_count].Control = i;
								p_states[p_states_count].CID = p_states[p_states_count].Control << 1;
								p_states[p_states_count].Frequency = (gCPUStructure.FSBFrequency / 1000000) * i;
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
			int i;
			
			struct aml_chunk* root = aml_create_node(NULL);
				aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
					struct aml_chunk* scop = aml_add_scope(root, "\\_PR_");
						struct aml_chunk* name = aml_add_name(scop, "PSS_");
							struct aml_chunk* pack = aml_add_package(name);
			
								for (i = 0; i < p_states_count; i++) 
								{
									struct aml_chunk* pstt = aml_add_package(pack);
									
									aml_add_dword(pstt, p_states[i].Frequency);
									aml_add_dword(pstt, 0x00000000); // Power
									aml_add_dword(pstt, 0x0000000A); // Latency
									aml_add_dword(pstt, 0x0000000A); // Latency
									aml_add_dword(pstt, p_states[i].Control);
									aml_add_dword(pstt, i+1); // Status
								}
				
			// Add aliaces
			for (i = 0; i < acpi_cpu_count; i++) 
			{
				CHAR8 name[9];
				AsciiSPrint(name, 8, "_PR_%c%c%c%c", acpi_cpu_name[i][0], acpi_cpu_name[i][1], acpi_cpu_name[i][2], acpi_cpu_name[i][3]);
				
				scop = aml_add_scope(root, name);
				aml_add_alias(scop, "PSS_", "_PSS");
			}
			
			aml_calculate_size(root);
			
			struct acpi_2_ssdt *ssdt = (struct acpi_2_ssdt *)AllocateKernelMemory(root->Size);
			
			aml_write_node(root, (VOID*)ssdt, 0);
			
			ssdt->Length = root->Size;
			ssdt->Checksum = 0;
			ssdt->Checksum = 256 - checksum8(ssdt, ssdt->Length);
			
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


