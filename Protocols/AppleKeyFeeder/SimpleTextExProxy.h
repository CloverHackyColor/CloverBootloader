/*
 *  SimpleTextExProxy.h
 *  
 *  Created by Jief on 25 May 2018.
 *
 */

extern EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL AppleKeyFeederSimpleTextExProxy;


EFI_STATUS
EFIAPI SimpleTextExProxyReadKeyStroke(
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  );


void
EFIAPI
SimpleTextExProxyBufferKeyStroke (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );


VOID
EFIAPI
WaitForKeyExEvent (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  );


EFI_STATUS
EFIAPI
SimpleTextExProxyInit(
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    );
