//
//  BootInstaller.swift
//  CloverDaemonNew
//
//  Created by vector sigma on 03/01/2020.
//  Copyright © 2020 CloverHackyColor. All rights reserved.
//

import Foundation


final class Installer: NSObject {
  let ktempLogPath = "/tmp/cltmplog"
  let kDiskUtilListPath = "/tmp/diskutil.List"
  private var realTime : Bool = true
  private var gTargetVolume : String? = nil
  
  public var realTimeOutPut:Bool {
    get {
      return self.realTime
    }
    set {
      self.realTime = newValue
    }
  }
  
  private func cleanUp() {
    let t = Process()
    t.environment = ProcessInfo().environment
    if #available(OSX 10.13, *) {
      t.executableURL = URL(fileURLWithPath: "/bin/bash")
    } else {
      t.launchPath = "/bin/bash"
    }
    t.arguments = ["-c", "rm -rf /tmp/Clover* && rm -f /tmp/boot0* && rm -f /tmp/boot1*"]
    t.launch()
    t.waitUntilExit()
  }
  
  private func saveLog() {
    if (self.gTargetVolume != nil)
      && fm.fileExists(atPath: self.gTargetVolume!)
      && fm.fileExists(atPath: ktempLogPath) {
      let finalLogPath = self.gTargetVolume!.addPath("EFI/CLOVER/Clover.app_install.log")
      do {
        let data = try Data(contentsOf: URL(fileURLWithPath: ktempLogPath))
        (data as NSData).write(toFile: finalLogPath, atomically: false)
      } catch {
        print(error)
      }
    }
  }
  
  private func addToLog(_ str: String) {
    let output : String = str.hasSuffix("\n") ? str : "\n\(str)"
    if let data = output.data(using: .utf8) {
      if fm.fileExists(atPath: ktempLogPath) {
        if let fh = try? FileHandle(forWritingTo: URL(fileURLWithPath: ktempLogPath)) {
          fh.seekToEndOfFile()
          fh.write(data)
          fh.closeFile()
        }
      } else {
        (data as NSData).write(toFile: ktempLogPath, atomically: false)
      }
    }
  }
  
  private func log(_ msg: String) {
    self.addToLog(msg)
    print(msg)
  }

  private func exit(_ msg: String) {
    print(msg)
    self.addToLog(msg)
    self.saveLog()
    self.cleanUp()
    Darwin.exit(EXIT_FAILURE)
  }
  
  func disableInsexing(for volume: String) {
    if fm.fileExists(atPath: volume) {
      var file = volume.addPath(".metadata_never_index")
      if !fm.fileExists(atPath: file) {
        try? "".write(toFile: file, atomically: false, encoding: .utf8)
      }
      
      file = volume.addPath(".Spotlight-V100")
      if fm.fileExists(atPath: file) {
        try? fm.removeItem(atPath: file)
      }
      
      /*
       Clean everythings that starts with "._"
       This of course only for ESPs
       */
      
      let enumerator = fm.enumerator(atPath: volume)
      while let file = enumerator?.nextObject() as? String {
        let fullPath = volume.addPath(file)
        if file.hasPrefix("._") {
          try? fm.removeItem(atPath: fullPath)
        }
      }
    }
  }
  
  private func copyReplace(src: String, dst: String, attr: [FileAttributeKey: Any]?, log: Bool) -> Bool {
    var attributes : [FileAttributeKey: Any]? = attr
    let upperDir = dst.deletingLastPath
    if log {
      if upperDir == "/" {
        self.log("+ \(dst)")
      } else {
        self.log("+ ../\(upperDir.lastPath)/\(dst.lastPath)")
      }
    }
    
    if !fm.fileExists(atPath: src) {
      self.log("Error: \(src) doesn't exist.")
      return false
    }
    
    // if attr is nil take attributes from source
    if attr == nil {
      do {
        attributes = try fm.attributesOfItem(atPath: src)
      } catch {
        print("Warning: can't get attributes from '\(src)'")
      }
    }
    
    // remove destination if already exist
    if fm.fileExists(atPath: dst) {
      do {
        try fm.removeItem(atPath: dst)
      } catch {
        print(error)
        return false
      }
    }
    
    // create upper directory if needed
    if !fm.fileExists(atPath: upperDir) {
      do {
        try fm.createDirectory(atPath: upperDir,
                               withIntermediateDirectories: true,
                               attributes: attributes)
      } catch {
        print(error)
        return false
      }
    }

    // copy file to destination
    do {
      try fm.copyItem(atPath: src, toPath: dst)
    } catch {
      print(error)
      return false
    }
    return true
  }
  
  private func createDirectory(at path: String, attr: [FileAttributeKey: Any]?, exitOnError: Bool) {
    var isDir : ObjCBool = false
    if fm.fileExists(atPath: path, isDirectory: &isDir) {
      if isDir.boolValue {
        return
      } else {
        do {
          try fm.removeItem(atPath: path)
        } catch {
          exit("\(error)")
        }
      }
    }
    do {
      try fm.createDirectory(atPath: path,
                             withIntermediateDirectories: true,
                             attributes: attr)
    } catch {
      exit("\(error)")
    }
  }
  
  // MARK: Install
  func install() {
    let df = DateFormatter()
    df.locale = Locale(identifier: "en_US")
    df.dateFormat = "yyyy-MM-dd hh:mm:ss"
    let now = df.string(from: Date())
    
    if fm.fileExists(atPath: ktempLogPath) {
      try? fm.removeItem(atPath: ktempLogPath)
    }
    
    guard let CloverappDict = NSDictionary(contentsOfFile: "/tmp/Cloverapp") as?  [String:AnyObject] else {
      exit("Error: can't load Cloverapp dictionary.")
      return // make the compiler happy
    }
    cleanUp()
    
    let bootSectorsInstall = "/tmp/bootsectors-install"
    
    let targetVol             = CloverappDict["targetVol"] as! String
    let disk                  = CloverappDict["disk"] as! String
    let filesystem            = CloverappDict["filesystem"] as! String
    let shemeMap              = CloverappDict["shemeMap"] as! String
    let boot0                 = CloverappDict["boot0"] as? String
    let boot1                 = CloverappDict["boot1"] as? String
    let boot2                 = CloverappDict["boot2"] as? String
    let CloverV2              = CloverappDict["CloverV2"] as! String
    let boot1installPath      = CloverappDict["boot1install"] as? String
    let bootSectorsInstallSrc = CloverappDict["bootsectors-install"] as? String
    let backUpPath            = CloverappDict["BackUpPath"] as? String
    let version               = CloverappDict["version"] as! String
    let isESP                 = (CloverappDict["isESP"] as! NSNumber).boolValue
    
    let alt : Bool            = (CloverappDict["alt"] as? NSNumber)?.boolValue ?? false

    log("\(version) (installer library v\(daemonVersion)), \(now)")
    log("macOS \(ProcessInfo().operatingSystemVersionString)")
    log("SELF = \(CommandLine.arguments[0])")
    if geteuid() != 0 {
      exit("Error: you don't have root permissions.")
    }
    
    if (backUpPath != nil) {
      log("Backup made at:\n\(backUpPath!).")
    }
  
    if fm.fileExists(atPath: bootSectorsInstall) {
      do {
        try fm.removeItem(atPath: bootSectorsInstall)
      } catch {
        exit("Error: can't remove old bootsectors-install.")
      }
    }

    if (bootSectorsInstallSrc != nil) {
      log("bootSectorsInstallSrc = \(bootSectorsInstallSrc!)")
    }
    
    // MARK: Preferences dict init
    let preferences = NSMutableDictionary()
    
    // MARK: Attributes init
    let attributes : [FileAttributeKey: Any] = [FileAttributeKey.posixPermissions: 0777]
    
    // MARK: Check Volume

    self.gTargetVolume = targetVol
    if !fm.fileExists(atPath: targetVol) {
      exit("Error: target volume \"\(targetVol)\" doesn't exist.")
    }
    
    if !fm.fileExists(atPath: CloverV2) {
      exit("Error: cannot found CloverV2 directory.")
    }
    
    log("Target volume: \(targetVol)")
    // get additional info about the target volume
    log("BSD Name:      \(getBSDName(of: targetVol) ?? kNotAvailable)")
    log("Filesystem:    \(getFS(from: targetVol) ?? kNotAvailable)")
    log("Is internal:   \(isInternalDevice(diskOrMtp: targetVol))")
    log("Is writable:   \(isWritable(diskOrMtp: targetVol))")
    
    if !fm.isWritableFile(atPath: targetVol) {
      self.log("o_Ops: '\(targetVol)' appear to be not writable, remounting read/write.")
      /*
       https://github.com/CloverHackyColor/CloverBootloader/issues/50
       if we are here is because DiskArbitration tell Us this disk is writable,
       but now FileManager just say the opposite.
       This can be the result of mount????
       
       So try to umount the disk and re mount it read write!
       */
      let task = Process()
      task.environment = ProcessInfo().environment
      task.launchPath = "/sbin/mount"
      task.arguments = ["-uw", targetVol]
      task.launch()
      task.waitUntilExit()
      
      self.log("sleeping 6 seconds..")
      sleep(6)
      if !fm.isWritableFile(atPath: targetVol) {
        exit("Error: target volume \"\(targetVol)\" is not writable (may be running from a Virtual Machine??).")
      } else {
        self.log("'\(targetVol)' is now read/write.")
      }
    }
    
    if isESP {
      self.disableInsexing(for: targetVol)
    }
    
    // MARK: Check paths
    var boot0Path: String? = nil
    var boot1Path: String? = nil
    var boot2Path: String? = nil
    if (boot0 != nil) {
      log("boot0: \(boot0!)")
      preferences.setValue(boot0!, forKey: "boot0")
      boot0Path = CloverV2.addPath("BootSectors").addPath(boot0!)
      if !fm.fileExists(atPath: boot0Path!) {
        exit("Error: cannot found \"\(boot0!)\".")
      }
    }
    
    if (boot1 != nil) {
      log("boot1: \(boot1!)")
      boot1Path = CloverV2.addPath("BootSectors").addPath(boot1!)
      if !fm.fileExists(atPath: boot1Path!) {
        exit("Error: cannot found \"\(boot1!)\".")
      }
    }
    
    if (boot2 != nil) {
      log("Installation type: BIOS")
      log("boot2: \(boot2!)")
      preferences.setValue(boot2!, forKey: "boot2")
      boot2Path = CloverV2.addPath("Bootloaders/x64").addPath(boot2!)
      if !fm.fileExists(atPath: boot2Path!) {
        exit("Error: cannot found \"\(boot2!)\".")
      }
    } else {
      log("Installation type: UEFI")
    }
    
    if let dlist = try? String(contentsOfFile: kDiskUtilListPath) {
      log("\n\n\(dlist)\n")
    }
    
    if fm.fileExists(atPath: kDiskUtilListPath) {
      try? fm.removeItem(atPath: kDiskUtilListPath)
    }
   
    // MARK: Create Directories
    createDirectory(at: targetVol.addPath("EFI/CLOVER"), attr: attributes, exitOnError: true)
    createDirectory(at: targetVol.addPath("EFI/BOOT"), attr: attributes, exitOnError: true)
    createDirectory(at: targetVol.addPath("EFI/CLOVER/misc"), attr: attributes, exitOnError: true)
    
    let subDirs = ["ACPI/origin",
                   "ACPI/patched",
                   "ACPI/WINDOWS",
                   "kexts/10",
                   "kexts/10_recovery",
                   "kexts/10_installer",
                   "kexts/10.11",
                   "kexts/10.12",
                   "kexts/10.13",
                   "kexts/10.14",
                   "kexts/10.15",
                   "kexts/Other",
                   "ROM"]
    
    for dir in subDirs {
      createDirectory(at: targetVol.addPath("EFI/CLOVER").addPath(dir),
                      attr: attributes,
                      exitOnError: true)
      
      
    }
    
    for dir in subDirs {
      if !fm.fileExists(atPath: targetVol.addPath("EFI/CLOVER/OEM")) {
        createDirectory(at: targetVol.addPath("EFI/CLOVER/OEM/SystemProductName").addPath(dir),
                        attr: attributes,
                        exitOnError: true)
        
        if dir != "ROM" {
          createDirectory(at: targetVol.addPath("EFI/CLOVER/OEM/SystemProductName/UEFI").addPath(dir),
                          attr: attributes,
                          exitOnError: true)
        }
      }
    }
    
    // MARK: Install Clover and drivers
    self.log("\nInstalling/Updating Clover:")
    if !copyReplace(src: CloverV2.addPath("EFI/BOOT/BOOTX64.efi"),
                    dst: targetVol.addPath("EFI/BOOT/BOOTX64.efi"),
                    attr: attributes,
                    log: true) {
      exit("Error: cannot copy BOOTX64.efi to destination.")
    }
    
    if !copyReplace(src: CloverV2.addPath("EFI/CLOVER/CLOVERX64.efi"),
                    dst: targetVol.addPath("EFI/CLOVER/CLOVERX64.efi"),
                    attr: attributes,
                    log: true) {
      exit("Error: cannot copy CLOVERX64.efi to destination.")
    }
    
    self.log("\nInstalling/Updating drivers:")
    if let toDelete = CloverappDict["toDelete"] as? [String] {
      for dpath in toDelete {
        if fm.fileExists(atPath: dpath) {
          self.log("- ../\(dpath.deletingLastPath.lastPath)/\(dpath.lastPath)")
          do {
            try fm.removeItem(atPath: dpath)
          } catch {
            exit("\(error)")
          }
        }
      }
    }
    
    let UEFIdest = targetVol.addPath("EFI/CLOVER/drivers/UEFI")
    let BIOSdest = targetVol.addPath("EFI/CLOVER/drivers/BIOS")
  
    if let UEFI = CloverappDict["UEFI"] as? [String] {
      for dpath in UEFI {
        if !copyReplace(src: dpath,
                        dst: UEFIdest.addPath(dpath.lastPath),
                        attr: attributes,
                        log: true) {
          exit("Error: cannot copy '\(dpath)' to destination.")
        }
      }
    }
    
    if let BIOS = CloverappDict["BIOS"] as? [String] {
      for dpath in BIOS {
        if !copyReplace(src: dpath,
                        dst: BIOSdest.addPath(dpath.lastPath),
                        attr: attributes,
                        log: true) {
          exit("Error: cannot copy '\(dpath)' to destination.")
        }
      }
    }
    
    // MARK: Install tools
    let cv2tools = CloverV2.addPath("EFI/CLOVER/tools")
    if fm.fileExists(atPath: cv2tools) {
      self.log("\nInstalling/Updating tools:")
      var tools : [String] = [String]()
      do {
         tools = try fm.contentsOfDirectory(atPath: cv2tools)
      } catch { }
      
      for t in tools {
        if t.fileExtension == "efi" {
          if !copyReplace(src: cv2tools.addPath(t),
                          dst: targetVol.addPath("EFI/CLOVER/tools").addPath(t),
                          attr: attributes,
                          log: true) {
            exit("Error: cannot copy '\(cv2tools.addPath(t))' to destination.")
          }
        }
      }
    }
    
    // MARK: Install docs
    let cv2docs = CloverV2.addPath("EFI/CLOVER/doc")
    if fm.fileExists(atPath: cv2docs) {
      self.log("\nInstalling/Updating docs:")
      var docs : [String] = [String]()
      do {
        docs = try fm.contentsOfDirectory(atPath: cv2docs)
      } catch { }
      
      for d in docs {
        if !d.hasPrefix(".") {
          var a : [FileAttributeKey: Any]? = nil
          do {
            a = try fm.attributesOfItem(atPath: cv2docs.addPath(d))
          } catch { }
          if !copyReplace(src: cv2docs.addPath(d),
                          dst: targetVol.addPath("EFI/CLOVER/doc").addPath(d),
                          attr: a,
                          log: true) {
            // do not fail the installation fro a document. So just log:
            self.log("Error: cannot copy '\(cv2tools.addPath(d))' to destination.")
          }
        }
      }
    }
    
    // MARK: Create config.plist
    let configPath = targetVol.addPath("EFI/CLOVER/config.plist")
    let configSamplePath = CloverV2.addPath("EFI/CLOVER/config-sample.plist")
    if !fm.fileExists(atPath: configPath) {
      if !copyReplace(src: configSamplePath,
                      dst: configPath,
                      attr: attributes,
                      log: true) {
        self.log("Error: cannot copy '\(configSamplePath)' to destination.")
      }
    }

    // MARK: Install theme
    // install the theme defined in config.plist
    // Also do nothing if the theme directory already exist
    // as Clover is already installed and user as its own free will,
    // even to run w/o a theme
    if let config = NSDictionary(contentsOfFile: configPath) {
      let themesSourceDir = CloverV2.addPath("themespkg")
      let themesDestDir   = targetVol.addPath("EFI/CLOVER/themes")
      if !fm.fileExists(atPath: themesDestDir) {
        if let GUI = config.object(forKey: "GUI") as? NSDictionary {
          if let Theme = GUI.object(forKey: "Theme") as? String {
            if fm.fileExists(atPath: themesSourceDir.addPath(Theme)) {
              self.log("\nInstalling Theme \"\(Theme)\":")
              if !copyReplace(src: themesSourceDir.addPath(Theme),
                              dst: themesDestDir.addPath(Theme),
                              attr: attributes,
                              log: true) {
                // do not fail for a theme, just log:
                self.log("Error: cannot copy '\(themesSourceDir.addPath(Theme))' to destination.")
              }
            } else {
              self.log("Warning: cannot found Theme '\(Theme)' defined in config.")
            }
          }
        }
      }
    } else {
      self.log("Warning: cannot read '\(configPath)' into a valid Dictionary.")
    }

    // MARK: Stage 2 Installation
    if boot2Path != nil {
      self.log("\nInstalling stage 2..")
      if !copyReplace(src: boot2Path!,
                      dst: targetVol.addPath("boot"),
                      attr: attributes,
                      log: true) {
        self.log("Error: cannot copy '\(boot2Path!)' to destination.")
      }
      
      if alt {
        preferences.setValue(true, forKey: "boot2Alt")
        let bootX64path = CloverV2.addPath("Bootloaders/x64")
        var loaders : [String] = [String]()
        do {
          loaders = try fm.contentsOfDirectory(atPath: bootX64path)
        } catch { }

        for boot in loaders {
          if boot.hasPrefix("boot") {
            self.log("\nInstalling stage 2 \"\(boot) (alt)\".")
            if !copyReplace(src: bootX64path.addPath(boot),
                            dst: targetVol.addPath(boot),
                            attr: attributes,
                            log: true) {
              exit("Error: cannot copy '\(bootX64path.addPath(boot))' to destination.")
            }
          }
        }
      }
      // ------------- end stage 2
    }
    
    // MARK: Write Preferences
    preferences.write(toFile: targetVol.addPath("EFI/CLOVER/pref.plist"), atomically: true)
    try? fm.setAttributes(attributes, ofItemAtPath: targetVol.addPath("EFI/CLOVER/pref.plist"))
    saveLog()
    
    // MARK: Boot sectors installation
    if (boot0Path != nil && boot1Path != nil && bootSectorsInstallSrc != nil) {
      if !copyReplace(src: bootSectorsInstallSrc!,
                      dst: bootSectorsInstall,
                      attr: nil,
                      log: false) {
        exit("Error: cannot copy '\(bootSectorsInstallSrc!)' to destination.")
      }
      
      
      if !copyReplace(src: boot0Path!,
                      dst: "/tmp".addPath(boot0!),
                      attr: nil,
                      log: false) {
        exit("Error: cannot copy '\(boot0Path!)' to destination.")
      }
      
      if !copyReplace(src: boot1Path!,
                      dst: "/tmp".addPath(boot1!),
                      attr: nil,
                      log: false) {
        exit("Error: cannot copy '\(boot1Path!)' to destination.")
      }
      
      if !copyReplace(src: boot1installPath!,
                      dst: "/tmp/boot1-install",
                      attr: nil,
                      log: false) {
        exit("Error: cannot copy '\(boot1installPath!)' to destination.")
      }
      
      let esp = isESP ? "ESP" : "OTHER"
      let task = Process()
      task.environment = ProcessInfo().environment
      if #available(OSX 10.13, *) {
        task.executableURL = URL(fileURLWithPath: bootSectorsInstall)
      } else {
        task.launchPath = bootSectorsInstall
      }
      task.arguments = [ disk, filesystem, shemeMap, boot0!, boot1!, esp ]
      let pipe = Pipe()
      
      if self.realTime {
        task.standardOutput = pipe
        task.standardError  = pipe
        let fh = pipe.fileHandleForReading
        fh.waitForDataInBackgroundAndNotify()
        
        var op1 : NSObjectProtocol!
        op1 = NotificationCenter.default.addObserver(forName: NSNotification.Name.NSFileHandleDataAvailable,
                                                     object: fh, queue: nil) {
                                                      notification -> Void in
                                                      let data = fh.availableData
                                                      if data.count > 0 {
                                                        let output = String(decoding: data, as: UTF8.self)
                                                        self.log(output)
                                                        fh.waitForDataInBackgroundAndNotify()
                                                      }
        }
        
        var op2 : NSObjectProtocol!
        op2 = NotificationCenter.default.addObserver(forName: Process.didTerminateNotification,
                                                     object: task, queue: nil) {
                                                      notification -> Void in
                                                      NotificationCenter.default.removeObserver(op2 as Any)
                                                      if task.terminationStatus != 0 {
                                                        self.exit("Error: failed installing boot sectors.")
                                                      }
                                                      NotificationCenter.default.removeObserver(op1 as Any)
        }
        
        task.launch()
        task.waitUntilExit()
      } else {
        task.standardOutput = pipe
        task.standardError = pipe
        
        let fh = pipe.fileHandleForReading
        task.terminationHandler = { (t) in
          if t.terminationStatus != 0 {
            self.exit("Error: failed installing boot sectors.")
          }
        }
        task.launch()
        task.waitUntilExit()
        
        // TODO: make output in real time
        let data = fh.readDataToEndOfFile()
        if let out = String(data: data, encoding: .utf8) {
          self.log(out)
        }
      }
    } else {
      saveLog()
      if (isESP) {
        let task = Process()
        task.environment = ProcessInfo().environment
        if #available(OSX 10.13, *) {
          task.executableURL = URL(fileURLWithPath: "/usr/sbin/diskutil")
        } else {
          task.launchPath = "/usr/sbin/diskutil"
        }
        task.arguments = [ "umount", "force", disk ]
        task.launch()
        task.waitUntilExit()
      }
      cleanUp()
    }
 
    Darwin.exit(EXIT_SUCCESS)
    // ------------- end
  }
}
