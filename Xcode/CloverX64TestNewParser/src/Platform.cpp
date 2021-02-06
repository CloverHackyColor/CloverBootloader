//
//  Platform.cpp
//  cpp_tests
//
//  Created by jief on 23.02.20.
//  Copyright © 2020 JF Knudsen. All rights reserved.
//

#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <wchar.h>
#include <locale.h>

#include <string>
#include <locale>
#include <codecvt>
#include <vector>

#include "../../../rEFIt_UEFI/cpp_foundation/unicode_conversions.h"

void CpuDeadLoop(void)
{
	exit(1);
}
