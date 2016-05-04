@echo off
rem # windows batch script for building clover
rem # 2012-09-06 apianti
rem # 2016 cecekpawon

set "CURRENTDIR=%CD%"

rem # parse parameters for what we need
set ENABLE_SECURE_BOOT=0
set MULTIARCH=0
set TARGETARCH=
set TOOLCHAIN=
set BUILDTARGET=
set DSCFILE=
set CLEANING=
set BOOTSECTOR=
set errorlevel=0
set THREADNUMBER=0
set SHOW_USAGE=0
set EDK_BUILDINFOS=
set NOLOGO=0
set LOGOSHOWN=0

set EDK2_BUILD_OPTIONS=-D NO_GRUB_DRIVERS
set VBIOSPATCHCLOVEREFI=0
set ONLYSATA0PATCH=0
set USE_BIOS_BLOCKIO=0
set USE_LOW_EBDA=1
set GENPAGE=0
set MSG=
set DEVSTAGE=

set F_TMP_TXT=tmp.txt
set F_TMP_VBS=tmp.vbs
set F_TMP_SH=tmp.sh
set F_VER_TXT=vers.txt
set F_VER_H=Version.h

call:parseArguments %*
if errorlevel 1 (
  set MSG=Unknown error
  goto failscript
)

rem # get the current revision number
:getrevision
  cd "%CURRENTDIR%"
  rem  get svn revision number
  svnversion -n > %F_VER_TXT%
  set /p s= < %F_VER_TXT%
  rem del %F_VER_TXT%
  set SVNREVISION=

rem # get the current revision number
:fixrevision
  if ["%s%"] == [""] goto init
  set c=%s:~0,1%
  set s=%s:~1%
  if ["%c::=%"] == [""] goto init
  if ["%c:M=%"] == [""] goto init
  if ["%c:S=%"] == [""] goto init
  if ["%c:P=%"] == [""] goto init
  set SVNREVISION=%SVNREVISION%%c%
  goto fixrevision

rem # initialize
:init
  call:printLogo TRUE
  if ["%SVNREVISION%"] == [""] (
    set MSG=Invalid ^(local^) source
    goto failscript
  ) else (
    set SVNREVISION=%SVNREVISION%%DEVSTAGE%
  )

  rem # pass 1-call param
  if ["%SHOW_USAGE%"] == ["1"] goto usage

  rem # setup current dir and edk2 if needed
  pushd .
  if not defined WORKSPACE (
    echo Searching for EDK2:
    goto searchforedk
  ) else (
    goto prebuild
  )

rem # search edk path
:searchforedk
  if exist edksetup.bat (
    goto foundedk
  )
  if ["%CD%"] == ["%~d0%\"] (
    cd "%CURRENTDIR%"
    echo ==^> EDK2 not found ^<==
    goto failscript
  )
  cd ..
  goto searchforedk

rem # have edk2 prepare to build
:foundedk
  echo ==^> Found EDK2 ^<==
  echo.
  call edksetup.bat
  @echo off

