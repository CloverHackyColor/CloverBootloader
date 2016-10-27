//
//  AppleProtocols.h
//  Clover
//
//  Created by Slice on 14.10.16.
//

#ifndef Clover_AppleProtocols_h
#define Clover_AppleProtocols_h


EFI_STATUS EFIAPI
OvrAppleSMC(VOID);

EFI_STATUS EFIAPI
OvrAppleImageCodec(VOID);

EFI_STATUS EFIAPI
OvrAppleKeyState(VOID);

EFI_STATUS EFIAPI
OvrOSInfo(VOID);

EFI_STATUS EFIAPI
OvrGraphConfig(VOID);

EFI_STATUS EFIAPI
OvrFirmwareVolume(VOID);


EFI_STATUS EFIAPI
OvrEfiKeyboardInfo(VOID);

EFI_STATUS EFIAPI
OvrAppleKeyMapDb(VOID);


#endif
