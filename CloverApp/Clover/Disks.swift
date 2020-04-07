//
//  Disks.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa
import IOKit
import IOKit.serial
import IOKit.kext
import CoreFoundation
import DiskArbitration
import SystemConfiguration

let kNotAvailable : String = "N/A"
let kBannedMedia = ["Recovery HD", "Recovery", "VM", "Preboot"]

/// Get DADisk dictionary from DiskArbitration from the given disk name or mount point.
func getDAdiskDescription(from diskOrMtp: String) -> NSDictionary? {
  var dict : NSDictionary? = nil
  if let session = DASessionCreate(kCFAllocatorDefault) {
    if diskOrMtp.hasPrefix("/") &&
      !diskOrMtp.hasPrefix("/dev/disk") &&
      FileManager.default.fileExists(atPath: diskOrMtp) {
      let volume : CFURL =  URL(fileURLWithPath: diskOrMtp) as CFURL
      if let disk = DADiskCreateFromVolumePath(kCFAllocatorDefault, session, volume) {
        dict = DADiskCopyDescription(disk)
      }
    } else if diskOrMtp.hasPrefix("/dev/disk") ||
      diskOrMtp.hasPrefix("disk") {
      var ndisk = diskOrMtp
      if ndisk.hasPrefix("/dev/disk") {
        ndisk = (ndisk as NSString).replacingOccurrences(of: "/dev/", with: "")
      }
      if let disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, ndisk) {
        dict = DADiskCopyDescription(disk)
      }
    }
  }

  return dict
}

/// Check disk or mount point is writable (kDADiskDescriptionMediaWritableKey).
func isWritable(diskOrMtp: String) -> Bool {
  var isWritable : Bool = false
  if let val = getDAdiskDescription(from: diskOrMtp)?.object(forKey: kDADiskDescriptionMediaWritableKey) as? NSNumber {
    isWritable = val.boolValue
  }
  
  return isWritable
}

/// Get the media content name (kDADiskDescriptionMediaContentKey).
func getMediaContent(from diskOrMtp: String) -> String? {
  if let content = getDAdiskDescription(from: diskOrMtp)?.object(forKey: kDADiskDescriptionMediaContentKey) as? String {
    return content
  }
  
  return nil
}

/// get the Partition Scheme Map from the parent disk (kDADiskDescriptionMediaContentKey).
func getPartitionSchemeMap(from diskOrMtp: String) -> String? {
  if let dadisk = getBSDParent(of: diskOrMtp) {
    if let scheme = getDAdiskDescription(from: dadisk)?.object(forKey: kDADiskDescriptionMediaContentKey) as? String {
      return scheme
    }
  }
  return nil
}

/// Find the mountpoint for the given disk object. You can pass also a mount point to se if it is valid.
func getMountPoint(from diskOrMtp: String) -> String? {
  var mountPoint : String? = nil
  if let dict : NSDictionary = getDAdiskDescription(from: diskOrMtp) {
    if (dict.object(forKey: kDADiskDescriptionVolumePathKey) != nil) {
      let temp : AnyObject = dict.object(forKey: kDADiskDescriptionVolumePathKey) as AnyObject
      if temp is NSURL {
        mountPoint = (dict.object(forKey: kDADiskDescriptionVolumePathKey) as? URL)?.path
      } else if temp is NSString {
        mountPoint = dict.object(forKey: kDADiskDescriptionVolumePathKey) as? String
      }
    }
  }
  
  return mountPoint
}

/// Find the Volume name: be aware that this is not the mount point name.
func getVolumeName(from diskOrMtp: String) -> String? {
  if let name = getDAdiskDescription(from: diskOrMtp)?.object(forKey: kDADiskDescriptionVolumeNameKey) as? String {
    return name
  }
  
  return nil
}

/// Get partition UUID for the given volume:  This is not a disk UUID.
func getVolumeUUID(from diskOrMtp: String) -> String? {
  var uuid : String? = nil
  if let dict : NSDictionary = getDAdiskDescription(from: diskOrMtp) {
    if (dict.object(forKey: kDADiskDescriptionVolumeUUIDKey) != nil) {
      
      let cfuuid :CFUUID = dict.object(forKey: kDADiskDescriptionVolumeUUIDKey) as! CFUUID
      uuid = CFUUIDCreateString(kCFAllocatorDefault, cfuuid)! as String
    }
  }
  return uuid
}

