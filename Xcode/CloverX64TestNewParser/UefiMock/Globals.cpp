//
//  Globals.c
//  cpp_tests_compare_settings
//
//  Created by Jief on 05/02/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#include <Platform.h>
#include <Efi.h>

//EFI_HANDLE         gImageHandle = NULL;
//EFI_SYSTEM_TABLE   *gST = NULL;
//EFI_BOOT_SERVICES  *gBS = NULL;
//EFI_RUNTIME_SERVICES  *gRT = NULL;
//EFI_DXE_SERVICES  *gDS = NULL;

static class Init {
  public:
    Init() {
      gImageHandle = NULL;
      gST = NULL;
      gBS = NULL;
      gRT = NULL;
      gDS = NULL;
    }
} init;