rem # setup
:prebuild
  cd "%CURRENTDIR%"

  rem # fix any parameters not set
  set "BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32"
  if defined EDK_TOOLS_BIN (
    set "BASETOOLS_DIR=%EDK_TOOLS_BIN%"
  )

  rem # pass 1-call param
  if not ["%EDK_BUILDINFOS%"] == [""] goto getEDKBuildInfos

  set CONFIG_FILE="%WORKSPACE%\Conf\target.txt"

  set DEFAULT_TOOLCHAIN=MYTOOLS
  set DEFAULT_BUILDTARGET=RELEASE
  set DEFAULT_TARGETARCH=X64
  set DEFAULT_THREADNUMBER=%NUMBER_OF_PROCESSORS%

  rem # Read target.txt. Dont look TARGET_ARCH, we build multi ARCH if undefined
  findstr /v /r /c:"^#" /c:"^$" %CONFIG_FILE% > %F_TMP_TXT%
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /i "TOOL_CHAIN_TAG"') do set SCAN_TOOLCHAIN%%j
  rem for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /i "TARGETARCH"') do set SCAN_TARGETARCH%%j
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /v /r /c:"TARGET_ARCH"  ^| findstr /i "TARGET"') do set SCAN_BUILDTARGET%%j
  del %F_TMP_TXT%

  set SCAN_TOOLCHAIN=%SCAN_TOOLCHAIN: =%
  if not ["%SCAN_TOOLCHAIN%"] == [""] set DEFAULT_TOOLCHAIN=%SCAN_TOOLCHAIN%
  rem set SCAN_TARGETARCH=%SCAN_TARGETARCH: =%
  rem if not ["%SCAN_TARGETARCH%"] == [""] set DEFAULT_TARGETARCH=%SCAN_TARGETARCH: =%
  set SCAN_BUILDTARGET=%SCAN_BUILDTARGET: =%
  if not ["%SCAN_BUILDTARGET%"] == [""] set DEFAULT_BUILDTARGET=%SCAN_BUILDTARGET: =%

  if ["%TOOLCHAIN%"] == [""] set TOOLCHAIN=%DEFAULT_TOOLCHAIN%
  if ["%BUILDTARGET%"] == [""] set BUILDTARGET=%DEFAULT_BUILDTARGET%
  rem if ["%TARGETARCH%"] == [""] set TARGETARCH=%DEFAULT_TARGETARCH%
  if ["%THREADNUMBER%"] == [""] set THREADNUMBER=%DEFAULT_THREADNUMBER%

  rem # check DSC: Clover default
  if ["%DSCFILE%"] == [""] set DSCFILE="%CURRENTDIR%\Clover.dsc"

  if not ["%TARGETARCH%"] == [""] goto buildall
  if ["%CLEANING%"] == [""] echo Building selected ^(X64 ^& IA32^) ...
  set MULTIARCH=1

:buildall
  if ["%TARGETARCH%"] == ["IA32"] goto build32

  set "TARGETARCH=X64"
  goto startbuild

:build32
  set "TARGETARCH=IA32"
  goto startbuild

:startbuild
  if not exist %DSCFILE% (
    set MSG=No Platform
    goto usage
  )

  echo.
  if ["%CLEANING%"] == [""] (
    echo Building Clover ^(%TARGETARCH%^) ...
    echo Generating %CURRENTDIR%\%F_VER_H%
  ) else (
    echo Clean build ^(%CLEANING%^) ...
  )
  echo.

  set "MY_ARCH=-a %TARGETARCH%"
  set "MY_TOOLCHAIN=-t %TOOLCHAIN%"
  set "MY_BUILDTARGET=-b %BUILDTARGET%"
  set "MY_THREADNUMBER=-n %THREADNUMBER%"

  for /f "tokens=2 delims=[]" %%x in ('ver') do set WINVER=%%x
  set WINVER=%WINVER:Version =%

  set "CMD_BUILD=build -p %DSCFILE% %EDK2_BUILD_OPTIONS% %MY_ARCH% %MY_TOOLCHAIN% %MY_BUILDTARGET% %MY_THREADNUMBER% %CLEANING%"

  set clover_build_info=%CMD_BUILD%
  set clover_build_info=%clover_build_info:\=\\%
  set clover_build_info=%clover_build_info:"=\"%
  set clover_build_info="Args: %~nx0 %* | Command: %clover_build_info% | OS: Win %WINVER%"

  rem # generate build date and time
  set BUILDDATE=
  echo Dim cdt, output, temp > %F_TMP_VBS%
  rem # output year
  echo cdt = Now >> %F_TMP_VBS%
  echo output = Year(cdt) ^& "-" >> %F_TMP_VBS%
  rem # output month
  echo temp = Month(cdt) >> %F_TMP_VBS%
  echo If temp ^< 10 Then >> %F_TMP_VBS%
  echo    output = output ^& "0" >> %F_TMP_VBS%
  echo End If >> %F_TMP_VBS%
  echo output = output ^& temp ^& "-" >> %F_TMP_VBS%
  rem # output day
  echo temp = Day(cdt) >> %F_TMP_VBS%
  echo If temp ^< 10 Then >> %F_TMP_VBS%
  echo    output = output ^& "0" >> %F_TMP_VBS%
  echo End If >> %F_TMP_VBS%
  echo output = output ^& temp ^& " " >> %F_TMP_VBS%
  rem # output hours
  echo temp = Hour(cdt) >> %F_TMP_VBS%
  echo If temp ^< 10 Then >> %F_TMP_VBS%
  echo    output = output ^& "0" >> %F_TMP_VBS%
  echo End If >> %F_TMP_VBS%
  echo output = output ^& temp ^& ":" >> %F_TMP_VBS%
  rem # output minutes
  echo temp = Minute(cdt) >> %F_TMP_VBS%
  echo If temp ^< 10 Then >> %F_TMP_VBS%
  echo    output = output ^& "0" >> %F_TMP_VBS%
  echo End If >> %F_TMP_VBS%
  echo output = output ^& temp ^& ":" >> %F_TMP_VBS%
  rem # output seconds
  echo temp = Second(cdt) >> %F_TMP_VBS%
  echo If temp ^< 10 Then >> %F_TMP_VBS%
  echo    output = output ^& "0" >> %F_TMP_VBS%
  echo End If >> %F_TMP_VBS%
  echo output = output ^& temp >> %F_TMP_VBS%
  echo Wscript.Echo output >> %F_TMP_VBS%
  cscript //Nologo %F_TMP_VBS% > %F_TMP_TXT%
  del %F_TMP_VBS%
  set /p BUILDDATE= < %F_TMP_TXT%
  del %F_TMP_TXT%

  rem # generate version.h
  echo // Autogenerated %F_VER_H%> %F_VER_H%
  echo #define FIRMWARE_VERSION "2.31" >> %F_VER_H%
  echo #define FIRMWARE_BUILDDATE "%BUILDDATE%" >> %F_VER_H%
  echo #define FIRMWARE_REVISION L"%SVNREVISION%" >> %F_VER_H%
  echo #define REVISION_STR "Clover revision: %SVNREVISION%" >> %F_VER_H%
  echo #define BUILDINFOS_STR %clover_build_info% >> %F_VER_H%

  rem # launch build
  %CMD_BUILD%

  if errorlevel 1 (
    set MSG=Error while building
    goto failscript
  )

  if not ["%CLEANING%"] == [""] goto:eof
  goto postbuild

