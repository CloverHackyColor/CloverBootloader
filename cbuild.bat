@echo off
:: windows batch script for building clover
:: 2012-09-06 apianti

set ENABLE_SECURE_BOOT=0

:: setup current dir and edk2 if needed
pushd .
set "CURRENTDIR=%CD%"
if not defined WORKSPACE (
  echo Searching for EDK2
  goto searchforedk
)

set "BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32"
if defined EDK_TOOLS_BIN (
  set "BASETOOLS_DIR=%EDK_TOOLS_BIN%"
)

:searchforedk
  if exist edksetup.bat (
    call edksetup.bat
    @echo off
    goto foundedk
  )
  if x"%CD%" == x"%~d0%\" (
    cd "%CURRENTDIR%"
    echo No EDK found!
    goto failscript
  )
  cd ..
  goto searchforedk

:: have edk2 prepare to build
:foundedk
  echo Found EDK2. Generating %WORKSPACE%\Clover\Version.h
  cd "%WORKSPACE%\Clover"
  :: get svn revision number
  svnversion -n > vers.txt
  set /p s= < vers.txt
  del vers.txt
  set SVNREVISION=

:: get the current revision number
:fixrevision
  if x"%s%" == x"" goto prebuild
  set c=%s:~0,1%
  set s=%s:~1%
  if x"%c::=%" == x"" goto prebuild
  if x"%c:M=%" == x"" goto prebuild
  if x"%c:S=%" == x"" goto prebuild
  if x"%c:P=%" == x"" goto prebuild
  set SVNREVISION=%SVNREVISION%%c%
  goto fixrevision

:prebuild
  cd "%CURRENTDIR%"

  :: check for revision number
  if x"%SVNREVISION%" == x"" goto failscript

  :: parse parameters for what we need
  set TARGETARCH=
  set TOOLCHAIN=
  set BUILDTARGET=
  set DSCFILE=
  set CLEANING=
  set errorlevel=0
  set THREADNUMBER=

  set EDK2_BUILD_OPTIONS=
  set VBIOSPATCHCLOVEREFI=0
  set ONLYSATA0PATCH=0
  set USE_BIOS_BLOCKIO=0
  set USE_LOW_EBDA=1
  set GENPAGE=0
  set DISABLE_USB_SUPPORT=0

  call:parseArguments %*
  if errorlevel 1 goto failscript

  :: fix any parameters not set
  set CONFIG_FILE="%WORKSPACE%\Conf\target.txt"

  set DEFAULT_TOOLCHAIN=MYTOOLS
  set DEFAULT_BUILDTARGET=RELEASE
  set DEFAULT_TARGETARCH=X64
  set DEFAULT_THREADNUMBER=%NUMBER_OF_PROCESSORS%

  for /f "tokens=1*" %%i in ('type %CONFIG_FILE% ^| find "TOOL_CHAIN_TAG" ^| find /V "#"') do set SCAN_TOOLCHAIN%%j
  if not x"%SCAN_TOOLCHAIN%" == x"" set DEFAULT_TOOLCHAIN=%SCAN_TOOLCHAIN%

  for /f "tokens=1*" %%i in ('type %CONFIG_FILE% ^| find "TARGET" ^| find /V "#" ^| find /V "TARGET_ARCH"') do set SCAN_BUILDTARGET%%j
  if not x"%SCAN_BUILDTARGET%" == x"" set DEFAULT_BUILDTARGET=%SCAN_BUILDTARGET%

  :: no TARGETARCH = ALL
  :: for /f "tokens=1*" %%i in ('type %CONFIG_FILE% ^| find "TARGET_ARCH" ^| find /V "#"') do set SCAN_TARGETARCH%%j
  :: if not x"%SCAN_TARGETARCH%" == x"" set DEFAULT_TARGETARCH=%SCAN_TARGETARCH%

  if x"%TOOLCHAIN%" == x"" set TOOLCHAIN=%DEFAULT_TOOLCHAIN%
  if x"%BUILDTARGET%" == x"" set BUILDTARGET=%DEFAULT_BUILDTARGET%
  :: if x"%TARGETARCH%" == x"" set TARGETARCH=%DEFAULT_TARGETARCH%%
  if x"%THREADNUMBER%" == x"" set THREADNUMBER=%DEFAULT_THREADNUMBER%

  :: Apply options
  if x"%USE_BIOS_BLOCKIO%" == x"1" call:addEdk2BuildMacro "USE_BIOS_BLOCKIO"
  if x"%VBIOSPATCHCLOVEREFI%" == x"1" call:addEdk2BuildMacro "ENABLE_VBIOS_PATCH_CLOVEREFI"
  if x"%ONLYSATA0PATCH%" == x"1" call:addEdk2BuildMacro "ONLY_SATA_0"
  if x"%USE_LOW_EBDA%" == x"1" call:addEdk2BuildMacro "USE_LOW_EBDA"
  if x"%DISABLE_USB_SUPPORT%" == x"1" call:addEdk2BuildMacro "DISABLE_USB_SUPPORT"

  :: check DSC: Clover default
  if x"%DSCFILE%" == x"" set DSCFILE="%CURRENTDIR%\Clover.dsc"

  set "ARCH_ARGUMENT="
  if not x"%TARGETARCH%" == x"" goto buildall
  echo Building selected (X64 ^& IA32) ...

