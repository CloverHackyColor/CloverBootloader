/**
 * ModernCPUQuirks_Integration.md
 * 
 * Guide for integrating ModernCPUQuirks into Clover Bootloader
 * 
 * This document explains how to integrate the experimental ModernCPUQuirks
 * module into the Clover boot flow to enable automatic quirk detection
 * and application for modern CPUs.
 */

# ModernCPUQuirks Integration Guide

## Overview

The `ModernCPUQuirks` module provides intelligent detection of modern CPUs
(Intel 12th-14th Gen, AMD Zen3-Zen5) and automatically recommends/applies
the appropriate OpenCore kernel quirks for optimal macOS compatibility.

## How It Works

```
┌────────────────────────────────────────────────────────────────────────────┐
│                         CLOVER BOOT FLOW                                    │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────┐     ┌──────────────────────┐     ┌──────────────────────┐ │
│  │ OC_CPU_INFO │────▶│ ModernCpuDetect()    │────▶│ MODERN_CPU_INFO      │ │
│  │ (from       │     │ - Identifies CPU Gen │     │ - Generation         │ │
│  │  OcCpuLib)  │     │ - Detects hybrid arch│     │ - ArchType           │ │
│  └─────────────┘     │ - Sets compat flags  │     │ - Compatibility flags│ │
│                      └──────────────────────┘     └──────────┬───────────┘ │
│                                                              │             │
│                                                              ▼             │
│                      ┌──────────────────────────────────────────────────┐  │
│                      │ ModernCpuGetQuirkRecommendation()                │  │
│                      │ - Analyzes CPU requirements                      │  │
│                      │ - Generates QUIRK_RECOMMENDATION                 │  │
│                      │ - Includes CPUID spoof values for unsupported CPUs│ │
│                      └──────────────────────────────┬───────────────────┘  │
│                                                     │                      │
│                                                     ▼                      │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │ ModernCpuApplyQuirks()                                             │   │
│  │ - Applies recommendations to mOpenCoreConfiguration                │   │
│  │ - Respects existing user settings (unless ForceApply=TRUE)         │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                     │                      │
│                                                     ▼                      │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │ OcLoadKernelSupport()                                              │   │
│  │ - OpenCore's kernel patching engine                                │   │
│  │ - Now has correctly configured quirks for modern CPUs              │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└────────────────────────────────────────────────────────────────────────────┘
```

## Integration Points in main.cpp

### Step 1: Add Include

In `rEFIt_UEFI/refit/main.cpp`, add the include:

```cpp
#include "../Platform/Experimental/ModernCPUQuirks.h"
```

### Step 2: Add Auto-Detection Call

In the `LOADER_ENTRY::StartLoader()` function, after `OcMiscLateInit()` and 
before `OcLoadKernelSupport()`, add:

```cpp
// === EXPERIMENTAL: Modern CPU Auto-Quirks ===
#ifdef ENABLE_MODERN_CPU_QUIRKS
{
  MODERN_CPU_INFO ModernCpuInfo;
  QUIRK_RECOMMENDATION QuirkRecommendation;
  EFI_STATUS QuirkStatus;
  
  // Detect modern CPU
  QuirkStatus = ModernCpuDetect(&mOpenCoreCpuInfo, &ModernCpuInfo);
  
  if (!EFI_ERROR(QuirkStatus)) {
    // Get quirk recommendations
    QuirkStatus = ModernCpuGetQuirkRecommendation(&ModernCpuInfo, &QuirkRecommendation);
    
    if (!EFI_ERROR(QuirkStatus)) {
      DBG("ModernCPU: %a (Confidence: %d%%)\n", 
          QuirkRecommendation.Description,
          QuirkRecommendation.ConfidenceLevel);
      
      // Apply quirks if auto-mode is enabled
      if (gSettings.Quirks.AutoModernCPUQuirks) {
        QuirkStatus = ModernCpuApplyQuirks(
          &QuirkRecommendation,
          &mOpenCoreConfiguration,
          FALSE  // Don't override user settings
        );
        
        if (!EFI_ERROR(QuirkStatus)) {
          DBG("ModernCPU: Applied automatic quirks\n");
        }
      }
      
      // Generate report if debug enabled
      if (gSettings.KernelAndKextPatches.KPDebug) {
        CHAR8 Report[2048];
        ModernCpuGenerateReport(&ModernCpuInfo, &QuirkRecommendation, Report, sizeof(Report));
        DBG("%a\n", Report);
      }
    }
  }
}
#endif // ENABLE_MODERN_CPU_QUIRKS
```

### Step 3: Add Config Option

In `ConfigPlist.h` or equivalent, add a new setting:

```cpp
// In Quirks section
XBool AutoModernCPUQuirks;  // Enable automatic quirk detection for modern CPUs
```

### Step 4: Compile Flag

Add to the build configuration (in GNUmakefile or relevant build file):

```makefile
# Enable experimental Modern CPU Quirks
EXPERIMENTAL_FLAGS += -DENABLE_MODERN_CPU_QUIRKS
```

## Usage in config.plist

```xml
<key>Quirks</key>
<dict>
    <!-- Enable automatic detection and application of quirks for modern CPUs -->
    <key>AutoModernCPUQuirks</key>
    <true/>
    
    <!-- Existing quirks still work and override auto-detected values -->
    <key>ProvideCurrentCpuInfo</key>
    <true/>
</dict>
```

## Priority Order

1. **User-defined quirks** (in config.plist) take priority
2. **Auto-detected quirks** fill in gaps when `AutoModernCPUQuirks` is enabled
3. **Default values** are used if neither is set

## Supported CPUs

### Intel
- **12th Gen (Alder Lake)**: Full support, CPUID spoof to Comet Lake
- **13th Gen (Raptor Lake)**: Full support, CPUID spoof to Comet Lake
- **14th Gen (Raptor Lake Refresh)**: Full support, CPUID spoof to Comet Lake
- **15th Gen (Meteor Lake)**: Experimental support
- **Arrow Lake and beyond**: Basic support, may need additional patches

### AMD
- **Zen3 (Ryzen 5000)**: Full support, requires AMD patches kexts
- **Zen4 (Ryzen 7000)**: Good support, requires AMD patches kexts
- **Zen5 (Ryzen 9000)**: Experimental support

## Testing

Before enabling in production:

1. Build Clover with `ENABLE_MODERN_CPU_QUIRKS` defined
2. Set `AutoModernCPUQuirks` to `true` in config.plist
3. Enable debug logging (`KPDebug` = `true`)
4. Boot and check `preboot.log` for the detection report
5. Verify that the detected quirks match your expectations
6. If issues occur, disable `AutoModernCPUQuirks` and use manual quirks

## Future Enhancements

1. **GPU-based detection**: Auto-detect AMD/NVIDIA GPU quirks
2. **Motherboard-specific quirks**: Handle VT-d, CFG Lock per manufacturer
3. **macOS version awareness**: Adjust quirks based on target macOS version
4. **Learning mode**: Learn from successful boots and suggest optimizations
