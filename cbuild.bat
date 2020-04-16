@echo off
rem # windows batch script for building clover
rem # 2012-09-06 apianti
rem # 2016 cecekpawon

pushd .

rem # parse parameters for what we need
set ENABLE_SECURE_BOOT=0
set TARGETARCH=
set TOOLCHAIN=
set BUILDTARGET=
set DSCFILE=
set CLEANING=
set THREADNUMBER=
set SHOW_USAGE=0
set EDK_BUILDINFOS=
set EDK2SHELL=
set BUILD_IA32=
set BUILD_X64=
set EDK2_BUILD_OPTIONS=-D NO_GRUB_DRIVERS
set VBIOSPATCHCLOVEREFI=0
set ONLY_SATA_0=0
set USE_BIOS_BLOCKIO=0
set USE_LOW_EBDA=1
set DISABLE_USB_SUPPORT=0
set GENPAGE=0
set MSG=
set DEVSTAGE=
set IASL_PREFIX=c:\ASL\

set DEFAULT_CYGWIN_HOME=c:\cygwin
rem set DEFAULT_PYTHONHOME=d:\Program File\Python38
rem set DEFAULT_PYTHONPATH=d:\Program File\Python38\Lib
rem set DEFAULT_PYTHON_FREEZER_PATH=%PYTHON_HOME%\Scripts
rem set "PYTHONHOME=d:\Program File\Python37"
rem set "PYTHON_FREEZER_PATH=%PYTHONHOME%\Scripts"
set PYTHON3_ENABLE=TRUE
set DEFAULT_NASM_PREFIX=
rem C:\Program Files (x86)\NASM"
rem # %DEFAULT_CYGWIN_HOME%\bin
set DEFAULT_TOOLCHAIN=VS2015x86
set DEFAULT_BUILDTARGET=RELEASE
set DEFAULT_TARGETARCH=X64
set DEFAULT_THREADNUMBER=%NUMBER_OF_PROCESSORS%
set WIN_IASL_BIN=c:\ASL
set WIN_ASL_BIN=c:\ASL

call:parseArguments %*
if errorlevel 1 (
  set MSG=Unknown error
  goto failscript
)

rem # get the current revision number
:getrevision
  rem  get svn revision number
rem  set F_VER_TXT=vers.txt
rem #  svnversion -n>%F_VER_TXT%
rem  set /P s=<%F_VER_TXT%
rem #  del %F_VER_TXT%
rem   git rev-list --tags --max-count=1 > revs.txt
rem   set /p c=< revs.txt
rem  del revs.txt
rem  git describe --tags %c% > vers.txt
  git describe --tags --abbrev=0 > vers.txt
  set /P s=< vers.txt
  del vers.txt
  
  set SVNREVISION=

rem # get the current revision number
:fixrevision
  if not defined s goto init
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
  goto:eof

