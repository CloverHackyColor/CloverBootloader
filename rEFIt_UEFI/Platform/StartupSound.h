/*
 * StartupSound.h
 *
 *  Created on: 4 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_STARTUPSOUND_H_
#define PLATFORM_STARTUPSOUND_H_


#define BOOT_CHIME_VAR_ATTRIBUTES   (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)
#define BOOT_CHIME_VAR_DEVICE       (L"Device")
#define BOOT_CHIME_VAR_DEVICE_PATH  (L"device_path")
#define BOOT_CHIME_VAR_INDEX        (L"Index")
#define BOOT_CHIME_VAR_VOLUME       (L"Volume")



extern EFI_AUDIO_IO_PROTOCOL    *AudioIo;


EFI_STATUS
StartupSoundPlay(const EFI_FILE* Dir, CONST CHAR16* SoundFile);

void GetOutputs();

EFI_STATUS CheckSyncSound(BOOLEAN Stop);

#endif /* PLATFORM_STARTUPSOUND_H_ */