:buildall
  if x"%TARGETARCH%" == x"IA32" goto build32

  set "TARGETARCH=X64"

  echo Building Clover (X64) ...
  goto startbuild

:build32
  set "TARGETARCH=IA32"

  echo Building Clover (IA32) ...
  goto startbuild

:startbuild
  if x"%DSCFILE%" == x"" goto failscript

  if not x"%CLEANING%" == x"" echo Clean build (%CLEANING%) ...

  set "MY_ARCH=-a %TARGETARCH%"
  set "MY_TOOLCHAIN=-t %TOOLCHAIN%"
  set "MY_BUILDTARGET=-b %BUILDTARGET%"
  set "MY_THREADNUMBER=-n %THREADNUMBER%"

  for /f "tokens=2 delims=[]" %%x in ('ver') do set WINVER=%%x
  set WINVER=%WINVER:Version =%

  set "CMD_BUILD=build -p %DSCFILE% -DNO_GRUB_DRIVERS %EDK2_BUILD_OPTIONS% %MY_ARCH% %MY_TOOLCHAIN% %MY_BUILDTARGET% %MY_THREADNUMBER% %CLEANING%"

  set clover_build_info=%CMD_BUILD%
  set clover_build_info=%clover_build_info:\=\\%
  set clover_build_info=%clover_build_info:"=\"%
  set clover_build_info="Args: %~nx0 %* | Command: %clover_build_info% | OS: Win %WINVER%"

  :: generate build date and time
  set BUILDDATE=
  echo Dim cdt, output, temp > buildtime.vbs
  :: output year
  echo cdt = Now >> buildtime.vbs
  echo output = Year(cdt) ^& "-" >> buildtime.vbs
  :: output month
  echo temp = Month(cdt) >> buildtime.vbs
  echo If temp ^< 10 Then >> buildtime.vbs
  echo    output = output ^& "0" >> buildtime.vbs
  echo End If >> buildtime.vbs
  echo output = output ^& temp ^& "-" >> buildtime.vbs
  :: output day
  echo temp = Day(cdt) >> buildtime.vbs
  echo If temp ^< 10 Then >> buildtime.vbs
  echo    output = output ^& "0" >> buildtime.vbs
  echo End If >> buildtime.vbs
  echo output = output ^& temp ^& " " >> buildtime.vbs
  :: output hours
  echo temp = Hour(cdt) >> buildtime.vbs
  echo If temp ^< 10 Then >> buildtime.vbs
  echo    output = output ^& "0" >> buildtime.vbs
  echo End If >> buildtime.vbs
  echo output = output ^& temp ^& ":" >> buildtime.vbs
  :: output minutes
  echo temp = Minute(cdt) >> buildtime.vbs
  echo If temp ^< 10 Then >> buildtime.vbs
  echo    output = output ^& "0" >> buildtime.vbs
  echo End If >> buildtime.vbs
  echo output = output ^& temp ^& ":" >> buildtime.vbs
  :: output seconds
  echo temp = Second(cdt) >> buildtime.vbs
  echo If temp ^< 10 Then >> buildtime.vbs
  echo    output = output ^& "0" >> buildtime.vbs
  echo End If >> buildtime.vbs
  echo output = output ^& temp >> buildtime.vbs
  echo Wscript.Echo output >> buildtime.vbs
  cscript //Nologo buildtime.vbs > buildtime.txt
  del buildtime.vbs
  set /p BUILDDATE= < buildtime.txt
  del buildtime.txt

  :: generate version.h
  echo // Autogenerated Version.h> Version.h
  echo #define FIRMWARE_VERSION "2.31">> Version.h
  echo #define FIRMWARE_BUILDDATE "%BUILDDATE%">> Version.h
  echo #define FIRMWARE_REVISION L"%SVNREVISION%">> Version.h
  echo #define REVISION_STR "Clover revision: %SVNREVISION%">> Version.h
  echo #define BUILDINFOS_STR %clover_build_info%>> Version.h

  %CMD_BUILD%

  if errorlevel 1 goto failscript

  if not x"%CLEANING%" == x"" goto:eof
  goto postbuild

