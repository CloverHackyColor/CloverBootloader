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
set BOOTSECTOR=1
set errorlevel=0
set THREADNUMBER=0
set SHOW_USAGE=0
set EDK_BUILDINFOS=
set NOLOGO=0
set LOGOSHOWN=0
set UPDATETOOLS=0
set UPDATEEDK=0
set UPDATECLOVER=0
set EDK2SHELL=

set EDK2_SETUP_OPTIONS=
set EDK2_BUILD_OPTIONS=-D NO_GRUB_DRIVERS
set VBIOSPATCHCLOVEREFI=0
set ONLY_SATA_0=0
set USE_BIOS_BLOCKIO=0
set MCP=0
set USE_LOW_EBDA=1
set DISABLE_USB_SUPPORT=0
set GENPAGE=0
set MSG=
set DEVSTAGE=

set DEFAULT_CYGWIN_HOME=c:\cygwin
set DEFAULT_PYTHONHOME=c:\Python27
set DEFAULT_PYTHON_FREEZER_PATH=%PYTHON_HOME%\Scripts
set DEFAULT_NASM_PREFIX=%DEFAULT_CYGWIN_HOME%\bin
set DEFAULT_TOOLCHAIN=VS2012x86
set DEFAULT_BUILDTARGET=RELEASE
set DEFAULT_TARGETARCH=X64
set DEFAULT_THREADNUMBER=%NUMBER_OF_PROCESSORS%

set EDKSETUP=edksetup.bat
set EDK2SETUP=Edk2Setup.bat

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
  svnversion -n>%F_VER_TXT%
  set /P s=<%F_VER_TXT%
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

:createDir
  if not exist "%~1" mkdir "%~1"
  goto:eof

rem # copybin
:copybin
  if not exist "%~1" goto:eof
  call:createDir "%~2"
  echo -^> "%~3"
  copy /B /Y "%~1" "%~2\%~3">nul
  goto:eof

rem # sometimes broken
:getFilesize
  set filesize=%~z1
  rem echo Set objFS = CreateObject("Scripting.FileSystemObject")>%F_TMP_VBS%
  rem echo WScript.Echo objFS.GetFile("%~1").Size>>%F_TMP_VBS%
  rem cscript //Nologo %F_TMP_VBS%>%F_TMP_TXT%
  rem del %F_TMP_VBS%
  rem set /P filesize= < %F_TMP_TXT%
  rem del %F_TMP_TXT%
  goto:eof

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

  if not defined CYGWIN_HOME (
    set CYGWIN_HOME=%DEFAULT_CYGWIN_HOME%
  )
  if not defined PYTHONHOME (
    set PYTHONHOME=%DEFAULT_PYTHONHOME%
  )
  if not defined PYTHON_HOME (
    set PYTHON_HOME=%PYTHONHOME%
  )
  if not defined PYTHON_PATH (
    set PYTHON_PATH=%PYTHON_HOME%
  )
  if not defined PYTHON_FREEZER_PATH (
    set PYTHON_FREEZER_PATH=%DEFAULT_PYTHON_FREEZER_PATH%
  )
  if not defined NASM_PREFIX (
    set NASM_PREFIX=%DEFAULT_NASM_PREFIX%
  )

  rem # setup current dir and edk2 if needed
  if not defined WORKSPACE (
    echo Searching for EDK2 ...
    goto searchforedk
  ) else (
    if not ["%EDK2_SETUP_OPTIONS%"] == [""] (
      echo Resetup EDK ...
      goto setupedk
    ) else (
      goto prebuild
    )
  )

rem # search edk path
:searchforedk
  if exist %EDKSETUP% (
    echo ==^> Found EDK2 ^<==
    echo.
    set "WORKSPACE=%CD%"
    goto setupedk
  )
  if ["%CD%"] == ["%~d0%\"] (
    cd "%CURRENTDIR%"
    echo ==^> EDK2 not found ^<==
    goto failscript
  )
  cd ..
  goto searchforedk

rem # have edk2 prepare to build
:setupedk
  call %WORKSPACE%\%EDKSETUP% %EDK2_SETUP_OPTIONS%
  @echo off

