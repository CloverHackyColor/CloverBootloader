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
//    REFIT_VOLUME* getApfsPartitionWithUUID(const XString8& ApfsContainerUUID, const XString8& APFSTargetUUID);
    REFIT_VOLUME* getVolumeWithApfsContainerUUIDAndFileSystemUUID(const XString8& ApfsContainerUUID, const XString8& ApfsFileSystemUUID);
    /*
     * Return : NULL if not found OR more than one partition with this role is found in this container
     */
    REFIT_VOLUME* getVolumeWithApfsContainerUUIDAndRole(const XString8& ApfsContainerUUID, APPLE_APFS_VOLUME_ROLE roleMask);

};

extern VolumesArrayClass Volumes;
extern REFIT_VOLUME     *SelfVolume;




#endif /* PLATFORM_VOLUMES_H_ */
