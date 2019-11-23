//
//  SettingsView.swift
//  Clover
//
//  Created by vector sigma on 30/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

class SettingsViewController: NSViewController, NSTextFieldDelegate, URLSessionDownloadDelegate {
  // MARK: Variables
  @IBOutlet var currentRevField : NSTextField!
  @IBOutlet var bootDeviceField : NSTextField!
  @IBOutlet var configPathField : FWTextField!
  @IBOutlet var disksPopUp : NSPopUpButton!
  @IBOutlet var unmountButton : NSButton!
  @IBOutlet var themeField : FWTextField!
  @IBOutlet var soundField : FWTextField!
  @IBOutlet var disbaleSleepProxyButton : NSButton!
  @IBOutlet var makeRootRWButton : NSButton!
  @IBOutlet var installDaemonButton : NSButton!
  @IBOutlet var unInstallDaemonButton : NSButton!
  @IBOutlet var timeIntervalPopUp : NSPopUpButton!
  @IBOutlet var checkNowButton : NSButton!
  @IBOutlet var lastUpdateCheckField : NSTextField!
  @IBOutlet var runAtLoginButton : NSButton!
  
  @IBOutlet var updateCloverButton : NSButton!
  @IBOutlet var installCloverButton : NSButton!
  
  @IBOutlet var progressBar : NSProgressIndicator!
  
  @IBOutlet var appVersionField : NSTextField!
  @IBOutlet var infoImageView : NSImageView!
  
  var lastReleaseRev : String? = nil
  var lastReleaseLink : String? = nil
  var currentRev : String = findCloverRevision() ?? kNotAvailable.locale
  var bootDevice : String? = findBootPartitionDevice()
  
  var timerUpdate : Timer? = nil
  var timeUpdateInterval : TimeInterval = UpdateInterval.never.rawValue
  
  var downloadTask : URLSessionDownloadTask? = nil
  
