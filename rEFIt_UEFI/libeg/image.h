/*
 * image.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef LIBEG_IMAGE_H_
#define LIBEG_IMAGE_H_



EG_IMAGE
*egDecodePNG (
  IN const UINT8 *FileData,
  IN UINTN FileDataLength,
  IN BOOLEAN WantAlpha
  );



#endif /* LIBEG_IMAGE_H_ */
