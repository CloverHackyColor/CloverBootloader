/**
 * ModernCPUQuirks.c
 *
 * Experimental Modern CPU Detection and Automatic Quirk Application
 * Implementation
 *
 * Copyright (c) 2024-2026 Clover Hackintosh & Beyond Team
 * Author: Clover Team & Hnanoto
 */

#include "ModernCPUQuirks.h"

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

/* ============================================================================
 * Intel CPU Model Definitions (from Intel SDM)
 * ============================================================================
 */

// Intel Family 6 Models
#define INTEL_MODEL_SKYLAKE_DT     0x5E     // Skylake Desktop
#define INTEL_MODEL_SKYLAKE_MB     0x4E     // Skylake Mobile
#define INTEL_MODEL_SKYLAKE_X      0x55     // Skylake-X/W
#define INTEL_MODEL_KABY_LAKE_DT   0x9E     // Kaby Lake Desktop
#define INTEL_MODEL_KABY_LAKE_MB   0x8E     // Kaby Lake Mobile
#define INTEL_MODEL_COFFEE_LAKE_DT 0x9E     // Coffee Lake Desktop (same as KL)
#define INTEL_MODEL_COFFEE_LAKE_R_DT 0x9E   // Coffee Lake-R (stepping diff)
#define INTEL_MODEL_COMET_LAKE_DT   0xA5    // Comet Lake Desktop
#define INTEL_MODEL_COMET_LAKE_MB   0xA6    // Comet Lake Mobile
#define INTEL_MODEL_ICELAKE_MB      0x7E    // Ice Lake Mobile
#define INTEL_MODEL_ICELAKE_DT      0x7D    // Ice Lake Desktop
#define INTEL_MODEL_TIGER_LAKE_MB   0x8C    // Tiger Lake Mobile
#define INTEL_MODEL_TIGER_LAKE_R    0x8D    // Tiger Lake-R
#define INTEL_MODEL_ROCKET_LAKE     0xA7    // Rocket Lake
#define INTEL_MODEL_ALDER_LAKE_DT   0x97    // Alder Lake Desktop
#define INTEL_MODEL_ALDER_LAKE_MB   0x9A    // Alder Lake Mobile
#define INTEL_MODEL_RAPTOR_LAKE_DT  0xB7    // Raptor Lake Desktop
#define INTEL_MODEL_RAPTOR_LAKE_MB  0xBA    // Raptor Lake Mobile
#define INTEL_MODEL_RAPTOR_LAKE_R   0xBF    // Raptor Lake Refresh 
#define INTEL_MODEL_METEOR_LAKE     0xAC    // Meteor Lake

#define INTEL_MODEL_METEOR_LAKE_14  0xAA  /* 14h Meteor Lake */
#define INTEL_MODEL_ARROW_LAKE		0xC6
#define INTEL_MODEL_ARROW_LAKE_X    0xC5  /* 15h Arrow Lake */
#define INTEL_MODEL_ARROW_LAKE_U    0xB5  /* 15h Arrow Lake */

/* ============================================================================
 * AMD CPU Family/Model Definitions
 * ============================================================================
 */

#define AMD_FAMILY_ZEN 0x17  // Zen, Zen+, Zen2
#define AMD_FAMILY_ZEN3 0x19 // Zen3, Zen4
#define AMD_FAMILY_ZEN5 0x1A // Zen5 (expected)

// Zen Family (0x17) Models
#define AMD_MODEL_ZEN_SUMMIT 0x01   // Summit Ridge (Ryzen 1000)
#define AMD_MODEL_ZEN_NAPLES 0x01   // Naples (EPYC 7001)
#define AMD_MODEL_ZEN_PINNACLE 0x08 // Pinnacle Ridge (Ryzen 2000)
#define AMD_MODEL_ZEN2_MATISSE 0x71 // Matisse (Ryzen 3000)
#define AMD_MODEL_ZEN2_ROME 0x31    // Rome (EPYC 7002)

// Zen3/4 Family (0x19) Models
#define AMD_MODEL_ZEN3_VERMEER 0x21 // Vermeer (Ryzen 5000)
#define AMD_MODEL_ZEN3_MILAN 0x01   // Milan (EPYC 7003)
#define AMD_MODEL_ZEN4_RAPHAEL 0x61 // Raphael (Ryzen 7000)
#define AMD_MODEL_ZEN4_GENOA 0x11   // Genoa (EPYC 9004)
#define AMD_MODEL_ZEN4_PHX 0x74     // Phoenix (Ryzen 7040)
#define AMD_MODEL_ZEN4_HWK 0x78     // Hawk Point (Ryzen 8000)

