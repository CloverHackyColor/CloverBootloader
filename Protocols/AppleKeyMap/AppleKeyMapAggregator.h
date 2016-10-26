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
  APPLE_KEY                  Keys;  ///<
} APPLE_KEY_STROKES_INFO;


// APPLE_KEY_MAP_AGGREGATOR
typedef struct {
  UINTN                             Signature;           ///<
  UINTN                             NextKeyStrokeIndex;  ///<
  APPLE_KEY                         *KeyBuffer;          ///<
  UINTN                             KeyBuffersSize;      ///<
  LIST_ENTRY                        KeyStrokesInfoList;  ///<
  APPLE_KEY_MAP_DATABASE_PROTOCOL   DatabaseProtocol;    ///<
  APPLE_KEY_STATE_PROTOCOL          AggregatorProtocol;  ///<
} APPLE_KEY_MAP_AGGREGATOR;



#endif