rem #
:postbuild
  echo.
  echo Performing post build operations ...
  echo.
  set SIGNTOOL_BUILD_DIR=%WORKSPACE%\Clover\SignTool
  set SIGNTOOL_BUILD=BuildSignTool.bat
  set SIGNTOOL=%WORKSPACE%\Clover\SignTool\SignTool.exe
  set BUILD_DIR=%WORKSPACE%\Build\Clover\%BUILDTARGET%_%TOOLCHAIN%
  set DEST_DIR=%WORKSPACE%\Clover\CloverPackage\CloverV2
  set BOOTSECTOR_BIN_DIR=%WORKSPACE%\Clover\BootSector\bin
  set BUILD_DIR_ARCH=%BUILD_DIR%\%TARGETARCH%

  rem # fixme: openssl compilation error
  if ["%ENABLE_SECURE_BOOT%"] == ["1"] (
    rem echo Building signing tool ...
    rem pushd .
    rem cd "%SIGNTOOL_BUILD_DIR%"
    rem call "%SIGNTOOL_BUILD%"
    rem popd
    rem if errorlevel 1 (
    rem   set MSG=Error while signing
    rem   goto failscript
    rem )
    echo.
    echo "ENABLE_SECURE_BOOT" doesnt work ATM ...
    echo.
  )

  set TARGETARCH_INT=%TARGETARCH%
  set TARGETARCH_INT=%TARGETARCH_INT:ia=%
  set TARGETARCH_INT=%TARGETARCH_INT:x=%

  set "DEST_BOOTSECTORS=%DEST_DIR%\BootSectors"
  set "DEST_BOOTLOADERS=%DEST_DIR%\Bootloaders"
  set "DEST_BOOT=%DEST_DIR%\EFI\BOOT"
  set "DEST_CLOVER=%DEST_DIR%\EFI\CLOVER"
  set "DEST_TOOLS=%DEST_CLOVER%\tools"
  set "DEST_DRIVER=%DEST_CLOVER%\drivers%TARGETARCH_INT%"
  set "DEST_OFF=%DEST_DIR%\drivers-Off\drivers%TARGETARCH_INT%"

  rem # Be sure that all needed directories exists
  call:createDir %DEST_BOOTSECTORS%
  call:createDir %DEST_BOOT%
  call:createDir %DEST_TOOLS%
  call:createDir %DEST_DRIVER%
  call:createDir %DEST_DRIVER%UEFI
  call:createDir %DEST_OFF%
  call:createDir %DEST_OFF%UEFI

  if ["%TARGETARCH%"] == ["IA32"] goto postbuild32

  call:createDir %DEST_BOOTLOADERS%\x%TARGETARCH_INT%

  echo Compressing DUETEFIMainFv.FV ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.Fv"

  echo Compressing DxeMain.efi ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\%TARGETARCH%\DxeCore.efi"

  echo Compressing DxeIpl.efi ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\%TARGETARCH%\DxeIpl.efi"

  echo Generating Loader Image ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\EfiLdrImage.exe" -o "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\%TARGETARCH%\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z"

  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start%TARGETARCH_INT%.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\EfildrPure" > nul
  rem "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\EfildrPure" -o "%BUILD_DIR%\FV\Efildr"
  rem copy /B "%BOOTSECTOR_BIN_DIR%\St16_%TARGETARCH_INT%.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr16Pure" > nul
  rem "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr16Pure -o "%BUILD_DIR%\FV\Efildr16"

  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start%TARGETARCH_INT%H.com"+"%BOOTSECTOR_BIN_DIR%\efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr20Pure" > nul
  rem "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr20Pure" -o "%BUILD_DIR%\FV\Efildr20"

  set EDBA_MAX=417792

  rem call:getFilesize "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%"
  echo Set objFS = CreateObject("Scripting.FileSystemObject") > %F_TMP_VBS%
  echo WScript.Echo objFS.GetFile("%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%").Size >> %F_TMP_VBS%
  cscript //Nologo %F_TMP_VBS% > %F_TMP_TXT%
  del %F_TMP_VBS%
  set /p filesize= < %F_TMP_TXT%
  del %F_TMP_TXT%

  if ["%GENPAGE%"] == ["0"] (
    if not ["%USE_LOW_EBDA%"] == ["0"] (
      if not ["%filesize%"] == [""] (
        if %filesize% gtr %EDBA_MAX% (
          echo warning: boot file bigger than low-ebda permits, switching to --std-ebda
          set USE_LOW_EBDA=0
        )
      )
    )
  )

  Setlocal EnableDelayedExpansion
    rem # first key index 0/1?
    rem set COM_NAMES=(H H2 H3 H4 H5 H6 H5 H6)
    set COM_NAMES[0]=H
    set COM_NAMES[1]=H2
    set COM_NAMES[2]=H3
    set COM_NAMES[3]=H4
    set COM_NAMES[4]=H5
    set COM_NAMES[5]=H6
    set /A "block=%GENPAGE% << 2 | %USE_LOW_EBDA% << 1 | %USE_BIOS_BLOCKIO%"
    set "block=COM_NAMES[%block%]"
    set startBlock=Start%TARGETARCH_INT%!%block%!.com
    set GEN64=Efildr20Pure
    if not ["%GENPAGE%"] == ["0"] set GEN64=boot%TARGETARCH_INT%
    copy /B "%BOOTSECTOR_BIN_DIR%\%startBlock%"+"%BOOTSECTOR_BIN_DIR%\efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\%GEN%TARGETARCH_INT%%" > nul
    set GEN64=-o "%BUILD_DIR%\FV\Efildr20"
    if not ["%USE_LOW_EBDA%"] == ["0"] set GEN64=-b 0x88000 -f 0x68000 %GEN64%
    set GEN64="%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr20Pure" %GEN64%
    %GEN64%
  endlocal

  "%BASETOOLS_DIR%\Split.exe" -f "%BUILD_DIR%\FV\Efildr20" -p %BUILD_DIR%\FV\ -o Efildr20.1 -t boot%TARGETARCH_INT% -s 512
  del "%BUILD_DIR%\FV\Efildr20.1"

  set /A "cloverEFIFile=(6 + %USE_BIOS_BLOCKIO%)"
  set cloverEFIFile=boot%cloverEFIFile%

  echo.
  echo Start copying drivers:

  rem # Mandatory drivers (UEFI)
  set DRV_LIST=(FSInject OsxFatBinaryDrv VBoxHfs)
  set CP_DEST=%DEST_DRIVER%UEFI

  echo.
  echo Mandatory to "%CP_DEST%":
  echo.
  for %%i in %DRV_LIST% do (
    echo -^> "%%i"
    copy /B /Y "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%\%%i-%TARGETARCH_INT%.efi" > nul
  )

  rem # Optional drivers (UEFI)
  set DRV_LIST=(CsmVideoDxe DataHubDxe EmuVariableUefi OsxAptioFixDrv OsxAptioFix2Drv OsxLowMemFixDrv PartitionDxe)
  set CP_DEST=%DEST_OFF%UEFI

  echo.
  echo Optional UEFI to "%CP_DEST%":
  echo.
  for %%i in %DRV_LIST% do (
    echo -^> "%%i"
    copy /B /Y "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%\%%i-%TARGETARCH_INT%.efi" > nul
  )

  rem # Optional drivers
  rem GrubEXFAT GrubISO9660 GrubNTFS GrubUDF Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe
  set DRV_LIST=(Ps2MouseDxe UsbMouseDxe NvmExpressDxe VBoxIso9600 VBoxExt2 VBoxExt4 XhciDxe)
  set CP_DEST=%DEST_OFF%

  echo.
  echo Optional to "%CP_DEST%":
  echo.
  for %%i in %DRV_LIST% do (
    echo -^> "%%i"
    copy /B /Y "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%\%%i-%TARGETARCH_INT%.efi" > nul
  )

  rem # CloverEFI + Applications
  echo.
  echo CloverEFI + Applications ...
  copy /B /Y "%BUILD_DIR%\FV\boot%TARGETARCH_INT%" "%DEST_BOOTLOADERS%\x%TARGETARCH_INT%\%cloverEFIFile%" > nul
  copy /B /Y "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_TOOLS%\bdmesg-%TARGETARCH_INT%.efi" > nul
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVER%TARGETARCH%.efi" "%DEST_CLOVER%\CLOVER%TARGETARCH%.efi" > nul
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVER%TARGETARCH%.efi" "%DEST_BOOT%\BOOT%TARGETARCH%.efi" > nul

  echo.
  echo Done copying ...
  echo.

  if ["%MULTIARCH%"] == ["1"] goto build32
  goto done