rem # setup build
:prebuild
  cd "%CURRENTDIR%"

  if ["%UPDATEEDK%"] == ["1"] call:update "EDK"
  if ["%UPDATETOOLS%"] == ["1"] call:update "TOOLS"
  if ["%UPDATECLOVER%"] == ["1"] call:update "CLOVER"

  rem # fix any parameters not set
  set "BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32"
  if defined EDK_TOOLS_BIN (
    set "BASETOOLS_DIR=%EDK_TOOLS_BIN%"
  )

  rem # pass 1-call param
  if not ["%EDK_BUILDINFOS%"] == [""] goto getEDKBuildInfos

  if ["%MCP%"] == ["1"] (
    set TARGETARCH=X64
    set USE_BIOS_BLOCKIO=1
  )

  if ["%USE_BIOS_BLOCKIO%"] == ["1"] call:addEdk2BuildMacro "USE_BIOS_BLOCKIO"
  if ["%VBIOSPATCHCLOVEREFI%"] == ["1"] call:addEdk2BuildMacro "VBIOSPATCHCLOVEREFI"
  if ["%ONLY_SATA_0%"] == ["1"] call:addEdk2BuildMacro "ONLY_SATA_0"
  if ["%USE_LOW_EBDA%"] == ["1"] call:addEdk2BuildMacro "USE_LOW_EBDA"
  if ["%DISABLE_USB_SUPPORT%"] == ["1"] call:addEdk2BuildMacro "DISABLE_USB_SUPPORT"

:doTemplate
  echo Process template ...

  set EDK_CONF=%WORKSPACE%\Conf
  set EDK_CONF_TPL=%WORKSPACE%\BaseTools\Conf

  set EDK_CONF_TARGET_TXT=%EDK_CONF%\target.txt
  set EDK_CONF_TARGET_TXT_BAK=%EDK_CONF%\target.txt.bak
  set EDK_CONF_TARGET_TPL=%EDK_CONF_TPL%\target.template

  set CLOVER_CONF=%CURRENTDIR%\Conf
  set CLOVER_CONF_TPL=%CLOVER_CONF%\Tpl
  set CLOVER_CONF_TPL_CURRENT=%CLOVER_CONF_TPL%\MSFT

  set CLOVER_CONF_TARGET_TXT=%CLOVER_CONF%\target.txt
  set CLOVER_CONF_TARGET_TPL=%CLOVER_CONF_TPL_CURRENT%\target.template
  set CLOVER_CONF_TARGET_TPL_TMP=%CLOVER_CONF_TPL_CURRENT%\target.tmp

  set CLOVER_CONF_BUILD_RULE_TXT=%CLOVER_CONF%\build_rule.txt
  set CLOVER_CONF_BUILD_RULE_TXT_TPL=%CLOVER_CONF_TPL_CURRENT%\build_rule.template

  set CLOVER_CONF_TOOLS_DEF_TXT=%CLOVER_CONF%\tools_def.txt
  set CLOVER_CONF_TOOLS_DEF_TXT_TPL=%CLOVER_CONF_TPL_CURRENT%\tools_def.template

  set F_TMP_TXT=%CLOVER_CONF_TPL_CURRENT%\tmp.txt
  set F_TMP_TXT2=%CLOVER_CONF_TPL_CURRENT%\tmp2.txt

  rem # user doesnt have tpl, svn up to receive Clover/Conf dir?
  if not exist "%CLOVER_CONF_TARGET_TPL%" (
    goto readTemplate
  )

  rem # create target.txt backup
  if not exist "%EDK_CONF_TARGET_TXT_BAK%" (
    if exist "%EDK_CONF_TARGET_TXT%" (
      copy %EDK_CONF_TARGET_TXT%  %EDK_CONF_TARGET_TXT_BAK%>nul
    ) else (
      if exist "%EDK_CONF_TARGET_TPL%" (
        copy %EDK_CONF_TARGET_TPL%  %EDK_CONF_TARGET_TXT_BAK%>nul
      )
    )
  )

  rem # if script found target.txt / tools_def.txt / build_rule.txt in Clover/Conf we can jump to call edksetup
  if exist "%CLOVER_CONF_TARGET_TXT%" (
    copy %CLOVER_CONF_TARGET_TXT% %EDK_CONF_TARGET_TXT%>nul
    goto readTemplate
  )

  rem # clone Clover target.txt tpl for process
  copy %CLOVER_CONF_TARGET_TPL%  %CLOVER_CONF_TARGET_TPL_TMP%>nul

  rem # remove / ignore commented (use EDK default later) & blanklines in CLOVER target.txt
  findstr /v /r /c:"^#" /c:"^$" %CLOVER_CONF_TARGET_TPL%>%F_TMP_TXT%

  rem # seek stripped tpl
  rem # replace Clover path pattern '@CLOVER_PATH@' to current working path
  rem # remove EDK default, replace later w/ user defined
  for /f "tokens=1,3" %%a in (%F_TMP_TXT%) do (
    if not ["%%~a"] == [""] (
      if not ["%%~b"] == [""] (
        rem # if any user given params, then set it here
        set val=%%b
        call set val=%%val:@CLOVER_PATH@=%CURRENTDIR%%%
        call echo %%~a ^= %%val%%
        rem # set script var for process later
        set "CFG_%%~a=%val%"
        rem # remove defined
        findstr /v /r /c:"^%%~a " %CLOVER_CONF_TARGET_TPL_TMP%>%CLOVER_CONF_TARGET_TPL_TMP%
      )
    )
  )>>%F_TMP_TXT2%

  move %F_TMP_TXT2% %F_TMP_TXT%>nul

  rem # merge edk default & user defined
  for /f "tokens=*" %%a in (%F_TMP_TXT%) do (
    echo %%~a
  )>>%CLOVER_CONF_TARGET_TPL_TMP%

  del %F_TMP_TXT%

  rem # finally copy our target.txt to EDK Conf
  copy %CLOVER_CONF_TARGET_TPL_TMP% %CLOVER_CONF_TARGET_TXT%>nul
  move %CLOVER_CONF_TARGET_TPL_TMP% %EDK_CONF_TARGET_TXT%>nul

  rem # we had custom path for tools_def.txt & build_rule.txt already set in Clover target.txt now
  rem # we can also copy it to EDK/Conf OR keep it as custom path (Clover/Conf OR whatever)
  if exist "%CLOVER_CONF_BUILD_RULE_TXT_TPL%" (
    if not exist "%CLOVER_CONF_BUILD_RULE_TXT%" (
      copy %CLOVER_CONF_BUILD_RULE_TXT_TPL%  %CLOVER_CONF_BUILD_RULE_TXT%>nul
    )
    rem copy %CLOVER_CONF_BUILD_RULE_TXT%  %EDK_CONF_BUILD_RULE_TXT%>nul
  )

  if exist "%CLOVER_CONF_TOOLS_DEF_TXT_TPL%" (
    if not exist "%CLOVER_CONF_TOOLS_DEF_TXT%" (
      copy %CLOVER_CONF_TOOLS_DEF_TXT_TPL%  %CLOVER_CONF_TOOLS_DEF_TXT%>nul
    )
    rem copy %CLOVER_CONF_TOOLS_DEF_TXT%  %EDK_CONF_TOOLS_DEF_TXT%>nul
  )

