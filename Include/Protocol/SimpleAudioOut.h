//
// This file contains 'Framework Code' and is licensed as such 
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.                 
//
/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SimpleAudioOut.h
    
Abstract:

    EFI Simple Audio Out Protocol 

Revision History

--*/

#ifndef _EFI_SIMPLE_AUDIO_OUT_H_
#define _EFI_SIMPLE_AUDIO_OUT_H_

//
// GUID for Simple Audio Out Protocol
//
#define EFI_SIMPLE_AUDIO_OUT_PROTOCOL_GUID \
  { \
    0xc723f288, 0x52f9, 0x4d80, 0xb6, 0x33, 0xe1, 0x52, 0xf9, 0x30, 0xa0, 0xdc \
  }

EFI_FORWARD_DECLARATION (EFI_SIMPLE_AUDIO_OUT_PROTOCOL);

typedef enum {
  EfiAudioOutStart,
  EfiAudioOutPause,
  EfiAudioOutContinue,
  EfiAudioOutStop
} EFI_SIMPLE_AUDIO_OUT_OPERATION;

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_AUDIO_OUT_STREAM_OPERATION) (
  IN EFI_SIMPLE_AUDIO_OUT_PROTOCOL                         * This,
  IN     EFI_SIMPLE_AUDIO_OUT_OPERATION                    Operation,
  IN     UINT8                                             *Buffer OPTIONAL,
  IN     UINT32                                            BufferSize OPTIONAL
  );

#define EFI_SIMPLE_AUDIO_OUT_STATE_BUFFER_DRAIN 0x00000001

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_AUDIO_OUT_QUERY_STATE) (
  IN EFI_SIMPLE_AUDIO_OUT_PROTOCOL           * This,
  OUT UINT32                                 *State
  );

//
// Ac97 Audio channel definition
//
typedef enum _AC97_AUDIO_CHANNEL_NUMBER
{
  Ac97Channels2       = 0,
  Ac97Channels4       = 1,
  Ac97Channels6       = 2,
} AC97_AUDIO_CHANNEL_NUMBER;

//
// Ac97 Audio mode definition
//
typedef enum _AC97_AUDIO_PCM_MODE
{
  Ac97Audio16Bit      = 0,
  Ac97Audio20Bit      = 1,
} AC97_AUDIO_PCM_MODE;

//
// Ac97 Audio volume definition
//
#pragma pack(1)

typedef struct {
  UINT16  RigthChannelVol : 6;
  UINT16  Reserved1 : 2;
  UINT16  LeftChannelVol : 6;
  UINT16  Reserved2 : 1;
  UINT16  Mute : 1;
} AC97_AUDIO_GENERAL_VOLUME;

typedef struct {
  UINT16  Treble : 4;
  UINT16  Reserved1 : 4;
  UINT16  Bass : 4;
  UINT16  Reserved2 : 4;
} AC97_AUDIO_GENERAL_TONE;

#pragma pack()
//
// Ac97 Audio driver attribute definition
//
typedef enum {
  EfiAudioOutChannelNumber,
  EfiAudioOutSampleRate,
  EfiAudioOutPcmMode,
  EfiAudioOutMasterVolume,
  EfiAudioOutAuxVolume,
  EfiAudioOutMonoVolume,
  EfiAudioOutMasterTone,
} EFI_SIMPLE_AUDIO_OUT_ATTRIBUTE_TARGET;

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_AUDIO_OUT_GET_ATTRIBUTE) (
  IN EFI_SIMPLE_AUDIO_OUT_PROTOCOL              * This,
  IN     EFI_SIMPLE_AUDIO_OUT_ATTRIBUTE_TARGET  Target,
  OUT VOID                                      *Buffer,
  IN     UINT32                                 BufferSize
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_AUDIO_OUT_SET_ATTRIBUTE) (
  IN EFI_SIMPLE_AUDIO_OUT_PROTOCOL              * This,
  IN     EFI_SIMPLE_AUDIO_OUT_ATTRIBUTE_TARGET  Target,
  IN     VOID                                   *Buffer,
  IN     UINT32                                 BufferSize
  );

typedef struct _EFI_SIMPLE_AUDIO_OUT_PROTOCOL {
  EFI_SIMPLE_AUDIO_OUT_STREAM_OPERATION StreamOperation;
  EFI_SIMPLE_AUDIO_OUT_QUERY_STATE      QueryState;
  EFI_SIMPLE_AUDIO_OUT_GET_ATTRIBUTE    GetAttribute;
  EFI_SIMPLE_AUDIO_OUT_SET_ATTRIBUTE    SetAttribute;
} EFI_SIMPLE_AUDIO_OUT_PROTOCOL;

extern EFI_GUID gEfiSimpleAudioOutProtocolGuid;

#endif
