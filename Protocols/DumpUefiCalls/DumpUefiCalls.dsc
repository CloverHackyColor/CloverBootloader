##  @file
#
#
##

[Defines]
  PLATFORM_NAME                  = DumpUefiCalls
  PLATFORM_GUID                  = B7F22AE1-9D87-4CD6-8EB1-171573908835
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
!if $(ARCH) == IA32
  OUTPUT_DIRECTORY               = Build/DumpUefiCallsIA32
!else
  OUTPUT_DIRECTORY               = Build/DumpUefiCallsX64
!endif
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

################################################################################
#
# Library Class section - list of all Library Classes needed by this Driver.
#
################################################################################
[LibraryClasses]

  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PlatformHookLib|MdeModulePkg/Library/BasePlatformHookLibNull/BasePlatformHookLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  SerialPortLib|MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf

[Components]
  Protocols/DumpUefiCalls/DumpUefiCalls.inf


[PcdsFixedAtBuild]

  #
  # PcdDebugPropertyMask Bits
  #

  # DEBUG_ASSERT_ENABLED       0x01
  # DEBUG_PRINT_ENABLED        0x02
  # DEBUG_CODE_ENABLED         0x04
  # CLEAR_MEMORY_ENABLED       0x08
  # ASSERT_BREAKPOINT_ENABLED  0x10
  # ASSERT_DEADLOOP_ENABLED    0x20

  !if $(TARGET) == DEBUG
     gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x7
  !else
     gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x3
  !endif

  #
  # PcdDebugPrintErrorLevel Bits
  #

  # DEBUG_INIT      0x00000001  Initialization
  # DEBUG_WARN      0x00000002  Warnings
  # DEBUG_LOAD      0x00000004  Load events
  # DEBUG_FS        0x00000008  EFI File system
  # DEBUG_POOL      0x00000010  Alloc & Free's
  # DEBUG_PAGE      0x00000020  Alloc & Free's
  # DEBUG_INFO      0x00000040  Informational debug messages
  # DEBUG_DISPATCH  0x00000080  PEI/DXE/SMM Dispatchers
  # DEBUG_VARIABLE  0x00000100  Variable
  #                 0x00000200  
  # DEBUG_BM        0x00000400  Boot Manager
  #                 0x00000800  
  # DEBUG_BLKIO     0x00001000  BlkIo Driver
  #                 0x00002000  
  # DEBUG_NET       0x00004000  SNI Driver
  #                 0x00008000  
  # DEBUG_UNDI      0x00010000  UNDI Driver
  # DEBUG_LOADFILE  0x00020000  UNDI Driver
  #                 0x00040000  
  # DEBUG_EVENT     0x00080000  Event messages
  # DEBUG_GCD       0x00100000  Global Coherency Database changes
  # DEBUG_CACHE     0x00200000  Memory range cachability changes
  # DEBUG_VERBOSE   0x00400000  Detailed debug messages that may significantly impact boot performance
  #                 0x01000000  
  #                 0x02000000  
  #                 0x04000000  
  #                 0x10000000  
  #                 0x20000000  
  #                 0x40000000  
  # DEBUG_ERROR     0x80000000  Error

  !if $(TARGET) == DEBUG
    gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x804A054F
  !else
    gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000442
  !endif

[BuildOptions]
  INTEL:DEBUG_*_*_CC_FLAGS   =
  INTEL:RELEASE_*_*_CC_FLAGS = /D MDEPKG_NDEBUG
  GCC:DEBUG_*_*_CC_FLAGS     = 
  GCC:RELEASE_*_*_CC_FLAGS   = -DMDEPKG_NDEBUG
  MSFT:DEBUG_*_*_CC_FLAGS    =
  MSFT:RELEASE_*_*_CC_FLAGS  = /D MDEPKG_NDEBUG
  XCODE:DEBUG_*_*_CC_FLAGS   =
  XCODE:RELEASE_*_*_CC_FLAGS = -DMDEPKG_NDEBUG
