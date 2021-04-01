/*
 * TagTypes.h
 *
 *  Created on: Mar 30, 2021
 *      Author: jief
 */

#ifndef INCLUDE_TAGTYPES_H_
#define INCLUDE_TAGTYPES_H_



typedef enum {
  kTagTypeNone,
  kTagTypeDict,   // 1
  kTagTypeKey,    // 2
  kTagTypeString, // 3
  kTagTypeInteger,// 4
  kTagTypeData,   // 5
  kTagTypeDate,   // 6
  kTagTypeFalse,  // 7
  kTagTypeTrue,   // 8
  kTagTypeArray,  // 9
  kTagTypeFloat   // 10
} TAG_TYPE;




#endif /* INCLUDE_TAGTYPES_H_ */
