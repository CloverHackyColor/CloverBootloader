/*
 * File: HdaModels.h
 *
 * Copyright (c) 2018 John Davis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// From the VoodooHDA project (https://sourceforge.net/p/voodoohda),
// ALSA (Linux kernel), and vendor datasheets.

#ifndef _EFI_HDA_MODELS_H_
#define _EFI_HDA_MODELS_H_

#include <Uefi.h>

// Generic names.
#define HDA_CONTROLLER_MODEL_GENERIC    L"HD Audio Controller"
#define HDA_CODEC_MODEL_GENERIC         L"Unknown Codec"

#define GET_PCI_VENDOR_ID(a)    (a & 0xFFFF)
#define GET_PCI_DEVICE_ID(a)    ((a >> 16) & 0xFFFF)
#define GET_PCI_GENERIC_ID(a)   ((0xFFFF << 16) | a)
#define GET_CODEC_VENDOR_ID(a)  ((a >> 16) & 0xFFFF)
#define GET_CODEC_DEVICE_ID(a)  (a & 0xFFFF)
#define GET_CODEC_GENERIC_ID(a) (a | 0xFFFF)

#define HDA_VMIN   0x02 // Minor, Major Version
#define HDA_GCTL   0x08 // Global Control Register
#define HDA_ICO    0x60 // Immediate Command Output Interface
#define HDA_IRI    0x64 // Immediate Response Input Interface
#define HDA_ICS    0x68 // Immediate Command Status


// Vendor IDs.
#define VEN_AMD_ID              0x1002
#define VEN_ANALOGDEVICES_ID    0x11D4
#define VEN_AGERE_ID            0x11c1
#define VEN_CIRRUSLOGIC_ID      0x1013
#define VEN_CHRONTEL_ID         0x17e8
#define VEN_CONEXANT_ID         0x14F1
#define VEN_CREATIVE_ID         0x1102
#define VEN_IDT_ID              0x111D
#define VEN_INTEL_ID            0x8086
#define VEN_LG_ID               0x1854
#define VEN_NVIDIA_ID           0x10DE
#define VEN_QEMU_ID             0x1AF4
#define VEN_REALTEK_ID          0x10EC
#define VEN_SIGMATEL_ID         0x8384
#define VEN_VIA_ID              0x1106
#define VEN_CMEDIA_ID           0x13f6
#define VEN_CMEDIA2_ID          0x434d
#define VEN_RDC_ID              0x17f3
#define VEN_SIS_ID              0x1039
#define VEN_ULI_ID              0x10b9
#define VEN_MOTO_ID             0x1057
#define VEN_SII_ID              0x1095
#define VEN_WOLFSON_ID          0x14ec

#define VEN_INVALID_ID          0xFFFF


#define HDA_CONTROLLER(vendor, id) (((UINT32) (id) << 16) | ((VEN_##vendor##_ID) & 0xFFFF))
#define HDA_CODEC(vendor, id) (((UINT32) (VEN_##vendor##_ID) << 16) | ((id) & 0xFFFF))


// Controller name strings.
typedef struct {
    UINT32 Id;
    CHAR8  *Name;
} HDA_CONTROLLER_LIST_ENTRY;

// Codec name strings.
typedef struct {
    UINT32 Id;
    UINT16 Rev;
    CHAR8  *Name;
} HDA_CODEC_LIST_ENTRY;


/* get HDA device name */
VOID
EFIAPI
HdaControllerGetName(IN UINT32 ControllerID, OUT CHAR16 **Name);

VOID
EFIAPI
HdaCodecGetName(IN UINT32 CodecID, IN UINT16 RevisionId, OUT CHAR16 **Name);

//BOOLEAN
//EFIAPI
//IsHDMIAudio(EFI_HANDLE PciDevHandle);

#endif
