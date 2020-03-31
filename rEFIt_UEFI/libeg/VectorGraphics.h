/*
 * VectorGraphics.h
 *
 *  Created on: 31 Mar 2020
 *      Author: jief
 */

#ifndef LIBEG_VECTORGRAPHICS_H_
#define LIBEG_VECTORGRAPHICS_H_

#include "../cpp_foundation/XStringW.h"
#include "../Platform/plist.h"
#include "XImage.h"

EFI_STATUS ParseSVGTheme(CONST CHAR8* buffer, TagPtr * dict);

#if USE_XTHEME
INTN renderSVGtext(XImage* TextBufferXY, INTN posX, INTN posY, INTN textType, const XStringW& string, UINTN Cursor);
#else
INTN renderSVGtext(EG_IMAGE* TextBufferXY, INTN posX, INTN posY, INTN textType, CONST CHAR16* text, UINTN Cursor);
#endif

VOID testSVG(VOID);


#endif /* LIBEG_VECTORGRAPHICS_H_ */