:postbuild
  echo Performing post build operations ...
  set SIGNTOOL_BUILD_DIR=%WORKSPACE%\Clover\SignTool
  set SIGNTOOL_BUILD=BuildSignTool.bat
  set SIGNTOOL=%WORKSPACE%\Clover\SignTool\SignTool.exe
  set BUILD_DIR=%WORKSPACE%\Build\Clover\%BUILDTARGET%_%TOOLCHAIN%
  set DEST_DIR=%WORKSPACE%\Clover\CloverPackage\CloverV2
  :: set BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32
  set BOOTSECTOR_BIN_DIR=%WORKSPACE%\Clover\BootSector\bin
  set BUILD_DIR_ARCH=%BUILD_DIR%\%TARGETARCH%

  if x"%ENABLE_SECURE_BOOT%" == x"1" (
    echo Building signing tool ...
    pushd .
    cd "%SIGNTOOL_BUILD_DIR%"
    call "%SIGNTOOL_BUILD%"
    popd
    if errorlevel 1 goto failscript
  )

  if x"%TARGETARCH%" == x"IA32" goto postbuild32

  echo Compressing DUETEFIMainFv.FV (X64) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DUETEFIMAINFVX64.z" "%BUILD_DIR%\FV\DUETEFIMAINFVX64.Fv"

  echo Compressing DxeMain.efi (X64) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeMainX64.z" "%BUILD_DIR%\X64\DxeCore.efi"

  echo Compressing DxeIpl.efi (X64) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeIplX64.z" "%BUILD_DIR%\X64\DxeIpl.efi"

  echo Generating Loader Image (X64) ...
  "%BASETOOLS_DIR%\EfiLdrImage.exe" -o "%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\X64\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIplX64.z" "%BUILD_DIR%\FV\DxeMainX64.z" "%BUILD_DIR%\FV\DUETEFIMAINFVX64.z"

  :: copy /B "%BOOTSECTOR_BIN_DIR%\Start64.com"+"%BOOTSECTOR_BIN_DIR%\Efi64.com2"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\EfildrPure"
  :: "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\EfildrPure" -o "%BUILD_DIR%\FV\Efildr"
  :: copy /B "%BOOTSECTOR_BIN_DIR%\St16_64.com"+"%BOOTSECTOR_BIN_DIR%\Efi64.com2"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\Efildr16Pure"
  :: "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr16Pure -o "%BUILD_DIR%\FV\Efildr16"

  :: copy /B "%BOOTSECTOR_BIN_DIR%\Start64H.com"+"%BOOTSECTOR_BIN_DIR%\efi64.com3"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\Efildr20Pure"
  :: "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr20Pure" -o "%BUILD_DIR%\FV\Efildr20"

  if x"%GENPAGE%" == x"0" (
    if not x"%USE_LOW_EBDA%" == x"0" (
      set filesize=0
      call:getFilesize "%BUILD_DIR%\FV\Efildr64"
      if %filesize% gtr 417792 (
        echo warning: boot file bigger than low-ebda permits, switching to --std-ebda
        set USE_LOW_EBDA=0
      )
    )
  )

  Setlocal EnableDelayedExpansion
    :: set COM_NAMES=(H H2 H3 H4 H5 H6 H5 H6)
    set COM_NAMES[0]=H
    set COM_NAMES[1]=H2
    set COM_NAMES[2]=H3
    set COM_NAMES[3]=H4
    set COM_NAMES[4]=H5
    set COM_NAMES[5]=H6
    set /A "block=%GENPAGE% << 2 | %USE_LOW_EBDA% << 1 | %USE_BIOS_BLOCKIO%"
    set "block=COM_NAMES[%block%]"
    set startBlock=Start64!%block%!.com
    set GEN64=Efildr20Pure
    if not x"%GENPAGE%" == x"0" set GEN64=boot
    copy /B "%BOOTSECTOR_BIN_DIR%\%startBlock%"+"%BOOTSECTOR_BIN_DIR%\efi64.com3"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\%GEN64%"
    set GEN64=-o "%BUILD_DIR%\FV\Efildr20"
    if not x"%USE_LOW_EBDA%" == x"0" set GEN64=-b 0x88000 -f 0x68000 %GEN64%
    set GEN64="%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr20Pure" %GEN64%
    %GEN64%
  endlocal

  "%BASETOOLS_DIR%\Split.exe" -f "%BUILD_DIR%\FV\Efildr20" -p %BUILD_DIR%\FV\ -o Efildr20.1 -t boot64 -s 512
  del "%BUILD_DIR%\FV\Efildr20.1"

  :: Be sure that all needed directories exists
  call:createDir "%DEST_DIR%\Bootloaders\x64"
  call:createDir "%DEST_DIR%\EFI\BOOT"
  call:createDir "%DEST_DIR%\EFI\CLOVER\tools"
  call:createDir "%DEST_DIR%\EFI\CLOVER\drivers64"
  call:createDir "%DEST_DIR%\EFI\CLOVER\drivers64UEFI"
  call:createDir "%DEST_DIR%\drivers-Off\drivers64"
  call:createDir "%DEST_DIR%\drivers-Off\drivers64UEFI"

  set /A "cloverEFIFile=(6 + %USE_BIOS_BLOCKIO%)"
  set cloverEFIFile=boot%cloverEFIFile%

  :: CloverEFI
  copy /B /Y "%BUILD_DIR%\FV\boot64" "%DEST_DIR%\Bootloaders\X64\%cloverEFIFile%"

  :: Mandatory drivers
  copy /B /Y "%BUILD_DIR_ARCH%\FSInject.efi" "%DEST_DIR%\EFI\Clover\drivers64\FSInject-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\FSInject.efi" "%DEST_DIR%\EFI\Clover\drivers64UEFI\FSInject-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\OsxFatBinaryDrv.efi" "%DEST_DIR%\EFI\Clover\drivers64UEFI\OsxFatBinaryDrv-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxHfs.efi" "%DEST_DIR%\EFI\Clover\drivers64UEFI\VBoxHfs-64.efi"

  :: Optional drivers : drivers64UEFI
  copy /B /Y "%BUILD_DIR_ARCH%\EmuVariableUefi.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\EmuVariableUefi-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\CsmVideoDxe.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\CsmVideoDxe-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\OsxLowMemFixDrv.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\OsxLowMemFixDrv-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\OsxAptioFixDrv.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\OsxAptioFixDrv-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\OsxAptioFix2Drv.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\OsxAptioFix2Drv-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\DataHubDxe.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\DataHubDxe-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\PartitionDxe.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\PartitionDxe-64.efi"

  :: Optional drivers : drivers64
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxIso9600.efi" "%DEST_DIR%\drivers-Off\drivers64\VBoxIso9600-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxExt2.efi" "%DEST_DIR%\drivers-Off\drivers64\VBoxExt2-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxExt4.efi" "%DEST_DIR%\drivers-Off\drivers64\VBoxExt4-64.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubEXFAT.efi" "%DEST_DIR%\drivers-Off\drivers64\GrubEXFAT-64.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubISO9660.efi" "%DEST_DIR%\drivers-Off\drivers64\GrubISO9660-64.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubNTFS.efi" "%DEST_DIR%\drivers-Off\drivers64\GrubNTFS-64.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubUDF.efi" "%DEST_DIR%\drivers-Off\drivers64\GrubUDF-64.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\Ps2KeyboardDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\Ps2KeyboardDxe-64.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\Ps2MouseAbsolutePointerDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\Ps2MouseAbsolutePointerDxe-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\Ps2MouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\Ps2MouseDxe-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\UsbMouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\UsbMouseDxe-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\XhciDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\XhciDxe-64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\NvmExpressDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\NvmExpressDxe-64.efi"

  :: Applications
  copy /B /Y "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_DIR%\EFI\CLOVER\tools\bdmesg.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVERX64.efi" "%DEST_DIR%\EFI\CLOVER\CLOVERX64.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVERX64.efi" "%DEST_DIR%\EFI\BOOT\BOOTX64.efi"

  call:createBootsector
  if x"%TARGETARCH%" == x"" goto build32
  goto done