/// Get disk uuid for the given hole diskx. This is not a Volume UUID but a media UUID.
func getDiskUUID(from diskOrMtp: String) -> String? {
  var uuid : String? = nil
  if let dict : NSDictionary = getDAdiskDescription(from: getBSDParent(of: diskOrMtp)!) {
    if (dict.object(forKey: kDADiskDescriptionMediaUUIDKey) != nil) {
      let temp : AnyObject = dict.object(forKey: kDADiskDescriptionMediaUUIDKey) as AnyObject
      if temp is NSUUID {
        uuid = (dict.object(forKey: kDADiskDescriptionMediaUUIDKey) as? UUID)?.uuidString
      } else if temp is NSString {
        uuid = dict.object(forKey: kDADiskDescriptionMediaUUIDKey) as? String
      }
    }
  }
  return uuid
}

/// Get media uuid for the given hole diskx or slice.
func getMediaUUID(from diskOrMtp: String) -> String? {
  var uuid : String? = nil
  if let dict : NSDictionary = getDAdiskDescription(from: diskOrMtp) {
    if let temp  = dict.object(forKey: kDADiskDescriptionMediaUUIDKey) {
      let cu : CFUUID = temp as! CFUUID
      let cus : CFString = CFUUIDCreateString(kCFAllocatorDefault, cu)
      uuid = "\(cus)"
    }
  }
  return uuid
}

/// Get Media Name (kDADiskDescriptionMediaNameKey).
func getMediaName(from diskOrMtp: String) -> String? {
  if let name = getDAdiskDescription(from: diskOrMtp)?.object(forKey: kDADiskDescriptionMediaNameKey) as? String {
    return name
  }
  
  return nil
}

/// Get Media Name (kDADiskDescriptionDeviceProtocolKey).
func getDeviceProtocol(from diskOrMtp: String) -> String? {
  if let prot = getDAdiskDescription(from: diskOrMtp)?.object(forKey: kDADiskDescriptionDeviceProtocolKey) as? String {
    return prot
  }
  
  return nil
}

/// Get all BSDName in the System.
func getAlldisks() -> NSDictionary {
  let match_dictionary: CFMutableDictionary = IOServiceMatching("IOMedia")
  var entry_iterator: io_iterator_t = 0
  let allDisks = NSMutableDictionary()
  if IOServiceGetMatchingServices(kIOMasterPortDefault, match_dictionary, &entry_iterator) == kIOReturnSuccess {
    var serviceObject : io_registry_entry_t = 0
    
    repeat {
      serviceObject = IOIteratorNext(entry_iterator)
      if serviceObject != 0 {
        var serviceDictionary : Unmanaged<CFMutableDictionary>?
        if (IORegistryEntryCreateCFProperties(serviceObject,
                                              &serviceDictionary,
                                              kCFAllocatorDefault,
                                              0) != kIOReturnSuccess) {
          IOObjectRelease(serviceObject)
          continue
        }
        
        let d : NSDictionary = (serviceDictionary?.takeRetainedValue())!
        
        if (d.object(forKey: kIOBSDNameKey) != nil) {
          allDisks.setValue(d, forKey: (d.object(forKey: kIOBSDNameKey) as! String))
        }
      }
    } while serviceObject != 0
    IOObjectRelease(entry_iterator)
  }
  
  return allDisks
}

