/**
 * ModernCPUQuirks.h
 *
 * Experimental Modern CPU Detection and Automatic Quirk Application
 *
 * This module provides intelligent detection of modern CPUs (Ryzen 7000/9000,
 * Intel 14th Gen, etc.) and automatically recommends/applies the appropriate
 * OpenCore kernel quirks for optimal macOS compatibility.
 *
 * Copyright (c) 2024-2026 Clover Hackintosh & Beyond Team
 * Author: Clover Team
 *
 * This is an EXPERIMENTAL module for testing in Clover Hackintosh & Beyond
 * before potential inclusion in the official Clover repository.
 */

#ifndef MODERN_CPU_QUIRKS_H
#define MODERN_CPU_QUIRKS_H

#include "../../include/OC.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <Library/OcConfigurationLib.h>
#include <Library/OcCpuLib.h>
#include <Uefi.h>

/* ============================================================================
 * CPU Generation Definitions
 * ============================================================================
 */

// Intel CPU Generations (Desktop/Mobile)
typedef enum {
  CPU_GEN_UNKNOWN = 0,

  // Intel Legacy (before 6th Gen) - Not targeted
  CPU_GEN_INTEL_LEGACY = 1,

  // Intel Modern Generations
  CPU_GEN_INTEL_SKYLAKE = 6,        // 6th Gen
  CPU_GEN_INTEL_KABY_LAKE = 7,      // 7th Gen
  CPU_GEN_INTEL_COFFEE_LAKE = 8,    // 8th/9th Gen
  CPU_GEN_INTEL_COMET_LAKE = 10,    // 10th Gen
  CPU_GEN_INTEL_ROCKET_LAKE = 11,   // 11th Gen
  CPU_GEN_INTEL_ALDER_LAKE = 12,    // 12th Gen
  CPU_GEN_INTEL_RAPTOR_LAKE = 13,   // 13th Gen
  CPU_GEN_INTEL_RAPTOR_LAKE_R = 14, // 14th Gen (Raptor Lake Refresh)
  CPU_GEN_INTEL_METEOR_LAKE = 15,   // Future
  CPU_GEN_INTEL_ARROW_LAKE = 16,    // Future

  // AMD CPU Generations
  CPU_GEN_AMD_LEGACY = 100,
  CPU_GEN_AMD_ZEN = 101,      // Ryzen 1000 series
  CPU_GEN_AMD_ZEN_PLUS = 102, // Ryzen 2000 series
  CPU_GEN_AMD_ZEN2 = 103,     // Ryzen 3000 series
  CPU_GEN_AMD_ZEN3 = 104,     // Ryzen 5000 series
  CPU_GEN_AMD_ZEN4 = 105,     // Ryzen 7000 series
  CPU_GEN_AMD_ZEN5 = 106,     // Ryzen 9000 series (Future)

} CPU_GENERATION;

// CPU Architecture Type
typedef enum {
  CPU_ARCH_UNKNOWN = 0,
  CPU_ARCH_X86_64_INTEL = 1,
  CPU_ARCH_X86_64_AMD = 2,
  CPU_ARCH_HYBRID_INTEL = 3, // P-cores + E-cores (Alder Lake+)
} CPU_ARCHITECTURE_TYPE;

/* ============================================================================
 * Modern CPU Detection Result
 * ============================================================================
 */

typedef struct {
  CPU_GENERATION Generation;
  CPU_ARCHITECTURE_TYPE ArchType;

  // CPU Details
  UINT32 Family;
  UINT32 Model;
  UINT32 Stepping;
  UINT32 ExtFamily;
  UINT32 ExtModel;

  // Core Configuration (for hybrid CPUs)
  UINT32 PerfCoreCount; // P-cores (Performance)
  UINT32 EffCoreCount;  // E-cores (Efficiency)
  UINT32 TotalCoreCount;
  UINT32 ThreadCount;

  // Features
  BOOLEAN HasAVX512;
  BOOLEAN HasHybridArch;
  BOOLEAN HasTME; // Total Memory Encryption

  // Compatibility Flags
  BOOLEAN NeedsProvideCurrentCpuInfo;
  BOOLEAN NeedsCpuid1DataSpoof;
  BOOLEAN NeedsXcpmPatches;
  BOOLEAN NeedsIoMapperDisable;
  BOOLEAN NeedsSpecialPowerManagement;

} MODERN_CPU_INFO;

/* ============================================================================
 * Quirk Recommendation Result
 * ============================================================================
 */

