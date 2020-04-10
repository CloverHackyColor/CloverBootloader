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

INTN renderSVGtext(XImage* TextBufferXY, INTN posX, INTN posY, INTN textType, const XStringW& string, UINTN Cursor);

VOID testSVG(VOID);


#endif /* LIBEG_VECTORGRAPHICS_H_ */