rem # initialize
:init
  if not defined SVNREVISION (
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
 rem  if not defined PYTHONHOME (
rem     set PYTHONHOME=%DEFAULT_PYTHONHOME%
rem   )
rem   if not defined PYTHON_HOME (
rem     set PYTHON_HOME=%PYTHONHOME%
rem   )
rem   if not defined PYTHON_PATH (
rem     set PYTHON_PATH=%PYTHON_HOME%
rem   )
rem   if not defined PYTHON_FREEZER_PATH (
rem     set PYTHON_FREEZER_PATH=%DEFAULT_PYTHON_FREEZER_PATH%
rem   )
  if not defined NASM_PREFIX (
    set NASM_PREFIX=%DEFAULT_NASM_PREFIX%
  )

  rem # setup current dir and edk2 if needed
  if defined WORKSPACE goto prebuild
  echo Searching for EDK2 ...

rem # search edk path
:searchforedk
   if exist edksetup.bat (
    echo ==^> Found EDK2 ^<==
     echo.
     call edksetup.bat
     @echo off
     goto prebuild
   )
rem #   if ["%CD%"] == ["%~d0%\"] (
rem #     popd
rem #     echo ==^> EDK2 not found ^<==
rem #     goto failscript
rem #   )
rem #   cd ..
   goto searchforedk

rem # setup build
:prebuild
  popd

  rem # fix any parameters not set
  set "BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32"
  if defined EDK_TOOLS_BIN (
    set "BASETOOLS_DIR=%EDK_TOOLS_BIN%"
  )
  set NASM_INC=

  rem # pass 1-call param
  if defined EDK_BUILDINFOS goto getEDKBuildInfos

  if ["%USE_BIOS_BLOCKIO%"] == ["1"] call:addEdk2BuildOption "-D USE_BIOS_BLOCKIO"
  if ["%VBIOSPATCHCLOVEREFI%"] == ["1"] call:addEdk2BuildOption "-D VBIOSPATCHCLOVEREFI"
  if ["%ONLY_SATA_0%"] == ["1"] call:addEdk2BuildOption "-D ONLY_SATA_0"
  if ["%USE_LOW_EBDA%"] == ["1"] call:addEdk2BuildOption "-D USE_LOW_EBDA"
  if ["%DISABLE_USB_SUPPORT%"] == ["1"] call:addEdk2BuildOption "-D DISABLE_USB_SUPPORT"

:readTemplate
  rem # Read target.txt. Dont look TARGET_ARCH, we build multi ARCH if undefined

  set F_TMP_TXT=tmp.txt
  set EDK_CONF_TARGET_TXT=Conf\target.txt
  findstr /v /r /c:"^#" /c:"^$" "%EDK_CONF_TARGET_TXT%">%F_TMP_TXT%
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /i "TOOL_CHAIN_TAG"') do set SCAN_TOOLCHAIN%%j
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /i "TARGET_ARCH"') do set SCAN_TARGETARCH%%j
  for /f "tokens=1*" %%i in ('type %F_TMP_TXT% ^| findstr /v /r /c:"TARGET_ARCH"  ^| findstr /i "TARGET"') do set SCAN_BUILDTARGET%%j
  del %F_TMP_TXT%

  if defined SCAN_TOOLCHAIN (
    set SCAN_TOOLCHAIN=%SCAN_TOOLCHAIN: =%
    if not ["%SCAN_TOOLCHAIN: =%"] == [""] set DEFAULT_TOOLCHAIN=%SCAN_TOOLCHAIN: =%
  )

  if not ["%BUILD_X64%"] == ["1"] (
    if not ["%BUILD_IA32%"] == ["1"] (
      if not defined SCAN_TARGETARCH set "SCAN_TARGETARCH=%DEFAULT_TARGETARCH%"
      if defined SCAN_TARGETARCH (
        for %%i in (%SCAN_TARGETARCH%) do (
          if ["%%i"] == ["X64"] (
            set BUILD_X64=1
          ) else (
            if ["%%i"] == ["IA32"] (
              set BUILD_IA32=1
            )
          )
        )
      )
    )
  )

  if defined SCAN_BUILDTARGET (
    set SCAN_BUILDTARGET=%SCAN_BUILDTARGET: =%
    if not ["%SCAN_BUILDTARGET: =%"] == [""] set DEFAULT_BUILDTARGET=%SCAN_BUILDTARGET: =%
  )

  if not defined TOOLCHAIN set TOOLCHAIN=%DEFAULT_TOOLCHAIN%
  if not defined BUILDTARGET set BUILDTARGET=%DEFAULT_BUILDTARGET%
  if not defined THREADNUMBER set THREADNUMBER=%DEFAULT_THREADNUMBER%

  rem # check DSC: Clover default
  if not defined DSCFILE set DSCFILE="%CD%\Clover.dsc"
  if not exist %DSCFILE% (
    set MSG=No Platform
    goto usage
  )

  if ["%BUILD_X64%"] == ["1"] (
    if ["%BUILD_IA32%"] == ["1"] (
      if defined CLEANING (
        echo Cleaning architecture ^(X64 ^& IA32^) ...
      ) else (
        echo Building architecture ^(X64 ^& IA32^) ...
      )
    ) else (
      if defined CLEANING (
        echo Cleaning architecture ^(X64^) ...
      ) else (
        echo Building architecture ^(X64^) ...
      )
    )
  ) else (
    if ["%BUILD_IA32%"] == ["1"] (
      if defined CLEANING (
        echo Cleaning architecture ^(IA32^) ...
      ) else (
        echo Building architecture ^(IA32^) ...
      )
      goto build32
    ) else (
      set MSG=No build architecture
      goto usage
    )
  )

  set "TARGETARCH=X64"
  set UEFI_DRV_LIST=(FSInject OsxFatBinaryDrv VBoxHfs)
  set UEFI_OFF_DRV_LIST=(CsmVideoDxe DataHubDxe EmuVariableUefi OsxAptioFixDrv OsxAptioFix2Drv OsxLowMemFixDrv PartitionDxe)
  set DRV_LIST=(NvmExpressDxe Ps2MouseDxe UsbMouseDxe VBoxIso9600 VBoxExt2 VBoxExt4 XhciDxe)
  goto startbuild

:build32
  set "TARGETARCH=IA32"
  set UEFI_DRV_LIST=(FSInject OsxFatBinaryDrv VBoxHfs)
  set UEFI_OFF_DRV_LIST=(CsmVideoDxe)
  set DRV_LIST=(Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe Ps2MouseDxe UsbMouseDxe VBoxExt2 VBoxExt4 XhciDxe)

:startbuild
  set "CMD_BUILD=build -a %TARGETARCH% -t %TOOLCHAIN% -b %BUILDTARGET% -n %THREADNUMBER% %CLEANING%"
  set F_VER_H=Version.h
  echo.
  if not defined CLEANING (
    echo Building Clover ^(%TARGETARCH%^) ...
    echo.
    echo Generating %CD%\%F_VER_H%
    echo.
  ) else (
    echo Start ^(%CLEANING%^ %TARGETARCH%^) build ...
    echo.
    goto callbuild
  )

  for /f "tokens=2 delims=[]" %%x in ('ver') do set WINVER=%%x
  set WINVER=%WINVER:Version =%

  set "CMD_BUILD=%CMD_BUILD% -p %DSCFILE% %EDK2_BUILD_OPTIONS%"

  set clover_build_info="cbuild.bat"
  set clover_build_info=%clover_build_info:\=\\%
  set clover_build_info=%clover_build_info:"=\"%
  for /f "tokens=* delims= " %%A in ('echo %clover_build_info% ') do set clover_build_info=%%A
  set clover_build_info=%clover_build_info:~0,-1%
  set clover_build_info="Command: %clover_build_info% | OS: Win %WINVER%"

  rem # generate build date and time
  set BUILDDATE=%date:~10,4%-%date:~4,2%-%date:~7,2% %time:~0,-3%

  rem # generate version.h
  echo // Autogenerated %F_VER_H%>%F_VER_H%
  echo #define FIRMWARE_VERSION "2.31">>%F_VER_H%
  echo #define FIRMWARE_BUILDDATE "%BUILDDATE%">>%F_VER_H%
  echo #define FIRMWARE_REVISION L"%SVNREVISION%">>%F_VER_H%
  echo #define REVISION_STR "Clover revision: %SVNREVISION%">>%F_VER_H%
  echo #define BUILDINFOS_STR %clover_build_info%>>%F_VER_H%
 rem # copy %F_VER_H% rEFIt_UEFI\%F_VER_H%

:callbuild
  rem # launch build
  call %CMD_BUILD%

  if errorlevel 1 (
    set MSG=Error while building
    goto failscript
  )

rem # drop compiled files to EFI folder
:postbuild
  set SIGNTOOL_BUILD_DIR=%CD%\SignTool
  set SIGNTOOL_BUILD=BuildSignTool.bat
  set SIGNTOOL=%CD%\SignTool\SignTool
  set BUILD_DIR=%WORKSPACE%\Build\Clover\%BUILDTARGET%_%TOOLCHAIN%
  set DEST_DIR=%CD%\CloverPackage\CloverV2
  set BOOTSECTOR_BIN_DIR=%CD%\CloverEFI\BootSector\bin
  set BUILD_DIR_ARCH=%BUILD_DIR%\%TARGETARCH%

  set TARGETARCH_INT=%TARGETARCH:ia=%
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

  if not defined CLEANING goto finalizebuild

  for /R "%DEST_BOOTSECTORS%" %%i in (boot0* boot1*) do del "%%i"
  for /R "%DEST_BOOTLOADERS%\%TARGETARCH%" %%i in (boot*) do del "%%i"
  for /R "%DEST_TOOLS%" %%i in (Shell*.efi) do ren "%%i" "%%~nxi.ctmp"
  for /R "%DEST_DIR%" %%i in (*%TARGETARCH%.efi *-%TARGETARCH_INT%.efi) do del /F /Q "%%i"
  for /R "%DEST_TOOLS%" %%i in (*.ctmp) do ren "%%i" "%%~ni"

  echo.
  echo End ^(%CLEANING% %TARGETARCH%^) build ...
  echo.
  if ["%BUILD_IA32%"] == ["1"] (
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
  goto noboot
  call:createDir %DEST_BOOTLOADERS%\%TARGETARCH%

  echo Compressing DUETEFIMainFv.FV ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.Fv"

  echo Compressing DxeMain.efi ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\%TARGETARCH%\DxeCore.efi"

  echo Compressing DxeIpl.efi ^(%TARGETARCH%^) ...
  LzmaCompress -e -q -o "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\%TARGETARCH%\DxeIpl.efi"

  echo Generating Loader Image ^(%TARGETARCH%^) ...
  EfiLdrImage -o "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%BUILD_DIR%\%TARGETARCH%\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIpl%TARGETARCH%.z" "%BUILD_DIR%\FV\DxeMain%TARGETARCH%.z" "%BUILD_DIR%\FV\DUETEFIMAINFV%TARGETARCH%.z"

  set EDBA_MAX=417792
  set filesize=

  call:getFilesize "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%"

  if ["%GENPAGE%"] == ["0"] (
    if not ["%USE_LOW_EBDA%"] == ["0"] (
      if defined filesize (
        if %filesize% gtr %EDBA_MAX% (
          echo warning: boot file bigger than low-ebda permits, switching to --std-ebda
          set USE_LOW_EBDA=0
        )
      )
    )
  )

  if ["%TARGETARCH%"] == ["X64"] (
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
    Endlocal
  ) else if ["%USE_BIOS_BLOCKIO%"] == ["1"] (
    set startBlock=Start%TARGETARCH_INT%H.com2
  ) else (
    set startBlock=Start%TARGETARCH_INT%.com
  )
  set GENLDR=Efildr%TARGETARCH_INT%Pure
  if not ["%GENPAGE%"] == ["0"] set GENLDR=boot%TARGETARCH_INT%
  set GENLDR=%BUILD_DIR%\FV\%GENLDR%
  copy /B /Y "%BOOTSECTOR_BIN_DIR%\%startBlock%"+"%BOOTSECTOR_BIN_DIR%\efi%TARGETARCH_INT%.com3"+"%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" "%GENLDR%">nul
  set GENINT=-o "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%"
  if not ["%USE_LOW_EBDA%"] == ["0"] set GENINT= -b 0x88000 -f 0x68000 %GENINT%
  GenPage "%GENLDR%" %GENINT%
  Split -f "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%" -p %BUILD_DIR%\FV\ -o Efildr%TARGETARCH_INT%.1 -t boot%TARGETARCH_INT% -s 512
  del "%BUILD_DIR%\FV\Efildr%TARGETARCH_INT%.1"

  set /A "cloverEFIFile=(%TARGETARCH_INT:~0,1% + %USE_BIOS_BLOCKIO%)"
  set cloverEFIFile=boot%cloverEFIFile%
:noboot
  echo.
  echo Start copying:

  echo.
  echo Mandatory ^(UEFI^) drivers:
  echo.

  set CP_DEST=%DEST_DRIVER%UEFI
  for %%i in %UEFI_DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo Optional ^(UEFI^) drivers:
  echo.

  set CP_DEST=%DEST_OFF%UEFI
  for %%i in %UEFI_OFF_DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo Optional drivers:
  echo.

  set CP_DEST=%DEST_OFF%
  for %%i in %DRV_LIST% do (
    call:copybin "%BUILD_DIR_ARCH%\%%i.efi" "%CP_DEST%" "%%i-%TARGETARCH_INT%.efi"
  )

  echo.
  echo CloverEFI + Applications:
  echo.

  call:copybin "%BUILD_DIR%\FV\boot%TARGETARCH_INT%" "%DEST_BOOTLOADERS%\%TARGETARCH%" "%cloverEFIFile%"
  call:copybin "%BUILD_DIR_ARCH%\bdmesg.efi" "%DEST_TOOLS%" "bdmesg-%TARGETARCH_INT%.efi"
  call:copybin "%BUILD_DIR_ARCH%\CLOVERX64.efi" "%DEST_CLOVER%" "CLOVER%TARGETARCH%.efi"
  call:copybin "%BUILD_DIR_ARCH%\CLOVERX64.efi" "%DEST_BOOT%" "BOOT%TARGETARCH%.efi"

  if not defined CLOVER_EDK2SHELL goto donebuild

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

:donebuild
  echo.
  echo End copying ...
  echo.

  if ["%BUILD_IA32%"] == ["1"] (
    if not ["%TARGETARCH%"] == ["IA32"] goto build32
  )

  if defined CLEANING goto done

  echo Generating BootSectors ...
  echo.

  pushd BootHFS
  set DESTDIR=%DEST_BOOTSECTORS%
  nmake /nologo /c /a /f Makefile.win
  popd

:done
  echo.
  echo ### Build dir: "%BUILD_DIR%"
  echo ### EFI dir: "%DEST_DIR%\EFI"
  echo.
  echo Done!
  exit /b 0

rem # add build option
:addEdk2BuildOption
  set "%~1=1"
  set "EDK2_BUILD_OPTIONS=%EDK2_BUILD_OPTIONS% %~1"
  goto:eof

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
  if not defined MSG (
    set MSG=Build failed
  )
  echo.
  echo !!! %MSG% !!!
  exit /b 0

rem # Print the usage
:usage
  if defined MSG (
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
  echo -m, --module ^<MODULEFILE^> : build only the module specified by the INF argument
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
  rem echo.
  rem echo Report bugs to https://sourceforge.net/p/cloverefiboot/discussion/1726372/
  exit /b 0

:parseArguments
  if ["%~1"] == [""] exit /b 0
  if ["%~1"] == ["--cygwin"] (
    if not ["%~2"] == [""] (
      set CYGWIN_HOME="%~2"
    )
  )
  if ["%~1"] == ["--nasmprefix"] (
    if not ["%~2"] == [""] (
      set NASM_PREFIX="%~2"
    )
  )
  if ["%~1"] == ["--pythonhome"] (
    if not ["%~2"] == [""] (
      set PYTHONHOME="%~2"
    )
  )
  if ["%~1"] == ["--pythonfreezer"] (
    if not ["%~2"] == [""] (
      set PYTHON_FREEZER_PATH="%~2"
    )
  )
  if ["%~1"] == ["-d"] (
    if not ["%~2"] == [""] (
      call:addEdk2BuildOption "-D %~2"
    )
  )
  if ["%~1"] == ["--define"] (
    if not ["%~2"] == [""] (
      call:addEdk2BuildOption "-D %~2"
    )
  )
  if ["%~1"] == ["-m"] (
    if not ["%~2"] == [""] (
      call:addEdk2BuildOption "-m %~2"
    )
  )
  if ["%~1"] == ["-b"] (
    set BUILDTARGET=%2
  )
  if ["%~1"] == ["--buildtarget"] (
    set BUILDTARGET=%2
  )
  if ["%~1"] == ["--debug"] (
    set BUILDTARGET=DEBUG
  )
  if ["%~1"] == ["--release"] (
    set BUILDTARGET=RELEASE
  )
  if ["%~1"] == ["-n"] (
    set THREADNUMBER=%2
  )
  if ["%~1"] == ["--threadnumber"] (
    set THREADNUMBER=%2
  )
  if ["%~1"] == ["-t"] (
    set TOOLCHAIN=%2
  )
  if ["%~1"] == ["--tagname"] (
    set TOOLCHAIN=%2
  )
  if ["%~1"] == ["-a"] (
    if /i ["%~2"] == ["X64"] (
      set BUILD_X64=1
    ) else (
      if /i ["%~2"] == ["IA32"] set BUILD_IA32=1
    )
  )
  if ["%~1"] == ["--arch"] (
    if /i ["%~2"] == ["X64"] (
      set BUILD_X64=1
    ) else (
      if /i ["%~2"] == ["IA32"] set BUILD_IA32=1
    )
  )
  if ["%~1"] == ["-p"] (
    set DSCFILE=%2
  )
  if ["%~1"] == ["--platform"] (
    set DSCFILE=%2
  )
  if ["%~1"] == ["--vbios-patch-cloverefi"] (
    set VBIOSPATCHCLOVEREFI=1
  )
  if ["%~1"] == ["--only-sata0"] (
    set ONLY_SATA_0=1
  )
  if ["%~1"] == ["--std-ebda"] (
    set USE_LOW_EBDA=0
  )
  if ["%~1"] == ["--genpage"] (
    set GENPAGE=1
  )
  if ["%~1"] == ["--mc"] (
    set BUILD_X64=1
    set USE_BIOS_BLOCKIO=1
  )
  if ["%~1"] == ["--no-usb"] (
    set DISABLE_USB_SUPPORT=1
  )
  if ["%~1"] == ["--edk2shell"] (
    set EDK2SHELL=%2
  )
  if ["%~1"] == ["--beta"] (
    set DEVSTAGE=b
  )
  if ["%~1"] == ["-h"] (
    set EDK_BUILDINFOS=%1
  )
  if ["%~1"] == ["--help"] (
    set EDK_BUILDINFOS=%1
  )
  if ["%~1"] == ["--version"] (
    set EDK_BUILDINFOS=%1
  )
  if ["%~1"] == ["--usage"] (
    set SHOW_USAGE=1
  )
  if /i ["%~1"] == ["clean"] (
    set CLEANING=clean
  )
  if /i ["%~1"] == ["cleanall"] (
    set CLEANING=cleanall
  )
  shift
  goto parseArguments