/// Get FileSystem for the given disk or mount point.
func getFS(from diskOrMtp: String) -> String? {
  var fs : String? = nil
  if let dict : NSDictionary = getDAdiskDescription(from: diskOrMtp) {
    var temp : String = ""
    
    if (dict.object(forKey: kDADiskDescriptionVolumeKindKey) != nil) {
      temp = dict.object(forKey: kDADiskDescriptionVolumeKindKey) as! String
      // if msdos we would know if is fat32, exfat etc..
      if temp.lowercased() == "msdos" {
        if #available(OSX 10.11, *) {
          /*
           Since last few OSes (..on 10.11) the DAVolumeType
           contains a string like ""MS-DOS (FAT32)" so that we can identify the real fs
           w/o using statfs (which require root privileges).
           
           BUT,
           
           kDADiskDescriptionVolumeTypeKey is not present before 10.11:
           dyld: Symbol not found: _kDADiskDescriptionVolumeTypeKey
           */
          let DAVolumeType : CFString = "DAVolumeType" as CFString
          if let volType = dict.object(forKey: DAVolumeType) as? String {
            if (volType.lowercased().range(of: "exfat") != nil) {
              temp = "exfat"
            } else if (volType.lowercased().range(of: "fat16") != nil) {
              temp = "fat16"
            } else if (volType.lowercased().range(of: "fat32") != nil) {
              temp = "fat32"
            }
          }
        } else {
          /*
           An old OS, the filesystem personality can only be read when the volume is up and mounted,
           but while the app will show "msdos" as filesystem, the installer will read the correct one just
           before going to install Clover because you can only install if the chosen volume has a mount point.
           */
          if let disk = getBSDName(of: diskOrMtp) {
            let cmd = "diskutil info \(disk) | grep -i 'file system personality:'"
            var output : String? = nil
            checkBashOutput(cmd: cmd, output: &output)
            
            if (output != nil) {
              if (output!.lowercased().range(of: "exfat") != nil) {
                temp = "exfat"
              } else if (output!.lowercased().range(of: "fat16") != nil) {
                temp = "fat16"
              } else if (output!.lowercased().range(of: "fat32") != nil) {
                temp = "fat32"
              }
            }
          }
        }
      }
      fs = temp
    }
  }
  return fs?.uppercased()
}

/// Get all ESP (EFI System Partition) in the System.
func getAllESPs() -> [String] {
  var  allEsps : [String] = [String]()
  for  disk in getAlldisks().allKeys {
    let mediaContent = getMediaContent(from: disk as! String) ?? ""
    if getMediaName(from: disk as! String) == "EFI System Partition" &&
      mediaContent == "C12A7328-F81F-11D2-BA4B-00A0C93EC93B"{
      if !allEsps.contains(disk as! String) {
        allEsps.append(disk as! String)
      }
    }
  }
  return allEsps
}

/// Get a list of ESP that have a mount point.
func getListOfMountedEsp() -> [String] {
  var  mounted : [String] = [String]()
  for bsdName in getAllESPs() {
    if isMountPoint(path: bsdName) {
      mounted.append(bsdName)
    }
  }

  return mounted
}

/// get and array of currently mounted volumes
func getVolumes() -> [String] {
  var  mounted : [String] = [String]()
  let all = getAlldisks().allKeys
  for b in all {
    let bsd : String = b as! String
    if let mp = getMountPoint(from: bsd) {
      if mp != "/private/var/vm" {
        mounted.append(mp)
      }
    }
  }
  return mounted
}

/// Find the BSDName of the given mount point.
func getBSDName(of mountpoint: String) -> String? {
  if let name = getDAdiskDescription(from: mountpoint)?.object(forKey: kDADiskDescriptionMediaBSDNameKey) as? String {
    return name
  }
  return nil
}

/// Find the BSDName of the given parent disk.
func getBSDParent(of mountpointORDevDisk: String) -> String? {
  if let dict : NSDictionary = getDAdiskDescription(from: mountpointORDevDisk) {
    if (dict.object(forKey: kDADiskDescriptionMediaBSDUnitKey) != nil) {
      return "disk" + ((dict.object(forKey: kDADiskDescriptionMediaBSDUnitKey) as? NSNumber)?.stringValue)!
    }
  }
  return nil
}

/// Return the partition slice number (as string) for the given disk or mount point.
func getPartitionSlice(of mountpointORDevDisk: String) -> String? {
  if let dict : NSDictionary = getDAdiskDescription(from: mountpointORDevDisk) {
    if (dict.object(forKey: kDADiskDescriptionMediaBSDNameKey) != nil) {
      let disk = (dict.object(forKey: kDADiskDescriptionMediaBSDNameKey) as? String)!
      if (disk.range(of: "s") != nil) {
        let arr : [String] = disk.components(separatedBy: "s")
        if arr.count == 3 { /* dis k0s 1 */
          return arr.last
        }
      }
    }
  }
  return nil
}

