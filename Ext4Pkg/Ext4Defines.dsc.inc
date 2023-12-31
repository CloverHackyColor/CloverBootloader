## @file
# Ext4 DSC include file for [Defines] section of all Architectures.
#
# SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

!ifndef EXT4_ENABLE
  #
  # This flag is to enable or disable the ext4 feature.
  #
  DEFINE EXT4_ENABLE = TRUE
!endif
