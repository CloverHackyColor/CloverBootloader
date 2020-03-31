/*
 * plist.h
 *
 *  Created on: 31 Mar 2020
 *      Author: jief
 */

#ifndef PLATFORM_PLIST_H_
#define PLATFORM_PLIST_H_



typedef struct TagStruct {

  UINTN  type;
  CHAR8  *string;
  UINT8  *data;
  UINTN  dataLen;
  UINTN  offset;
  struct TagStruct *tag;
  struct TagStruct *tagNext;

} TagStruct, *TagPtr;



#endif /* PLATFORM_PLIST_H_ */