/// Return the image (NSImage) for the given mount point or disk object.
func getIconFor(volume mountpointORDevDisk: String) -> NSImage? {
  var image : NSImage? = nil
  // get a customized icon.. if any
  if isMountPoint(path: mountpointORDevDisk) {
    let mtp : String = getMountPoint(from: mountpointORDevDisk)!
    if FileManager.default.fileExists(atPath: mtp + "/.VolumeIcon.icns") {
      image = NSImage(byReferencingFile: mtp + "/.VolumeIcon.icns")
      return image
    }
  }
  // .. otherwise get a System icon
  if let daDict : NSDictionary = getDAdiskDescription(from: mountpointORDevDisk) {
    if let iconDict = daDict.object(forKey: kDADiskDescriptionMediaIconKey) as? NSDictionary,
      let iconName = iconDict.object(forKey: kIOBundleResourceFileKey ) as? String {
      let identifier =  iconDict.object(forKey: kCFBundleIdentifierKey as String) as! CFString
      
      let url : CFURL = Unmanaged.takeRetainedValue(KextManagerCreateURLForBundleIdentifier(kCFAllocatorDefault, identifier))()
      if let kb = Bundle(url: url as URL) {
        image = NSImage(byReferencingFile:
          kb.path(forResource: iconName.deletingFileExtension, ofType: iconName.fileExtension) ?? "")
      }
    }
  }
  return image
}

/// Boolean value indicanting if the given mount point or disk is internal.
func isInternalDevice(diskOrMtp: String) -> Bool {
  if let dict : NSDictionary = getDAdiskDescription(from: diskOrMtp) {
    if (dict.object(forKey: kDADiskDescriptionDeviceInternalKey) != nil) {
      return ((dict.object(forKey: kDADiskDescriptionDeviceInternalKey) as? NSNumber)?.boolValue)!
    }
  }
  return false
}

/// Boolean value indicanting if the given path is a valid mount point for a disk object.
func isMountPoint(path: String) -> Bool {
  let mtp : String? = getMountPoint(from: path)
  return (mtp != nil)
}

/// mount the given disk object. The path for the mount point is optional.
func mount(disk bsdName: String, at path: String?) {
  var disk : String = bsdName
  if disk.hasPrefix("disk") || disk.hasPrefix("/dev/disk") {
    if disk.hasPrefix("/dev/disk") {
      disk = disk.components(separatedBy: "dev/")[1]
    }
    
    var isLeaf : Bool = false
    let dict : NSDictionary? = getAlldisks().object(forKey: disk) as? NSDictionary
    isLeaf = (dict?.object(forKey: "Leaf") != nil) && (dict?.object(forKey: "Leaf") as! NSNumber).boolValue
    
    if disk.components(separatedBy: "s").count == 3 && isLeaf { /* di s k0 s 1 */
      let mountpoint : String? = getMountPoint(from: disk)
      if ((mountpoint != nil) && FileManager.default.fileExists(atPath: mountpoint!)) {
        // already mounted
        return
      }
      
      if let session = DASessionCreate(kCFAllocatorDefault) {
        if let bsd = DADiskCreateFromBSDName(kCFAllocatorDefault, session, disk) {
          var url : CFURL? = nil
          if (path != nil) {
            url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                          path?.toPointer(),
                                                          (path?.count)!,
                                                          true)
          }
          
          var context = UnsafeMutablePointer<Int>.allocate(capacity: 1)
          context.initialize(repeating: 0, count: 1)
          context.pointee = 0
          
          DASessionScheduleWithRunLoop(session,
                                       CFRunLoopGetCurrent(),
                                       CFRunLoopMode.defaultMode.rawValue)
          
          DADiskMountWithArguments(bsd,
                                   url,
                                   DADiskMountOptions(kDADiskMountOptionDefault), {
                                    (o, dis, ctx) in
                                    if (dis != nil) && (ctx != nil) {
                                      print("mount failure: " + printDAReturn(r: DADissenterGetStatus(dis!)))
                                    }
                                    CFRunLoopStop(CFRunLoopGetCurrent())
          }, &context, nil)
          
          CFRunLoopRun()
          
          DASessionUnscheduleFromRunLoop(session,
                                         CFRunLoopGetCurrent(),
                                         CFRunLoopMode.defaultMode.rawValue)
          context.deallocate()
        }
      }
    }
  }
}

