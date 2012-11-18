/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "915resolution.h"
#include "gui.h"

void Resolution_start()
{
    UInt32 bp = 0;
    UInt32 x, y;
	patchVideoBios();
    getResolution(&x, &y, &bp);
    gui.screen.width = x;
    gui.screen.height = y;
}

