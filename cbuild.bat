@echo off
rem windows batch script for building clover
rem 2012-09-06 apianti

set ENABLE_SECURE_BOOT=0

rem setup current dir and edk2 if needed
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

rem have edk2 prepare to build
:foundedk
   echo Found EDK2. Generating %WORKSPACE%\Clover\Version.h
   cd "%WORKSPACE%\Clover"
   rem get svn revision number
   svnversion -n > vers.txt
   set /p s= < vers.txt
   del vers.txt
   set SVNREVISION=

   rem get the current revision number
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

      rem check for revision number
      if x"%SVNREVISION%" == x"" goto failscript

  rem parse parameters for what we need
  set BUILD_ARCH=
  set TOOL_CHAIN_TAG=
  set TARGET=
  set DSCFILE=
  set CLEANING=
  set errorlevel=0
  set THREADNUMBER=

  call:parseArguments %*
  if errorlevel 1 goto failscript

  rem fix any parameters not set
  set CONFIG_FILE="%WORKSPACE%\Conf\target.txt"
  set DEFAULT_TOOL_CHAIN_TAG=MYTOOLS
  set DEFAULT_TARGET=DEBUG
  for /f "tokens=1*" %%i in ('type %CONFIG_FILE% ^| find "TOOL_CHAIN_TAG" ^| find /V "#"') do set DEFAULT_TOOL_CHAIN_TAG%%j
  for /f "tokens=*" %%i in ("%DEFAULT_TOOL_CHAIN_TAG%") do set DEFAULT_TOOL_CHAIN_TAG=%%i
  for /f "tokens=1*" %%i in ('type %CONFIG_FILE% ^| find "TARGET" ^| find /V "#" ^| find /V "TARGET_ARCH"') do set DEFAULT_TARGET%%j
  for /f "tokens=*" %%i in ("%DEFAULT_TARGET%") do set DEFAULT_TARGET=%%i
  if x"%DEFAULT_TOOL_CHAIN_TAG%" == x"" set DEFAULT_TOOL_CHAIN_TAG=MYTOOLS
  if x"%DEFAULT_TARGET%" == x"" set DEFAULT_TARGET=DEBUG
  if x"%TOOL_CHAIN_TAG%" == x"" set TOOL_CHAIN_TAG=%DEFAULT_TOOL_CHAIN_TAG%
  if x"%TARGET%" == x"" set TARGET=%DEFAULT_TARGET%
  set DEFAULT_THREADNUMBER=%NUMBER_OF_PROCESSORS%
  if x"%THREADNUMBER%" == x"" set THREADNUMBER=%DEFAULT_THREADNUMBER%


  rem check DSC: Clover default
  if x"%DSCFILE%" == x"" set DSCFILE="%CURRENTDIR%\Clover.dsc"

  set "ARCH_ARGUMENT="
  if not x"%BUILD_ARCH%" == x"" goto buildall
  echo Building selected (X64 ^& IA32) ...

:buildall
  if x"%BUILD_ARCH%" == x"IA32" goto build32

  set "ARCH_ARGUMENT=-a X64"

  echo Building Clover (X64) ...
  goto startbuild

:build32
  set "ARCH_ARGUMENT=-a IA32"
  set "BUILD_ARCH=IA32"

  echo Building Clover (IA32) ...
  goto startbuild

