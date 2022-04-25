/*
 * Volumes.h
 *
 *  Created on: Feb 4, 2021
 *      Author: jief
 */

#ifndef PLATFORM_VOLUMES_H_
#define PLATFORM_VOLUMES_H_

#include "Volume.h"

extern "C" {
//#include <Library/OcConfigurationLib.h>
  #include <Guid/AppleApfsInfo.h>
}




class VolumesArrayClass : public XObjArray<REFIT_VOLUME>
{
  public:
    REFIT_VOLUME* getVolumeWithApfsContainerUUIDAndFileSystemUUID(const EFI_GUID& ApfsContainerUUID, const EFI_GUID& ApfsFileSystemUUID);
    /*
     * Return : NULL if not found OR more than one partition with this role is found in this container
     */
    REFIT_VOLUME* getVolumeWithApfsContainerUUIDAndRole(const EFI_GUID& ApfsContainerUUID, APPLE_APFS_VOLUME_ROLE roleMask);

};

extern VolumesArrayClass Volumes;
extern REFIT_VOLUME     *SelfVolume;




#endif /* PLATFORM_VOLUMES_H_ */
