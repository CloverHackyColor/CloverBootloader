/*
 * Volumes.cpp
 *
 *  Created on: Feb 4, 2021
 *      Author: jief
 */

#include "Volumes.h"


REFIT_VOLUME     *SelfVolume = NULL;
VolumesArrayClass Volumes;

//REFIT_VOLUME* VolumesArrayClass::getApfsPartitionWithUUID(const XString8& ApfsContainerUUID, const XString8& APFSTargetUUID)
//{
//}


REFIT_VOLUME* VolumesArrayClass::getVolumeWithApfsContainerUUIDAndFileSystemUUID(const XString8& ApfsContainerUUID, const XString8& ApfsFileSystemUUID)
{
  for (size_t VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    REFIT_VOLUME* Volume = &Volumes[VolumeIndex];
//DBG("idx=%zu  name %ls  uuid=%s \n", VolumeIndex2, Volume2->VolName.wc_str(), Volume2->ApfsFileSystemUUID.c_str());
    if ( Volume->ApfsContainerUUID == ApfsContainerUUID ) {
      if ( Volume->ApfsFileSystemUUID == ApfsFileSystemUUID ) {
        return Volume;
      }
    }
  }
  return NULL;
}

REFIT_VOLUME* VolumesArrayClass::getVolumeWithApfsContainerUUIDAndRole(const XString8& ApfsContainerUUID, APPLE_APFS_VOLUME_ROLE roleMask)
{
  REFIT_VOLUME* targetVolume = NULL;
  for (size_t VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    REFIT_VOLUME* Volume = &Volumes[VolumeIndex];
//DBG("idx=%zu  name %ls  uuid=%s \n", VolumeIndex2, Volume2->VolName.wc_str(), Volume2->ApfsFileSystemUUID.c_str());
    if ( Volume->ApfsContainerUUID == ApfsContainerUUID ) {
      if ( (Volume->ApfsRole & roleMask) != 0 ) {
        if ( !targetVolume ) {
          targetVolume = Volume;
        }else{
          // More than one partition with this role in container.
          return NULL;
        }
      }
    }
  }
  return targetVolume;
}

