## @file
#  An EFI/Framework Emulation Platform with UEFI HII interface supported.
#
#  Developer's UEFI Emulation. DUET provides an EFI/UEFI IA32/X64 environment on legacy BIOS,
#  to help developing and debugging native EFI/UEFI drivers.
#
#  Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution. The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##
[Defines]
  PLATFORM_NAME                  = Clover
  PLATFORM_GUID                  = 199E24E0-0989-42aa-87F2-611A8C397E72
  PLATFORM_VERSION               = 0.92
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/Clover
  SUPPORTED_ARCHITECTURES        = X64|IA32
  BUILD_TARGETS                  = RELEASE|DEBUG
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Clover/Clover.fdf

  !ifndef OPENSSL_VERSION
    DEFINE OPENSSL_VERSION       = 1.0.1e
  !endif

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  #
  # Entry point
  #
  PeimEntryPoint|Clover/MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  DxeCoreEntryPoint|Clover/MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  UefiDriverEntryPoint|Clover/MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|Clover/MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  #
  # Basic
  #
  BaseLib|Clover/MdePkg/Library/BaseLib/BaseLib.inf
  SynchronizationLib|Clover/MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  BaseMemoryLib|Clover/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  #BaseMemoryLib|Clover/MdePkg/Library/UefiMemoryLib/UefiMemoryLib.inf
  #BaseMemoryLib|Clover/MdePkg/Library/BaseMemoryLibSse2/BaseMemoryLibSse2.inf
  PrintLib|Clover/MdePkg/Library/BasePrintLib/BasePrintLib.inf
  CpuLib|Clover/MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  IoLib|Clover/MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciLib|Clover/MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|Clover/MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib|Clover/MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  #PciLib|Clover/MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  #PciExpressLib|Clover/MdePkg/Library/DxeRuntimePciExpressLib/DxeRuntimePciExpressLib.inf
  CacheMaintenanceLib|Clover/MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  #PeCoffLib|Clover/MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffLib|Clover/Library/VBoxPeCoffLib/VBoxPeCoffLib.inf
  PeCoffExtraActionLib|Clover/MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PeCoffGetEntryPointLib|Clover/MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  CustomizedDisplayLib|Clover/MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  #
  # UEFI & PI
  #
  UefiBootServicesTableLib|Clover/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|Clover/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiRuntimeLib|Clover/MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiLib|Clover/MdePkg/Library/UefiLib/UefiLib.inf
  #UefiHiiServicesLib|Clover/MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  UefiHiiServicesLib|Clover/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|Clover/MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  DevicePathLib|Clover/MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiDecompressLib|Clover/MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  DxeServicesLib|Clover/MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|Clover/MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiBootManagerLib|Clover/MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  #EfiFileLib|EmbeddedPkg/Library/EfiFileLib/EfiFileLib.inf
  #EblNetworkLib|EmbeddedPkg/Library/EblNetworkLib/EblNetworkLib.inf
  #EblCmdLib|EmbeddedPkg/Library/EblCmdLibNull/EblCmdLibNull.inf
  FileHandleLib|Clover/MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|Clover/MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiCpuLib|Clover/CloverEFI/UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf
!ifdef ENABLE_SECURE_BOOT
  OpensslLib|Clover/Library/OpensslLib/openssl-$(OPENSSL_VERSION)/OpensslLib.inf
  IntrinsicLib|Clover/Library/IntrinsicLib/IntrinsicLib.inf
!else
  OpensslLib|Clover/Library/OpensslLib/OpensslLibNull.inf
!endif

  #
  # Generic Modules
  #
  UefiUsbLib|Clover/MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  UefiScsiLib|Clover/MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  OemHookStatusCodeLib|Clover/MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  #GenericBdsLib|IntelFrameworkModulePkg/Library/GenericBdsLib/GenericBdsLib.inf
  GenericBdsLib|Clover/Library/GenericBdsLib/GenericBdsLib.inf
  SecurityManagementLib|Clover/MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  CapsuleLib|Clover/MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  #PeCoffExtraActionLib|Clover/MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  NetLib|Clover/MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
  #
  # Platform
  #
  #PlatformBdsLib|DuetPkg/Library/DuetBdsLib/PlatformBds.inf
  PlatformBdsLib|Clover/Library/OsxBdsPlatformLib/PlatformBds.inf
  #TimerLib|DuetPkg/Library/DuetTimerLib/DuetTimerLib.inf
  TimerLib|Clover/Library/DuetTimerLib/DuetTimerLib.inf
  #
  # Misc
  #
  PerformanceLib|Clover/MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  DebugAgentLib|Clover/MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  PcdLib|Clover/MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
