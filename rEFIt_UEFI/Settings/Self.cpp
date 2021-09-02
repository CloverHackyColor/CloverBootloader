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

EFI_STATUS Self::__initialize(bool debugMsg, EFI_HANDLE SelfImageHandle, EFI_LOADED_IMAGE** SelfLoadedImagePtr, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL** SelfSimpleVolumePtr, EFI_FILE** SelfVolumeRootDirPtr, XStringW* CloverDirFullPathPtr, XStringW* efiFileNamePtr, EFI_FILE** CloverDirPtr)
{
  EFI_STATUS Status;

  Status = gBS->HandleProtocol(SelfImageHandle, &gEfiLoadedImageProtocolGuid, (void **)SelfLoadedImagePtr);
  if ( EFI_ERROR(Status) ) {
    *SelfLoadedImagePtr = NULL;
    return RETURN_LOAD_ERROR;
  }
  EFI_LOADED_IMAGE* SelfLoadedImage = *SelfLoadedImagePtr;
  if ( SelfLoadedImage->DeviceHandle == NULL ) {
    *SelfLoadedImagePtr = NULL;
    return RETURN_LOAD_ERROR;
  }

  Status = gBS->HandleProtocol(SelfLoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)SelfSimpleVolumePtr);
  if ( EFI_ERROR(Status) ) {
    *SelfSimpleVolumePtr = NULL;
    return RETURN_LOAD_ERROR;
  }
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SelfSimpleVolume = *SelfSimpleVolumePtr;
  
  Status = SelfSimpleVolume->OpenVolume(SelfSimpleVolume, SelfVolumeRootDirPtr);
  if ( EFI_ERROR(Status) ) {
    *SelfVolumeRootDirPtr = NULL;
    return RETURN_LOAD_ERROR;
  }
  EFI_FILE* SelfVolumeRootDir = *SelfVolumeRootDirPtr;
  
#ifdef JIEF_DEBUG
  if ( debugMsg ) DBG("SelfVolumeRootDir = %lld\n", uintptr_t(SelfVolumeRootDir));
#endif

  // find the current directory
  *CloverDirFullPathPtr = FileDevicePathToXStringW(SelfLoadedImage->FilePath);
  if ( CloverDirFullPathPtr->isEmpty() ) {
    return RETURN_LOAD_ERROR;
  }
  XStringW& CloverDirFullPath = *CloverDirFullPathPtr;

  // Do this before the next check.
  if ( !CloverDirFullPath.startWith('\\') ) {
    // Some firmware seems to not put a '\' at the begining. Do not panic anymore, just add it.
    CloverDirFullPath.insertAtPos('\\', 0);
  }

  *efiFileNamePtr = CloverDirFullPath.basename();
#ifdef JIEF_DEBUG
  if ( debugMsg ) {
    XStringW& efiFileName = *efiFileNamePtr;
    DBG("efiFileName=%ls\n", efiFileName.wc_str());
  }
#endif

  // History : if this Clover was started as BootX64.efi, redirect to /EFI/CLOVER
  if ( CloverDirFullPath.isEqualIC("\\EFI\\Boot\\BootX64.efi") ) { // keep getCloverDir() in sync !
    CloverDirFullPath.takeValueFrom("\\EFI\\CLOVER\\CloverX64.efi"); // keep getCloverDir() in sync !
  }

  if ( CloverDirFullPath.lastChar() == U'\\' ) { // keep getCloverDir() in sync !
    log_technical_bug("CloverDirFullPath.lastChar() == U'\\'"); // Just to see if that happens.
    CloverDirFullPath.deleteCharsAtPos(CloverDirFullPath.length()-1, 1);
  }

  size_t i = CloverDirFullPath.rindexOf(U'\\', SIZE_T_MAX-1); // keep getCloverDir() in sync !
  if ( i != SIZE_T_MAX && i > 0 ) CloverDirFullPath.deleteCharsAtPos(i, SIZE_T_MAX); // keep getCloverDir() in sync !

#ifdef JIEF_DEBUG
  if ( debugMsg ) {
    DBG("SelfDirPath = %ls\n", CloverDirFullPath.wc_str());
  }
#endif

  Status = SelfVolumeRootDir->Open(SelfVolumeRootDir, CloverDirPtr, CloverDirFullPath.wc_str(), EFI_FILE_MODE_READ, 0);
  if ( EFI_ERROR(Status) ) {
    *CloverDirPtr = NULL;
    return RETURN_LOAD_ERROR;
  }
