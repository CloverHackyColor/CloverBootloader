@REM ## @file
@REM #
@REM #  Currently, Build system does not provide post build mechanism for module 
@REM #  and platform building, so just use a bat file to do post build commands.
@REM #  Originally, following post building command is for EfiLoader module.
@REM #
@REM #  Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
@REM #
@REM #  This program and the accompanying materials
@REM #  are licensed and made available under the terms and conditions of the BSD License
@REM #  which accompanies this distribution. The full text of the license may be found at
@REM #  http://opensource.org/licenses/bsd-license.php
@REM #  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
@REM #  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@REM #
@REM #
@REM ##

@set BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32
@set BOOTSECTOR_BIN_DIR=%WORKSPACE%\Clover\BootSector\bin
@set PROCESSOR=""
@call %WORKSPACE%\Clover\GetVariables.bat

@if NOT "%1"=="" @set TARGET_ARCH=%1
@if "%TARGET_ARCH%"=="IA32" set PROCESSOR=IA32
@if "%TARGET_ARCH%"=="X64" set PROCESSOR=X64
@if %PROCESSOR%=="" goto WrongArch

@set BUILD_DIR=%WORKSPACE%\Build\Clover\%TARGET%_%TOOL_CHAIN_TAG%


@echo Compressing DUETEFIMainFv.FV ...
@%BASETOOLS_DIR%\LzmaCompress -e -o %BUILD_DIR%\FV\DUETEFIMAINFV.z %BUILD_DIR%\FV\DUETEFIMAINFV.Fv

@echo Compressing DxeMain.efi ...
@%BASETOOLS_DIR%\LzmaCompress -e -o %BUILD_DIR%\FV\DxeMain.z %BUILD_DIR%\%PROCESSOR%\DxeCore.efi

@echo Compressing DxeIpl.efi ...
@%BASETOOLS_DIR%\LzmaCompress -e -o %BUILD_DIR%\FV\DxeIpl.z %BUILD_DIR%\%PROCESSOR%\DxeIpl.efi

@echo Generate Loader Image ...
@if "%PROCESSOR%"=="IA32" goto GENERATE_IMAGE_IA32
@if "%PROCESSOR%"=="X64" goto GENERATE_IMAGE_X64

:GENERATE_IMAGE_IA32
@%BASETOOLS_DIR%\EfiLdrImage.exe -o %BUILD_DIR%\FV\Efildr32 %BUILD_DIR%\%PROCESSOR%\EfiLoader.efi %BUILD_DIR%\FV\DxeIpl.z %BUILD_DIR%\FV\DxeMain.z %BUILD_DIR%\FV\DUETEFIMAINFV.z
@rem @copy /b %BOOTSECTOR_BIN_DIR%\Start.com+%BOOTSECTOR_BIN_DIR%\Efi32.com2+%BUILD_DIR%\FV\Efildr32 %BUILD_DIR%\FV\Efildr
@rem @copy /b %BOOTSECTOR_BIN_DIR%\Start16.com+%BOOTSECTOR_BIN_DIR%\Efi32.com2+%BUILD_DIR%\FV\Efildr32 %BUILD_DIR%\FV\Efildr16
@rem @copy /b %BOOTSECTOR_BIN_DIR%\Start32.com+%BOOTSECTOR_BIN_DIR%\Efi32.com3+%BUILD_DIR%\FV\Efildr32 %BUILD_DIR%\FV\Efildr20
@copy /b %BOOTSECTOR_BIN_DIR%\start32H.com2+%BOOTSECTOR_BIN_DIR%\efi32.com3+%BUILD_DIR%\FV\Efildr32 %BUILD_DIR%\FV\boot32

copy /b %BUILD_DIR%\FV\boot32 %WORKSPACE%\Clover\CloverPackage\CloverV2\Bootloaders\ia32\boot
copy /b %BUILD_DIR%\%PROCESSOR%\FSInject.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\EFI\drivers32\FSInject-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\VBoxIso9600.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\VBoxIso9600-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\VBoxExt2.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\VBoxExt2-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\Ps2KeyboardDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\Ps2KeyboardDxe-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\Ps2MouseAbsolutePointerDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\Ps2MouseAbsolutePointerDxe-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\Ps2MouseDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\Ps2MouseDxe-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\UsbMouseDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\UsbMouseDxe-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\XhciDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32\XhciDxe-32.efi
copy /b %BUILD_DIR%\%PROCESSOR%\OsxFatBinaryDrv.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers32UEFI\OsxFatBinaryDrv-32.efi

