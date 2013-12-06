@echo off
rem windows batch script for building clover signing tool
rem 2013-12-02 apianti

if defined VCINSTALLDIR goto VisualStudioAvailable
if defined VS110COMNTOOLS (
  call "%VS110COMNTOOLS%\vsvars32.bat"
) else (
  if defined VS100COMNTOOLS (
    call "%VS100COMNTOOLS%\vsvars32.bat"
  ) else (
    if defined VS90COMNTOOLS (
      call "%VS90COMNTOOLS%\vsvars32.bat"
    ) else (
      if defined VS80COMNTOOLS (
        call "%VS80COMNTOOLS%\vsvars32.bat"
      ) else (
        if defined VS71COMNTOOLS (
          call "%VS71COMNTOOLS%\vsvars32.bat"
        ) else (
          echo Cannot find Visual Studio, required to build signing tool!
          goto failscript
        )
      )
    )
  )
)
if errorlevel 1 goto failscript

:VisualStudioAvailable

  set OPENSSL_VERSION=%1
  rem if x"%OPENSSL_VERSION%" == x"" set OPENSSL_VERSION=0.9.8w
  if x"%OPENSSL_VERSION%" == x"" set OPENSSL_VERSION=1.0.1e

  set OPENSSL_DIR=..\Library\OpensslLib\openssl-%OPENSSL_VERSION%\
  set "BUILD_INCLUDE=/I%OPENSSL_DIR% /I%OPENSSL_DIR%include /I%OPENSSL_DIR%crypto"

  pushd .
  cd %OPENSSL_DIR%
  nmake.exe /nologo nt.mak
  popd
  if errorlevel 1 goto failscript
  copy /b /y %OPENSSL_DIR%\out32\libeay32.lib .\libeay32.lib
  if errorlevel 1 goto failscript

  if not exist libeay32.lib (
    echo OpenSSL cryptography library [libeay32.lib] missing!
    goto failscript
  )
  cl.exe /nologo %BUILD_INCLUDE% /FeSignTool.exe SignTool.c libeay32.lib
  if errorlevel 1 goto failscript

  goto:eof

:failscript
   echo Build failed!
   exit /b 1
