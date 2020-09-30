/*
 * Self.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#include "Self.h"
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

EFI_STATUS Self::initialize(EFI_HANDLE ImageHandle)
{
  EFI_STATUS Status;
  m_SelfImageHandle = ImageHandle;
  Status = gBS->HandleProtocol(self.getSelfImageHandle(), &gEfiLoadedImageProtocolGuid, (VOID **) &m_SelfLoadedImage);
  if ( EFI_ERROR(Status) ) panic("Cannot get SelfLoadedImage");
  if ( self.getSelfDeviceHandle() == NULL )  panic("self.getSelfDeviceHandle() == NULL");

  m_SelfDevicePath = DuplicateDevicePath(DevicePathFromHandle(self.getSelfDeviceHandle()));
  if ( m_SelfDevicePath == NULL )  panic("m_SelfDevicePath == NULL");

  DBG("self.getSelfDevicePath()=%ls @%llX\n", FileDevicePathToXStringW(&self.getSelfDevicePath()).wc_str(), (uintptr_t)self.getSelfDeviceHandle());

  return EFI_SUCCESS;
}