typedef struct {
  // Essential Quirks (almost always needed for modern CPUs)
  BOOLEAN ProvideCurrentCpuInfo;
  BOOLEAN AppleXcpmCfgLock;
  BOOLEAN AppleCpuPmCfgLock;
  BOOLEAN DisableLinkeditJettison;

  // Architecture-Specific Quirks
  BOOLEAN AppleXcpmExtraMsrs;
  BOOLEAN AppleXcpmForceBoost;
  BOOLEAN PowerTimeoutKernelPanic;

  // Hardware-Specific Quirks
  BOOLEAN DisableIoMapper;
  BOOLEAN DisableIoMapperMapping;
  BOOLEAN LapicKernelPanic;
  BOOLEAN PanicNoKextDump;

  // Optional Quirks
  BOOLEAN XhciPortLimit;
  BOOLEAN ThirdPartyDrives;
  BOOLEAN ExtendBTFeatureFlags;
  BOOLEAN ForceAquantiaEthernet;
  BOOLEAN IncreasePciBarSize;

  // AMD-Specific (typically just markers, actual patches come from kexts)
  BOOLEAN NeedsAMDPatches;

  // CPUID Spoofing (for unsupported CPUs)
  BOOLEAN NeedsCpuidSpoof;
  UINT32 SpoofedCpuid1Data[4];
  UINT32 SpoofedCpuid1Mask[4];

  // Confidence Level (0-100)
  UINT8 ConfidenceLevel;

  // Description of detected configuration
  CHAR8 Description[256];

} QUIRK_RECOMMENDATION;

/* ============================================================================
 * Function Declarations
 * ============================================================================
 */

/**
 * Detect the current CPU and determine its generation and architecture.
 *
 * @param[in]  CpuInfo    The OC_CPU_INFO structure from OpenCore's OcCpuLib
 * @param[out] ModernInfo Filled with modern CPU detection results
 *
 * @return EFI_SUCCESS if detection completed successfully
 */
EFI_STATUS
ModernCpuDetect(IN OC_CPU_INFO *CpuInfo, OUT MODERN_CPU_INFO *ModernInfo);

/**
 * Get recommended quirks for the detected modern CPU.
 *
 * @param[in]  ModernInfo      The detected modern CPU information
 * @param[out] Recommendation  Filled with recommended quirk settings
 *
 * @return EFI_SUCCESS if recommendations were generated
 */
EFI_STATUS
ModernCpuGetQuirkRecommendation(IN MODERN_CPU_INFO *ModernInfo,
                                OUT QUIRK_RECOMMENDATION *Recommendation);

/**
 * Apply recommended quirks to the OpenCore configuration.
 * This function modifies the config in-place based on recommendations.
 *
 * @param[in]      Recommendation  The quirk recommendations to apply
 * @param[in,out]  Config          The OC_GLOBAL_CONFIG to modify
 * @param[in]      ForceApply      If TRUE, overwrite existing quirk settings
 *                                 If FALSE, only set quirks that are currently
 * disabled
 *
 * @return EFI_SUCCESS if quirks were applied successfully
 */
EFI_STATUS
ModernCpuApplyQuirks(IN QUIRK_RECOMMENDATION *Recommendation,
                     IN OUT OC_GLOBAL_CONFIG *Config, IN BOOLEAN ForceApply);

/**
 * Generate a human-readable report of the CPU detection and recommendations.
 *
 * @param[in]  ModernInfo      The detected modern CPU information
 * @param[in]  Recommendation  The quirk recommendations
 * @param[out] Report          Buffer to receive the report (must be at least
 * 2048 bytes)
 * @param[in]  ReportSize      Size of the Report buffer
 *
 * @return EFI_SUCCESS if report was generated
 */
EFI_STATUS
ModernCpuGenerateReport(IN MODERN_CPU_INFO *ModernInfo,
                        IN QUIRK_RECOMMENDATION *Recommendation,
                        OUT CHAR8 *Report, IN UINTN ReportSize);

/**
 * Get the string name for a CPU generation enum value.
 *
 * @param[in] Generation  The CPU generation value
 *
 * @return Pointer to a static string describing the generation
 */
CONST CHAR8 *ModernCpuGetGenerationName(IN CPU_GENERATION Generation);

/**
 * Check if this CPU is known to have specific compatibility issues.
 *
 * @param[in]  ModernInfo  The detected modern CPU information
 * @param[out] Issues      Buffer to receive issue descriptions (optional)
 * @param[in]  IssuesSize  Size of the Issues buffer
 *
 * @return TRUE if known issues exist, FALSE otherwise
 */
BOOLEAN
ModernCpuHasKnownIssues(IN MODERN_CPU_INFO *ModernInfo,
                        OUT CHAR8 *Issues OPTIONAL, IN UINTN IssuesSize);

/* ============================================================================
 * Helper Macros
 * ============================================================================
 */

#define IS_INTEL_CPU(ModernInfo)                                               \
  ((ModernInfo)->ArchType == CPU_ARCH_X86_64_INTEL ||                          \
   (ModernInfo)->ArchType == CPU_ARCH_HYBRID_INTEL)

#define IS_AMD_CPU(ModernInfo) ((ModernInfo)->ArchType == CPU_ARCH_X86_64_AMD)

#define IS_HYBRID_CPU(ModernInfo)                                              \
  ((ModernInfo)->ArchType == CPU_ARCH_HYBRID_INTEL)

#define IS_MODERN_INTEL(ModernInfo)                                            \
  (IS_INTEL_CPU(ModernInfo) &&                                                 \
   (ModernInfo)->Generation >= CPU_GEN_INTEL_ALDER_LAKE)

#define IS_MODERN_AMD(ModernInfo)                                              \
  (IS_AMD_CPU(ModernInfo) && (ModernInfo)->Generation >= CPU_GEN_AMD_ZEN4)

#ifdef __cplusplus
}
#endif

#endif /* MODERN_CPU_QUIRKS_H */
