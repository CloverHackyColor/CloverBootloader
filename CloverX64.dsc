################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = CloverX64
  PLATFORM_GUID                  = 199E24E0-0989-42aa-87F2-611A8C397E72
  PLATFORM_VERSION               = 0.92
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/Clover
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = RELEASE|DEBUG
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Clover/CloverX64.fdf

# Common statements
!include Clover64Common.dsc
