//
//  OpenCore.c
//  cpp_tests_compare_settings
//
//  Created by Jief on 05/02/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

//#include "OpenCore.h"

#include <Efi.h>
#include "../../../rEFIt_UEFI/include/OC.h"

OC_GLOBAL_CONFIG mOpenCoreConfiguration = {{{0},{0},{0},{0}},{{0},{0}},{{0},{0}},{{0},{0},{{0},{0}},{0},{0},{0},{{0},{0},0}},{{0},{{0},{0},0,0,0,0,0,0,0,0},{0},{{0},{0},{0},0,0,0,0,0,0,{0},{0},{0},0,0},{0},{0}},{{0},{0},{0},0,0,0},{0},{0}};
OC_STORAGE_CONTEXT mOpenCoreStorage = {0};