:readTemplate
  rem # Read target.txt. Dont look TARGET_ARCH, we build multi ARCH if undefined

  findstr /v /r /c:"^#" /c:"^$" %EDK_CONF_TARGET_TXT%>%F_TMP_TXT%
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /i "TOOL_CHAIN_TAG"') do set SCAN_TOOLCHAIN%%j
  rem for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /i "TARGETARCH"') do set SCAN_TARGETARCH%%j
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /v /r /c:"TARGET_ARCH"  ^| findstr /i "TARGET"') do set SCAN_BUILDTARGET%%j
  del %F_TMP_TXT%

  if defined SCAN_TOOLCHAIN (
    set SCAN_TOOLCHAIN=%SCAN_TOOLCHAIN: =%
    if not ["%SCAN_TOOLCHAIN%"] == [""] set DEFAULT_TOOLCHAIN=%SCAN_TOOLCHAIN: =%
  )

  rem if defined SCAN_TARGETARCH (
  rem   set SCAN_TARGETARCH=%SCAN_TARGETARCH: =%
  rem   if not ["%SCAN_TARGETARCH%"] == [""] set DEFAULT_TARGETARCH=%SCAN_TARGETARCH: =%
  rem )

  if defined SCAN_BUILDTARGET (
    set SCAN_BUILDTARGET=%SCAN_BUILDTARGET: =%
    if not ["%SCAN_BUILDTARGET%"] == [""] set DEFAULT_BUILDTARGET=%SCAN_BUILDTARGET: =%
  )

  if ["%TOOLCHAIN%"] == [""] set TOOLCHAIN=%DEFAULT_TOOLCHAIN%
  if ["%BUILDTARGET%"] == [""] set BUILDTARGET=%DEFAULT_BUILDTARGET%
  rem if ["%TARGETARCH%"] == [""] set TARGETARCH=%DEFAULT_TARGETARCH%
  if ["%THREADNUMBER%"] == ["0"] set THREADNUMBER=%DEFAULT_THREADNUMBER%

  rem # check DSC: Clover default
  if ["%DSCFILE%"] == [""] set DSCFILE="%CURRENTDIR%\Clover.dsc"
  if not exist %DSCFILE% (
    set MSG=No Platform
    goto usage
  )

  if ["%TARGETARCH%"] == [""] (
    set MULTIARCH=1
    if not ["%CLEANING%"] == [""] (
      echo Cleaning selected ^(X64 ^& IA32^) ...
    ) else (
      echo Building selected ^(X64 ^& IA32^) ...
    )
  ) else (
    if ["%TARGETARCH%"] == ["IA32"] goto build32
  )

:build64
  set "TARGETARCH=X64"
  goto startbuild

:build32
  set "TARGETARCH=IA32"
  goto startbuild

