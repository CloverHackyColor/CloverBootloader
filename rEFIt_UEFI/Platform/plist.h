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



CHAR8*
XMLDecode (
  CHAR8 *src
  );

EFI_STATUS
ParseXML (
  CONST CHAR8  *buffer,
        TagPtr *dict,
        UINT32 bufSize
  );


//VOID RenderSVGfont(NSVGfont  *fontSVG);

TagPtr
GetProperty (
        TagPtr dict,
  CONST CHAR8* key
  );

EFI_STATUS
XMLParseNextTag (
  CHAR8  *buffer,
  TagPtr *tag,
  UINT32 *lenPtr
  );

VOID
FreeTag (
  TagPtr tag
  );

EFI_STATUS
GetNextTag (
  UINT8  *buffer,
  CHAR8  **tag,
  UINT32 *start,
  UINT32 *length
  );

INTN
GetTagCount (
  TagPtr dict
  );

EFI_STATUS
GetElement (
  TagPtr dict,
  INTN   id,
  TagPtr *dict1
);

BOOLEAN
IsPropertyTrue (
  TagPtr Prop
  );

BOOLEAN
IsPropertyFalse (
  TagPtr Prop
  );

INTN
GetPropertyInteger (
  TagPtr Prop,
  INTN Default
  );


#endif /* PLATFORM_PLIST_H_ */