#  PcdLib|Clover/MdePkg/Library/DxePcdLib/DxePcdLib.inf
  PeiServicesLib|Clover/MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  MemoryAllocationLib|Clover/MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  HobLib|Clover/MdePkg/Library/DxeHobLib/DxeHobLib.inf
  LockBoxLib|Clover/MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf
  CpuExceptionHandlerLib|Clover/MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  SmbusLib|Clover/MdePkg/Library/DxeSmbusLib/DxeSmbusLib.inf
  S3BootScriptLib|Clover/MdeModulePkg/Library/PiDxeS3BootScriptLib/DxeS3BootScriptLib.inf
  ExtractGuidedSectionLib|Clover/MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PlatformHookLib|Clover/MdeModulePkg/Library/BasePlatformHookLibNull/BasePlatformHookLibNull.inf

  #SerialPortLib|PcAtChipsetPkg/Library/SerialIoLib/SerialIoLib.inf
  SerialPortLib|Clover/MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  MtrrLib|Clover/CloverEFI/UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  IoApicLib|Clover/PcAtChipsetPkg/Library/BaseIoApicLib/BaseIoApicLib.inf
  LocalApicLib|Clover/CloverEFI/UefiCpuPkg/Library/BaseXApicLib/BaseXApicLib.inf
  #LocalApicLib|Clover/CloverEFI/UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf

  #
  # To save size, use NULL library for DebugLib and ReportStatusCodeLib.
  # If need status code output, do library instance overriden as below DxeMain.inf does
  #
  DebugLib|Clover/MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugPrintErrorLevelLib|Clover/MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  ReportStatusCodeLib|Clover/MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf

  TpmMeasurementLib|Clover/MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  AuthVariableLib|Clover/MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
  VarCheckLib|Clover/MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf

  #
  # Our libs
  #
  MemLogLib|Clover/Library/MemLogLibDefault/MemLogLibDefault.inf
  VideoBiosPatchLib|Clover/Library/VideoBiosPatchLib/VideoBiosPatchLib.inf
  WaveLib|Clover/Library/WaveLib/WaveLib.inf
    
  ShellLib|Clover/ShellPkg/Library/UefiShellLib/UefiShellLib.inf
!ifndef NO_CLOVER_SHELL
  #Shell
  ShellCommandLib|Clover/ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  ShellCEntryLib|Clover/ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  HandleParsingLib|Clover/ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
  BcfgCommandLib|Clover/ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
!endif

[LibraryClasses.common.DXE_CORE]
  HobLib|Clover/MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  BaseMemoryLib|Clover/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib|Clover/MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf

[LibraryClasses.common.PEIM]
  MemoryAllocationLib|Clover/MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  HobLib|Clover/MdePkg/Library/PeiHobLib/PeiHobLib.inf
  LockBoxLib|Clover/MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxPeiLib.inf

[LibraryClasses.IA32.PEIM, LibraryClasses.X64.PEIM]
  PeiServicesTablePointerLib|Clover/MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf

[LibraryClasses.common.DXE_DRIVER]
  MemoryAllocationLib|Clover/MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  SmmServicesTableLib|Clover/MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  MemoryAllocationLib|Clover/MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf



