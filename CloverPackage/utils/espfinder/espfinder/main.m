//
//  main.m
//  espfinder
//
//  Created by vector sigma on 24/07/2019.
//  Copyright © 2019 Clover. All rights reserved.
//

/*
 DISK IDENTIFIER
 The (BSD) disk identifier string variously identifies a physical or logi-
 cal device unit, a session (if any) upon that device, a partition (slice)
 upon that session (if any), or a virtual logical volume.  It may take the
 form of diskU, diskUsS, diskUsQ, or diskCsV, where U, C, S, Q and V are
 positive decimal integers (possibly multi-digit), and where:
 
 o   U is the device unit.  It may refer to hardware (e.g. a hard
 drive, optical drive, or memory card) or a virtual "drive" con-
 structed by software (e.g. an AppleRAID Set, disk image,
 CoreStorage LV, etc).
 
 o   C is an APFS Container.  This is a a virtual disk constructed
 by APFS to represent a collection of APFS Volumes.
 
 o   Q is the session and is only included for optical media; it
 refers to the number of times recording has taken place on the
 currently-inserted medium (disc).
 
 o   S is the "slice"; it refers to a partition.  Upon this parti-
 tion, the raw data that underlies a user-visible file system is
 usually present, but it may also contain specialized data for
 certain 3rd-party database programs, or data required for the
 system software (e.g. EFI partitions, booter partitions, APM
 partition map data, etc), or, notably, it might contain back-
 ing-store physical volumes for AppleRAID, CoreStorage, APFS, or
 3rd-party Storage Systems.
 
 o   V is an APFS Volume; it refers to a virtual logical volume that
 is shared out of an APFS Container.  For example, if an
 Apple_APFS-typed partition is on disk5s2, then disk5s2 is
 termed the APFS Physical Store which is imported into an APFS
 Container. The APFS Container might then export e.g. disk8s1,
 which is termed an APFS Volume, which is mountable as a file
 system. Multiple APFS Volumes can be exported from a single
 APFS Container.
 
 Some units (e.g. floppy disks, RAID sets) contain file system data upon
 their "whole" device instead of containing a partitioning scheme with
 partitions.
 
 Note that some of the forms appear the same and must be distinguished by
 context.  For example, diskUsQ, diskUsS, and diskCsV are all 2-part forms
 that can mean different things: For non-optical media, it identifies a
 partition (on a partition map) upon which (file system) data is stored;
 for optical media, it identifies a session upon which an entire partition
 map (with its partitions with file systems) is stored; for an APFS setup,
 it identifies an APFS Volume. As another example, in "stacked" cases
 (CoreStorage on AppleRAID or APFS on AppleRAID), the 1-part diskU form
 becomes a CoreStorage PV or APFS PhysicalStore, in contrast with the
 more-common 2-part form.
 
 It is important for software to avoid relying on numerical ordering of
 any of the parts.  Activities including but not limited to partition
 deletions and insertions, partition resizing, virtual volume deletions
 and additions, device ejects and attachments due to media insertion
 cycles, plug cycles, authentication lock cycles or reboots, can all cause
 (temporary) gaps and non-increments in the numerical ordering of any of
 the parts. You must rely on more persistent means of identification, such
 as the various UUIDs.
 */

#import "Support.h"