:postbuild32
  echo Compressing DUETEFIMainFv.FV (IA32) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DUETEFIMAINFVIA32.z" "%BUILD_DIR%\FV\DUETEFIMAINFVIA32.Fv"

  echo Compressing DxeMain.efi (IA32) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeMainIA32.z" "%BUILD_DIR%\IA32\DxeCore.efi"

  echo Compressing DxeIpl.efi (IA32) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeIplIA32.z" "%BUILD_DIR%\IA32\DxeIpl.efi"

  echo Generating Loader Image (IA32) ...
  "%BASETOOLS_DIR%\EfiLdrImage.exe" -o "%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\IA32\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIplIA32.z" "%BUILD_DIR%\FV\DxeMainIA32.z" "%BUILD_DIR%\FV\DUETEFIMAINFVIA32.z"
  :: copy /B "%BOOTSECTOR_BIN_DIR%\Start.com"+"%BOOTSECTOR_BIN_DIR%\Efi32.com2"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\Efildr"
  :: copy /B "%BOOTSECTOR_BIN_DIR%\Start16.com"+"%BOOTSECTOR_BIN_DIR%\Efi32.com2"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\Efildr16"
  :: copy /B "%BOOTSECTOR_BIN_DIR%\Start32.com"+"%BOOTSECTOR_BIN_DIR%\Efi32.com3"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\Efildr20"
  copy /B "%BOOTSECTOR_BIN_DIR%\start32H.com2"+"%BOOTSECTOR_BIN_DIR%\efi32.com3"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\boot32"

  :: Be sure that all needed directories exists
  call:createDir "%DEST_DIR%\Bootloaders\ia32"
  call:createDir "%DEST_DIR%\EFI\BOOT"
  call:createDir "%DEST_DIR%\EFI\CLOVER\tools"
  call:createDir "%DEST_DIR%\EFI\CLOVER\drivers32"
  call:createDir "%DEST_DIR%\EFI\CLOVER\drivers32UEFI"
  call:createDir "%DEST_DIR%\drivers-Off\drivers32"
  call:createDir "%DEST_DIR%\drivers-Off\drivers32UEFI"

  set cloverEFIFile=boot3

  :: CloverEFI
  copy /B /Y "%BUILD_DIR%\FV\boot32" "%DEST_DIR%\Bootloaders\ia32\%cloverEFIFile%"

  :: Mandatory drivers
  echo "%BUILD_DIR_ARCH%\FSInject.efi" "%DEST_DIR%\EFI\CLOVER\drivers32\FSInject-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\FSInject.efi" "%DEST_DIR%\EFI\CLOVER\drivers32\FSInject-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\FSInject.efi" "%DEST_DIR%\EFI\CLOVER\drivers32UEFI\FSInject-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\OsxFatBinaryDrv.efi" "%DEST_DIR%\EFI\CLOVER\drivers32UEFI\OsxFatBinaryDrv-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxHfs.efi" "%DEST_DIR%\EFI\CLOVER\drivers32UEFI\VBoxHfs-32.efi"

  :: Optional drivers
  :: copy /B /Y "%BUILD_DIR_ARCH%\VBoxIso9600.efi" "%DEST_DIR%\drivers-Off\drivers32\VBoxIso9600-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxExt2.efi" "%DEST_DIR%\drivers-Off\drivers32\VBoxExt2-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\VBoxExt4.efi" "%DEST_DIR%\drivers-Off\drivers32\VBoxExt4-32.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubEXFAT.efi" "%DEST_DIR%\drivers-Off\drivers32\GrubEXFAT-32.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubISO9660.efi" "%DEST_DIR%\drivers-Off\drivers32\GrubISO9660-32.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubNTFS.efi" "%DEST_DIR%\drivers-Off\drivers32\GrubNTFS-32.efi"
  :: copy /B /Y "%BUILD_DIR_ARCH%\GrubUDF.efi" "%DEST_DIR%\drivers-Off\drivers32\GrubUDF-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\Ps2KeyboardDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\Ps2KeyboardDxe-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\Ps2MouseAbsolutePointerDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\Ps2MouseAbsolutePointerDxe-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\Ps2MouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\Ps2MouseDxe-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\UsbMouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\UsbMouseDxe-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\XhciDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\XhciDxe-32.efi"

  :: Applications
  copy /B /Y "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_DIR%\EFI\CLOVER\tools\bdmesg-32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVERIA32.efi" "%DEST_DIR%\EFI\CLOVER\CLOVERIA32.efi"
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVERIA32.efi" "%DEST_DIR%\EFI\BOOT\BOOTIA32.efi"

  call:createBootsector
  goto done

