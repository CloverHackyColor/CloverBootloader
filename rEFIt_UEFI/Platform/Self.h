/*
 * Self.h
 *
 *  Created on: Sep 28, 2020
 *      Author: jief
 */

#ifndef PLATFORM_SELF_H_
#define PLATFORM_SELF_H_

#include <Platform.h>

class Self
{
protected:
  EFI_HANDLE       m_SelfImageHandle;
  EFI_LOADED_IMAGE *m_SelfLoadedImage;
  EFI_DEVICE_PATH  *m_SelfDevicePath;

public:
  Self () : m_SelfImageHandle(NULL), m_SelfLoadedImage{0}, m_SelfDevicePath(NULL) {};
  Self(const Self&) = delete;
  Self& operator = (const Self&) = delete;

  ~Self () {};

  EFI_STATUS initialize(EFI_HANDLE ImageHandle);

  EFI_HANDLE getSelfImageHandle() { return m_SelfImageHandle; }
  const EFI_LOADED_IMAGE& getSelfLoadedImage() { return *m_SelfLoadedImage; }
  EFI_HANDLE getSelfDeviceHandle() { return getSelfLoadedImage().DeviceHandle; }
  const EFI_DEVICE_PATH& getSelfDevicePath() { return *m_SelfDevicePath; }

};

extern Self self;

#endif /* PLATFORM_SELF_H_ */