int main (int argc, char **argv)/*main(int argc, const char * argv[])*/ {
  @autoreleasepool {
    if (argc < 3) {
      usage();
    }
    
    char *target = NULL;
    int c;
    
    opterr = 0;
    bool debug = false;
    
    while ((c = getopt(argc, argv, "dt:")) != -1) {
      switch (c) {
        case 'd':
          debug = true;
          break;
        case 't':
          target = optarg;
          break;
        case '?':
          if (optopt == 't') {
            printf("Option -%c requires an argument.\n", optopt); usage();
          } else if (isprint (optopt)) {
            printf("Unknown option `-%c'.\n", optopt); usage();
          } else {
            printf("Unknown option character `\\x%x'.\n", optopt); usage();
          }
        default:
          abort ();
      }
    }
    
    NSString *info = getBSDNameAndUUIDFrom(target);
    if (info == nil) {
      error([[NSString stringWithFormat:@"%s is not a disk object", argv[1]] UTF8String]);
    }
    
    NSString *disk = [[info componentsSeparatedByString:@" "] objectAtIndex:0];
    NSString *uuid = [[info componentsSeparatedByString:@" "] objectAtIndex:1];
    
    NSString *ESP = nil;
    NSString *parentDisk = [NSString stringWithFormat:@"dis%@",
                            [[disk componentsSeparatedByString:@"s"] objectAtIndex:1]];
    NSUInteger WholeDisksIndex = 10000;
    
    NSMutableDictionary *list = [NSMutableDictionary dictionaryWithDictionary:getAllDisks()];
    
    if (list) {
      if (debug) {
        [list setObject:disk forKey:@"Target"];
        [list setObject:kcmdver forKey:kcmdname];
        [list setObject:diskutil(@"list") forKey:@"diskutil list Terminal Output"];
      }
      if ([list objectForKey:@kWholeDisks] && [[list objectForKey:@kWholeDisks] isKindOfClass:[NSArray class]]) {
        NSArray *WholeDisks = [list objectForKey:@kWholeDisks];
        if (![WholeDisks containsObject:parentDisk]) {
          error([NSString stringWithFormat:@"cannot found %@ in WholeDisks", disk].UTF8String);
        }
        WholeDisksIndex = [WholeDisks indexOfObject:parentDisk];
      } else {
        error("cannot found WholeDisks array");
      }
      
      
      if ([list objectForKey:@kAllDisks] && [[list objectForKey:@kAllDisks] isKindOfClass:[NSArray class]]) {
        NSArray *AllDisks = [list objectForKey:@kAllDisks];
        if (![AllDisks containsObject:disk]) {
          error([NSString stringWithFormat:@"cannot found %@ in AllDisks", disk].UTF8String);
        }
        if ([list objectForKey:@kAllDisksAndPartitions] &&
            [[list objectForKey:@kAllDisksAndPartitions] isKindOfClass:[NSArray class]]) {
          NSArray *AllDisksAndPartitions = [list objectForKey:@kAllDisksAndPartitions];
          
          NSDictionary *item = [AllDisksAndPartitions objectAtIndex:WholeDisksIndex];
          
          if (![[item objectForKey:@kDeviceIdentifier] isEqualToString:parentDisk]) {
            error("Device Identifier is not equal to parent disk");
          }
          
          NSArray *Partitions = [item objectForKey:@kPartitions];
    
          // old Style?
          if (Partitions != nil) {
            for (int i = 0; i < [Partitions count]; i++) {
              NSDictionary *partition = [Partitions objectAtIndex:i];
              NSString *content = [partition objectForKey:@kContent];
              if ([content isEqualToString:@kEFIContent]) {
                ESP = [partition objectForKey:@kDeviceIdentifier];
                break;
              }
            }
          }
          
          // RAID mirror?
          if (ESP == nil) {
            NSDictionary *dinfo = getDiskInfo(parentDisk);
            if (dinfo != nil) {
              NSArray *RAIDSetMembers = [dinfo objectForKey:@kRAIDSetMembers];
              if (RAIDSetMembers != nil && [RAIDSetMembers count] > 0) {
                if (debug) {
                  [list setObject:dinfo forKey:@"RAID target info"];
                }
                //we are on RAID, keep first disk as our whole disk
                NSString *member0 = [RAIDSetMembers objectAtIndex:0];
                // member0 is probably an UUID oject..
                NSDictionary *memberInfo = getDiskInfo(member0);
                if (memberInfo != nil) {
                  NSString *memberDeviceIdentifier = [memberInfo objectForKey:@kDeviceIdentifier];
                  if (memberDeviceIdentifier != nil) {
                    NSString *memberParent = [NSString stringWithFormat:@"dis%@",
                                              [[memberDeviceIdentifier componentsSeparatedByString:@"s"] objectAtIndex:1]];
                    // looks for the ESP in AllDisksAndPartitions
                    for (int i = 0; i < [AllDisksAndPartitions count]; i++) {
                      NSDictionary *ADAPItem = [AllDisksAndPartitions objectAtIndex:i];
                      if ([[ADAPItem objectForKey:@kDeviceIdentifier] isEqualToString:memberParent]) {
                        NSArray *ADAPPartitions = [ADAPItem objectForKey:@kPartitions];
                        for (int pi = 0; pi < [ADAPPartitions count]; pi++) {
                          NSDictionary *ADAPPartitionsItem = [ADAPPartitions objectAtIndex:pi];
                          if ([[ADAPPartitionsItem objectForKey:@kContent] isEqualToString:@kEFIContent]) {
                            // ESP found!
                            ESP = [ADAPPartitionsItem objectForKey:@kDeviceIdentifier];
                            break;
                          }
                        }
                        break;
                      }
                    }
                    
                  }
                }
              }
            }
          }
          // APFS Fusion?
          if (ESP == nil) {
            NSArray *APFSPhysicalStores = [item objectForKey:@kAPFSPhysicalStores]; // apfs Fusion
            if (APFSPhysicalStores != nil && [APFSPhysicalStores count] > 0) {
              // Is a Fusion between apfs containers. Keep APFS Physical Store at index 0 as main disk object
              NSDictionary *aps = [APFSPhysicalStores objectAtIndex:0];
              NSString *DeviceIdentifier = [aps objectForKey:@kDeviceIdentifier];
              if (DeviceIdentifier != nil) {
                // We have a physical store :-), extract the parent!
                NSString *PhysicalStore = [NSString stringWithFormat:@"dis%@",
                                           [[DeviceIdentifier componentsSeparatedByString:@"s"] objectAtIndex:1]];
                
                // looks for the ESP in AllDisksAndPartitions
                for (int i = 0; i < [AllDisksAndPartitions count]; i++) {
                  NSDictionary *ADAPItem = [AllDisksAndPartitions objectAtIndex:i];
                  if ([[ADAPItem objectForKey:@kDeviceIdentifier] isEqualToString:PhysicalStore]) {
                    NSArray *ADAPPartitions = [ADAPItem objectForKey:@kPartitions];
                    for (int pi = 0; pi < [ADAPPartitions count]; pi++) {
                      NSDictionary *ADAPPartitionsItem = [ADAPPartitions objectAtIndex:pi];
                      if ([[ADAPPartitionsItem objectForKey:@kContent] isEqualToString:@kEFIContent]) {
                        // ESP found!
                        ESP = [ADAPPartitionsItem objectForKey:@kDeviceIdentifier];
                        break;
                      }
                    }
                    break;
                  }
                }
              }
            }
          }
          
          // just APFS?
          if (ESP == nil) {
            // ok, our disk is inside a contenitor, or isn't under GUID_partition_scheme.
            // apfs?
            NSDictionary *apfslist = getAllAPFSDisks();
            if (apfslist != nil) {
              if (debug) {
                [list setObject:apfslist forKey:@"APFS only"];
              }
              NSArray *Containers = [apfslist objectForKey:@kContainers];
              for (int i = 0; i < [Containers count]; i++) {
                NSDictionary *apItem = [Containers objectAtIndex:i];
                NSString *ContainerReference = [apItem objectForKey:@kContainerReference];
                if ([ContainerReference isEqualToString:parentDisk]) {
                  NSString *DesignatedPhysicalStore = [apItem objectForKey:@kDesignatedPhysicalStore];
                  if (DesignatedPhysicalStore != nil) {
                    NSDictionary *dinfo = getDiskInfo(DesignatedPhysicalStore);
                    NSArray *RAIDSetMembers = nil;
                    if (dinfo != nil) {
                      RAIDSetMembers = [dinfo objectForKey:@kRAIDSetMembers];
                    }
                    if (RAIDSetMembers != nil && [RAIDSetMembers count] > 0) {
                      if (debug) {
                        [list setObject:dinfo forKey:@"RAID target info"];
                      }
                      //we are on RAID, keep first disk as our whole disk
                      NSString *member0 = [RAIDSetMembers objectAtIndex:0];
                      // member0 is probably an UUID oject..
                      NSDictionary *memberInfo = getDiskInfo(member0);
                      if (memberInfo != nil) {
                        NSString *memberDeviceIdentifier = [memberInfo objectForKey:@kDeviceIdentifier];
                        if (memberDeviceIdentifier != nil) {
                          NSString *memberParent = [NSString stringWithFormat:@"dis%@",
                                                    [[memberDeviceIdentifier componentsSeparatedByString:@"s"] objectAtIndex:1]];
                          // looks for the ESP in AllDisksAndPartitions
                          for (int i = 0; i < [AllDisksAndPartitions count]; i++) {
                            NSDictionary *ADAPItem = [AllDisksAndPartitions objectAtIndex:i];
                            if ([[ADAPItem objectForKey:@kDeviceIdentifier] isEqualToString:memberParent]) {
                              NSArray *ADAPPartitions = [ADAPItem objectForKey:@kPartitions];
                              for (int pi = 0; pi < [ADAPPartitions count]; pi++) {
                                NSDictionary *ADAPPartitionsItem = [ADAPPartitions objectAtIndex:pi];
                                if ([[ADAPPartitionsItem objectForKey:@kContent] isEqualToString:@kEFIContent]) {
                                  // ESP found!
                                  ESP = [ADAPPartitionsItem objectForKey:@kDeviceIdentifier];
                                  break;
                                }
                              }
                              break;
                            }
                          }
                          
                        }
                      }
                    } else {
                      
                      // We have a physical store :-), extract the parent!
                      NSString *PhysicalStore = [NSString stringWithFormat:@"dis%@",
                                                 [[DesignatedPhysicalStore componentsSeparatedByString:@"s"] objectAtIndex:1]];
                      
                      // looks for the ESP in AllDisksAndPartitions
                      for (int idx = 0; idx < [AllDisksAndPartitions count]; idx++) {
                        NSDictionary *ADAPItem = [AllDisksAndPartitions objectAtIndex:idx];
                        if ([[ADAPItem objectForKey:@kDeviceIdentifier] isEqualToString:PhysicalStore]) {
                          NSArray *ADAPPartitions = [ADAPItem objectForKey:@kPartitions];
                          for (int pi = 0; pi < [ADAPPartitions count]; pi++) {
                            NSDictionary *ADAPPartitionsItem = [ADAPPartitions objectAtIndex:pi];
                            if ([[ADAPPartitionsItem objectForKey:@kContent] isEqualToString:@kEFIContent]) {
                              // ESP found!
                              ESP = [ADAPPartitionsItem objectForKey:@kDeviceIdentifier];
                              break;
                            }
                          }
                          break;
                        }
                      }
                    }
                  }
                  break;
                }
              }
            }
          }
          
          // CoreStorage?
          if (ESP == nil) {
            NSDictionary *cslist = getAllCSDisks();
            if (cslist != nil) {
              if (debug) {
                [list setObject:cslist forKey:@"CoreStorage only"];
              }
              
              NSArray *CoreStorageLogicalVolumeGroups = [cslist objectForKey:@kCoreStorageLogicalVolumeGroups];
              if (CoreStorageLogicalVolumeGroups != nil) {
                for (int i = 0; i < [CoreStorageLogicalVolumeGroups count]; i++) {
                  NSDictionary *CSLVG = [CoreStorageLogicalVolumeGroups objectAtIndex:i];
                  if (CSLVG == nil) { break; }
                  NSArray *CoreStorageLogicalVolumeFamilies = [CSLVG objectForKey:@kCoreStorageLogicalVolumeFamilies];
                  NSArray *CoreStoragePhysicalVolumes = [CSLVG objectForKey:@kCoreStoragePhysicalVolumes];
                  if (CoreStorageLogicalVolumeFamilies == nil) { break; }
                  if (CoreStoragePhysicalVolumes == nil) { break; }
                  for (int cslvfi = 0; cslvfi < [CoreStorageLogicalVolumeFamilies count]; cslvfi++) {
                    NSDictionary *cslvfitem = [CoreStorageLogicalVolumeFamilies objectAtIndex:cslvfi];
                    if (cslvfitem == nil) { break; }
                    NSArray *CoreStorageLogicalVolumes = [cslvfitem objectForKey:@kCoreStorageLogicalVolumes];
                    if (CoreStorageLogicalVolumes == nil) { break; }
                    
                    for (int cslvi = 0; cslvi < [CoreStorageLogicalVolumes count]; cslvi++) {
                      NSDictionary *cslvitem = [CoreStorageLogicalVolumes objectAtIndex:cslvi];
                      if (cslvitem == nil) { break; }
                      NSString *CoreStorageUUID = [cslvitem objectForKey:@kCoreStorageUUID];
                      if (CoreStorageUUID != nil && [CoreStorageUUID isEqualToString:uuid]) {
                        // We have a match.. our physical volume target is the first PV in CoreStoragePhysicalVolumes
                        if ([CoreStoragePhysicalVolumes count] > 0) {
                          NSDictionary *CSPVD = [CoreStoragePhysicalVolumes objectAtIndex:0];
                          NSString *CSPVDUUID = [CSPVD objectForKey:@kCoreStorageUUID];
                          if (CSPVDUUID == nil) { break; }
                          // ..we have the UUID of the physical volume! Now get the bsd name
                          NSDictionary *CSPVDUUIDinfo = getDiskInfo(CSPVDUUID);
                          if (CSPVDUUIDinfo == nil) { break; }
                          NSString *PVBSDName = [CSPVDUUIDinfo objectForKey:@kDeviceIdentifier];
                          if (PVBSDName != nil && [PVBSDName hasPrefix:@"disk"]) {
                            NSString *PVWholeDisk = [NSString stringWithFormat:@"dis%@",
                                                     [[PVBSDName componentsSeparatedByString:@"s"] objectAtIndex:1]];
                            
                            // looks for the ESP in AllDisksAndPartitions
                            for (int idx = 0; idx < [AllDisksAndPartitions count]; idx++) {
                              NSDictionary *ADAPItem = [AllDisksAndPartitions objectAtIndex:idx];
                              if ([[ADAPItem objectForKey:@kDeviceIdentifier] isEqualToString:PVWholeDisk]) {
                                NSArray *ADAPPartitions = [ADAPItem objectForKey:@kPartitions];
                                for (int pi = 0; pi < [ADAPPartitions count]; pi++) {
                                  NSDictionary *ADAPPartitionsItem = [ADAPPartitions objectAtIndex:pi];
                                  if ([[ADAPPartitionsItem objectForKey:@kContent] isEqualToString:@kEFIContent]) {
                                    // ESP found!
                                    ESP = [ADAPPartitionsItem objectForKey:@kDeviceIdentifier];
                                    break;
                                  }
                                }
                                break;
                              }
                            }
                            break;
                          }
                        }
                        break;
                      }
                    }
                  }
                  
                }
              }
            }
          }
          
        } else {
          error("cannot found AllDisksAndPartitions array");
        }
      } else {
        error("cannot found AllDisks array");
      }
      
    } else {
      error("cannot found disks informations");
    }
    
    NSString *result = (ESP == nil) ? @"ESP not found\n" : ESP;
    printf("%s\n", [result UTF8String]);
    if (debug) {
      [list setObject:result forKey:@"Result"];
      [list writeToFile:kdumpPath atomically:NO];
    }
    return (ESP == nil) ? 1 : 0;
  }
  return 0;
}