/* ============================================================================
 * CPU Detection Implementation
 * ============================================================================
 */

EFI_STATUS
ModernCpuDetect(IN OC_CPU_INFO *CpuInfo, OUT MODERN_CPU_INFO *ModernInfo) {
  if (CpuInfo == NULL || ModernInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem(ModernInfo, sizeof(MODERN_CPU_INFO));

  // Extract CPUID information
  ModernInfo->Family = CpuInfo->Family;
  ModernInfo->Model = CpuInfo->Model;
  ModernInfo->Stepping = CpuInfo->Stepping;
  ModernInfo->ExtFamily = CpuInfo->ExtFamily; // OcCpuLib already extends
  ModernInfo->ExtModel = CpuInfo->ExtModel;

  // Core counts
  ModernInfo->TotalCoreCount = CpuInfo->CoreCount;
  ModernInfo->ThreadCount = CpuInfo->ThreadCount;

  // Detect vendor and architecture
  if (CpuInfo->Vendor[0] == CPUID_VENDOR_INTEL) {
    // Detect Intel hybrid architecture (Alder Lake+)
    if (ModernInfo->Model == INTEL_MODEL_ALDER_LAKE_DT ||
        ModernInfo->Model == INTEL_MODEL_ALDER_LAKE_MB ||
        ModernInfo->Model == INTEL_MODEL_RAPTOR_LAKE_DT ||
        ModernInfo->Model == INTEL_MODEL_RAPTOR_LAKE_MB ||
        ModernInfo->Model == INTEL_MODEL_RAPTOR_LAKE_R ||
        ModernInfo->Model == INTEL_MODEL_METEOR_LAKE ||
		ModernInfo->Model == INTEL_MODEL_METEOR_LAKE_14 ||
		ModernInfo->Model == INTEL_MODEL_ARROW_LAKE ||
		ModernInfo->Model == INTEL_MODEL_ARROW_LAKE_X ||
        ModernInfo->Model == INTEL_MODEL_ARROW_LAKE_U) {
      ModernInfo->ArchType = CPU_ARCH_HYBRID_INTEL;
      ModernInfo->HasHybridArch = TRUE;

      // Note: Accurate P/E core detection requires more complex logic
      // For now, we use heuristics
      ModernInfo->PerfCoreCount = CpuInfo->CoreCount / 2;
      ModernInfo->EffCoreCount = CpuInfo->CoreCount - ModernInfo->PerfCoreCount;
    } else {
      ModernInfo->ArchType = CPU_ARCH_X86_64_INTEL;
      ModernInfo->HasHybridArch = FALSE;
      ModernInfo->PerfCoreCount = CpuInfo->CoreCount;
      ModernInfo->EffCoreCount = 0;
    }

    // Determine Intel Generation
    switch (ModernInfo->Model) {
    case INTEL_MODEL_SKYLAKE_DT:
    case INTEL_MODEL_SKYLAKE_MB:
    case INTEL_MODEL_SKYLAKE_X:
      ModernInfo->Generation = CPU_GEN_INTEL_SKYLAKE;
      break;

    case INTEL_MODEL_KABY_LAKE_DT:
    case INTEL_MODEL_KABY_LAKE_MB:
      // Need to distinguish Coffee Lake by stepping
      if (CpuInfo->Stepping >= 10) {
        ModernInfo->Generation = CPU_GEN_INTEL_COFFEE_LAKE;
      } else {
        ModernInfo->Generation = CPU_GEN_INTEL_KABY_LAKE;
      }
      break;

    case INTEL_MODEL_COMET_LAKE_DT:
    case INTEL_MODEL_COMET_LAKE_MB:
      ModernInfo->Generation = CPU_GEN_INTEL_COMET_LAKE;
      break;

    case INTEL_MODEL_ROCKET_LAKE:
      ModernInfo->Generation = CPU_GEN_INTEL_ROCKET_LAKE;
      break;

    case INTEL_MODEL_ALDER_LAKE_DT:
    case INTEL_MODEL_ALDER_LAKE_MB:
      ModernInfo->Generation = CPU_GEN_INTEL_ALDER_LAKE;
      break;

    case INTEL_MODEL_RAPTOR_LAKE_DT:
    case INTEL_MODEL_RAPTOR_LAKE_MB:
      ModernInfo->Generation = CPU_GEN_INTEL_RAPTOR_LAKE;
      break;

    case INTEL_MODEL_RAPTOR_LAKE_R:
      ModernInfo->Generation = CPU_GEN_INTEL_RAPTOR_LAKE_R;
      break;

    case INTEL_MODEL_METEOR_LAKE:
	case INTEL_MODEL_METEOR_LAKE_14:
      ModernInfo->Generation = CPU_GEN_INTEL_METEOR_LAKE;
      break;

    case INTEL_MODEL_ARROW_LAKE:
	case INTEL_MODEL_ARROW_LAKE_X:
	case INTEL_MODEL_ARROW_LAKE_U:
      ModernInfo->Generation = CPU_GEN_INTEL_ARROW_LAKE;
      break;

    default:
      if (ModernInfo->Family == 6 && ModernInfo->Model >= 0x90) {
        // Unknown but modern
        ModernInfo->Generation = CPU_GEN_INTEL_RAPTOR_LAKE;
      } else {
        ModernInfo->Generation = CPU_GEN_INTEL_LEGACY;
      }
      break;
    }
  } else if (CpuInfo->Vendor[0] == CPUID_VENDOR_AMD) {
    ModernInfo->ArchType = CPU_ARCH_X86_64_AMD;
    ModernInfo->HasHybridArch = FALSE;
    ModernInfo->PerfCoreCount = CpuInfo->CoreCount;
    ModernInfo->EffCoreCount = 0;

    // Determine AMD Generation
    if (ModernInfo->Family == AMD_FAMILY_ZEN) {
      if (ModernInfo->Model >= AMD_MODEL_ZEN2_MATISSE) {
        ModernInfo->Generation = CPU_GEN_AMD_ZEN2;
      } else if (ModernInfo->Model >= AMD_MODEL_ZEN_PINNACLE) {
        ModernInfo->Generation = CPU_GEN_AMD_ZEN_PLUS;
      } else {
        ModernInfo->Generation = CPU_GEN_AMD_ZEN;
      }
    } else if (ModernInfo->Family == AMD_FAMILY_ZEN3) {
      if (ModernInfo->Model >= AMD_MODEL_ZEN4_RAPHAEL) {
        ModernInfo->Generation = CPU_GEN_AMD_ZEN4;
      } else {
        ModernInfo->Generation = CPU_GEN_AMD_ZEN3;
      }
    } else if (ModernInfo->Family == AMD_FAMILY_ZEN5) {
      ModernInfo->Generation = CPU_GEN_AMD_ZEN5;
    } else {
      ModernInfo->Generation = CPU_GEN_AMD_LEGACY;
    }
  } else {
    ModernInfo->ArchType = CPU_ARCH_UNKNOWN;
    ModernInfo->Generation = CPU_GEN_UNKNOWN;
  }

  // Set compatibility flags based on detection
  ModernInfo->NeedsProvideCurrentCpuInfo =
      (ModernInfo->Generation >= CPU_GEN_INTEL_ALDER_LAKE ||
       ModernInfo->Generation >= CPU_GEN_AMD_ZEN3);

  ModernInfo->NeedsCpuid1DataSpoof =
      (ModernInfo->Generation >= CPU_GEN_INTEL_ALDER_LAKE ||
       IS_AMD_CPU(ModernInfo));

  ModernInfo->NeedsXcpmPatches =
      (IS_INTEL_CPU(ModernInfo) &&
       ModernInfo->Generation >= CPU_GEN_INTEL_SKYLAKE);

  ModernInfo->NeedsIoMapperDisable =
      (ModernInfo->Generation >= CPU_GEN_INTEL_COFFEE_LAKE);

  ModernInfo->NeedsSpecialPowerManagement =
      (IS_HYBRID_CPU(ModernInfo) || IS_AMD_CPU(ModernInfo));

  DEBUG((DEBUG_INFO, "ModernCPU: Detected %a CPU, Gen=%d, Arch=%d\n",
         IS_INTEL_CPU(ModernInfo)
             ? "Intel"
             : (IS_AMD_CPU(ModernInfo) ? "AMD" : "Unknown"),
         ModernInfo->Generation, ModernInfo->ArchType));

  return EFI_SUCCESS;
}

/* ============================================================================
 * Quirk Recommendation Implementation
 * ============================================================================
 */

EFI_STATUS
ModernCpuGetQuirkRecommendation(IN MODERN_CPU_INFO *ModernInfo,
                                OUT QUIRK_RECOMMENDATION *Recommendation) {
  if (ModernInfo == NULL || Recommendation == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem(Recommendation, sizeof(QUIRK_RECOMMENDATION));
  Recommendation->ConfidenceLevel = 0;

  // Base quirks for all modern macOS
  Recommendation->DisableLinkeditJettison = TRUE;
  Recommendation->PanicNoKextDump = TRUE;

  if (IS_INTEL_CPU(ModernInfo)) {
    // Intel-specific recommendations

    if (ModernInfo->Generation >= CPU_GEN_INTEL_SKYLAKE) {
      // CFG Lock unlock is almost always needed
      Recommendation->AppleXcpmCfgLock = TRUE;
      Recommendation->AppleCpuPmCfgLock = TRUE;
      Recommendation->ConfidenceLevel = 80;
    }

    if (ModernInfo->Generation >= CPU_GEN_INTEL_ALDER_LAKE) {
      // Modern hybrid architectures need special handling
      Recommendation->ProvideCurrentCpuInfo = TRUE;
      Recommendation->DisableIoMapperMapping = TRUE;
      Recommendation->PowerTimeoutKernelPanic = TRUE;
      Recommendation->ConfidenceLevel = 95;

      // CPUID spoofing for hybrid CPUs
      Recommendation->NeedsCpuidSpoof = TRUE;

      // Spoof to Comet Lake (0x0A0655) for best compatibility
      Recommendation->SpoofedCpuid1Data[0] = 0x0A0655;
      Recommendation->SpoofedCpuid1Data[1] = 0;
      Recommendation->SpoofedCpuid1Data[2] = 0;
      Recommendation->SpoofedCpuid1Data[3] = 0;

      Recommendation->SpoofedCpuid1Mask[0] = 0xFFFFFFFF;
      Recommendation->SpoofedCpuid1Mask[1] = 0;
      Recommendation->SpoofedCpuid1Mask[2] = 0;
      Recommendation->SpoofedCpuid1Mask[3] = 0;

      if (IS_HYBRID_CPU(ModernInfo)) {
        AsciiSPrint(
            Recommendation->Description, sizeof(Recommendation->Description),
            "Intel %a Gen Hybrid (P+E cores) - Requires CPUID spoof and "
            "ProvideCurrentCpuInfo",
            ModernInfo->Generation == CPU_GEN_INTEL_ALDER_LAKE
                ? "12th"
                : (ModernInfo->Generation == CPU_GEN_INTEL_RAPTOR_LAKE
                       ? "13th"
                       : (ModernInfo->Generation == CPU_GEN_INTEL_RAPTOR_LAKE_R
                              ? "14th"
                              : "15th+")));
      }
    } else if (ModernInfo->Generation >= CPU_GEN_INTEL_COFFEE_LAKE) {
      Recommendation->DisableIoMapper = TRUE;
      Recommendation->ConfidenceLevel = 85;

      AsciiSPrint(
          Recommendation->Description, sizeof(Recommendation->Description),
          "Intel %dth Gen - Standard quirks recommended",
          ModernInfo->Generation == CPU_GEN_INTEL_COFFEE_LAKE
              ? 8
              : (ModernInfo->Generation == CPU_GEN_INTEL_COMET_LAKE ? 10 : 11));
    } else {
      AsciiSPrint(
          Recommendation->Description, sizeof(Recommendation->Description),
          "Intel %dth Gen - Minimal quirks needed", ModernInfo->Generation);
    }
  } else if (IS_AMD_CPU(ModernInfo)) {
    // AMD-specific recommendations
    Recommendation->NeedsAMDPatches = TRUE;
    Recommendation->ProvideCurrentCpuInfo = TRUE;
    Recommendation->PowerTimeoutKernelPanic = TRUE;

    // AMD needs CPUID spoofing to look like Intel
    Recommendation->NeedsCpuidSpoof = TRUE;

    // Spoof to Haswell for AMD compatibility
    Recommendation->SpoofedCpuid1Data[0] = 0x0306C3; // Haswell
    Recommendation->SpoofedCpuid1Data[1] = 0;
    Recommendation->SpoofedCpuid1Data[2] = 0;
    Recommendation->SpoofedCpuid1Data[3] = 0;

    Recommendation->SpoofedCpuid1Mask[0] = 0xFFFFFFFF;
    Recommendation->SpoofedCpuid1Mask[1] = 0;
    Recommendation->SpoofedCpuid1Mask[2] = 0;
    Recommendation->SpoofedCpuid1Mask[3] = 0;

    if (ModernInfo->Generation >= CPU_GEN_AMD_ZEN4) {
      Recommendation->ConfidenceLevel = 90;
      AsciiSPrint(
          Recommendation->Description, sizeof(Recommendation->Description),
          "AMD Zen4 (Ryzen 7000) - Requires AMD patches kexts and CPUID spoof");
    } else if (ModernInfo->Generation >= CPU_GEN_AMD_ZEN3) {
      Recommendation->ConfidenceLevel = 92;
      AsciiSPrint(
          Recommendation->Description, sizeof(Recommendation->Description),
          "AMD Zen3 (Ryzen 5000) - Requires AMD patches kexts and CPUID spoof");
    } else if (ModernInfo->Generation >= CPU_GEN_AMD_ZEN2) {
      Recommendation->ConfidenceLevel = 95;
      AsciiSPrint(
          Recommendation->Description, sizeof(Recommendation->Description),
          "AMD Zen2 (Ryzen 3000) - Well-tested, requires AMD patches kexts");
    } else {
      Recommendation->ConfidenceLevel = 85;
      AsciiSPrint(Recommendation->Description,
                  sizeof(Recommendation->Description),
                  "AMD Zen/Zen+ - Good support with AMD patches kexts");
    }
  } else {
    Recommendation->ConfidenceLevel = 10;
    AsciiSPrint(Recommendation->Description,
                sizeof(Recommendation->Description),
                "Unknown CPU vendor - Cannot provide reliable recommendations");
  }

  DEBUG((DEBUG_INFO, "ModernCPU: Quirk recommendation: %a (Confidence: %d%%)\n",
         Recommendation->Description, Recommendation->ConfidenceLevel));

  return EFI_SUCCESS;
}

/* ============================================================================
 * Apply Quirks to Configuration
 * ============================================================================
 */

EFI_STATUS
ModernCpuApplyQuirks(IN QUIRK_RECOMMENDATION *Recommendation,
                     IN OUT OC_GLOBAL_CONFIG *Config, IN BOOLEAN ForceApply) {
  if (Recommendation == NULL || Config == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG((DEBUG_INFO, "ModernCPU: Applying quirks (Force=%d)\n", ForceApply));

// Apply each quirk if recommended and (force or not already set)
#define APPLY_QUIRK(quirk)                                                     \
  if (Recommendation->quirk && (ForceApply || !Config->Kernel.Quirks.quirk)) { \
    Config->Kernel.Quirks.quirk = TRUE;                                        \
    DEBUG((DEBUG_INFO, "ModernCPU: Enabled %a\n", #quirk));                    \
  }

  APPLY_QUIRK(ProvideCurrentCpuInfo);
  APPLY_QUIRK(AppleXcpmCfgLock);
  APPLY_QUIRK(AppleCpuPmCfgLock);
  APPLY_QUIRK(AppleXcpmExtraMsrs);
  APPLY_QUIRK(AppleXcpmForceBoost);
  APPLY_QUIRK(DisableLinkeditJettison);
  APPLY_QUIRK(DisableIoMapper);
  APPLY_QUIRK(DisableIoMapperMapping);
  APPLY_QUIRK(LapicKernelPanic);
  APPLY_QUIRK(PanicNoKextDump);
  APPLY_QUIRK(PowerTimeoutKernelPanic);
  APPLY_QUIRK(XhciPortLimit);
  APPLY_QUIRK(ThirdPartyDrives);
  APPLY_QUIRK(ExtendBTFeatureFlags);
  APPLY_QUIRK(ForceAquantiaEthernet);
  APPLY_QUIRK(IncreasePciBarSize);

#undef APPLY_QUIRK

  // Apply CPUID spoofing if needed
  if (Recommendation->NeedsCpuidSpoof) {
    if (ForceApply || Config->Kernel.Emulate.Cpuid1Data[0] == 0) {
      CopyMem(Config->Kernel.Emulate.Cpuid1Data,
              Recommendation->SpoofedCpuid1Data,
              sizeof(Config->Kernel.Emulate.Cpuid1Data));
      CopyMem(Config->Kernel.Emulate.Cpuid1Mask,
              Recommendation->SpoofedCpuid1Mask,
              sizeof(Config->Kernel.Emulate.Cpuid1Mask));
      DEBUG((DEBUG_INFO, "ModernCPU: Applied CPUID spoof: 0x%08X\n",
             Recommendation->SpoofedCpuid1Data[0]));
    }
  }

  return EFI_SUCCESS;
}

/* ============================================================================
 * Report Generation
 * ============================================================================
 */

EFI_STATUS
ModernCpuGenerateReport(IN MODERN_CPU_INFO *ModernInfo,
                        IN QUIRK_RECOMMENDATION *Recommendation,
                        OUT CHAR8 *Report, IN UINTN ReportSize) {
  UINTN Offset;

  if (ModernInfo == NULL || Recommendation == NULL || Report == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Offset = AsciiSPrint(
      Report, ReportSize,
      "=== Modern CPU Detection Report ===\n\n"
      "CPU Information:\n"
      "  Vendor: %a\n"
      "  Generation: %a (%d)\n"
      "  Family: 0x%02X, Model: 0x%02X, Stepping: %d\n"
      "  Core Count: %d total",
      IS_INTEL_CPU(ModernInfo) ? "Intel"
                               : (IS_AMD_CPU(ModernInfo) ? "AMD" : "Unknown"),
      ModernCpuGetGenerationName(ModernInfo->Generation),
      ModernInfo->Generation, ModernInfo->Family, ModernInfo->Model,
      ModernInfo->Stepping, ModernInfo->TotalCoreCount);

  if (IS_HYBRID_CPU(ModernInfo)) {
    Offset += AsciiSPrint(Report + Offset, ReportSize - Offset,
                          " (%d P-cores + %d E-cores)",
                          ModernInfo->PerfCoreCount, ModernInfo->EffCoreCount);
  }

  Offset += AsciiSPrint(
      Report + Offset, ReportSize - Offset,
      "\n  Threads: %d\n"
      "  Hybrid Architecture: %a\n\n"
      "Recommended Quirks (Confidence: %d%%):\n"
      "  %a\n\n"
      "Essential Quirks:\n",
      ModernInfo->ThreadCount, ModernInfo->HasHybridArch ? "Yes" : "No",
      Recommendation->ConfidenceLevel, Recommendation->Description);

// List enabled quirks
#define REPORT_QUIRK(quirk)                                                    \
  if (Recommendation->quirk) {                                                 \
    Offset +=                                                                  \
        AsciiSPrint(Report + Offset, ReportSize - Offset, "  - %a\n", #quirk); \
  }

  REPORT_QUIRK(ProvideCurrentCpuInfo);
  REPORT_QUIRK(AppleXcpmCfgLock);
  REPORT_QUIRK(AppleCpuPmCfgLock);
  REPORT_QUIRK(DisableLinkeditJettison);
  REPORT_QUIRK(DisableIoMapper);
  REPORT_QUIRK(DisableIoMapperMapping);
  REPORT_QUIRK(PanicNoKextDump);
  REPORT_QUIRK(PowerTimeoutKernelPanic);

#undef REPORT_QUIRK

  if (Recommendation->NeedsCpuidSpoof) {
    Offset += AsciiSPrint(Report + Offset, ReportSize - Offset,
                          "\nCPUID Spoofing:\n"
                          "  Spoof to: 0x%08X\n",
                          Recommendation->SpoofedCpuid1Data[0]);
  }

  if (Recommendation->NeedsAMDPatches) {
    Offset += AsciiSPrint(
        Report + Offset, ReportSize - Offset,
        "\nRequired Kexts for AMD:\n"
        "  - AMDRyzenCPUPowerManagement.kext\n"
        "  - SMCAMDProcessor.kext\n"
        "  - AMD kernel patches (via OpenCore or patches section)\n");
  }

  AsciiSPrint(Report + Offset, ReportSize - Offset,
              "\n=== End of Report ===\n");

  return EFI_SUCCESS;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================
 */

CONST CHAR8 *ModernCpuGetGenerationName(IN CPU_GENERATION Generation) {
  switch (Generation) {
  case CPU_GEN_INTEL_SKYLAKE:
    return "Intel Skylake (6th Gen)";
  case CPU_GEN_INTEL_KABY_LAKE:
    return "Intel Kaby Lake (7th Gen)";
  case CPU_GEN_INTEL_COFFEE_LAKE:
    return "Intel Coffee Lake (8th/9th Gen)";
  case CPU_GEN_INTEL_COMET_LAKE:
    return "Intel Comet Lake (10th Gen)";
  case CPU_GEN_INTEL_ROCKET_LAKE:
    return "Intel Rocket Lake (11th Gen)";
  case CPU_GEN_INTEL_ALDER_LAKE:
    return "Intel Alder Lake (12th Gen)";
  case CPU_GEN_INTEL_RAPTOR_LAKE:
    return "Intel Raptor Lake (13th Gen)";
  case CPU_GEN_INTEL_RAPTOR_LAKE_R:
    return "Intel Raptor Lake Refresh (14th Gen)";
  case CPU_GEN_INTEL_METEOR_LAKE:
    return "Intel Meteor Lake (15th Gen)";
  case CPU_GEN_INTEL_ARROW_LAKE:
    return "Intel Arrow Lake (Future)";

  case CPU_GEN_AMD_ZEN:
    return "AMD Zen (Ryzen 1000)";
  case CPU_GEN_AMD_ZEN_PLUS:
    return "AMD Zen+ (Ryzen 2000)";
  case CPU_GEN_AMD_ZEN2:
    return "AMD Zen2 (Ryzen 3000)";
  case CPU_GEN_AMD_ZEN3:
    return "AMD Zen3 (Ryzen 5000)";
  case CPU_GEN_AMD_ZEN4:
    return "AMD Zen4 (Ryzen 7000)";
  case CPU_GEN_AMD_ZEN5:
    return "AMD Zen5 (Ryzen 9000)";

  case CPU_GEN_INTEL_LEGACY:
    return "Intel Legacy (Pre-6th Gen)";
  case CPU_GEN_AMD_LEGACY:
    return "AMD Legacy (Pre-Zen)";
  default:
    return "Unknown";
  }
}

BOOLEAN
ModernCpuHasKnownIssues(IN MODERN_CPU_INFO *ModernInfo,
                        OUT CHAR8 *Issues OPTIONAL, IN UINTN IssuesSize) {
  BOOLEAN HasIssues = FALSE;
  UINTN Offset = 0;

  if (ModernInfo == NULL) {
    return FALSE;
  }

  if (Issues != NULL) {
    Issues[0] = '\0';
  }

  // Check for known issues
  if (IS_HYBRID_CPU(ModernInfo)) {
    HasIssues = TRUE;
    if (Issues != NULL && IssuesSize > Offset) {
      Offset +=
          AsciiSPrint(Issues + Offset, IssuesSize - Offset,
                      "- Hybrid architecture requires ProvideCurrentCpuInfo "
                      "and scheduler workarounds\n");
    }
  }

  if (ModernInfo->Generation >= CPU_GEN_INTEL_RAPTOR_LAKE_R) {
    HasIssues = TRUE;
    if (Issues != NULL && IssuesSize > Offset) {
      Offset += AsciiSPrint(Issues + Offset, IssuesSize - Offset,
                            "- 14th Gen Intel may have stability issues on "
                            "macOS Ventura and below\n");
    }
  }

  if (ModernInfo->Generation >= CPU_GEN_AMD_ZEN4) {
    HasIssues = TRUE;
    if (Issues != NULL && IssuesSize > Offset) {
      Offset += AsciiSPrint(Issues + Offset, IssuesSize - Offset,
                            "- AMD Zen4 requires latest AMD patches and may "
                            "have USB controller issues\n");
    }
  }

  if (ModernInfo->Generation == CPU_GEN_AMD_ZEN5) {
    HasIssues = TRUE;
    if (Issues != NULL && IssuesSize > Offset) {
      Offset += AsciiSPrint(
          Issues + Offset, IssuesSize - Offset,
          "- AMD Zen5 is cutting-edge and may have incomplete macOS support\n");
    }
  }

  return HasIssues;
}