#ifdef JIEF_DEBUG
  if ( debugMsg ) {
    EFI_FILE* CloverDir = *CloverDirPtr;
    DBG("CloverDir = %lld\n", uintptr_t(CloverDir));
  }
#endif

  return EFI_SUCCESS;
}

const EFI_FILE_PROTOCOL* Self::getCloverDirAndEfiFileName(EFI_HANDLE ImageHandle, XStringW* efiFileName)
{
    EFI_LOADED_IMAGE* SelfLoadedImage;            // this efi.
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SelfSimpleVolume;  // Volume containing this efi.
    EFI_FILE*         SelfVolumeRootDir;          // Root dir of the volume containing this efi.
    XStringW          CloverDirFullPath;          // full path of folder containing this efi.
    EFI_FILE*         CloverDir;                  // opened folder containing this efi

  /*EFI_STATUS Status = */__initialize(false, ImageHandle, &SelfLoadedImage, &SelfSimpleVolume, &SelfVolumeRootDir, &CloverDirFullPath, efiFileName, &CloverDir);
  if ( efiFileName->isEmpty() ) {
    if ( CloverDir != NULL ) CloverDir->Close(CloverDir);
    return NULL;
  }
  return CloverDir;
}



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
//  EFI_STATUS Status;

  /*Status = */__initialize(true, m_SelfImageHandle, &m_SelfLoadedImage, &m_SelfSimpleVolume, &m_SelfVolumeRootDir, &m_CloverDirFullPath, &m_efiFileName, &m_CloverDir);
  if ( m_SelfLoadedImage == NULL ) log_technical_bug("Cannot get SelfLoadedImage");
  if ( m_SelfLoadedImage->DeviceHandle == NULL ) log_technical_bug("m_SelfLoadedImage->DeviceHandle == NULL");
  if ( m_SelfSimpleVolume == NULL ) log_technical_bug("Cannot get m_SelfSimpleVolume");
  if ( m_SelfVolumeRootDir == NULL ) log_technical_bug("Cannot get m_SelfVolumeRootDir");
  if ( m_CloverDirFullPath.isEmpty() ) log_technical_bug("Cannot get m_CloverDirFullPath");
  if ( m_efiFileName.isEmpty() ) log_technical_bug("Cannot get m_efiFileName");
  if ( m_CloverDir == NULL ) panic("Cannot open getSelfRootDir()"); // We have to panic, nothing would work without m_CloverDir
  
  m_SelfDevicePath = NULL;
  if ( m_SelfLoadedImage && m_SelfLoadedImage->DeviceHandle ) {
    m_SelfDevicePath = FileDevicePath(m_SelfLoadedImage->DeviceHandle, m_CloverDirFullPath + '\\' + m_efiFileName);
  }
  if ( m_SelfDevicePath == NULL ) {
    log_technical_bug("m_SelfDevicePath == NULL"); // not sure what can work without m_SelfDevicePath
  }else{
    #ifdef JIEF_DEBUG
      DBG("Self DevicePath()=%ls @%llX\n", FileDevicePathToXStringW(m_SelfDevicePath).wc_str(), (uintptr_t)m_SelfLoadedImage->DeviceHandle);
    #endif
  }
//    m_SelfDevicePathAsXStringW = FileDevicePathToXStringW(m_SelfDevicePath);

  _openDir(THEMES_DIRNAME, &m_ThemesDirExists, &m_ThemesDir); // don't need to check returned status as it's ok to not have a themes dir.

  return EFI_SUCCESS;


