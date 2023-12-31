## @file
# Mde DSC include file for [LibraryClasses*] section of all Architectures.
#
# This file can be included to the [LibraryClasses*] section(s) of a platform DSC file
# by using "!include MdePkg/MdeLibs.dsc.inc" to specify the library instances
# of some EDKII basic/common library classes.
#
# Copyright (c) 2021 - 2022, Intel Corporation. All rights reserved.<BR>
#
#    SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[LibraryClasses]
  ArmTrngLib|MdePkg/Library/BaseArmTrngLibNull/BaseArmTrngLibNull.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
#  SmmCpuRendezvousLib|MdePkg/Library/SmmCpuRendezvousLibNull/SmmCpuRendezvousLibNull.inf
  BaseOverflowLib|MdePkg/Library/BaseOverflowLib/BaseOverflowLib.inf

  MemoryProfileLib|MdeModulePkg/Library/BaseMemoryProfileLibNull/BaseMemoryProfileLibNull.inf
  CommonMemoryAllocationLib|MdeModulePkg/Library/CommonMemoryAllocationLib/CommonMemoryAllocationLib.inf

  #
  # For security and code size optimization reasons, the UefiImageLib design is
  # complicated. To not expose FV-only UEFI image formats via, e.g., DxeCore to
  # untrusted sources, UefiImageLib can be configured to support different UEFI
  # image formats per source. While compilers generally manage to optimize the
  # initial parsing steps, complex control flow can make them lose track of the
  # UefiImageLib source property and thus conditionally call into code for UEFI
  # image formats that are not required by the module at all (e.g., because it
  # only ever deals with FV images).
  #
  # To have the code for unsupported UEFI image file formats removed,
  # UefiImageLib comes in multiple flavours that support FV-only, non-FV-only,
  # or all UEFI image formats. These flavours are supposed to be used by
  # top-level modules that have a clear scope. As libraries may depend on
  # UefiImageLib too, however, they should consume the general UefiImageLib with
  # no clear flavour.
  #
  # If they consume libraries with UefiImageLib dependencies, modules must have
  # their general UefiImageLib set to their corresponding flavour for correct
  # linkage. In order to make this easier, there is a heuristic below, based on
  # module types, to set the most likely instance of UefiImageLib. In rare
  # cases, manual overriding in the platform DSC may be necessary.
  #
  UefiImageOnlyNonFvLib|MdePkg/Library/BaseUefiImageLib/BaseUefiImageOnlyNonFvLib.inf
  UefiImageOnlyFvLib|MdePkg/Library/BaseUefiImageLib/BaseUefiImageOnlyFvLib.inf
  UefiImageAllLib|MdePkg/Library/BaseUefiImageLib/BaseUefiImageAllLib.inf
  UeImageLib|MdePkg/Library/BaseUeImageLib/BaseUeImageLib.inf

[LibraryClasses.common.USER_DEFINED, LibraryClasses.common.SEC, LibraryClasses.common.PEI_CORE, LibraryClasses.common.PEIM, LibraryClasses.common.SMM_CORE, LibraryClasses.common.MM_CORE_STANDALONE, LibraryClasses.common.DXE_SMM_DRIVER, LibraryClasses.common.MM_STANDALONE]
  UefiImageLib|MdePkg/Library/BaseUefiImageLib/BaseUefiImageOnlyFvLib.inf

[LibraryClasses.common.DXE_CORE, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  UefiImageLib|MdePkg/Library/BaseUefiImageLib/BaseUefiImageAllLib.inf