/// mount the given disk object. The path for the mount point is optional. Code executed in a closure that return a boolean value.
func mount(disk bsdName: String,
           at path: String?,
           reply: @escaping (Bool) -> ()) {
  var disk : String = bsdName
  if disk.hasPrefix("disk") || disk.hasPrefix("/dev/disk") {
    if disk.hasPrefix("/dev/disk") {
      disk = disk.components(separatedBy: "dev/")[1]
    }
    
    var isLeaf : Bool = false
    let dict : NSDictionary? = getAlldisks().object(forKey: disk) as? NSDictionary
    isLeaf = (dict?.object(forKey: "Leaf") != nil) && (dict?.object(forKey: "Leaf") as! NSNumber).boolValue
    
    
    if disk.components(separatedBy: "s").count == 3 && isLeaf { /* di s k0 s 1 (is a partition?)*/
      let mountpoint : String? = getMountPoint(from: disk)
      if ((mountpoint != nil) && FileManager.default.fileExists(atPath: mountpoint!)) {
        // already mounted
        reply(true)
        return
      }
      
      if let session = DASessionCreate(kCFAllocatorDefault) {
        if let bsd = DADiskCreateFromBSDName(kCFAllocatorDefault, session, disk) {
          var url : CFURL? = nil
          if (path != nil) {
            url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                          path?.toPointer(),
                                                          (path?.count)!,
                                                          true)
          }
          var context = UnsafeMutablePointer<Int>.allocate(capacity: 1)
          context.initialize(repeating: 0, count: 1)
          context.pointee = 0
          
          DASessionScheduleWithRunLoop(session,
                                       CFRunLoopGetCurrent(),
                                       CFRunLoopMode.defaultMode.rawValue)
          
          DADiskMountWithArguments(bsd, url, DADiskMountOptions(kDADiskMountOptionDefault), {
            (o, dis, ctx) in
            if (dis != nil) && (ctx != nil) {
              print("mount failure: " + printDAReturn(r: DADissenterGetStatus(dis!)))
            }
            CFRunLoopStop(CFRunLoopGetCurrent())
          }, &context, nil)
          
          let result : Bool = (context.pointee == 0)
          
          CFRunLoopRun()
          
          DASessionUnscheduleFromRunLoop(session,
                                         CFRunLoopGetCurrent(),
                                         CFRunLoopMode.defaultMode.rawValue)
          context.deallocate()
          
          reply(result)
          return
        }
      }
      
    }
  }
  reply(false)
}

/// unmount the given disk object or mount point. force used to kill any pid is using the disk.
func umount(disk diskOrMtp: String, force: Bool) {
  let disk : String = diskOrMtp
  let mtp : String? = getMountPoint(from: diskOrMtp)
  if mtp != nil {
    return
  }

  if disk.hasPrefix("disk") {
    var isLeaf : Bool = false
    let dict : NSDictionary? = getAlldisks().object(forKey: disk) as? NSDictionary
    isLeaf = (dict?.object(forKey: "Leaf") != nil) && (dict?.object(forKey: "Leaf") as! NSNumber).boolValue
    
    if disk.components(separatedBy: "s").count == 3 && isLeaf { /* di s k0 s 1 */
      let mountpoint : String? = getMountPoint(from: disk)
      if (mountpoint == nil) {
        return
      }
      
      if let session = DASessionCreate(kCFAllocatorDefault) {
        if let bsd = DADiskCreateFromBSDName(kCFAllocatorDefault, session, disk) {
          var context = UnsafeMutablePointer<Int>.allocate(capacity: 1)
          context.initialize(repeating: 0, count: 1)
          context.pointee = 0
          
          DASessionScheduleWithRunLoop(session,
                                       CFRunLoopGetCurrent(),
                                       CFRunLoopMode.defaultMode.rawValue)
          
          DADiskUnmount(bsd,
                        DADiskUnmountOptions(force ? kDADiskUnmountOptionForce : kDADiskUnmountOptionDefault),
                        { (dadisk, dissenter, ctx) in
                          if (dissenter != nil) && (ctx != nil) {
                            print("un mount failure: " + printDAReturn(r: DADissenterGetStatus(dissenter!)))
                          }
                          CFRunLoopStop(CFRunLoopGetCurrent())
          }, &context)
          
          CFRunLoopRun()
          DASessionUnscheduleFromRunLoop(session,
                                         CFRunLoopGetCurrent(),
                                         CFRunLoopMode.defaultMode.rawValue)
          
          
          context.deallocate()
        }
      }
    }
  }
}

