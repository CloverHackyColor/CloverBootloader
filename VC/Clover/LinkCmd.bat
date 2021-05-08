@echo on
set curdir=%cd%

set TARGETDIR=.\x64\Debug
if NOT ["%~1"] == [""] (
	set TARGETDIR=%~1
)
IF %TARGETDIR:~-1%==\ SET TARGETDIR=%TARGETDIR:~0,-1%

set TARGETFILENAME=CloverX64.lib
if NOT ["%~2"] == [""] (
	set TARGETFILENAME=%~2
)
for %%i in (%TARGETFILENAME%) do @set TARGETBASENAME=%%~ni

set GENFWPATH=..\..\BaseTools\Bin\Win32\GenFw.exe


@echo curdir=%curdir%
@echo TARGETDIR=%TARGETDIR%
@echo TARGETFILENAME=%TARGETFILENAME%
@echo TARGETBASENAME=%TARGETBASENAME%
@echo GENFWPATH=%GENFWPATH%


if NOT exist "Clover.vcxproj" (
	echo This script must be run from VC++ Clover project dir, which nust be 2 levels down from main Clover folder.
	cd %curdir%
	exit /b 1
)

if NOT exist "%GENFWPATH%" (
	echo The VC++ Clover project dir must 2 levels down from main Clover folder. GenFW must be found at %GENFWPATH%.
	cd %curdir%
	exit /b 1
)


if not exist "%TARGETDIR%" (
	echo Target dir doesn't exist
	cd %curdir%
	exit /b 1
)

del %TARGETDIR%\static_library_files.lst
FOR /F "tokens=* skip=1" %%A IN ('type "..\..\Build\Clover\RELEASE_VS2017\X64\rEFIt_UEFI\refit\OUTPUT\static_library_files.lst"') DO @echo %%A >> %TARGETDIR%\static_library_files.lst
rem @echo "/JiefLand/5.Devel/Clover/Clover-projects/Clover--CloverHackyColor--masterVC/VC/Clover/x64/Debug/StaticLib1.lib" >> %TARGETDIR%\static_library_files.lst

rem ..\..\BaseTools\get_vsvars.bat <- not needed when it's called from VS ide

rem /IGNORE:4210 = .CRT section exists; there may be unhandled static initializers or terminators
rem /IGNORE:4254
rem /WHOLEARCHIVE : makes __chkstk to come up

rem edkII link cmd
rem "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\bin\Hostx86\x64\link.exe" /OUT:c:\users\jief\cloverbootloader_synced\Build\Clover\DEBUG_VS2017\X64\rEFIt_UEFI\refit\DEBUG\CLOVERX64.dll
rem /NOLOGO /NODEFAULTLIB /IGNORE:4001 /IGNORE:4281 /OPT:REF /OPT:ICF=10 /MAP /ALIGN:32 /SECTION:.xdata,D /SECTION:.pdata,D /Machine:X64 /LTCG /DLL /ENTRY:_ModuleEntryPoint /SUBSYSTEM:EFI_BOOT_SERVICE_DRIVER /SAFESEH:NO /BASE:0 /DRIVER /DEBUG /WHOLEARCHIVE
rem  @c:\users\jief\cloverbootloader_synced\Build\Clover\DEBUG_VS2017\X64\rEFIt_UEFI\refit\OUTPUT\static_library_files.lst

rem .cod seems to be generated in current dir. Didn't find options t change that. /Fa is for compiler, not linker.
cd %TARGETDIR%
link.exe /OUT:%TARGETBASENAME%.dll /NOLOGO /NODEFAULTLIB /IGNORE:4001 /IGNORE:4281 /IGNORE:4210 /OPT:REF /OPT:ICF=10 /MAP /ALIGN:32 /SECTION:.xdata,D /SECTION:.pdata,D /Machine:X64 /LTCG /DLL /ENTRY:_ModuleEntryPoint /SUBSYSTEM:EFI_BOOT_SERVICE_DRIVER /SAFESEH:NO /BASE:0 /DRIVER /DEBUG /WHOLEARCHIVE %TARGETFILENAME% @static_library_files.lst

if %ERRORLEVEL% neq 0 (
	cd %curdir%
	exit /b %ERRORLEVEL%
)
cd %curdir%


%GENFWPATH% -e UEFI_APPLICATION -o %TARGETDIR%\%TARGETBASENAME%.efi %TARGETDIR%\%TARGETBASENAME%.dll
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo EFI generated in %TARGETDIR%\%TARGETBASENAME%.efi

cd %curdir%
