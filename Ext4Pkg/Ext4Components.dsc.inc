## @file
# Ext4 DSC include file for [Components] section of all Architectures.
#
# SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

!if $(EXT4_ENABLE) == TRUE
  Ext4Pkg/Ext4Dxe/Ext4Dxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000007
  }
!endif
