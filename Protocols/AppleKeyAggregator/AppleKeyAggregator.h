//
//  AppleKeyMapAggregator.h
//  
//
//  Created by Slice on 26.10.16.
//  based on CupertinoNet
//

#ifndef _AppleKeyMapAggregator_h
#define _AppleKeyMapAggregator_h

#define APPLE_KEY_MAP_PROTOCOLS_REVISION  0x010000
#define APPLE_KEY_MAP_AGGREGATOR_SIGNATURE SIGNATURE_32 ('K', 'e', 'y', 'A')

#define APPLE_KEY_STROKES_INFO_SIGNATURE   SIGNATURE_32 ('K', 'e', 'y', 'S')

#define APPLE_KEY_MAP_AGGREGATOR_PRIVATE_FROM_AGGREGATOR(This) \
  CR (                                                              \
    (This),                                                         \
    APPLE_KEY_MAP_AGGREGATOR,                                       \
    AggregatorProtocol,                                             \
    APPLE_KEY_MAP_AGGREGATOR_SIGNATURE                              \
  )

#define APPLE_KEY_MAP_AGGREGATOR_PRIVATE_FROM_DATABASE(This)   \
  CR (                                                              \
    (This),                                                         \
    APPLE_KEY_MAP_AGGREGATOR,                                       \
    DatabaseProtocol,                                               \
    APPLE_KEY_MAP_AGGREGATOR_SIGNATURE                              \
  )

#define APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY(Entry)  \
((APPLE_KEY_STROKES_INFO *)(                         \
  CR (                                               \
    (Entry),                                         \
    APPLE_KEY_STROKES_INFO_HDR,                      \
    This,                                            \
    APPLE_KEY_STROKES_INFO_SIGNATURE                 \
  )                                                \
))
/*
typedef struct _EFI_LIST_ENTRY {
  struct _EFI_LIST_ENTRY  *ForwardLink;
  struct _EFI_LIST_ENTRY  *BackLink;
} EFI_LIST_ENTRY;

typedef EFI_LIST_ENTRY EFI_LIST;
*/

// APPLE_KEY_STROKES_INFO_HDR
typedef struct {
  UINTN              Signature;      ///<
  LIST_ENTRY         This;           ///<
  UINTN              Index;          ///<
  UINTN              KeyBufferSize;  ///<
  UINTN              NumberOfKeys;   ///<
  APPLE_MODIFIER_MAP Modifiers;      ///<
} APPLE_KEY_STROKES_INFO_HDR;

// APPLE_KEY_STROKES_INFO
typedef struct {
  APPLE_KEY_STROKES_INFO_HDR Hdr;   ///<
  APPLE_KEY_CODE                  Keys;  ///<
} APPLE_KEY_STROKES_INFO;


// APPLE_KEY_MAP_AGGREGATOR
typedef struct {
  UINTN                             Signature;           ///<0
  UINTN                             NextKeyStrokeIndex;  ///<0x08
  APPLE_KEY_CODE                         *KeyBuffer;          ///<0x10
  UINTN                             KeyBuffersSize;      ///<0x18
  LIST_ENTRY                        KeyStrokesInfoList;  ///<0x20
  APPLE_KEY_MAP_DATABASE_PROTOCOL   DatabaseProtocol;    ///<0x30 size=8*4
  APPLE_KEY_STATE_PROTOCOL          AggregatorProtocol;  ///<0x50 size=8*3
} APPLE_KEY_MAP_AGGREGATOR;  //size=0x68



#endif