###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################
[Components]
	#DuetPkg/BootSector/BootSector.inf

  #DuetPkg/DxeIpl/DxeIpl.inf {
  Clover/CloverEFI/OsxDxeIpl/DxeIpl.inf {
    <LibraryClasses>
      #
      # If no following overriden for ReportStatusCodeLib library class,
      # All other module can *not* output debug information even they are use not NULL library
      # instance for DebugLib and ReportStatusCodeLib
      #
      #ReportStatusCodeLib|Clover/MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
      ReportStatusCodeLib|Clover/MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  }
 #Clover/MdeModulePkg/Core/Dxe/DxeMain.inf {
 Clover/CloverEFI/OsxDxeCore/DxeMain.inf {
    #
    # Enable debug output for DxeCore module, this is a sample for how to enable debug output
    # for a module. If need turn on debug output for other module, please copy following overriden
    # PCD and library instance to other module's override section.
    #
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x82
      #0x82
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
      #0x80000042
    <LibraryClasses>
      BaseMemoryLib|Clover/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
      MemoryAllocationLib|Clover/MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
     # DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
     # ReportStatusCodeLib|DuetPkg/Library/DxeCoreReportStatusCodeLibFromHob/DxeCoreReportStatusCodeLibFromHob.inf
     DebugLib|Clover/MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
 	 ReportStatusCodeLib|Clover/MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
 	 PeCoffLib|Clover/Library/VBoxPeCoffLib/VBoxPeCoffLib.inf

  }

  Clover/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  Clover/MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  Clover/MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  Clover/MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf

  #DuetPkg/FSVariable/FSVariable.inf
!ifndef REAL_NVRAM
!ifdef HAVE_LEGACY_EMURUNTIMEDXE
  Clover/MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
!else
  Clover/MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE
    <LibraryClasses>
      AuthVariableLib|Clover/MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
      TpmMeasurementLib|Clover/MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
      VarCheckLib|Clover/MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  }
!endif
!else
  Clover/MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  Clover/MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|Clover/MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
  }
