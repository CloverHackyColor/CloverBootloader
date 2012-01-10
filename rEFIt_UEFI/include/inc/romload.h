#ifndef _EFI_ROMLOAD_H
#define _EFI_ROMLOAD_H

#define ROM_SIGNATURE 0xaa55
#define PCIDS_SIGNATURE "PCIR"
#pragma pack(push)
#pragma pack(1)
typedef struct 
{
    UINT8    Pcids_Sig[4];
    UINT16  VendId;
    UINT16  DevId;
    UINT16  Vpd_Off;
    UINT16  Size;
    UINT8 Rev;
    UINT8 Class_Code[3];
    UINT16  Image_Len;
    UINT16  Rev_Lvl;
    UINT8 Code_Type;
    UINT8 Indi;
    UINT16  Rsvd;
}PciDataStructure;
typedef struct
{
    UINT16 Size;
    UINT32 Header_Sig;
    UINT16 SubSystem;
    UINT16 MachineType;
    UINT8  Resvd[10];
    UINT16 EfiOffset;
}ArchData;
typedef struct 
{
    UINT16 Rom_Sig;
    ArchData Arch_Data;
    UINT16 Pcids_Off;
    UINT8 resvd[38];
}RomHeader;
#pragma pack(pop)

#endif
