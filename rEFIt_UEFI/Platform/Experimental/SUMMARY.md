# ModernCPUQuirks - Experimental Module

## Overview

This experimental module provides automatic CPU detection and quirk 
recommendation for modern Intel and AMD processors in Clover Bootloader.

## Technical Background

Clover already uses OpenCore's kernel patching engine (`OcLoadKernelSupport`).
This module enhances the automatic configuration of quirks for newer hardware.

## Supported CPUs

### Intel
- 12th Gen (Alder Lake) - Full support
- 13th Gen (Raptor Lake) - Full support  
- 14th Gen (Raptor Lake Refresh) - Full support
- 15th Gen (Meteor Lake) - Experimental

### AMD
- Zen3 (Ryzen 5000) - Full support
- Zen4 (Ryzen 7000) - Good support
- Zen5 (Ryzen 9000) - Experimental

## Files

- `ModernCPUQuirks.h` - API definitions
- `ModernCPUQuirks.c` - Implementation
- `README_Integration.md` - Integration guide

## Status

**EXPERIMENTAL** - Requires `ENABLE_MODERN_CPU_QUIRKS` build flag.

## License

Same as Clover Bootloader (BSD)