:startbuild
  set "MY_ARCH=-a %TARGETARCH%"
  set "MY_TOOLCHAIN=-t %TOOLCHAIN%"
  set "MY_BUILDTARGET=-b %BUILDTARGET%"
  set "MY_THREADNUMBER=-n %THREADNUMBER%"
  set "CMD_BUILD=build %MY_ARCH% %MY_TOOLCHAIN% %MY_BUILDTARGET% %MY_THREADNUMBER% %CLEANING%"

  echo.
  if ["%CLEANING%"] == [""] (
    echo Building Clover ^(%TARGETARCH%^) ...
    echo.
    echo Generating %CURRENTDIR%\%F_VER_H%
    echo.
  ) else (
    echo Start ^(%CLEANING%^ %TARGETARCH%^) build ...
    echo.
    goto callbuild
  )

  for /f "tokens=2 delims=[]" %%x in ('ver') do set WINVER=%%x
  set WINVER=%WINVER:Version =%

  set "CMD_BUILD=%CMD_BUILD% -p %DSCFILE% %EDK2_BUILD_OPTIONS%"

  set clover_build_info=%CMD_BUILD%
  set clover_build_info=%clover_build_info:\=\\%
  set clover_build_info=%clover_build_info:"=\"%
  for /f "tokens=* delims= " %%A in ('echo %clover_build_info% ') do set clover_build_info=%%A
  set clover_build_info=%clover_build_info:~0,-1%
  set clover_build_info="Args: %~nx0 %* | Command: %clover_build_info% | OS: Win %WINVER%"

  rem # generate build date and time
  set BUILDDATE=%date:~10,4%-%date:~4,2%-%date:~7,2% %time:~0,-3%

  rem # generate version.h
  echo // Autogenerated %F_VER_H%>%F_VER_H%
  echo #define FIRMWARE_VERSION "2.31">>%F_VER_H%
  echo #define FIRMWARE_BUILDDATE "%BUILDDATE%">>%F_VER_H%
  echo #define FIRMWARE_REVISION L"%SVNREVISION%">>%F_VER_H%
  echo #define REVISION_STR "Clover revision: %SVNREVISION%">>%F_VER_H%
  echo #define BUILDINFOS_STR %clover_build_info%>>%F_VER_H%

:callbuild
  rem # launch build
  %CMD_BUILD%

  if errorlevel 1 (
    set MSG=Error while building
    goto failscript
  )

rem # drop compiled files to EFI folder
:postbuild
  set SIGNTOOL_BUILD_DIR=%CURRENTDIR%\SignTool
  set SIGNTOOL_BUILD=BuildSignTool.bat
  set SIGNTOOL=%CURRENTDIR%\SignTool\SignTool
  set BUILD_DIR=%WORKSPACE%\Build\Clover\%BUILDTARGET%_%TOOLCHAIN%
  set DEST_DIR=%CURRENTDIR%\CloverPackage\CloverV2
  set BOOTSECTOR_BIN_DIR=%CURRENTDIR%\CloverEFI\BootSector\bin
  set BUILD_DIR_ARCH=%BUILD_DIR%\%TARGETARCH%

  set TARGETARCH_INT=%TARGETARCH%
  set TARGETARCH_INT=%TARGETARCH_INT:ia=%
  set TARGETARCH_INT=%TARGETARCH_INT:x=%

  set "DEST_BOOTSECTORS=%DEST_DIR%\BootSectors"
  set "DEST_BOOTLOADERS=%DEST_DIR%\Bootloaders"
  set "DEST_EFI=%DEST_DIR%\EFI"
  set "DEST_BOOT=%DEST_EFI%\BOOT"
  set "DEST_CLOVER=%DEST_EFI%\CLOVER"
  set "DEST_TOOLS=%DEST_CLOVER%\tools"
  set "DEST_DRIVER=%DEST_CLOVER%\drivers%TARGETARCH_INT%"
  set "DEST_OFF=%DEST_DIR%\drivers-Off\drivers%TARGETARCH_INT%"

  set "EDK2SHELL_PATH=%WORKSPACE%\EdkShellBinPkg\%EDK2SHELL%\%TARGETARCH%"
  set "EDK2SHELL_MIN=%EDK2SHELL_PATH%\Shell.efi"
  set "EDK2SHELL_FULL=%EDK2SHELL_PATH%\Shell_Full.efi"
  set CLOVER_EDK2SHELL=

  if exist "%EDK2SHELL_MIN%" (
    set CLOVER_EDK2SHELL=%EDK2SHELL_MIN%
  ) else (
    if exist "%EDK2SHELL_FULL%" (
      set CLOVER_EDK2SHELL=%EDK2SHELL_FULL%
    )
  )

  if ["%CLEANING%"] == [""] goto finalizebuild

  for /R "%DEST_BOOTSECTORS%" %%i in (boot0* boot1*) do del "%%i"
  for /R "%DEST_BOOTLOADERS%\%TARGETARCH%" %%i in (boot*) do del "%%i"
  for /R "%DEST_TOOLS%" %%i in (Shell*.efi) do ren "%%i" "%%~nxi.ctmp"
  for /R "%DEST_DIR%" %%i in (*%TARGETARCH%.efi *-%TARGETARCH_INT%.efi) do del /F /Q "%%i"
  for /R "%DEST_TOOLS%" %%i in (*.ctmp) do ren "%%i" "%%~ni"

  echo.
  echo End ^(%CLEANING% %TARGETARCH%^) build ...
  echo.
  if ["%MULTIARCH%"] == ["1"] (
    if not ["%TARGETARCH%"] == ["IA32"] goto build32
  )
  goto done

