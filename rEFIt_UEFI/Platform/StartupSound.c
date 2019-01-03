//
//  StartupSound.c
//  Clover
//
//  Created by Sergey on 02.01.2019.
// based on AudioPkg by Goldfish64
// https://github.com/Goldfish64/AudioPkg
//
/*
* Copyright (c) 2018 John Davis
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/


#include "Platform.h"
extern BOOLEAN DayLight;

#ifndef DEBUG_ALL
#define DEBUG_SOUND 1
#else
#define DEBUG_SOUND DEBUG_ALL
#endif

#if DEBUG_SOUND == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SOUND, __VA_ARGS__)
#endif


//EFI_GUID gBootChimeVendorVariableGuid = {0x89D4F995, 0x67E3, 0x4895, { 0x8F, 0x18, 0x45, 0x4B, 0x65, 0x1D, 0x92, 0x15 }};

extern EFI_GUID gBootChimeVendorVariableGuid;

#define BOOT_CHIME_VAR_ATTRIBUTES   (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)
#define BOOT_CHIME_VAR_DEVICE       (L"Device")
#define BOOT_CHIME_VAR_INDEX        (L"Index")
#define BOOT_CHIME_VAR_VOLUME       (L"Volume")

EFI_STATUS
BootChimeGetStoredOutput(
                         OUT EFI_AUDIO_IO_PROTOCOL **AudioIo,
                         OUT UINTN *Index,
                         OUT UINT8 *Volume);


EFI_STATUS
StartupSoundPlay(EFI_FILE *Dir, CHAR16* SoundFile)
{
  EFI_STATUS Status  = EFI_NOT_FOUND;
  UINT8           *FileData = NULL;
  UINTN           FileDataLength = 0U;
  WAVE_FILE_DATA WaveData;
  EFI_AUDIO_IO_PROTOCOL *AudioIo = NULL;
  UINTN OutputIndex = 0;
  UINT8 OutputVolume = 0;

  Status = egLoadFile(Dir, SoundFile, &FileData, &FileDataLength);
  if (EFI_ERROR(Status)) {
//    Status = egLoadFile(SelfRootDir, SoundFile, &FileData, &FileDataLength);
//    if (EFI_ERROR(Status)) {
    DBG("file sound not found\n");
      return Status;
//    }
  }

  Status = WaveGetFileData(FileData, (UINT32)FileDataLength, &WaveData);
  if (EFI_ERROR(Status)) {
    
    return Status;
  }

  MsgLog("  Channels: %u  Sample rate: %u Hz  Bits: %u\n", WaveData.Format->Channels, WaveData.Format->SamplesPerSec, WaveData.Format->BitsPerSample);

  EFI_AUDIO_IO_PROTOCOL_BITS bits;
  switch (WaveData.Format->BitsPerSample) {
    case 8:
      bits = EfiAudioIoBits8;
      break;
    case 16:
      bits = EfiAudioIoBits16;
      break;
    case 20:
      bits = EfiAudioIoBits20;
      break;
    case 24:
      bits = EfiAudioIoBits24;
      break;
    case 32:
      bits = EfiAudioIoBits32;
      break;
    default:
      return EFI_UNSUPPORTED;
  }

  EFI_AUDIO_IO_PROTOCOL_FREQ freq;
  switch (WaveData.Format->SamplesPerSec) {
    case 8000:
      freq = EfiAudioIoFreq8kHz;
      break;
    case 11000:
      freq = EfiAudioIoFreq11kHz;
      break;
    case 16000:
      freq = EfiAudioIoFreq16kHz;
      break;
    case 22050:
      freq = EfiAudioIoFreq22kHz;
      break;
    case 32000:
      freq = EfiAudioIoFreq32kHz;
      break;
    case 44100:
      freq = EfiAudioIoFreq44kHz;
      break;
    case 48000:
      freq = EfiAudioIoFreq48kHz;
      break;
    case 88000:
      freq = EfiAudioIoFreq88kHz;
      break;
    case 96000:
      freq = EfiAudioIoFreq96kHz;
      break;
    case 192000:
      freq = EfiAudioIoFreq192kHz;
      break;
    default:
      return EFI_UNSUPPORTED;
  }

  Status = BootChimeGetStoredOutput(&AudioIo, &OutputIndex, &OutputVolume);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DBG("output to channel %d with volume %d, len=%d\n", OutputIndex, OutputVolume, WaveData.SamplesLength);

  if (!WaveData.SamplesLength || !OutputVolume) {
    DBG("nothing to play\n");
    goto DONE_ERROR;
  }
  // Setup playback.
  Status = AudioIo->SetupPlayback(AudioIo, OutputIndex, OutputVolume,
                                  freq, bits, WaveData.Format->Channels);
  if (EFI_ERROR(Status)) {
    MsgLog("StartupSound: Error setting up playback: %r\n", Status);
    goto DONE_ERROR;
  }

  // Start playback.
//  Status = AudioIo->StartPlaybackAsync(AudioIo, WaveData.Samples, WaveData.SamplesLength, 0,                                       NULL, NULL);
  Status = AudioIo->StartPlayback(AudioIo, WaveData.Samples, WaveData.SamplesLength, 0);
  if (EFI_ERROR(Status)) {
    MsgLog("StartupSound: Error starting playback: %r\n", Status);
    goto DONE_ERROR;
  }

DONE_ERROR:
  if (FileData) {
    FreePool(FileData);
  }
  return Status;
}

EFI_STATUS
BootChimeGetStoredOutput(
                         OUT EFI_AUDIO_IO_PROTOCOL **AudioIo,
                         OUT UINTN *Index,
                         OUT UINT8 *Volume)
{
  // Create variables.
  EFI_STATUS Status;

  // Device Path.
  CHAR16 *StoredDevicePathStr = NULL;
  UINTN StoredDevicePathStrSize = 0;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;
  CHAR16 *DevicePathStr = NULL;

  // Audio I/O.
  EFI_HANDLE *AudioIoHandles = NULL;
  UINTN AudioIoHandleCount = 0;
  EFI_AUDIO_IO_PROTOCOL *AudioIoProto = NULL;

  // Output.
  UINTN OutputPortIndex;
  UINTN OutputPortIndexSize = sizeof(OutputPortIndex);
  UINT8 OutputVolume;
  UINTN OutputVolumeSize = sizeof(OutputVolume);

  // Get Audio I/O protocols.
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiAudioIoProtocolGuid, NULL, &AudioIoHandleCount, &AudioIoHandles);
  if (EFI_ERROR(Status)) {
    MsgLog("No AudioIoProtocol, status=%r\n", Status);
    goto DONE;
  }
  DBG("found %d handles with audio\n", AudioIoHandleCount);
  // Get stored device path string size.
  Status = gRT->GetVariable(BOOT_CHIME_VAR_DEVICE, &gBootChimeVendorVariableGuid, NULL,
                            &StoredDevicePathStrSize, NULL);
  if (EFI_ERROR(Status) && (Status != EFI_BUFFER_TOO_SMALL)) {
    MsgLog("No AudioIoDevice, status=%r\n", Status);
    goto DONE;
  }
  // Allocate space for device path string.
  StoredDevicePathStr = AllocateZeroPool(StoredDevicePathStrSize);
  if (StoredDevicePathStr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  // Get stored device path string.
  Status = gRT->GetVariable(BOOT_CHIME_VAR_DEVICE, &gBootChimeVendorVariableGuid, NULL,
                            &StoredDevicePathStrSize, StoredDevicePathStr);
  if (EFI_ERROR(Status)) {
    MsgLog("No AudioIoDevice, status=%r\n", Status);
    goto DONE;
  }
  // Try to find the matching device exposing an Audio I/O protocol.
  for (UINTN h = 0; h < AudioIoHandleCount; h++) {
    // Open Device Path protocol.
    Status = gBS->HandleProtocol(AudioIoHandles[h], &gEfiDevicePathProtocolGuid, (VOID**)&DevicePath);
    if (EFI_ERROR(Status)) {
      DBG("no DevicePath at %d handle AudioIo\n", h);
      continue;
    }

    // Convert Device Path to string for comparison.
    DevicePathStr = ConvertDevicePathToText(DevicePath, FALSE, FALSE);
    if (DevicePathStr == NULL) {
      DBG("dont convert DevicePath %d\n", h);
      continue;
    }
    DBG("stored device=%s\n", DevicePathStr);
    // Compare Device Paths. If they match, we have our Audio I/O device.
    if (StrCmp(StoredDevicePathStr, DevicePathStr) == 0) {
      // Open Audio I/O protocol.
      Status = gBS->HandleProtocol(AudioIoHandles[h], &gEfiAudioIoProtocolGuid, (VOID**)&AudioIoProto);
      if (EFI_ERROR(Status)) {
        DBG("dont handle AudioIo\n");
        goto DONE;
      }
      break;
    }
    FreePool(DevicePathStr);
    DevicePathStr = NULL;
  }

  // If the Audio I/O variable is still null, we couldn't find it.
  if (AudioIoProto == NULL) {
    Status = EFI_NOT_FOUND;
    DBG("not found AudioIo\n");
    goto DONE;
  }

  // Get stored device index.
  Status = gRT->GetVariable(BOOT_CHIME_VAR_INDEX, &gBootChimeVendorVariableGuid, NULL,
                            &OutputPortIndexSize, &OutputPortIndex);
  if (EFI_ERROR(Status)) {
    MsgLog("Bad output index, status=%r\n", Status);
    goto DONE;
  }
  DBG("got index=%d\n", OutputPortIndex);
  // Get stored volume. If this fails, just use the max.
  Status = gRT->GetVariable(BOOT_CHIME_VAR_VOLUME, &gBootChimeVendorVariableGuid, NULL,
                            &OutputVolumeSize, &OutputVolume);
  if (EFI_ERROR(Status)) {
    OutputVolume = EFI_AUDIO_IO_PROTOCOL_MAX_VOLUME;
    Status = EFI_SUCCESS;
  }
  DBG("got volume %d\n", OutputVolume);
  // Success.
  *AudioIo = AudioIoProto;
  *Index = OutputPortIndex;
  *Volume = OutputVolume;
  Status = EFI_SUCCESS;

DONE:
  if (StoredDevicePathStr != NULL)
    FreePool(StoredDevicePathStr);
  if (AudioIoHandles != NULL)
    FreePool(AudioIoHandles);
  if (DevicePathStr != NULL)
    FreePool(DevicePathStr);
  return Status;
}