:startbuild
  if x"%DSCFILE%" == x"" goto failscript

  if not x"%CLEANING%" == x"" echo Clean build (%CLEANING%) ...

  set "MY_TOOL_CHAIN_TAG=-t %TOOL_CHAIN_TAG%"
  set "MY_TARGET=-b %TARGET%"
  set "MY_THREADNUMBER=-n %THREADNUMBER%"

  for /f "tokens=2 delims=[]" %%x in ('ver') do set WINVER=%%x
  set WINVER=%WINVER:Version =%

  set "CMD_BUILD=build -p %DSCFILE% -DNO_GRUB_DRIVERS %ARCH_ARGUMENT% %MY_TOOL_CHAIN_TAG% %MY_TARGET% %MY_THREADNUMBER% %CLEANING%"

  set clover_build_info=%CMD_BUILD%
  set clover_build_info=%clover_build_info:\=\\%
  set clover_build_info=%clover_build_info:"=\"%
  set clover_build_info="Args: %~nx0 %* | Command: %clover_build_info% | OS: Win %WINVER%"

      rem generate build date and time
      set BUILDDATE=
      echo Dim cdt, output, temp > buildtime.vbs
      rem output year
      echo cdt = Now >> buildtime.vbs
      echo output = Year(cdt) ^& "-" >> buildtime.vbs
      rem output month
      echo temp = Month(cdt) >> buildtime.vbs
      echo If temp ^< 10 Then >> buildtime.vbs
      echo    output = output ^& "0" >> buildtime.vbs
      echo End If >> buildtime.vbs
      echo output = output ^& temp ^& "-" >> buildtime.vbs
      rem output day
      echo temp = Day(cdt) >> buildtime.vbs
      echo If temp ^< 10 Then >> buildtime.vbs
      echo    output = output ^& "0" >> buildtime.vbs
      echo End If >> buildtime.vbs
      echo output = output ^& temp ^& " " >> buildtime.vbs
      rem output hours
      echo temp = Hour(cdt) >> buildtime.vbs
      echo If temp ^< 10 Then >> buildtime.vbs
      echo    output = output ^& "0" >> buildtime.vbs
      echo End If >> buildtime.vbs
      echo output = output ^& temp ^& ":" >> buildtime.vbs
      rem output minutes
      echo temp = Minute(cdt) >> buildtime.vbs
      echo If temp ^< 10 Then >> buildtime.vbs
      echo    output = output ^& "0" >> buildtime.vbs
      echo End If >> buildtime.vbs
      echo output = output ^& temp ^& ":" >> buildtime.vbs
      rem output seconds
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

  rem pre - generate version.h
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
   set BUILD_DIR=%WORKSPACE%\Build\Clover\%TARGET%_%TOOL_CHAIN_TAG%
   set DEST_DIR=%WORKSPACE%\Clover\CloverPackage\CloverV2
  rem set BASETOOLS_DIR=%WORKSPACE_TOOLS_PATH%\Bin\Win32
   set BOOTSECTOR_BIN_DIR=%WORKSPACE%\Clover\BootSector\bin

   if x"%ENABLE_SECURE_BOOT%" == x"1" (
      echo Building signing tool ...
      pushd .
      cd "%SIGNTOOL_BUILD_DIR%"
      call "%SIGNTOOL_BUILD%"
      popd
      if errorlevel 1 goto failscript
   )

  if x"%BUILD_ARCH%" == x"IA32" goto postbuild32

   echo Compressing DUETEFIMainFv.FV (X64) ...
   "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DUETEFIMAINFVX64.z" "%BUILD_DIR%\FV\DUETEFIMAINFVX64.Fv"

   echo Compressing DxeMain.efi (X64) ...
   "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeMainX64.z" "%BUILD_DIR%\X64\DxeCore.efi"

   echo Compressing DxeIpl.efi (X64) ...
   "%BASETOOLS_DIR%\LzmaCompress" -e -o "%BUILD_DIR%\FV\DxeIplX64.z" "%BUILD_DIR%\X64\DxeIpl.efi"

   echo Generating Loader Image (X64) ...
   "%BASETOOLS_DIR%\EfiLdrImage.exe" -o "%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\X64\EfiLoader.efi" "%BUILD_DIR%\FV\DxeIplX64.z" "%BUILD_DIR%\FV\DxeMainX64.z" "%BUILD_DIR%\FV\DUETEFIMAINFVX64.z"
  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start64.com"+"%BOOTSECTOR_BIN_DIR%\Efi64.com2"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\EfildrPure"
   rem "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\EfildrPure" -o "%BUILD_DIR%\FV\Efildr"
  rem copy /B "%BOOTSECTOR_BIN_DIR%\St16_64.com"+"%BOOTSECTOR_BIN_DIR%\Efi64.com2"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\Efildr16Pure"
   rem "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr16Pure -o "%BUILD_DIR%\FV\Efildr16"
  copy /B "%BOOTSECTOR_BIN_DIR%\Start64H.com"+"%BOOTSECTOR_BIN_DIR%\efi64.com3"+"%BUILD_DIR%\FV\Efildr64" "%BUILD_DIR%\FV\Efildr20Pure"
   "%BASETOOLS_DIR%\GenPage.exe" "%BUILD_DIR%\FV\Efildr20Pure" -o "%BUILD_DIR%\FV\Efildr20"
   "%BASETOOLS_DIR%\Split.exe" -f "%BUILD_DIR%\FV\Efildr20" -p %BUILD_DIR%\FV\ -o Efildr20.1 -t boot64 -s 512
   del "%BUILD_DIR%\FV\Efildr20.1"

  copy /B /Y "%BUILD_DIR%\FV\boot64" "%DEST_DIR%\Bootloaders\X64\boot"
  copy /B /Y "%BUILD_DIR%\X64\FSInject.efi" "%DEST_DIR%\drivers-Off\drivers64\FSInject-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\FSInject.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\FSInject-64.efi"
  rem copy /B /Y "%BUILD_DIR%\X64\VBoxIso9600.efi" "%DEST_DIR%\drivers-Off\drivers64\VBoxIso9600-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\VBoxExt2.efi" "%DEST_DIR%\drivers-Off\drivers64\VBoxExt2-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\PartitionDxe.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\PartitionDxe-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\DataHubDxe.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\DataHubDxe-64.efi"

  rem copy /B /Y "%BUILD_DIR%\X64\Ps2KeyboardDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\Ps2KeyboardDxe-64.efi"
  rem copy /B /Y "%BUILD_DIR%\X64\Ps2MouseAbsolutePointerDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\Ps2MouseAbsolutePointerDxe-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\Ps2MouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\Ps2MouseDxe-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\UsbMouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\UsbMouseDxe-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\XhciDxe.efi" "%DEST_DIR%\drivers-Off\drivers64\XhciDxe-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\OsxFatBinaryDrv.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\OsxFatBinaryDrv-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\OsxAptioFixDrv.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\OsxAptioFixDrv-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\OsxLowMemFixDrv.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\OsxLowMemFixDrv-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\CsmVideoDxe.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\CsmVideoDxe-64.efi"
  copy /B /Y "%BUILD_DIR%\X64\EmuVariableUefi.efi" "%DEST_DIR%\drivers-Off\drivers64UEFI\EmuVariableUefi-64.efi"
  copy /B /Y "%WORKSPACE%\Build\Clover\%TARGET%_%TOOL_CHAIN_TAG%\X64\CLOVERX64.efi" "%DEST_DIR%\EFI\Clover\CLOVERX64.efi"
  copy /B /Y "%WORKSPACE%\Build\Clover\%TARGET%_%TOOL_CHAIN_TAG%\X64\CLOVERX64.efi" "%DEST_DIR%\EFI\BOOT\BOOTX64.efi"

  if x"%BUILD_ARCH%" == x"" goto build32
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
  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start.com"+"%BOOTSECTOR_BIN_DIR%\Efi32.com2"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\Efildr"
  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start16.com"+"%BOOTSECTOR_BIN_DIR%\Efi32.com2"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\Efildr16"
  rem copy /B "%BOOTSECTOR_BIN_DIR%\Start32.com"+"%BOOTSECTOR_BIN_DIR%\Efi32.com3"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\Efildr20"
  copy /B "%BOOTSECTOR_BIN_DIR%\start32H.com2"+"%BOOTSECTOR_BIN_DIR%\efi32.com3"+"%BUILD_DIR%\FV\Efildr32" "%BUILD_DIR%\FV\boot32"

  copy /B /Y "%BUILD_DIR%\FV\boot32" "%DEST_DIR%\Bootloaders\ia32\boot"
  copy /B /Y "%BUILD_DIR%\IA32\FSInject.efi" "%DEST_DIR%\drivers-Off\drivers32\FSInject-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\VBoxIso9600.efi" "%DEST_DIR%\drivers-Off\drivers32\VBoxIso9600-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\VBoxExt2.efi" "%DEST_DIR%\drivers-Off\drivers32\VBoxExt2-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\Ps2KeyboardDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\Ps2KeyboardDxe-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\Ps2MouseAbsolutePointerDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\Ps2MouseAbsolutePointerDxe-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\Ps2MouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\Ps2MouseDxe-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\UsbMouseDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\UsbMouseDxe-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\XhciDxe.efi" "%DEST_DIR%\drivers-Off\drivers32\XhciDxe-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\OsxFatBinaryDrv.efi" "%DEST_DIR%\drivers-Off\drivers32UEFI\OsxFatBinaryDrv-32.efi"
  copy /B /Y "%BUILD_DIR%\IA32\CsmVideoDxe.efi" "%DEST_DIR%\drivers-Off\drivers32UEFI\CsmVideoDxe-32.efi"
  copy /B /Y "%WORKSPACE%\Build\Clover\%TARGET%_%TOOL_CHAIN_TAG%\IA32\CLOVERIA32.efi" "%DEST_DIR%\EFI\Clover\CLOVERIA32.efi"

  goto done

:parseArguments
   if x"%1" == x"" goto:eof
   if x"%1" == x"-D" (
      if x"%2" == x"ENABLE_SECURE_BOOT" (
         set ENABLE_SECURE_BOOT=1
      )
   )
  if x"%1" == x"-n" (
    set THREADNUMBER=%2
  )
  if x"%1" == x"--THREADNUMBER" (
    set THREADNUMBER=%2
  )
   if x"%1" == x"-t" (
      set TOOL_CHAIN_TAG=%2
   )
   if x"%1" == x"--tagname" (
      set TOOL_CHAIN_TAG=%2
   )
   if x"%1" == x"-b" (
      set TARGET=%2
   )
   if x"%1" == x"--buildtarget" (
      set TARGET=%2
   )
   if x"%1" == x"-a" (
      set BUILD_ARCH=%2
   )
   if x"%1" == x"--arch" (
      set BUILD_ARCH=%2
   )
   if x"%1" == x"-p" (
      set DSCFILE=%2
   )
   if x"%1" == x"--platform" (
      set DSCFILE=%2
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