  // MARK: View customization
  override func viewDidLoad() {
    super.viewDidLoad()
    let appVersion = Bundle.main.infoDictionary?["CFBundleShortVersionString"] as! String + " Beta"
    self.appVersionField.stringValue = "v\(appVersion)"
    localize(view: self.view)
    self.view.wantsLayer = true
    self.view.layer?.backgroundColor = NSColor.clear.cgColor
    
    self.themeField.delegate = self
    self.soundField.delegate = self
    
    self.progressBar.isHidden = true
    
    self.runAtLoginButton.state = UDs.bool(forKey: kRunAtLogin) ? .on : .off
    self.unmountButton.isEnabled = false
    
    self.setUpdateInformations()
    
    let nvram = getNVRAM()
    var nvdata = nvram?.object(forKey: "Clover.Theme") as? Data
    
    self.themeField.placeholderString = kNotAvailable.locale
    self.themeField.stringValue = (nvdata != nil) ? String(decoding: nvdata!, as: UTF8.self) : ""
    self.themeField.cell?.representedObject = self.themeField.stringValue
    
    nvdata = nvram?.object(forKey: "Clover.Sound") as? Data
    self.soundField.placeholderString = kNotAvailable.locale
    self.soundField.stringValue = (nvdata != nil) ? String(decoding: nvdata!, as: UTF8.self) : ""
    self.soundField.cell?.representedObject = self.soundField.stringValue
    
    nvdata = nvram?.object(forKey: "Clover.RootRW") as? Data
    var value : String = String(decoding: nvdata ?? Data(), as: UTF8.self)
    self.makeRootRWButton.state = (value == "true") ? .on : .off
    
    nvdata = nvram?.object(forKey: "Clover.DisableSleepProxyClient") as? Data
    value = String(decoding: nvdata ?? Data(), as: UTF8.self)
    self.disbaleSleepProxyButton.state = (value == "true") ? .on : .off

    
    let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
    self.unInstallDaemonButton.isEnabled = daemonExist
    
    self.setUpdateButton()
    self.searchESPDisks()

    let itervals = ["never", "daily", "weekly", "monthly"]
    self.timeIntervalPopUp.removeAllItems()
    for i in itervals {
      self.timeIntervalPopUp.addItem(withTitle: i.locale)
      self.timeIntervalPopUp.lastItem?.representedObject = i
    }
    
    if (UDs.object(forKey: kUpdateSearchInterval) == nil) {
      // search update the first time..
      UDs.set("monthly", forKey: kUpdateSearchInterval)
      UDs.synchronize()
    }
    
    let def = UDs.string(forKey: kUpdateSearchInterval)!
    for i in self.timeIntervalPopUp.itemArray {
      let c = i.representedObject as! String
      if def == c {
        self.timeIntervalPopUp.select(i)
        break
      }
    }
    
    if let date : Date = UDs.object(forKey: kLastSearchUpdateDateKey) as? Date {
      let now = DateFormatter.localizedString(from: date, dateStyle: .short, timeStyle: .short)
      self.lastUpdateCheckField.stringValue = "\("last checked:".locale) \(now)"
    } else {
      self.lastUpdateCheckField.stringValue = "\("last checked:".locale) \("never".locale)"
    }

    self.timerUpdate = Timer.scheduledTimer(timeInterval: 60 * 60,
                                            target: self,
                                            selector: #selector(self.setUpdateTimer),
                                            userInfo: nil,
                                            repeats: true)
    
    let clickVersion = NSClickGestureRecognizer(target: self, action: #selector(goToWebSite))
    self.appVersionField.addGestureRecognizer(clickVersion)
    
    let topic = NSClickGestureRecognizer(target: self, action: #selector(goToTopic))
    self.infoImageView.addGestureRecognizer(topic)
  }
  
  func setUpdateInformations() {
    let rev = findCloverRevision(at: Bundle.main.sharedSupportPath!.addPath("CloverV2/EFI")) ?? "0000"
    var title = "\("Install Clover".locale) \(rev)"
    self.installCloverButton.title = title
    
    self.bootDevice = findBootPartitionDevice()
    title = "\("Boot device".locale): \(self.bootDevice ?? kNotAvailable.locale)"
    self.bootDeviceField.stringValue = title
    
    self.configPathField.stringValue = findConfigPath() ?? kNotAvailable.locale
    title = "\("Current Clover revision".locale): \(self.currentRev)"
    self.currentRevField.stringValue = title
    
    let last : String = (self.lastReleaseRev == nil) ? kNotAvailable.locale : "r\(self.lastReleaseRev!)"
    title = "\("Update Clover available".locale): \(last)"
    
    self.installCloverButton.isEnabled = true
  }
  
  override var representedObject: Any? {
    didSet {
      // Update the view, if already loaded.
    }
  }
  
  // MARK: Go to Website
  @objc func goToWebSite() {
    let link = "https://github.com/CloverHackyColor/CloverBootloader"
    NSWorkspace.shared.open(URL(string: link)!)
  }
  
  @objc func goToTopic() {
    let link = "https://www.insanelymac.com/forum/topic/341047-cloverapp-testing/"
    NSWorkspace.shared.open(URL(string: link)!)
  }
  
  // MARK: Disks
  func searchESPDisks() {
    self.unmountButton.isEnabled = false
    let selected = self.disksPopUp.selectedItem?.representedObject as? String
    
    self.disksPopUp.removeAllItems()
    self.disksPopUp.addItem(withTitle: "Select a disk..".locale)
    for e in getAllESPs() {
      if isWritable(diskOrMtp: e) {
        let fs = getFS(from: e) ?? kNotAvailable.locale
        let mp = getMountPoint(from: e) ?? kNotAvailable.locale
        let parentDiskName : String = getMediaName(from: getBSDParent(of: e) ?? "") ?? kNotAvailable.locale
        let title : String = "\(e), \(fs), \("mount point".locale): \(mp), \(parentDiskName)"
        self.disksPopUp.addItem(withTitle: title)
        self.disksPopUp.lastItem?.representedObject = e
        if e == self.bootDevice {
          let image : NSImage = NSImage(named: "NSApplicationIcon")!.copy() as! NSImage
          image.size = NSMakeSize(16, 16)
          self.disksPopUp.lastItem?.image = image
        } else if let image : NSImage = getIconFor(volume: e) {
          image.size = NSMakeSize(16, 16)
          self.disksPopUp.lastItem?.image = image
        }
      }
    }
    
    if (selected != nil) {
      for item in self.disksPopUp.itemArray {
        if let d = item.representedObject as? String {
          if d == selected {
            self.disksPopUp.select(item)
            if isMountPoint(path: d) {
              self.unmountButton.isEnabled = true
            }
            break
          }
        }
      }
    }
  }
  // MARK: Mount ESPs
  @IBAction func mountESP(_ sender: NSPopUpButton!) {
    self.unmountButton.isEnabled = false
    if let disk = sender.selectedItem?.representedObject as? String {
      if !isMountPoint(path: disk) {
        DispatchQueue.global(qos: .background).async {
          let cmd = "diskutil mount \(disk)"
          let msg = String(format: "Clover wants to mount %@", disk)
          let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
          
          let task = Process()
          task.launchPath = "/usr/bin/osascript"
          task.arguments = ["-e", script]
          
          task.terminationHandler = { t in
            if t.terminationStatus == 0 {
              DispatchQueue.main.async {
                self.unmountButton.isEnabled = true
              }
              NSWorkspace.shared.openFile(getMountPoint(from: disk) ?? "")
            } else {
              NSSound.beep()
            }
          }
          task.launch()
        }
      } else {
        self.unmountButton.isEnabled = true
        NSWorkspace.shared.openFile(getMountPoint(from: disk) ?? "")
      }
    }
  }
  
  // MARK: Umount
  @IBAction func umount(_ sender: NSButton!) {
    if let disk = self.disksPopUp.selectedItem?.representedObject as? String {
      if isMountPoint(path: disk) {
        DispatchQueue.global(qos: .background).async {
          let cmd = "diskutil umount \(disk)"
          let msg = String(format: "Clover wants to umount %@", disk)
          let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
          
          let task = Process()
          task.launchPath = "/usr/bin/osascript"
          task.arguments = ["-e", script]
          
          task.terminationHandler = { t in
            if t.terminationStatus != 0 {
              NSSound.beep()
            }
            DispatchQueue.main.async {
              self.searchESPDisks()
            }
          }
          task.launch()
        }
      }
    }
  }

  // MARK: Controls actions
  @IBAction func installClover(_ sender: NSButton!) {
    if (AppSD.installerWC == nil) {
      AppSD.installerWC = InstallerWindowController.loadFromNib()
    }
    
    AppSD.installerWC?.showWindow(self)
    NSApp.activate(ignoringOtherApps: true)
  }
  
  @IBAction func installDaemon(_ sender: NSButton!) {
    let daemonPath = Bundle.main.executablePath!.deletingLastPath.addPath("CloverDaemonNew")
    
    DispatchQueue.global(qos: .background).async {
      let task = Process()
      let msg = "Install CloverDaemonNew".locale
      
      let script = "do shell script \"\" & quoted form of \"\(daemonPath)\" & \" --install\" with prompt \"\(msg)\" with administrator privileges"
      
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      
      task.terminationHandler = { t in
        if t.terminationStatus != 0 {
          print(t.terminationReason)
          NSSound.beep()
        }
        
        DispatchQueue.main.async {
          let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
          self.unInstallDaemonButton.isEnabled = daemonExist
        }
      }
      task.launch()
    }
  }
  
  @IBAction func unInstallDaemon(_ sender: NSButton!) {
    let daemonPath = Bundle.main.executablePath!.deletingLastPath.addPath("CloverDaemonNew")
    
    DispatchQueue.global(qos: .background).async {
      let task = Process()
      let msg = "Install CloverDaemonNew".locale
      
      let script = "do shell script \"\" & quoted form of \"\(daemonPath)\" & \" --uninstall\" with prompt \"\(msg)\" with administrator privileges"
      
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      
      task.terminationHandler = { t in
        if t.terminationStatus != 0 {
          print(t.terminationReason)
          NSSound.beep()
        }
        
        DispatchQueue.main.async {
          let daemonExist = fm.fileExists(atPath: kDaemonPath) && fm.fileExists(atPath: kLaunchPlistPath)
          self.unInstallDaemonButton.isEnabled = daemonExist
        }
      }
      task.launch()
    }
  }
  
  @IBAction func setUpdateInterval(_ sender: NSPopUpButton!) {
    if let selected = sender.selectedItem?.representedObject as? String {
      UDs.set(selected, forKey: kUpdateSearchInterval)
      self.setUpdateTimer()
    }
  }
  
  @IBAction func checkNow(_ sender: NSButton!) {
    self.searchUpdate()
  }
  
  // MARK: Run At Login
  @IBAction func runAtLogin(_ sender: NSButton!) {
    if sender.state == .on {
      AppSD.setLaunchAtStartup()
    } else {
      AppSD.removeLaunchAtStartup()
    }
    
    // check the result
    sender.state = UDs.bool(forKey: kRunAtLogin) ? .on : .off
  }
  
  // MARK: NVRAM editing
  func controlTextDidEndEditing(_ obj: Notification) {
    if let field = obj.object as? NSTextField {
      let delete : Bool = field.stringValue.count == 0
      if field == self.themeField || field == soundField {
        if let old = field.cell?.representedObject as? String {
          let key = (field == self.themeField) ? "Clover.Theme" : "Clover.Sound"
          if old != field.stringValue {
            if delete {
              deleteNVRAM(key: key)
            } else {
              setNVRAM(key: key, stringValue: field.stringValue/*, error: &error*/)
            }
            
            let nvdata = getNVRAM()?.object(forKey: key) as? Data
            field.stringValue = (nvdata != nil) ? String(decoding: nvdata!, as: UTF8.self) : ""
            field.cell?.representedObject = field.stringValue
          }
        }
      }
    }
  }
  
  @IBAction func disableSleepProxy(_ sender: NSButton!) {
    let key = "Clover.DisableSleepProxyClient"
    if sender.state == .on {
      setNVRAM(key: key, stringValue: "true"/*, error: &error*/)
    } else {
      deleteNVRAM(key: key)
    }
    
    DispatchQueue.main.asyncAfter(deadline: .now() + 4.0) {
      var value = ""
      if let nvram = getNVRAM() {
        let nvdata = nvram.object(forKey: key) as? Data
        value = String(decoding: nvdata ?? Data(), as: UTF8.self)
      }
      self.disbaleSleepProxyButton.state = (value == "true") ? .on : .off
    }
  }
  
  @IBAction func makeRootRW(_ sender: NSButton!) {
    let key = "Clover.RootRW"
    if sender.state == .on {
      setNVRAM(key: key, stringValue: "true"/*, error: &error*/)
    } else {
      deleteNVRAM(key: key)
    }
    
    DispatchQueue.main.asyncAfter(deadline: .now() + 4.0) {
      var value = ""
      if let nvram = getNVRAM() {
        let nvdata = nvram.object(forKey: key) as? Data
        value = String(decoding: nvdata ?? Data(), as: UTF8.self)
      }
      self.makeRootRWButton.state = (value == "true") ? .on : .off
    }
  }
  
  @IBAction func readDaemonLog(_ sender: NSButton!) {
    DispatchQueue.global(qos: .background).async {
      let cmd = "cat /Library/Logs/CloverEFI/clover.daemon.log > /tmp/clover.daemon.log && open /tmp/clover.daemon.log"
      let msg = "CloverDaemon log"
      let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
      
      let task = Process()
      task.launchPath = "/usr/bin/osascript"
      task.arguments = ["-e", script]
      
      task.terminationHandler = { t in
        if t.terminationStatus == 0 {
          DispatchQueue.main.async {
            self.unmountButton.isEnabled = true
          }
          NSWorkspace.shared.openFile("/tmp/clover.daemon.log")
        } else {
          NSSound.beep()
        }
      }
      task.launch()
    }
  }
  
  @IBAction func readbdmesg(_ sender: NSButton!) {
    if let bdmesg = dumpBootlog() {
      do {
        try bdmesg.write(toFile: "/tmp/bdmesg.log", atomically: true, encoding: .utf8)
        NSWorkspace.shared.openFile("/tmp/bdmesg.log")
      } catch {
        NSSound.beep()
      }
    } else {
      NSSound.beep()
    }
  }
  
  
  // MARK: Search update
  @objc func setUpdateTimer() {
    // check last date update was checked
    var ti : TimeInterval = UpdateInterval.never.rawValue
    let def = (UDs.value(forKey: kUpdateSearchInterval) as? String) ?? "never"
    switch def {
    case "never":
      ti = UpdateInterval.never.rawValue
    case "daily":
      ti = UpdateInterval.daily.rawValue
    case "weekly":
      ti = UpdateInterval.weekly.rawValue
    case "monthly":
      ti = UpdateInterval.monthly.rawValue
    default:
      break
    }
    
    
    
    // Time interval is what user defines less time elapsed
    if ti > 0 {
      let lastCheckDate : Date = (UDs.object(forKey: kLastSearchUpdateDateKey) as? Date) ?? Date()
      let secElapsed = Date().timeIntervalSinceReferenceDate - lastCheckDate.timeIntervalSinceReferenceDate
      
      if secElapsed >= ti {
        print(secElapsed)
        self.searchUpdate()
      }
    }
  }
  
  func setUpdateButton() {
    if (self.lastReleaseRev != nil && self.lastReleaseLink != nil) {
      AppSD.statusItem.title = self.lastReleaseRev
      self.updateCloverButton.title = String(format: "Update to %@".locale, self.lastReleaseRev!)
      self.updateCloverButton.isEnabled = true
    } else {
      AppSD.statusItem.title = ""
      self.updateCloverButton.title = kNotAvailable.locale
      self.updateCloverButton.isEnabled = false
    }
  }
  
  @objc func searchUpdate() {
    getLatestRelease { (l, v) in
      self.lastReleaseLink = l
      self.lastReleaseRev = v
      let currRevNum : Int = Int(self.currentRev) ?? 0
      let lastRevNum : Int = Int(self.lastReleaseRev ?? "0") ?? 0
      
      if (self.lastReleaseLink != nil && self.lastReleaseRev != nil)
        && lastRevNum > 0
        && (lastRevNum > currRevNum) {
        UDs.set(self.lastReleaseLink!, forKey: kLastUpdateLink)
        UDs.set(self.lastReleaseRev!, forKey: kLastUpdateRevision)
        DispatchQueue.main.async {
          AppSD.statusItem.button?.title = "\(lastRevNum)"
          AppSD.statusItem.button?.imagePosition = .imageLeft
          self.updateCloverButton.isEnabled = true
          self.updateCloverButton.title = String(format: "Update to r%d".locale, lastRevNum)
        }
      } else {
        DispatchQueue.main.async {
          let ll = UDs.string(forKey: kLastUpdateLink)
          let lr = UDs.string(forKey: kLastUpdateRevision)
          let lrnum : Int = Int(lr ?? "0") ?? 0
          if ((ll != nil && lr != nil) && lrnum > currRevNum) {
            self.lastReleaseLink = ll
            self.lastReleaseRev = lr
            AppSD.statusItem.button?.title = "\(lastRevNum)"
            AppSD.statusItem.button?.imagePosition = .imageLeft
            self.updateCloverButton.isEnabled = true
            self.updateCloverButton.title = String(format: "Update to r%d".locale, lastRevNum)
          } else {
            AppSD.statusItem.button?.title = ""
            AppSD.statusItem.button?.imagePosition = .imageOnly
            self.updateCloverButton.isEnabled = false
            self.updateCloverButton.title = kNotAvailable.locale
          }
        }
      }
      
      let date = Date()
      let now = DateFormatter.localizedString(from: date, dateStyle: .short, timeStyle: .short)
      DispatchQueue.main.async {
        self.lastUpdateCheckField.stringValue = "\("last checked:".locale) \(now)"
      }
      UDs.set(date, forKey: kLastSearchUpdateDateKey)
      UDs.synchronize()
    }
  }
  
  // MARK: Download
  @IBAction func updateClover(_ sender: NSButton!) {
    if AppSD.isInstalling ||
      AppSD.isInstallerOpen ||
      self.lastReleaseLink == nil ||
      self.lastReleaseRev == nil {
      NSSound.beep()
      return
    }
    self.progressBar.isHidden = false
    self.progressBar.doubleValue = 0.0
    let url = URL(string: self.lastReleaseLink!)
    
    let b = URLSessionConfiguration.default
    let session = Foundation.URLSession(configuration: b, delegate: self, delegateQueue: nil)
    
    //let session = URLSession(configuration: .default)
    
    if (url != nil) {
      self.installCloverButton.isEnabled = false
      self.downloadTask = session.downloadTask(with: url!)
      self.downloadTask?.resume()
    }
  }
  
  func urlSession(_ session: URLSession,
                  downloadTask: URLSessionDownloadTask,
                  didFinishDownloadingTo location: URL) {
   
    DispatchQueue.main.async {
      self.progressBar.doubleValue = 100
      self.progressBar.isHidden = true
    }
    
    do {
      
      let lastPath = downloadTask.originalRequest!.url!.lastPathComponent
      let data = try Data(contentsOf: location)

      if lastPath.fileExtension == "zip" && lastPath.hasPrefix("CloverV2") {
        // ok, We have the download completed: replace CloverV2 inside SharedSupport directory!
        
        // Decompress the zip archive
        // NSUserName() ensure the user have read write permissions
        let tempDir = "/tmp/CloverXXXXX\(NSUserName())Update"
        if fm.fileExists(atPath: tempDir) {
          try fm.removeItem(atPath: tempDir)
          
        }
        try fm.createDirectory(atPath: tempDir,
                               withIntermediateDirectories: false,
                               attributes: nil)
        
        let file = tempDir.addPath(lastPath)
        try data.write(to: URL(fileURLWithPath: file))
        unzip(file: file, destination: tempDir) { (success) in
          if success {
            self.replaceCloverV2(with: tempDir.addPath("CloverV2"))
          }
        }
      }
      
    } catch {
      print(error)
    }
  }
  
  func urlSession(_ session: URLSession,
                  task: URLSessionTask,
                  didCompleteWithError error: Error?) {
    
    DispatchQueue.main.async {
      self.installCloverButton.isEnabled = true
    }
    if (error != nil) {
      print(error!.localizedDescription)
    }
  }
  
  func urlSession(_ session: URLSession,
                  downloadTask: URLSessionDownloadTask,
                  didWriteData bytesWritten: Int64,
                  totalBytesWritten: Int64,
                  totalBytesExpectedToWrite: Int64) {
    
    if totalBytesExpectedToWrite == NSURLSessionTransferSizeUnknown {
      DispatchQueue.main.async {
        self.progressBar.isIndeterminate = true
        self.progressBar.startAnimation(nil)
      }
    } else {
      let percentage : Double  = Double(totalBytesWritten) / Double(totalBytesExpectedToWrite) * 100

      DispatchQueue.main.async {
        self.progressBar?.isIndeterminate = false
        self.progressBar?.doubleValue = percentage
      }
    }
  }
  
  private func replaceCloverV2(with newOne: String) {
    var isDir : ObjCBool = false
    if fm.fileExists(atPath: newOne, isDirectory: &isDir) {
      if isDir.boolValue {
        do {
          try fm.removeItem(atPath: Cloverv2Path)
          try fm.copyItem(atPath: newOne, toPath: Cloverv2Path)
          DispatchQueue.main.async {
            self.lastReleaseRev = nil
            self.lastReleaseLink = nil
            self.setUpdateInformations()
            self.setUpdateButton()
          }
        } catch {
          print(error)
        }
      }
    }
  }
  
  // MARK: Close
  @IBAction func close(_ sender: NSButton!) {
    NSApp.terminate(nil)
  }
  
}



// MARK: Settings Window controller
class SettingsWindowController: NSWindowController, NSWindowDelegate {
  class func loadFromNib() -> SettingsWindowController {
    let wc = NSStoryboard(name: "Settings",
                          bundle: nil).instantiateController(withIdentifier: "SettingsWindowController") as! SettingsWindowController
    return wc
  }
}