:createBootsector
  :: fixme @apianti
  goto:eof

:createDir
  if not exist "%~1" mkdir "%~1"
  goto:eof

:getFilesize
  set filesize=%~z1
  goto:eof

:addEdk2BuildMacro
  set "EDK2_BUILD_OPTIONS=%EDK2_BUILD_OPTIONS%-D %~1 "
  goto:eof

:parseArguments
  if x"%1" == x"" goto:eof
  if x"%1" == x"-D" (
    if x"%2" == x"ENABLE_SECURE_BOOT" (
       set ENABLE_SECURE_BOOT=1
    )
  )
  if x"%1" == x"-b" (
    set BUILDTARGET=%2
  )
  if x"%1" == x"--buildtarget" (
    set BUILDTARGET=%2
  )
  if x"%1" == x"-d" (
    set "BUILDTARGET=DEBUG"
  )
  if x"%1" == x"-r" (
    set "BUILDTARGET=RELEASE"
  )
  if x"%1" == x"-n" (
    set THREADNUMBER=%2
  )
  if x"%1" == x"--THREADNUMBER" (
    set THREADNUMBER=%2
  )
  if x"%1" == x"-t" (
    set TOOLCHAIN=%2
  )
  if x"%1" == x"--tagname" (
    set TOOLCHAIN=%2
  )
  if x"%1" == x"-a" (
    set TARGETARCH=%2
  )
  if x"%1" == x"--arch" (
    set TARGETARCH=%2
  )
  if x"%1" == x"-p" (
    set DSCFILE=%2
  )
  if x"%1" == x"--platform" (
    set DSCFILE=%2
  )
  if x"%1" == x"--vbios-patch-cloverefi" (
    set VBIOSPATCHCLOVEREFI=1
  )
  if x"%1" == x"--only-sata0" (
    set ONLYSATA0PATCH=1
  )
  if x"%1" == x"--std-ebda" (
    set USE_LOW_EBDA=0
  )
  if x"%1" == x"--genpage" (
    set GENPAGE=1
  )
  if x"%1" == x"--no-usb" (
    set DISABLE_USB_SUPPORT=1
  )
  if x"%1" == x"-h" (
    build --help
    set errorlevel=1
    goto:eof
  )
  if x"%1" == x"--help" (
    build --help
    set errorlevel=1
    goto:eof
  )
  if x"%1" == x"--version" (
    build --version
    set errorlevel=1
    goto:eof
  )
  if x"%1" == x"clean" (
    set CLEANING=clean
  )
  if x"%1" == x"cleanall" (
    set CLEANING=cleanall
  )
  shift
  goto parseArguments

:failscript
  echo Build failed!
  exit /b 1

:done
  echo Build dir: "%BUILD_DIR%"
  echo EFI dir: "%DEST_DIR%\EFI"
  echo Done!