:finalizebuild
  echo.
  echo Performing post build operations ...
  echo.

  rem # Be sure that all needed directories exists
  call:createDir %DEST_BOOTSECTORS%
  call:createDir %DEST_BOOT%
  call:createDir %DEST_TOOLS%
  call:createDir %DEST_DRIVER%
  call:createDir %DEST_DRIVER%UEFI
  call:createDir %DEST_OFF%
  call:createDir %DEST_OFF%UEFI

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

  if ["%TARGETARCH%"] == ["IA32"] goto postbuild32

  call:createDir %DEST_BOOTLOADERS%\x%TARGETARCH_INT%

  echo Compressing DUETEFIMainFv.FV ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.Fv"

  echo Compressing DxeMain.efi ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\%TARGETARCH%\DxeCore.efi"

  echo Compressing DxeIpl.efi ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\%TARGETARCH%\DxeIpl.efi"

  echo Generating Loader Image ^(%TARGETARCH%^) ...
  EfiLdrImage -o "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\%TARGETARCH%\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z"

  rem copy /B /Y "%BOOTSECTOR_BIN_DIR%\Start%TARGETARCH_INT%.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\EfildrPure">nul
  rem GenPage "%BUILD_DIR%\FV\EfildrPure" -o "%BUILD_DIR%\FV\Efildr"
  rem copy /B /Y "%BOOTSECTOR_BIN_DIR%\St16_%TARGETARCH_INT%.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr16Pure">nul
  rem GenPage "%BUILD_DIR%\FV\Efildr16Pure -o "%BUILD_DIR%\FV\Efildr16"

  set EDBA_MAX=417792
  set filesize=

  call:getFilesize "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%"

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
    set GENLDR=Efildr20Pure
    if not ["%GENPAGE%"] == ["0"] set GENLDR=boot%TARGETARCH_INT%
    set GENLDR=%BUILD_DIR%\FV\%GENLDR%
    copy /B /Y "%BOOTSECTOR_BIN_DIR%\%startBlock%"+"%BOOTSECTOR_BIN_DIR%\efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%GENLDR%">nul
    set GEN64=-o "%BUILD_DIR%\FV\Efildr20"
    if not ["%USE_LOW_EBDA%"] == ["0"] set GEN64= -b 0x88000 -f 0x68000 %GEN64%
    GenPage "%GENLDR%" %GEN64%
  Endlocal

  Split -f "%BUILD_DIR%\FV\Efildr20" -p %BUILD_DIR%\FV\ -o Efildr20.1 -t boot%TARGETARCH_INT% -s 512
  del "%BUILD_DIR%\FV\Efildr20.1"

  set /A "cloverEFIFile=(6 + %USE_BIOS_BLOCKIO%)"
  set cloverEFIFile=boot%cloverEFIFile%

  echo.
  echo Start copying:

  echo.
  echo Mandatory ^(UEFI^) drivers:
  echo.

  set DRV_LIST=(FSInject OsxFatBinaryDrv VBoxHfs)
  set CP_DEST=%DEST_DRIVER%UEFI

  for %%i in %DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo Optional ^(UEFI^) drivers:
  echo.

  set DRV_LIST=(CsmVideoDxe DataHubDxe EmuVariableUefi OsxAptioFixDrv OsxAptioFix2Drv OsxLowMemFixDrv PartitionDxe)
  set CP_DEST=%DEST_OFF%UEFI

  for %%i in %DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo Optional drivers:
  echo.

  rem GrubEXFAT GrubISO9660 GrubNTFS GrubUDF Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe
  set DRV_LIST=(NvmExpressDxe Ps2MouseDxe UsbMouseDxe VBoxIso9600 VBoxExt2 VBoxExt4 XhciDxe)
  set CP_DEST=%DEST_OFF%

  for %%i in %DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo CloverEFI + Applications:
  echo.

  call:copybin "%BUILD_DIR%\FV\boot%TARGETARCH_INT%" "%DEST_BOOTLOADERS%\x%TARGETARCH_INT%" "%cloverEFIFile%"
  call:copybin "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_TOOLS%" "bdmesg-%TARGETARCH_INT%.efi"
  call:copybin "%BUILD_DIR_ARCH%\CLOVER.efi" "%DEST_CLOVER%" "CLOVER%TARGETARCH%.efi"
  call:copybin "%BUILD_DIR_ARCH%\CLOVER.efi" "%DEST_BOOT%" "BOOT%TARGETARCH%.efi"

  if ["%CLOVER_EDK2SHELL%"] == [""] goto donebuild64

  echo.
  echo EDK2 Shell ^(%EDK2SHELL%^):
  echo.

  set CLOVER_SHELL_LIST=(Shell%TARGETARCH_INT%U Shell%TARGETARCH_INT%)
  for %%i in %CLOVER_SHELL_LIST% do (
    if not exist "%DEST_TOOLS%\%%i.efi.bak" (
      if exist "%DEST_TOOLS%\%%i.efi" (
        ren "%DEST_TOOLS%\%%i.efi" "%%i.efi.bak"
      )
    )
  )
  call:copybin "%CLOVER_EDK2SHELL%" "%DEST_TOOLS%" "Shell%TARGETARCH_INT%.efi"

