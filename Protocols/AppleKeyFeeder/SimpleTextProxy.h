/*
 *  SimpleTextProxy.c
 *  
 *  Created by Jief on 25 May 2018.
 *
 */

extern EFI_SIMPLE_TEXT_INPUT_PROTOCOL AppleKeyFeederOriginalSimpleText;
extern EFI_SIMPLE_TEXT_INPUT_PROTOCOL AppleKeyFeederSimpleTextProxy;

/*
 * Buffering keys
 */
VOID
EFIAPI
SimpleTextProxyBufferKeyStroke (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  );


/**
  Reset the input device and optionally run diagnostics

  @param  This                 Protocol instance pointer.
  @param  ExtendedVerification Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS          The device was reset.
  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could not be reset.

**/

EFI_STATUS
(EFIAPI AppleKeyFeederSimpleTextProxyReset)(
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL       *This,
  IN BOOLEAN                              ExtendedVerification
  );

/**
  Reads the next keystroke from the input device. The WaitForKey Event can
  be used to test for existence of a keystroke via WaitForEvent () call.

  @param  This  Protocol instance pointer.
  @param  Key   A pointer to a buffer that is filled in with the keystroke
                information for the key that was pressed.

  @retval EFI_SUCCESS      The keystroke information was returned.
  @retval EFI_NOT_READY    There was no keystroke data available.
  @retval EFI_DEVICE_ERROR The keystroke information was not returned due to
                           hardware errors.

**/

EFI_STATUS
(EFIAPI AppleKeyFeederSimpleTextProxyReadKeyStroke)(
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL       *This,
  OUT EFI_INPUT_KEY                       *Key
  );


EFI_STATUS
EFIAPI
SimpleTextProxyInit(
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    );


/*
 *
 */
VOID
EFIAPI
WaitForKeyEvent (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  );