/// unmount the given disk object or mount point. force used to kill any pid is using the disk. Code executed in a closure that return a boolean value.
func umount(disk diskOrMtp: String,
            force: Bool,
            reply: @escaping (Bool) -> ()) {
  let disk : String = diskOrMtp
  let mtp : String? = getMountPoint(from: diskOrMtp)
  if (mtp == nil) || (mtp == "/private/var/vm" || mtp == "/") {
    reply(false)
    return
  }
  if disk.hasPrefix("disk") {
    var isLeaf : Bool = false
    let dict : NSDictionary? = getAlldisks().object(forKey: disk) as? NSDictionary
    isLeaf = (dict?.object(forKey: "Leaf") != nil) && (dict?.object(forKey: "Leaf") as! NSNumber).boolValue
    
    if disk.components(separatedBy: "s").count == 3 && isLeaf { /* di s k0 s 1 */
      let mountpoint : String? = getMountPoint(from: disk)
      if (mountpoint == nil) {
        reply(true)
        return
      }
      
      if let session = DASessionCreate(kCFAllocatorDefault) {
        if let bsd = DADiskCreateFromBSDName(kCFAllocatorDefault, session, disk) {
          var context = UnsafeMutablePointer<Int>.allocate(capacity: 1)
          context.initialize(repeating: 0, count: 1)
          context.pointee = 0
          
          DASessionScheduleWithRunLoop(session,
                                       CFRunLoopGetCurrent(),
                                       CFRunLoopMode.defaultMode.rawValue)
          
          DADiskUnmount(bsd,
                        DADiskUnmountOptions(force ? kDADiskUnmountOptionForce : kDADiskUnmountOptionDefault),
                        { (dadisk, dissenter, ctx) in
                          if (dissenter != nil) && (ctx != nil) {
                            print("un mount failure: " + printDAReturn(r: DADissenterGetStatus(dissenter!)))
                          }
                          CFRunLoopStop(CFRunLoopGetCurrent())
          }, &context)
          
          let result : Bool = (context.pointee == 0)
          CFRunLoopRun()
          
          DASessionUnscheduleFromRunLoop(session,
                                         CFRunLoopGetCurrent(),
                                         CFRunLoopMode.defaultMode.rawValue)
          
          
          context.deallocate()
          reply(result)
        }
      }
    }
  }
  reply(false)
}

/// Helper function for the mount/umout call back
fileprivate func printDAReturn(r: DAReturn) -> String {
  switch Int(r) {
  case kDAReturnError:
    return "Error"
  case kDAReturnBusy:
    return "Busy"
  case kDAReturnBadArgument:
    return "Bad Argument"
  case kDAReturnExclusiveAccess:
    return "Exclusive Access"
  case kDAReturnNoResources:
    return "No Resources"
  case kDAReturnNotFound:
    return "Not Found"
  case kDAReturnNotMounted:
    return "Not Mounted"
  case kDAReturnNotPermitted:
    return "Not Permitted"
  case kDAReturnNotPrivileged:
    return "Not Privileged"
  case kDAReturnNotReady:
    return "Not Ready"
  case kDAReturnNotWritable:
    return "Not Writable"
  case kDAReturnUnsupported:
    return "Unsupported"
  default:
    return "Unknown"
  }
}

fileprivate func checkBashOutput(cmd: String, output: inout String?) {
  let task : Process = Process()
  if #available(OSX 10.13, *) {
    task.executableURL = URL(fileURLWithPath: "/bin/bash")
  } else {
    task.launchPath = "/bin/bash"
  }
  task.arguments = ["-c", cmd]
  
  task.environment = ProcessInfo.init().environment
  
  let pipe: Pipe = Pipe()
  task.standardOutput = pipe
  task.standardError = pipe
  task.launch()
  task.waitUntilExit()
  let handle = pipe.fileHandleForReading
  let data = handle.readDataToEndOfFile()
  
  
  output = String(data: data, encoding: .utf8)
}
