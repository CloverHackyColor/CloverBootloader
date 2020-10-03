/*
 * VectorGraphics.h
 *
 *  Created on: 31 Mar 2020
 *      Author: jief
 */

#ifndef LIBEG_VECTORGRAPHICS_H_
#define LIBEG_VECTORGRAPHICS_H_

#include "../cpp_foundation/XString.h"
#include "XImage.h"

INTN renderSVGtext(XImage* TextBufferXY, INTN posX, INTN posY, INTN textType, const XStringW& string, UINTN Cursor);

void testSVG(void);


#endif /* LIBEG_VECTORGRAPHICS_H_ */