:postbuild32
  call:createDir %DEST_BOOTLOADERS%\ia%TARGETARCH_INT%

  echo Compressing DUETEFIMainFv.FV ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.Fv"

  echo Compressing DxeMain.efi ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR_ARCH%\DxeCore.efi"

  echo Compressing DxeIpl.efi ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR_ARCH%\DxeIpl.efi"

  echo Generating Loader Image ^(%TARGETARCH%^) ...
  "%BASETOOLS_DIR%\GenFw.exe" --rebase 0x10000 -o "%BUILD_DIR_ARCH%\EfiLoader.efi" "%BUILD_DIR_ARCH%\EfiLoader.efi"
  "%BASETOOLS_DIR%\EfiLdrImage.exe" -o "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR_ARCH%\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z"

  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr" > nul
  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start16.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr16" > nul
  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start%TARGETARCH_INT%.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr20" > nul

  copy /B "%BOOTSECTOR_BIN_DIR%\start%TARGETARCH_INT%H.com2"+"%BOOTSECTOR_BIN_DIR%\efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\boot%TARGETARCH_INT%" > nul

  set cloverEFIFile=boot3

  echo.
  echo Start copying drivers:

  rem # Mandatory drivers
  set DRV_LIST=(FSInject OsxFatBinaryDrv VBoxHfs)
  set CP_DEST=%DEST_DRIVER%UEFI

  echo.
  echo Mandatory to "%CP_DEST%":
  echo.
  for %%i in %DRV_LIST% do (
    echo -^> "%%i"
    copy /B /Y "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%\%%i-%TARGETARCH_INT%.efi" > nul
  )

  rem # Optional drivers
  rem GrubEXFAT GrubISO9660 GrubNTFS GrubUDF
  set DRV_LIST=(Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe Ps2MouseDxe UsbMouseDxe VBoxExt2 VBoxExt4 XhciDxe)
  set CP_DEST=%DEST_OFF%

  echo.
  echo Optional to "%CP_DEST%":
  echo.
  for %%i in %DRV_LIST% do (
    echo -^> "%%i"
    copy /B /Y "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%\%%i-%TARGETARCH_INT%.efi" > nul
  )

  rem # CloverEFI + Applications
  echo.
  echo CloverEFI + Applications ...
  copy /B /Y "%BUILD_DIR%\FV\boot%TARGETARCH_INT%" "%DEST_BOOTLOADERS%\ia%TARGETARCH_INT%\%cloverEFIFile%" > nul
  copy /B /Y "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_TOOLS%\bdmesg-%TARGETARCH_INT%.efi" > nul
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVER%TARGETARCH%.efi" "%DEST_CLOVER%\CLOVER%TARGETARCH%.efi" > nul
  copy /B /Y "%BUILD_DIR_ARCH%\CLOVER%TARGETARCH%.efi" "%DEST_BOOT%\BOOT%TARGETARCH%.efi" > nul

  echo.
  echo Done copying ...
  echo.

  goto done

