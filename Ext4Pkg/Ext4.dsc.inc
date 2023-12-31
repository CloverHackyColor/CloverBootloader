## @file
# Ext4 DSC include file for Platform DSC
#
# SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
!include Ext4Pkg/Ext4Defines.dsc.inc

[LibraryClasses]
!include Ext4Pkg/Ext4Libs.dsc.inc

[Components.common]
!include Ext4Pkg/Ext4Components.dsc.inc