:donebuild64
  echo.
  echo End copying ...
  echo.

  if ["%MULTIARCH%"] == ["1"] goto build32
  goto createBootsector

:postbuild32
  call:createDir %DEST_BOOTLOADERS%\ia%TARGETARCH_INT%

  echo Compressing DUETEFIMainFv.FV ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.Fv"

  echo Compressing DxeMain.efi ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR_ARCH%\DxeCore.efi"

  echo Compressing DxeIpl.efi ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR_ARCH%\DxeIpl.efi"

  echo Generating Loader Image ^(%TARGETARCH%^) ...
  GenFw --rebase 0x10000 -o "%BUILD_DIR_ARCH%\EfiLoader.efi" "%BUILD_DIR_ARCH%\EfiLoader.efi"
  EfiLdrImage -o "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR_ARCH%\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z"

  rem copy /B /Y "%BOOTSECTOR_BIN_DIR%\Start.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr">nul
  rem copy /B /Y "%BOOTSECTOR_BIN_DIR%\Start16.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com2"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr16">nul
  rem copy /B /Y "%BOOTSECTOR_BIN_DIR%\Start%TARGETARCH_INT%.com"+"%BOOTSECTOR_BIN_DIR%\Efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\Efildr20">nul

  copy /B /Y "%BOOTSECTOR_BIN_DIR%\start%TARGETARCH_INT%H.com2"+"%BOOTSECTOR_BIN_DIR%\efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\FV\boot%TARGETARCH_INT%">nul

  set cloverEFIFile=boot3

  echo.
  echo Start copying:

  echo.
  echo Mandatory ^(UEFI^) drivers:
  echo.

  set DRV_LIST=(FSInject OsxFatBinaryDrv VBoxHfs)
  set CP_DEST=%DEST_DRIVER%UEFI

  for %%i in %DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo Optional drivers:
  echo.

  rem GrubEXFAT GrubISO9660 GrubNTFS GrubUDF
  set DRV_LIST=(Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe Ps2MouseDxe UsbMouseDxe VBoxExt2 VBoxExt4 XhciDxe)
  set CP_DEST=%DEST_OFF%

  for %%i in %DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo CloverEFI + Applications:
  echo.

  call:copybin "%BUILD_DIR%\FV\boot%TARGETARCH_INT%" "%DEST_BOOTLOADERS%\ia%TARGETARCH_INT%" "%cloverEFIFile%"
  call:copybin "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_TOOLS%" "bdmesg-%TARGETARCH_INT%.efi"
  call:copybin "%BUILD_DIR_ARCH%\CLOVER.efi" "%DEST_CLOVER%" "CLOVER%TARGETARCH%.efi"
  call:copybin "%BUILD_DIR_ARCH%\CLOVER.efi" "%DEST_BOOT%" "BOOT%TARGETARCH%.efi"

  if ["%CLOVER_EDK2SHELL%"] == [""] goto donebuild32

  echo.
  echo EDK2 Shell ^(%EDK2SHELL%^):
  echo.

  set CLOVER_SHELL_LIST=(Shell%TARGETARCH_INT%)
  for %%i in %CLOVER_SHELL_LIST% do (
    if not exist "%DEST_TOOLS%\%%i.efi.bak" (
      if exist "%DEST_TOOLS%\%%i.efi" (
        ren "%DEST_TOOLS%\%%i.efi" "%%i.efi.bak"
      )
    )
  )
  call:copybin "%CLOVER_EDK2SHELL%" "%DEST_TOOLS%" "Shell%TARGETARCH_INT%.efi"

:donebuild32
  echo.
  echo End copying ...
  echo.

  goto createBootsector