rem # done by Cygwin
:createBootsector
  rem # fixme @apianti
  if not ["%BOOTSECTOR%"] == ["1"] goto:eof
  echo.
  echo Generating BootSectors ...
  echo #!/bin/bash > %F_TMP_SH%
  <nul set /p ".="DESTDIR="%DEST_BOOTSECTORS%" make -C "%CURRENTDIR%\BootHFS""" >> %F_TMP_SH%
  sh .\%F_TMP_SH%
  del %F_TMP_SH%
  goto:eof

:createDir
  if not exist "%~1" mkdir "%~1"
  goto:eof

rem # sometimes broken
rem :getFilesize
rem   set filesize=%~z1
rem   goto:eof

:addEdk2BuildMacro
  rem  Apply options
  set "%~1=1"
  if ["%~1"] == ["ENABLE_SECURE_BOOT"] (
    goto:eof
  )
  set "EDK2_BUILD_OPTIONS=%EDK2_BUILD_OPTIONS% %~1"
  goto:eof

rem # print build.exe infos
:getEDKBuildInfos
  if not exist %BASETOOLS_DIR% (
    set MSG=No basetools. Run edksetup
    goto failscript
  ) else (
    "%BASETOOLS_DIR%\build.exe" %EDK_BUILDINFOS%
    if errorlevel 1 (
      set MSG=Failed to retrieve infos
      goto failscript
    )
  )
  exit /b 0