!endif

  Clover/MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  Clover/MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf
  Clover/MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  Clover/MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  Clover/MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf {
    <LibraryClasses>
      PcdLib|Clover/MdePkg/Library/DxePcdLib/DxePcdLib.inf
    <PcdsPatchableInModule>
      gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0
      gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0
  }
  Clover/MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  Clover/MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  Clover/MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
  #Clover/MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  Clover/CloverEFI/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  #Clover/MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  Clover/MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  Clover/MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  #DuetPkg/SmbiosGenDxe/SmbiosGen.inf
  #Clover/OsxSmbiosDxe/SmbiosDxe.inf
  Clover/CloverEFI/OsxSmbiosGenDxe/SmbiosGen.inf

  #DuetPkg/EfiLdr/EfiLdr.inf {
  Clover/CloverEFI/OsxEfiLdr/EfiLdr.inf {
    <LibraryClasses>
      DebugLib|Clover/MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
      BaseMemoryLib|Clover/MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

      #NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
      NULL|Clover/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
  #IntelFrameworkModulePkg/Universal/BdsDxe/BdsDxe.inf {
  Clover/CloverEFI/OsxBdsDxe/BdsDxe.inf {
    <LibraryClasses>
      PcdLib|Clover/MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  Clover/MdeModulePkg/Universal/EbcDxe/EbcDxe.inf
  Clover/CloverEFI/UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf
  #UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf
  #Clover/UefiCpuPkg/CpuDxe/CpuDxe.inf
  #UefiCpuPkg/CpuDxe/CpuDxe.inf
  Clover/CloverEFI/CpuDxe/Cpu.inf
  Clover/PcAtChipsetPkg/8259InterruptControllerDxe/8259.inf {
      <PcdsFixedAtBuild>
      gPcAtChipsetPkgTokenSpaceGuid.Pcd8259LegacyModeMask|0xFFFC
  }
  #DuetPkg/AcpiResetDxe/Reset.inf
  Clover/CloverEFI/AcpiReset/Reset.inf
  #DuetPkg/LegacyMetronome/Metronome.inf
  Clover/MdeModulePkg/Universal/Metronome/Metronome.inf
# EdkCompatibilityPkg/Compatibility/MpServicesOnFrameworkMpServicesThunk/MpServicesOnFrameworkMpServicesThunk.inf

  #Chipset
  #PcAtChipsetPkg/PcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf
  Clover/CloverEFI/PcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf
  Clover/PcAtChipsetPkg/8254TimerDxe/8254Timer.inf
  #PcAtChipsetPkg/HpetTimerDxe/HpetTimerDxe.inf
  #PcAtChipsetPkg/PciHostBridgeDxe/PciHostBridgeDxe.inf
  #DuetPkg/PciRootBridgeNoEnumerationDxe/PciRootBridgeNoEnumeration.inf
  Clover/CloverEFI/PciRootBridgeDxe/PciRootBridge.inf
  #DuetPkg/PciBusNoEnumerationDxe/PciBusNoEnumeration.inf
  Clover/CloverEFI/OsxPciBusNoEnumerationDxe/PciBusNoEnumeration.inf
  #Clover/MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  #Clover/PciBusDxe/PciBusDxe.inf
  Clover/MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #DataHub
  #Clover/VBoxAppleSim/VBoxAppleSim.inf
  #IntelFrameworkModulePkg/Universal/DataHubDxe/DataHubDxe.inf
  Clover/Protocols/DataHubDxe/DataHubDxe.inf
  #Clover/Protocols/DataHubStdErrDxe/DataHubStdErrDxe.inf

  # foreign file system support
  Clover/Protocols/DriverOverride/DriverOverride.inf
  Clover/FileSystems/VBoxFsDxe/VBoxHfs.inf
  Clover/FileSystems/VBoxFsDxe/VBoxIso9660.inf
  #Clover/FileSystems/VBoxFsDxe/VBoxFsDxe.inf
  Clover/FileSystems/VBoxFsDxe/VBoxExt2.inf
  Clover/FileSystems/VBoxFsDxe/VBoxExt4.inf
  Clover/FileSystems/VBoxFsDxe/VBoxReiserFS.inf
  #EmbeddedPkg/Universal/MmcDxe/MmcDxe.inf
  #Clover/OsxMmcDxe/MmcDxe.inf
  Clover/FileSystems/FatPkg/EnhancedFatDxe/Fat.inf
  Clover/FileSystems/ApfsDriverLoader/ApfsDriverLoader.inf
  # FS from grub
!ifndef NO_GRUB_DRIVERS
  Clover/FileSystems/GrubFS/src/EXFAT.inf
  Clover/FileSystems/GrubFS/src/HFSPLUS.inf
  Clover/FileSystems/GrubFS/src/ISO9660.inf
  Clover/FileSystems/GrubFS/src/NTFS.inf
  Clover/FileSystems/GrubFS/src/UDF.inf
  #Clover/FileSystems/GrubFS/src/ZFS.inf
  #Clover/FileSystems/GrubFS/src/UFS.inf
  #Clover/FileSystems/GrubFS/src/UFS2.inf
  #Clover/FileSystems/GrubFS/src/XFS.inf
!endif

  #Video
  #IntelFrameworkModulePkg/Bus/Pci/VgaMiniPortDxe/VgaMiniPortDxe.inf
  #Clover/VBoxVgaMiniPort/VgaMiniPortDxe.inf
  #IntelFrameworkModulePkg/Universal/Console/VgaClassDxe/VgaClassDxe.inf
  #Clover/VgaClassDxe/VgaClassDxe.inf
  #Clover/IntelGmaDxe/Gop.inf
  #DuetPkg/BiosVideoThunkDxe/BiosVideo.inf
  Clover/CloverEFI/BiosVideo/BiosVideo.inf
  #Clover/BiosVideoAuto/BiosVideo.inf
  Clover/LegacyBios/VideoDxe/VideoDxe.inf
  #Clover/LegacyBios/VideoDxe/VideoDxe2.inf

  # IDE/AHCI Support
!ifdef USE_BIOS_BLOCKIO
  Clover/LegacyBios/BlockIoDxe/BlockIoDxe.inf
!else
  #Clover/Trash/VBoxIdeControllerDxe/VBoxIdeControllerDxe.inf
  #Clover/Trash/VBoxIdeBusDxe/VBoxIdeBusDxe.inf
  #DuetPkg/SataControllerDxe/SataControllerDxe.inf
  Clover/Drivers/SataControllerDxe/SataControllerDxe.inf
  #Clover/MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  Clover/Drivers/AtaAtapi/AtaAtapiPassThru.inf
  #Clover/MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  Clover/Drivers/AtaBus/AtaBusDxe.inf
  #Clover/MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  Clover/Drivers/DVDBus/ScsiBusDxe.inf
  #Clover/MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  Clover/Drivers/DVDDisk/ScsiDiskDxe.inf
  #IntelFrameworkModulePkg/Bus/Pci/IdeBusDxe/IdeBusDxe.inf
!endif

  # Usb Support
  Clover/MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  Clover/Drivers/OhciDxe/OhciDxe.inf
  Clover/MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
#  Clover/MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  Clover/Drivers/XhciDxe/XhciDxe.inf
#  Clover/MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  Clover/Drivers/UsbBusDxe/UsbBusDxe.inf
#  Clover/MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  Clover/Drivers/UsbKbDxe/UsbKbDxe.inf
  Clover/MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  Clover/MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  #Clover/Drivers/UsbMouseDxe/UsbMouseDxe.inf

  # ISA Support
  Clover/PcAtChipsetPkg/IsaAcpiDxe/IsaAcpi.inf
  Clover/Drivers/Isa/IsaBusDxe/IsaBusDxe.inf
  #IntelFrameworkModulePkg/Bus/Isa/IsaSerialDxe/IsaSerialDxe.inf
  Clover/Drivers/Isa/Ps2KeyboardDxe/Ps2keyboardDxe.inf
  #IntelFrameworkModulePkg/Bus/Isa/IsaFloppyDxe/IsaFloppyDxe.inf
  Clover/Drivers/Isa/Ps2MouseAbsolutePointerDxe/Ps2MouseAbsolutePointerDxe.inf
  #IntelFrameworkModulePkg/Bus/Isa/Ps2MouseDxe/Ps2MouseDxe.inf
  Clover/Drivers/Isa/Ps2MouseDxe/Ps2MouseDxe.inf

  # ACPI Support
  #Clover/MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  Clover/CloverEFI/OsxAcpiTableDxe/AcpiTableDxe.inf
  #Clover/MdeModulePkg/Universal/Acpi/AcpiPlatformDxe/AcpiPlatformDxe.inf
  #Clover/CloverEFI/OsxAcpiPlatformDxe/AcpiPlatformDxe.inf

  Clover/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  Clover/MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf {
    <LibraryClasses>
      PcdLib|Clover/MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  #Clover/MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  Clover/Drivers/PartitionDxe/PartitionDxe.inf

  #FD
  #IntelFrameworkModulePkg/Universal/Acpi/AcpiS3SaveDxe/AcpiS3SaveDxe.inf
  #Clover/SaveResume/AcpiS3SaveDxe/AcpiS3SaveDxe.inf
  #Clover/MdeModulePkg/Universal/Acpi/S3SaveStateDxe/S3SaveStateDxe.inf
  #Clover/MdeModulePkg/Universal/Acpi/SmmS3SaveState/SmmS3SaveState.inf
  #Clover/MdeModulePkg/Universal/Acpi/BootScriptExecutorDxe/BootScriptExecutorDxe.inf
  #Clover/SaveResume/BootScriptExecutorDxe/BootScriptExecutorDxe.inf
  #Clover/CloverEFI/UefiCpuPkg/Universal/Acpi/S3Resume2Pei/S3Resume2Pei.inf

  # Bios Thunk
  #IntelFrameworkModulePkg/Csm/BiosThunk/VideoDxe/VideoDxe.inf
  #IntelFrameworkModulePkg/Csm/LegacyBiosDxe/LegacyBiosDxe.inf
  #IntelFrameworkModulePkg/Csm/BiosThunk/BlockIoDxe/BlockIoDxe.inf
  #IntelFrameworkModulePkg/Csm/BiosThunk/KeyboardDxe/KeyboardDxe.inf
  Clover/CloverEFI/BiosKeyboard/KeyboardDxe.inf
  #IntelFrameworkModulePkg/Universal/LegacyRegionDxe/LegacyRegionDxe.inf
  #Clover/MdeModulePkg/Universal/LegacyRegion2Dxe/LegacyRegion2Dxe.inf
  Clover/LegacyBios/Region2Dxe/LegacyRegion2Dxe.inf

  # Misc
  Clover/FSInject/FSInject.inf
  Clover/Protocols/MsgLog/MsgLog.inf
  Clover/Protocols/SMCHelper/SMCHelper.inf
  Clover/Protocols/FirmwareVolume/FirmwareVolume.inf
  Clover/Protocols/AppleImageCodec/AppleImageCodec.inf
  Clover/Protocols/AppleUITheme/AppleUITheme.inf
  Clover/Protocols/HashServiceFix/HashServiceFix.inf
  Clover/Protocols/AppleKeyAggregator/AppleKeyAggregator.inf
  Clover/Protocols/AppleKeyFeeder/AppleKeyFeeder.inf
  

!ifdef DEBUG_ON_SERIAL_PORT

  Clover/Protocols/DumpUefiCalls/DumpUefiCalls.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
    <LibraryClasses>
      DebugLib|Clover/MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
      SerialPortLib|Clover/MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
  }

!else

  Clover/Protocols/DumpUefiCalls/DumpUefiCalls.inf {
  	<LibraryClasses>
  		PeCoffLib|Clover/Library/VBoxPeCoffLib/VBoxPeCoffLib.inf
  }


!endif

  # Drivers for Aptio loading - should go to Clover's /EFI/drivers64UEFI dir
  Clover/Protocols/OsxFatBinaryDrv/OsxFatBinaryDrv.inf

  # Drivers for Phoenix UEFI loading - should go to Clover's /EFI/drivers64UEFI dir
  Clover/Protocols/EmuVariableUefi/EmuVariableRuntimeDxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvStoreReserved|0
      gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x3000
      gEfiMdeModulePkgTokenSpaceGuid.PcdMaxHardwareErrorVariableSize|0x8000
      gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0x40000
      gEfiMdeModulePkgTokenSpaceGuid.PcdVariableCollectStatistics|FALSE
      gEfiMdeModulePkgTokenSpaceGuid.PcdHwErrStorageSize|0x0000
  }

  # Driver Audio
  Clover/Drivers/AudioDxe/AudioDxe.inf

  #
  # Sample Application
  #
  #Clover/MdeModulePkg/Application/HelloWorld/HelloWorld.inf
  #Clover/MdeModulePkg/Application/VariableInfo/VariableInfo.inf
  #Clover/Sample/Application/Sample.inf
  #Clover/gptsync/gptsync.inf
  Clover/bdmesg_efi/bdmesg.inf
  
!ifndef NO_CLOVER_SHELL
  Clover/ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
  Clover/ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf

  
  Clover/ShellPkg/Application/Shell/Shell.inf {
    <PcdsFixedAtBuild>
	  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
	  gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
	  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000
	!ifdef $(NO_SHELL_PROFILES)
	  gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0x00
	!endif #$(NO_SHELL_PROFILES)

    <LibraryClasses>
      PcdLib|Clover/MdePkg/Library/DxePcdLib/DxePcdLib.inf
      PeCoffGetEntryPointLib|Clover/MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
      UefiHiiServicesLib|Clover/MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
!ifndef $(NO_SHELL_PROFILES)
      NULL|Clover/ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      NULL|Clover/ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
!ifdef $(INCLUDE_DP)
      NULL|Clover/ShellPkg/Library/UefiDpLib/UefiDpLib.inf
!endif #$(INCLUDE_DP)
!ifdef $(INCLUDE_TFTP_COMMAND)
      NULL|Clover/ShellPkg/Library/UefiShellTftpCommandLib/UefiShellTftpCommandLib.inf
!endif #$(INCLUDE_TFTP_COMMAND)
!endif #$(NO_SHELL_PROFILES)
  }
!endif


!ifdef DEBUG_ON_SERIAL_PORT
	Clover/rEFIt_UEFI/refit.inf {
	#
     # Enable debug output.
     #
	<PcdsFixedAtBuild>
		gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
		gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
	<LibraryClasses>
		SerialPortLib|Clover/MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
		DebugLib|Clover/MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
	DebugPrintErrorLevelLib|Clover/MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
	}
!else
	Clover/rEFIt_UEFI/refit.inf {
    <LibraryClasses>
      BaseMemoryLib|Clover/MdePkg/Library/UefiMemoryLib/UefiMemoryLib.inf
  }
!endif

[Components.X64]

  Clover/OsxAptioFixDrv/OsxAptioFixDrv.inf
#  Clover/OsxAptioFixDrv/OsxAptioFix2Drv.inf
  Clover/OsxAptioFixDrv/OsxAptioFix3Drv.inf
  Clover/OsxLowMemFixDrv/OsxLowMemFixDrv.inf
  #Clover/OsxAptioFixDrv/OsxAptioFixDrv.inf {
    #
    # Enable debug output.
    #
   # <PcdsFixedAtBuild>
   #   gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
   #   gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
   # <LibraryClasses>
   #   SerialPortLib|Clover/MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
   #   DebugLib|Clover/MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
   #   DebugPrintErrorLevelLib|Clover/MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  #}

###################################################################################################
#
# BuildOptions Section - Define the module specific tool chain flags that should be used as
#                        the default flags for a module. These flags are appended to any
#                        standard flags that are defined by the build process. They can be
#                        applied for any modules or only those modules with the specific
#                        module style (EDK or EDKII) specified in [Components] section.
#
###################################################################################################
[BuildOptions]

!ifdef ENABLE_VBIOS_PATCH_CLOVEREFI
  DEFINE VBIOS_PATCH_CLOVEREFI_FLAG = -DCLOVER_VBIOS_PATCH_IN_CLOVEREFI
!endif

!ifdef ONLY_SATA_0
  DEFINE ONLY_SATA_0_FLAG = -DONLY_SATA_0
!endif

!ifdef USE_BIOS_BLOCKIO
  DEFINE BLOCKIO_FLAG = -DUSE_BIOS_BLOCKIO
!ifdef DISABLE_USB_CONTROLLERS_WHEN_BLOCKIO
  DEFINE DISABLE_USB_CONTROLLERS = 1
!endif
!endif

!ifdef DISABLE_USB_CONTROLLERS
  DEFINE NOUSB_FLAG = -DDISABLE_USB_SUPPORT
!endif

!ifdef DISABLE_USB_SUPPORT
  DEFINE NOUSB_FLAG = -DDISABLE_USB_SUPPORT
!endif

!ifdef DISABLE_UDMA_SUPPORT
  DEFINE NOUDMA_FLAG = -DDISABLE_UDMA_SUPPORT
!endif

# Slice: I propose this flag always true
#!ifdef AMD_SUPPORT
  DEFINE AMD_FLAG = -DAMD_SUPPORT
#!endif

!ifdef ENABLE_SECURE_BOOT
  DEFINE SECURE_BOOT_FLAG = -DENABLE_SECURE_BOOT
!endif

#!ifdef ANDX86
  DEFINE ANDX86_FLAG = -DANDX86
#!endif

#!ifdef LODEPNG
  DEFINE LODEPNG_FLAG = -DLODEPNG
#!endif

!ifdef ENABLE_PS2MOUSE_LEGACYBOOT
  DEFINE PS2MOUSE_LEGACYBOOT_FLAG = -DENABLE_PS2MOUSE_LEGACYBOOT
!endif

!ifdef DEBUG_ON_SERIAL_PORT
  DEFINE DEBUG_ON_SERIAL_PORT_FLAG = -DDEBUG_ON_SERIAL_PORT
!endif

!ifdef DISABLE_LTO
	DEFINE DISABLE_LTO_FLAG = -fno-lto -UUSING_LTO
!endif


!ifdef EXIT_USBKB
DEFINE EXIT_USBKB_FLAG = -DEXIT_USBKB
!endif


DEFINE BUILD_OPTIONS=-DMDEPKG_NDEBUG -DCLOVER_BUILD $(VBIOS_PATCH_CLOVEREFI_FLAG) $(ONLY_SATA_0_FLAG) $(BLOCKIO_FLAG) $(NOUSB_FLAG) $(NOUDMA_FLAG) $(AMD_FLAG) $(SECURE_BOOT_FLAG) $(ANDX86_FLAG) $(LODEPNG_FLAG) $(PS2MOUSE_LEGACYBOOT_FLAG) $(DEBUG_ON_SERIAL_PORT_FLAG) $(EXIT_USBKB_FLAG)

  #MSFT:*_*_*_CC_FLAGS  = /FAcs /FR$(@R).SBR /wd4701 /wd4703 $(BUILD_OPTIONS)
  MSFT:*_*_*_CC_FLAGS  = /FAcs /FR$(@R).SBR $(BUILD_OPTIONS) -Dinline=__inline

  XCODE:*_*_*_CC_FLAGS = -fno-unwind-tables -Wno-msvc-include -Os $(BUILD_OPTIONS) $(DISABLE_LTO_FLAG)
  GCC:*_*_*_CC_FLAGS   = $(BUILD_OPTIONS) $(DISABLE_LTO_FLAG)
  #-Wunused-but-set-variable
  # -Os -fno-omit-frame-pointer -maccumulate-outgoing-args