rem # update source
:update
  set UPDPATH=
  set UPDSTR=%~1

  if ["%UPDSTR%"] == ["EDK"] (
    set UPDPATH=%WORKSPACE%
    set "UPDPCMD=git pull"
  )
  if ["%UPDSTR%"] == ["TOOLS"] (
    set UPDPATH=%BASETOOLS_DIR%
    set "UPDPCMD=git pull"
  )
  if ["%UPDSTR%"] == ["CLOVER"] (
    set UPDPATH=%CURRENTDIR%
    set "UPDPCMD=svn up"
  )

  if not ["%UPDPATH%"] == [""] (
    echo Start updating %UPDSTR% ...
    echo.
    pushd .
    cd "%UPDPATH%"
    %UPDPCMD%
    popd
    if errorlevel 1 (
      set MSG=Error while updating %UPDSTR% ...
      goto failscript
    )
    echo.
    echo End updating %UPDSTR% ...
    echo.
  )
  goto:eof

rem # add build macros
:addEdk2BuildMacro
  rem  Apply options
  set "%~1=1"
  if ["%~1"] == ["ENABLE_SECURE_BOOT"] (
    goto:eof
  )
  set "EDK2_BUILD_OPTIONS=%EDK2_BUILD_OPTIONS% -D %~1"
  goto:eof

rem # add edksetup args
:addEdk2Args
  rem  Apply EDK pre-setup
  set "EDK2_SETUP_OPTIONS=%EDK2_SETUP_OPTIONS% %~1"
  goto:eof

rem # done by Cygwin
:createBootsector
  rem # fixme @apianti
  if not ["%CLEANING%"] == [""] goto done
  if not defined CYGWIN_HOME goto done
  if not exist "%CYGWIN_HOME%" goto done
  if not ["%BOOTSECTOR%"] == ["1"] goto done
  cd "%CURRENTDIR%"
  rem echo #!/bin/bash>%F_TMP_SH%
  rem <nul set /P ".="DESTDIR="%DEST_BOOTSECTORS%" make -C "%CURRENTDIR%\BootHFS""">>%F_TMP_SH%
  rem sh .\%F_TMP_SH%
  rem del %F_TMP_SH%
  echo Generating BootSectors ...
  echo.
  set DESTDIR=%DEST_BOOTSECTORS:\=\\%
  pushd .
  make -C "%CURRENTDIR%\BootHFS"
  popd
  goto done

rem # print build.exe infos
:getEDKBuildInfos
  if not exist %BASETOOLS_DIR% (
    set MSG=No basetools. Run edksetup
    goto failscript
  ) else (
    build %EDK_BUILDINFOS%
    if errorlevel 1 (
      set MSG=Failed to retrieve infos
      goto failscript
    )
  )
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
  echo.
  echo ### Build dir: "%BUILD_DIR%"
  echo ### EFI dir: "%DEST_DIR%\EFI"
  echo.
  echo Done!
  exit /b 0

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
  echo Configurations:
  echo -n, --threadnumber ^<THREADNUMBER^> : build with multi-threaded [default CPUs + 1]
  echo -t, --tagname ^<TOOLCHAIN^> : force to use a specific toolchain
  echo -a, --arch ^<TARGETARCH^> : overrides target.txt's TARGET_ARCH definition
  echo -p, --platform ^<PLATFORMFILE^> : build the platform specified by the DSC argument
  rem echo -m, --module ^<MODULEFILE^> : build only the module specified by the INF argument
  echo -d, --define=^<MACRO^>, ex: -D ENABLE_SECURE_BOOT
  echo -b, --buildtarget ^<BUILDTARGET^> : using the BUILDTARGET to build the platform, or:
  echo                         --debug : set DEBUG buildtarget
  echo                       --release : set RELEASE buildtarget
  echo.
  echo Options:
  echo --vbios-patch-cloverefi : activate vbios patch in CloverEFI
  echo --only-sata0 : activate only SATA0 patch
  echo --std-ebda : ebda offset dont shift to 0x88000
  echo --genpage : dynamically generate page table under ebda
  echo --no-usb : disable USB support
  echo --mc : build in 64-bit [boot7] using BiosBlockIO ^(compatible with MCP chipset^)
  echo --edk2shell ^<MinimumShell^|FullShell^> : copy edk2 Shell to EFI tools dir
  echo.
  echo Extras:
  echo --cygwin : set CYGWIN dir ^(def: %DEFAULT_CYGWIN_HOME%^)
  echo --nasmprefix : set NASM bin dir ^(def: %DEFAULT_NASM_PREFIX%^)
  echo --pythonhome : set PYTHON dir ^(def: %DEFAULT_PYTHONHOME%^)
  echo --pythonfreezer : set PYTHON Freeze dir ^(def: %DEFAULT_PYTHON_FREEZER_PATH%^)
  echo --nobootsector : skip create Bootsector ^(need Cygwin^)
  echo --updtools : udate basetools binaries from repo
  echo --updedk : update EDK source from repo
  echo --updclover : update CLOVER source from repo
  echo --edk2setup : using Edk2Setup.bat instead of edksetup.bat
  echo --edk2rebuild : soft compile basetools binaries
  echo --edk2forcerebuild : hard compile basetools binaries
  echo --edk2reconfig : recover target.txt, tools_def.txt and build_rule.txt from template
  rem echo.
  rem echo Report bugs to https://sourceforge.net/p/cloverefiboot/discussion/1726372/
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
    echo  /--------------------- [ WINDOWS BATCH SCRIPT FOR BUILDING ] ---------------------\
    echo.
    echo.
    echo           .d8888b.  888      .d88888b.  888     888 8888888888 8888888b.
    echo          d88P  Y88b 888     d88P   Y88b 888     888 888        888   Y88b
    echo          888    Y88 888     888     888 888     888 888        888    888
    echo          888        888     888     888 Y88b   d88P 8888888    888   d88P
    echo          888        888     888     888  Y88b d88P  8888888    8888888P
    echo          888    d88 888     888     888   Y88o88P   888        888 T88b
    echo          Y88b   88P 888     Y88b. .d88P    Y888P    888        888  T88b
    echo           Y888888P  88888888 Y8888888P      Y8P     8888888888 888   T88ba
    echo.
    echo.
    echo  \------------------------------ [ Under rev: %SVNREVISION% ] ------------------------------/
    echo.
  :setLogo
    set LOGOSHOWN=1
  goto:eof