rem # print Logo
:printLogo
  if not ["%~1"] == [""] (
    goto forceLogo
  ) else (
    if ["%LOGOSHOWN%"] == ["1"] (
      goto:eof
    ) else (
      if ["%NOLOGO%"] == ["1"] (
        goto setLogo
      )
    )
  )
  :forceLogo
    echo.
    echo  /------------ [ WINDOWS BATCH SCRIPT FOR BUILDING ] ------------\
    echo.
    echo.
    echo   .d8888b.  888      .d88888b.  888     888 8888888888 8888888b.
    echo  d88P  Y88b 888     d88P   Y88b 888     888 888        888   Y88b
    echo  888    888 888     888     888 888     888 888        888    888
    echo  888        888     888     888 Y88b   d88P 8888888    888   d88P
    echo  888        888     888     888  Y88b d88P  888        8888888P
    echo  888    888 888     888     888   Y88o88P   888        888 T88b
    echo  Y88b  d88P 888     Y88b. .d88P    Y888P    888        888  T88b
    echo   Y888888P  88888888 Y8888888P      Y8P     8888888888 888   T88ba
    echo.
    echo.
    echo  \--------------------- [ Under rev: %SVNREVISION% ] ---------------------/
    echo.
  :setLogo
    set LOGOSHOWN=1
  goto:eof

