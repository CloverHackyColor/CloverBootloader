/*
 *  AppleKeyMapUtils.c
 *  
 *  Created by Jief on 25 May 2018.
 *
 */

EFI_STATUS getAppleKeyMapDb();

VOID EFIAPI SendKeyRelease (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );

EFI_STATUS EFIAPI SendDataToAppleMap(IN EFI_KEY_DATA *KeyData);
EFI_STATUS EFIAPI AppleKeyMapUtilsInit (
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    );
