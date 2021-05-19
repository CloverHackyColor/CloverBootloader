/*
 * Self.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#include "../Settings/Self.h"
#include <Platform.h>
#include "../refit/lib.h"

#ifndef DEBUG_ALL
#define DEBUG_SELF 1
#else
#define DEBUG_SELF DEBUG_ALL
#endif

#if DEBUG_SELF == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SELF, __VA_ARGS__)
#endif


Self self;

constexpr const LStringW THEMES_DIRNAME = L"Themes"_XSW;

EFI_STATUS Self::_openDir(const XStringW& path, bool* b, EFI_FILE** efiDir)
{
  EFI_STATUS Status;
  Status = m_CloverDir->Open(m_CloverDir, efiDir, path.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) {
    DBG("Error when opening dir '%ls\\%ls' : %s\n", m_CloverDirFullPath.wc_str(), path.wc_str(), efiStrError(Status));
    *efiDir = NULL;
    *b = false;
  }else{
    *b = true;
  }
  return Status;
}

EFI_STATUS Self::_initialize()
{
  EFI_STATUS Status;

  Status = gBS->HandleProtocol(m_SelfImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&m_SelfLoadedImage);
#ifdef DEBUG
  if ( EFI_ERROR(Status) ) panic("Cannot get SelfLoadedImage");
  if ( m_SelfLoadedImage->DeviceHandle == NULL ) panic("m_SelfLoadedImage->DeviceHandle == NULL");
  m_SelfDevicePath = DuplicateDevicePath(DevicePathFromHandle(m_SelfLoadedImage->DeviceHandle));
  if ( m_SelfDevicePath == NULL )  panic("m_SelfDevicePath == NULL");
#else
  if ( EFI_ERROR(Status) ) return Status;
  if ( m_SelfLoadedImage->DeviceHandle == NULL ) return EFI_NOT_FOUND;
  m_SelfDevicePath = DuplicateDevicePath(DevicePathFromHandle(m_SelfLoadedImage->DeviceHandle));
  if ( m_SelfDevicePath == NULL ) return EFI_NOT_FOUND;
#endif
  
#ifdef JIEF_DEBUG
  DBG("Self DevicePath()=%ls @%llX\n", FileDevicePathToXStringW(m_SelfDevicePath).wc_str(), (uintptr_t)m_SelfLoadedImage->DeviceHandle);
#endif
  Status = gBS->HandleProtocol(m_SelfLoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&m_SelfSimpleVolume);
#ifdef DEBUG
  if ( EFI_ERROR(Status) ) panic("Cannot get m_SelfSimpleVolume");
  Status = m_SelfSimpleVolume->OpenVolume(m_SelfSimpleVolume, &m_SelfVolumeRootDir);
  if ( EFI_ERROR(Status) ) panic("Cannot get m_SelfRootDir");
#else
  if ( EFI_ERROR(Status) ) return Status;
  Status = m_SelfSimpleVolume->OpenVolume(m_SelfSimpleVolume, &m_SelfVolumeRootDir);
  if ( EFI_ERROR(Status) ) return Status;
#endif
  // find the current directory
  m_CloverDirFullPath = FileDevicePathToXStringW(m_SelfLoadedImage->FilePath);

  // Do this before the next check.
  if ( !m_CloverDirFullPath.startWith('\\') ) {
    //CHAR16* f = ConvertDevicePathToText(m_SelfLoadedImage->FilePath, TRUE, TRUE);
    //panic("Bad format for m_CloverDirFullPath(%ls). It must start with a '\\'.\nConvertDevicePathToText=%ls", m_CloverDirFullPath.wc_str(), f);
    //
    // Somefirmware seems to not put a '\' at the begining. Do not panic anymore, just add it.
    m_CloverDirFullPath.insertAtPos('\\', 0);
  }

  m_efiFileName = m_CloverDirFullPath.basename();
#ifdef JIEF_DEBUG
  DBG("m_efiFileName=%ls\n", m_efiFileName.wc_str());
#endif

  // History : if this Clover was started as BootX64.efi, redirect to /EFI/CLOVER
  if ( m_CloverDirFullPath.isEqualIC("\\EFI\\Boot\\BootX64.efi") ) {
    m_CloverDirFullPath.takeValueFrom("\\EFI\\CLOVER\\CloverX64.efi");
  }
#ifdef DEBUG
  if ( m_CloverDirFullPath.isEmpty() ) panic("m_CloverDirFullPath.isEmpty()");
#else
  if ( m_CloverDirFullPath.isEmpty() ) return EFI_NOT_FOUND;
#endif
  m_SelfDevicePath = FileDevicePath(m_SelfLoadedImage->DeviceHandle, m_CloverDirFullPath);
  m_SelfDevicePathAsXStringW = FileDevicePathToXStringW(m_SelfDevicePath);

#ifdef DEBUG
  if ( m_CloverDirFullPath.lastChar() == U'\\' ) panic("m_CloverDirFullPath.lastChar() == U'\\'");
//if ( m_CloverDirFullPath.endsWith('\\') ) panic("m_CloverDirFullPath.endsWith('\\')");
#else
  if ( m_CloverDirFullPath.lastChar() == U'\\' ) return EFI_NOT_FOUND;
#endif


  size_t i = m_CloverDirFullPath.rindexOf(U'\\', SIZE_T_MAX-1);
  if ( i != SIZE_T_MAX && i > 0 ) m_CloverDirFullPath.deleteCharsAtPos(i, SIZE_T_MAX);

#ifdef JIEF_DEBUG
  DBG("SelfDirPath = %ls\n", m_CloverDirFullPath.wc_str());
#endif
  Status = m_SelfVolumeRootDir->Open(m_SelfVolumeRootDir, &m_CloverDir, m_CloverDirFullPath.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) panic("Cannot open getSelfRootDir()");


  _openDir(THEMES_DIRNAME, &m_ThemesDirExists, &m_ThemesDir); // don't need to check returned status as it's ok to not have a themes dir.

  return EFI_SUCCESS;
}

EFI_STATUS Self::initialize(EFI_HANDLE ImageHandle)
{
  m_SelfImageHandle = ImageHandle;
  return _initialize();
}

EFI_STATUS Self::reInitialize()
{
  closeHandle();
  return _initialize();
}


void Self::closeHandle(void)
{
  // called before running external programs to close open file handles

  if (m_CloverDir != NULL) {
    m_CloverDir->Close(m_CloverDir);
    m_CloverDir = NULL;
  }

  if (m_SelfVolumeRootDir != NULL) {
    m_SelfVolumeRootDir->Close(m_SelfVolumeRootDir);
    m_SelfVolumeRootDir = NULL;
  }
}