//
//
//  Status = gBS->HandleProtocol(m_SelfImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&m_SelfLoadedImage);
//  if ( EFI_ERROR(Status) ) panic("Cannot get SelfLoadedImage"); // We have to panic, nothing would work without m_CloverDir, and m_SelfImageHandle is needed to get it
//  if ( m_SelfLoadedImage->DeviceHandle == NULL ) panic("m_SelfLoadedImage->DeviceHandle == NULL"); // We have to panic, nothing would work without m_CloverDir, and m_SelfLoadedImage->DeviceHandle is needed to get it
//  m_SelfDevicePath = DuplicateDevicePath(DevicePathFromHandle(m_SelfLoadedImage->DeviceHandle));
//  if ( m_SelfDevicePath == NULL ) {
//    log_technical_bug("m_SelfDevicePath == NULL"); // not sure what can work without m_SelfDevicePath
//  }
//
//#ifdef JIEF_DEBUG
//  DBG("Self DevicePath()=%ls @%llX\n", FileDevicePathToXStringW(m_SelfDevicePath).wc_str(), (uintptr_t)m_SelfLoadedImage->DeviceHandle);
//#endif
//
//  Status = gBS->HandleProtocol(m_SelfLoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&m_SelfSimpleVolume);
//  if ( EFI_ERROR(Status) ) panic("Cannot get m_SelfSimpleVolume"); // We have to panic, nothing would work without m_CloverDir, and m_SelfSimpleVolume is needed to get it
//  Status = m_SelfSimpleVolume->OpenVolume(m_SelfSimpleVolume, &m_SelfVolumeRootDir);
//  if ( EFI_ERROR(Status) ) panic("Cannot get m_SelfVolumeRootDir"); // We have to panic, nothing would work without m_CloverDir, and m_SelfVolumeRootDir is needed to get it
//
//#ifdef JIEF_DEBUG
//  DBG("m_SelfVolumeRootDir = %lld\n", uintptr_t(m_SelfVolumeRootDir));
//#endif
//
//  // find the current directory
//  m_CloverDirFullPath = FileDevicePathToXStringW(m_SelfLoadedImage->FilePath);
//
//  // Do this before the next check.
//  if ( !m_CloverDirFullPath.startWith('\\') ) {
//    //CHAR16* f = ConvertDevicePathToText(m_SelfLoadedImage->FilePath, TRUE, TRUE);
//    //panic("Bad format for m_CloverDirFullPath(%ls). It must start with a '\\'.\nConvertDevicePathToText=%ls", m_CloverDirFullPath.wc_str(), f);
//    //
//    // Some firmware seems to not put a '\' at the begining. Do not panic anymore, just add it.
//    m_CloverDirFullPath.insertAtPos('\\', 0);
//  }
//
//  m_efiFileName = m_CloverDirFullPath.basename();
//#ifdef JIEF_DEBUG
//  DBG("m_efiFileName=%ls\n", m_efiFileName.wc_str());
//#endif
//
//  // Code to find m_CloverDirFullPath is duplicated in BootLog.cpp in case USE_SELF_INSTANCE is not defined
//
//  // History : if this Clover was started as BootX64.efi, redirect to /EFI/CLOVER
//  if ( m_CloverDirFullPath.isEqualIC("\\EFI\\Boot\\BootX64.efi") ) { // keep getCloverDir() in sync !
//    m_CloverDirFullPath.takeValueFrom("\\EFI\\CLOVER\\CloverX64.efi"); // keep getCloverDir() in sync !
//  }
//  if ( m_CloverDirFullPath.isEmpty() ) log_technical_bug("m_CloverDirFullPath.isEmpty()");
//  m_SelfDevicePath = FileDevicePath(m_SelfLoadedImage->DeviceHandle, m_CloverDirFullPath);
////  m_SelfDevicePathAsXStringW = FileDevicePathToXStringW(m_SelfDevicePath);
//
//  if ( m_CloverDirFullPath.lastChar() == U'\\' ) { // keep getCloverDir() in sync !
//    log_technical_bug("m_CloverDirFullPath.lastChar() == U'\\'"); // Just to see if that happens.
//    m_CloverDirFullPath.deleteCharsAtPos(m_CloverDirFullPath.length()-1, 1);
//  }
//
//  size_t i = m_CloverDirFullPath.rindexOf(U'\\', SIZE_T_MAX-1); // keep getCloverDir() in sync !
//  if ( i != SIZE_T_MAX && i > 0 ) m_CloverDirFullPath.deleteCharsAtPos(i, SIZE_T_MAX); // keep getCloverDir() in sync !
//
//#ifdef JIEF_DEBUG
//  DBG("SelfDirPath = %ls\n", m_CloverDirFullPath.wc_str());
//#endif
//  Status = m_SelfVolumeRootDir->Open(m_SelfVolumeRootDir, &m_CloverDir, m_CloverDirFullPath.wc_str(), EFI_FILE_MODE_READ, 0);
//  if ( EFI_ERROR(Status) ) panic("Cannot open getSelfRootDir()"); // We have to panic, nothing would work without m_CloverDir
//#ifdef JIEF_DEBUG
//  DBG("m_CloverDir = %lld\n", uintptr_t(m_CloverDir));
//#endif
//
//
//  _openDir(THEMES_DIRNAME, &m_ThemesDirExists, &m_ThemesDir); // don't need to check returned status as it's ok to not have a themes dir.
//
//  return EFI_SUCCESS;
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