rem # Print the usage
:usage
  if not ["%MSG%"] == [""] (
    echo.
    echo !!! Error: %MSG% !!!
    echo.
  )
  rem echo.
  rem printf "Usage: %s [OPTIONS] [all|fds|genc|genmake|clean|cleanpkg|cleanall|cleanlib|modules|libraries]\n" "$SELF"
  echo Infos:
  echo --usage : print this message and exit
  echo --version : print build version and exit
  echo -h, --help : print build help and exit
  echo.
  echo Configuration:
  echo -n, --threadnumber ^<THREADNUMBER^> : build with multi-threaded [default CPUs + 1]
  echo -t, --tagname ^<TOOLCHAIN^> : force to use a specific toolchain
  echo -a, --arch ^<TARGETARCH^> : overrides target.txt's TARGET_ARCH definition
  echo -p, --platform ^<PLATFORMFILE^> : build the platform specified by the DSC argument
  rem echo -m, --module ^<MODULEFILE^> : build only the module specified by the INF argument
  echo -b, --buildtarget ^<BUILDTARGET^> : using the BUILDTARGET to build the platform
  echo.
  echo Options:
  echo -D, --define=^<MACRO^>, ex: -D ENABLE_SECURE_BOOT
  echo --vbios-patch-cloverefi : activate vbios patch in CloverEFI
  echo --only-sata0 : activate only SATA0 patch
  echo --std-ebda : ebda offset dont shift to 0x88000
  echo --genpage : dynamically generate page table under ebda
  echo --no-usb : disable USB support
  echo --bootsector : create Bootsector ^(need Cygwin^)
  rem echo.
  rem echo Report bugs to https://sourceforge.net/p/cloverefiboot/discussion/1726372/
  exit /b 0

:failscript
  call:printLogo
  if ["%MSG%"] == [""] (
    set MSG=Build failed
  )
  echo.
  echo !!! %MSG% !!!
  exit /b 0

:done
  call:createBootsector
  echo.
  echo Build dir: "%BUILD_DIR%"
  echo EFI dir: "%DEST_DIR%\EFI"
  echo Done!
  exit /b 0

:parseArguments
  if ["%1"] == [""] goto:eof
  if ["%1"] == ["-D"] (
    if not ["%2"] == [""] (
       call:addEdk2BuildMacro "-D %2"
    )
  )
  if ["%1"] == ["--define"] (
    if not ["%2"] == [""] (
       call:addEdk2BuildMacro "-D %2"
    )
  )
  rem if ["%1"] == ["-m"] (
  rem   if not ["%2"] == [""] (
  rem      call:addEdk2BuildMacro "-m %2"
  rem   )
  rem )
  if ["%1"] == ["-b"] (
    set BUILDTARGET=%2
  )
  if ["%1"] == ["--buildtarget"] (
    set BUILDTARGET=%2
  )
  if ["%1"] == ["-d"] (
    set "BUILDTARGET=DEBUG"
  )
  if ["%1"] == ["-r"] (
    set "BUILDTARGET=RELEASE"
  )
  if ["%1"] == ["-n"] (
    set THREADNUMBER=%2
  )
  if ["%1"] == ["--threadnumber"] (
    set THREADNUMBER=%2
  )
  if ["%1"] == ["-t"] (
    set TOOLCHAIN=%2
  )
  if ["%1"] == ["--tagname"] (
    set TOOLCHAIN=%2
  )
  if ["%1"] == ["-a"] (
    set TARGETARCH=%2
  )
  if ["%1"] == ["--arch"] (
    set TARGETARCH=%2
  )
  if ["%1"] == ["-p"] (
    set DSCFILE=%2
  )
  if ["%1"] == ["--platform"] (
    set DSCFILE=%2
  )
  if ["%1"] == ["--vbios-patch-cloverefi"] (
    set VBIOSPATCHCLOVEREFI=1
  )
  if ["%1"] == ["--only-sata0"] (
    set ONLYSATA0PATCH=1
  )
  if ["%1"] == ["--std-ebda"] (
    set USE_LOW_EBDA=0
  )
  if ["%1"] == ["--genpage"] (
    set GENPAGE=1
  )
  if ["%1"] == ["--no-usb"] (
    call:addEdk2BuildMacro "-D DISABLE_USB_SUPPORT"
  )
  if ["%1"] == ["--bootsector"] (
    set BOOTSECTOR=1
  )
  if ["%1"] == ["-h"] (
    set EDK_BUILDINFOS=%1
  )
  if ["%1"] == ["--help"] (
    set EDK_BUILDINFOS=%1
  )
  if ["%1"] == ["--version"] (
    set EDK_BUILDINFOS=%1
  )
  if ["%1"] == ["--usage"] (
    set SHOW_USAGE=1
  )
  if ["%1"] == ["clean"] (
    set CLEANING=clean
  )
  if ["%1"] == ["cleanall"] (
    set CLEANING=cleanall
  )
  if ["%1"] == ["--nologo"] (
    set NOLOGO=1
  )
  shift
  goto parseArguments