:parseArguments
  if ["%1"] == [""] goto:eof
  if ["%1"] == ["--cygwin"] (
    if not ["%~2"] == [""] (
      set CYGWIN_HOME="%~2"
    )
  )
  if ["%1"] == ["--nasmprefix"] (
    if not ["%~2"] == [""] (
      set NASM_PREFIX="%~2"
    )
  )
  if ["%1"] == ["--pythonhome"] (
    if not ["%~2"] == [""] (
      set PYTHONHOME="%~2"
    )
  )
  if ["%1"] == ["--pythonfreezer"] (
    if not ["%~2"] == [""] (
      set PYTHON_FREEZER_PATH="%~2"
    )
  )
  if ["%1"] == ["-d"] (
    if not ["%~2"] == [""] (
      call:addEdk2BuildMacro "%~2"
    )
  )
  if ["%1"] == ["--define"] (
    if not ["%~2"] == [""] (
      call:addEdk2BuildMacro "%~2"
    )
  )
  rem if ["%1"] == ["-m"] (
  rem   if not ["%2"] == [""] (
  rem     call:addEdk2BuildMacro "-m %2"
  rem   )
  rem )
  if ["%1"] == ["-b"] (
    set BUILDTARGET=%2
  )
  if ["%1"] == ["--buildtarget"] (
    set BUILDTARGET=%2
  )
  if ["%1"] == ["--debug"] (
    set BUILDTARGET=DEBUG
  )
  if ["%1"] == ["--release"] (
    set BUILDTARGET=RELEASE
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
    set ONLY_SATA_0=1
  )
  if ["%1"] == ["--std-ebda"] (
    set USE_LOW_EBDA=0
  )
  if ["%1"] == ["--genpage"] (
    set GENPAGE=1
  )
  if ["%1"] == ["--mc"] (
    set MCP=1
  )
  if ["%1"] == ["--no-usb"] (
    set DISABLE_USB_SUPPORT=1
  )
  if ["%1"] == ["--nobootsector"] (
    set BOOTSECTOR=0
  )
  if ["%1"] == ["--updtools"] (
    set UPDATETOOLS=1
  )
  if ["%1"] == ["--updedk"] (
    set UPDATEEDK=1
  )
  if ["%1"] == ["--updclover"] (
    set UPDATECLOVER=1
  )
  if ["%1"] == ["--edk2setup"] (
    set EDKSETUP=%EDK2SETUP%
  )
  if ["%1"] == ["--edk2forcerebuild"] (
    if ["%EDKSETUP%"] == ["%EDK2SETUP%"] (
      call:addEdk2Args "--rebuild"
    ) else (
      call:addEdk2Args "ForceRebuild"
    )
  )
  if ["%1"] == ["--edk2rebuild"] (
    if ["%EDKSETUP%"] == ["%EDK2SETUP%"] (
      call:addEdk2Args "--rebuild"
    ) else (
      call:addEdk2Args "Rebuild"
    )
  )
  if ["%1"] == ["--edk2reconfig"] (
    if ["%EDKSETUP%"] == ["%EDK2SETUP%"] (
      call:addEdk2Args "--reconfig"
    ) else (
      call:addEdk2Args "Reconfig"
    )
  )
  if ["%1"] == ["--edk2shell"] (
    set EDK2SHELL=%2
  )
  if ["%1"] == ["--beta"] (
    set DEVSTAGE=b
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