@goto end

:GENERATE_IMAGE_X64
@%BASETOOLS_DIR%\EfiLdrImage.exe -o %BUILD_DIR%\FV\Efildr64 %BUILD_DIR%\%PROCESSOR%\EfiLoader.efi %BUILD_DIR%\FV\DxeIpl.z %BUILD_DIR%\FV\DxeMain.z %BUILD_DIR%\FV\DUETEFIMAINFV.z
@rem @copy /b %BOOTSECTOR_BIN_DIR%\Start64.com+%BOOTSECTOR_BIN_DIR%\Efi64.com2+%BUILD_DIR%\FV\Efildr64 %BUILD_DIR%\FV\EfildrPure
@rem @%BASETOOLS_DIR%\GenPage.exe %BUILD_DIR%\FV\EfildrPure -o %BUILD_DIR%\FV\Efildr
@rem @copy /b %BOOTSECTOR_BIN_DIR%\St16_64.com+%BOOTSECTOR_BIN_DIR%\Efi64.com2+%BUILD_DIR%\FV\Efildr64 %BUILD_DIR%\FV\Efildr16Pure
@rem @%BASETOOLS_DIR%\GenPage.exe %BUILD_DIR%\FV\Efildr16Pure -o %BUILD_DIR%\FV\Efildr16
@copy /b %BOOTSECTOR_BIN_DIR%\Start64H.com+%BOOTSECTOR_BIN_DIR%\efi64.com3+%BUILD_DIR%\FV\Efildr64 %BUILD_DIR%\FV\Efildr20Pure
@%BASETOOLS_DIR%\GenPage.exe %BUILD_DIR%\FV\Efildr20Pure -o %BUILD_DIR%\FV\Efildr20
@%BASETOOLS_DIR%\Split.exe -f %BUILD_DIR%\FV\Efildr20 -p %BUILD_DIR%\FV\ -o Efildr20.1 -t boot64 -s 512
@del %BUILD_DIR%\FV\Efildr20.1

copy /b %BUILD_DIR%\FV\boot64 %WORKSPACE%\Clover\CloverPackage\CloverV2\Bootloaders\x64\boot
copy /b %BUILD_DIR%\%PROCESSOR%\FSInject.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\EFI\drivers64\FSInject-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\FSInject.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\EFI\drivers64UEFI\FSInject-64.efi
@rem copy /b %BUILD_DIR%\%PROCESSOR%\VBoxIso9600.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\VBoxIso9600-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\VBoxExt2.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\VBoxExt2-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\PartitionDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64UEFI\PartitionDxe-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\DataHubDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64UEFI\DataHubDxe-64.efi

@rem copy /b %BUILD_DIR%\%PROCESSOR%\Ps2KeyboardDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\Ps2KeyboardDxe-64.efi
@rem copy /b %BUILD_DIR%\%PROCESSOR%\Ps2MouseAbsolutePointerDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\Ps2MouseAbsolutePointerDxe-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\Ps2MouseDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\Ps2MouseDxe-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\UsbMouseDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\UsbMouseDxe-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\XhciDxe.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64\XhciDxe-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\OsxFatBinaryDrv.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64UEFI\OsxFatBinaryDrv-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\OsxAptioFixDrv.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64UEFI\OsxAptioFixDrv-64.efi
copy /b %BUILD_DIR%\%PROCESSOR%\OsxLowMemFixDrv.efi %WORKSPACE%\Clover\CloverPackage\CloverV2\drivers-Off\drivers64UEFI\OsxLowMemFixDrv-64.efi

@goto end


:WrongArch
@echo Error! Wrong architecture.
@goto Help

:Help
@echo Usage: "PostBuild [IA32|X64]"
:end