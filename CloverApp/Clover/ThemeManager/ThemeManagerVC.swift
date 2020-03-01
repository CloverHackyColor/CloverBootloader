//
//  ThemeManagerVC.swift
//  ThemeManager
//
//  Created by vector sigma on 07/01/2020.
//  Copyright © 2020 vectorsigma. All rights reserved.
//

import Cocoa
import WebKit

final class GradientView : NSView {
  override func draw(_ dirtyRect: NSRect) {
    super.draw(dirtyRect)
    /*
    let up = NSColor(red: 48 / 255, green: 35 / 255, blue: 255 / 255, alpha: 0.5)
    let down = NSColor(red: 150 / 255, green: 158 / 255, blue: 215 / 255, alpha: 0.5)
    let grad = NSGradient(colors: [down, up])
    grad?.draw(in: dirtyRect, angle: 45)*/
  }
}

final class ThemeManagerVC: NSViewController,
NSTableViewDelegate, NSTableViewDataSource, WebFrameLoadDelegate, WebUIDelegate {
  var targetVolume : String? = nil
  
  var manager : ThemeManager?
  @IBOutlet var installedThemesCheckBox : NSButton!
  
  @IBOutlet var webView : WebView!
  @IBOutlet var sidebar : NSTableView!
  @IBOutlet var nameBox : NSComboBox!
  @IBOutlet var authorField : NSTextField!
  @IBOutlet var infoField : NSTextField!
  
  @IBOutlet var targetPop : FWPopUpButton!
  @IBOutlet var spinner : NSProgressIndicator!
  @IBOutlet var installButton : NSButton!
  @IBOutlet var unistallButton : NSButton!
  
  var loaded : Bool = false
  var showInstalled : Bool = false
  
  override func awakeFromNib() {
    super.awakeFromNib()
    if !self.loaded {
      if #available(OSX 10.10, *) {} else {
        self.viewDidLoad()
      }
      self.loaded = true
    }
  }
  
  override func viewDidLoad() {
    
    if #available(OSX 10.10, *) {
      super.viewDidLoad()
    }
    self.loaded = true
    
    /*
    let winImagePath = "/Library/Desktop Pictures/Ink Cloud.jpg"

    if fm.fileExists(atPath: winImagePath) {
      let winImage = NSImage(byReferencingFile: winImagePath)
      let winImageSize = NSMakeSize(self.view.frame.width, self.view.frame.height)
      winImage?.size = winImageSize
      self.view.window?.backgroundColor = NSColor.init(patternImage: winImage!)
    }*/
    
    self.view.window?.title = self.view.window!.title.locale
    let settingVC = AppSD.settingsWC?.contentViewController as? SettingsViewController
    settingVC?.themeUserField.isEnabled = false
    settingVC?.themeRepoField.isEnabled = false
    self.webView.drawsBackground = false
    self.webView.uiDelegate = self
    
    self.webView.frameLoadDelegate = self
    self.webView.mainFrame.frameView.allowsScrolling = false
 
    
    localize(view: self.view)
    AppSD.isInstallerOpen = true
    settingVC?.disksPopUp.isEnabled = false
    settingVC?.updateCloverButton.isEnabled = false
    settingVC?.unmountButton.isEnabled = false
    
    AppSD.themes.removeAll()
    self.nameBox.completes = true
    
    self.nameBox.removeAllItems()
    self.sidebar.delegate = self
    self.sidebar.dataSource = self
    self.sidebar.backgroundColor = NSColor.clear
    self.sidebar.enclosingScrollView?.contentView.drawsBackground = false
    
    if #available(OSX 10.10, *) {
      self.sidebar.usesStaticContents = true
    }
    
    self.targetPop.removeAllItems()
    self.populateTargets()
    
    if let bootDevice = findBootPartitionDevice() {
      if let mountPoint = getMountPoint(from: bootDevice) {
        for i in self.targetPop.itemArray {
          if let target = i.representedObject as? String {
            if bootDevice == target {
              self.targetPop.select(i)
              self.targetVolume = mountPoint
              break
            }
          }
        }
      }
    }
    
    self.showIndexing()
    //return
    let themeManagerIndexDir = NSHomeDirectory().addPath("Library/Application Support/CloverApp/Themeindex/\(AppSD.themeUser)_\(AppSD.themeRepo)")
    
    self.manager = ThemeManager(user: AppSD.themeUser,
                                repo: AppSD.themeRepo,
                                basePath: themeManagerIndexDir,
                                indexDir: themeManagerIndexDir,
                                delegate: self)
    
    /*
     1) immediately load indexed themes (if any).
     Fast if thumbnails already exists.
     */
    
    for t in self.manager!.getIndexedThemes() {
      self.add(theme: t)
    }
    
    /*
     2) download thumbnails.
     Slower if thumbnails didn't exist or themes never indexed.
     */

    self.manager?.getThemes { (t) in
      let sorted = t.sorted()
      for t in sorted {
        DispatchQueue.main.async {
          self.add(theme: t)
        }
      }
    }
  }
  
  func dataSource() -> [String] {
    if showInstalled {
      return AppSD.installedThemes
    }
    return AppSD.themes
  }

  @IBAction func showInstalledThemes(_ sender: NSButton!) {
    DispatchQueue.main.asyncAfter(deadline: .now() + 0.01) {
      let copy = self.manager!.getIndexedThemes().sorted() // in cases of low downloads
      self.nameBox.removeAllItems()
      self.sidebar.noteNumberOfRowsChanged()
      AppSD.installedThemes.removeAll()
      AppSD.themes.removeAll()
     
      if sender.state == .on {
        if (self.targetVolume != nil && fm.fileExists(atPath: self.targetVolume!)) {
          let themeDir = self.targetVolume!.addPath("EFI/CLOVER/themes")
          var installed = [String]()
          for theme in copy {
            if fm.fileExists(atPath: themeDir.addPath(theme)) {
              installed.append(theme)
            }
          }
          
          if fm.fileExists(atPath: themeDir) {
            do {
              let userTheme = try fm.contentsOfDirectory(atPath: themeDir)
              for theme in userTheme {
                if (fm.fileExists(atPath: themeDir.addPath(theme).addPath("theme.plist"))
                  && fm.fileExists(atPath: themeDir.addPath(theme).addPath("screenshot.png"))) {
                  if !installed.contains(theme) {
                    installed.append(theme)
                  }
                } else if fm.fileExists(atPath: themeDir.addPath(theme).addPath("theme.svg")) {
                  if !installed.contains(theme) {
                    installed.append(theme)
                  }
                }
              }
            } catch { }
          }
          AppSD.installedThemes = installed.sorted()
          self.nameBox.addItems(withObjectValues: installed.sorted())
          if installed.count == 0 {
            self.showNoThemes()
          }
        }
        self.showInstalled = true
      } else {
        AppSD.themes = copy
        self.nameBox.addItems(withObjectValues: copy.sorted())
        self.showInstalled = false
        if copy.count == 0 {
          self.showNoThemes()
        }
      }
      self.sidebar.reloadData()
    }
  }
  
  func showNoThemes() {
    self.webView.mainFrame.loadHTMLString(
      """
      <html>
      <head>
      <style>
      .center {
      height: 100%;
      position: relative;
      border: 3px solid gray;
      }

      .center p {
      margin: 0;
      position: absolute;
      top: 50%;
      left: 50%;
      -ms-transform: translate(-50%, -50%);
      transform: translate(-50%, -50%);
      }
      </style>
      </head>
      <body>
      <div class="center">
      <p>\("No themes found".locale)</p>
      </div>
      </body>
      </html>
      """,
      baseURL: Bundle.main.bundleURL)
  }
  
  func showIndexing() {
    // https://www.w3schools.com/howto/howto_css_loader.asp
    self.webView.mainFrame.loadHTMLString("""
      <html>
      <head>
      <meta name="viewport" content="width=device-width, initial- scale=1">
      <style>
      .loader {
      position: absolute;
      left: 30%;
      top: 25%;
      border: 20px solid #f3f3f3;
      border-radius: 50%;
      border-top: 16px solid white;
      border-right: 16px solid gray;
      border-bottom: 16px solid white;
      border-left: 16px solid gray;
      width: 160px;
      height: 160px;
      -webkit-animation: spin 2s linear infinite;
      animation: spin 2s linear infinite;
      }

      .blinking{
        animation:blinkingText 1.2s infinite;
      }

      @keyframes blinkingText{
        0%{   color: gray; }
        49%{  color: gray; }
        60%{  color: transparent; }
        99%{  color:transparent; }
        100%{ color: gray; }
      }

      @-webkit-keyframes spin {
      0% { -webkit-transform: rotate(0deg); }
      100% { -webkit-transform: rotate(360deg); }
      }

      @keyframes spin {
      0% { transform: rotate(0deg); }
      100% { transform: rotate(360deg); }
      }
      </style>
      </head>
      <body>
      <h2><span class="blinking">Indexing..</span></h2>

      <div class="loader"></div>

      </body>
      </html>
      """, baseURL: Bundle.main.bundleURL)
  }
  
  @IBAction func unInstallPressed(_ sender: NSButton!) {
    if AppSD.isInstalling {
      NSSound.beep()
      return
    }
    if self.targetPop.indexOfSelectedItem == 0
      || self.targetVolume == nil
      || (self.targetVolume != nil && !fm.fileExists(atPath: self.targetVolume!)) {
      NSSound.beep()
      return // this should not happen
    }

    let theme = self.nameBox.stringValue
    let themePath = self.targetVolume!.addPath("EFI/CLOVER/themes").addPath(theme)
    
    let sr = self.sidebar.selectedRow
    if sr >= 0 && sr < self.dataSource().count {
        self.spinner.startAnimation(nil)
        do {
          if fm.fileExists(atPath: themePath) {
            try fm.removeItem(atPath: themePath)
            self.unistallButton.isEnabled = false
            self.unistallButton.animator().isHidden = true
            NSSound(contentsOfFile: "/System/Library/Components/CoreAudio.component/Contents/SharedSupport/SystemSounds/finder/empty trash.aif", byReference: false)?.play()
          }
          self.spinner.stopAnimation(nil)
          self.showInstalledThemes(self.installedThemesCheckBox)
        } catch {
          NSSound(named: "Basso")?.play()
          let alert = NSAlert()
          alert.messageText = "Can't remove the theme".locale
          alert.informativeText = "\(error.localizedDescription)"
          alert.addButton(withTitle: "OK")
          alert.runModal()
          self.spinner.stopAnimation(nil)
          self.showInstalledThemes(self.installedThemesCheckBox)
        }
    }
  }
  
  @IBAction func InstallPressed(_ sender: NSButton!) {
    if AppSD.isInstalling {
      NSSound.beep()
      return
    }
    
    if self.targetPop.indexOfSelectedItem == 0
      || self.targetVolume == nil
      || (self.targetVolume != nil && !fm.fileExists(atPath: self.targetVolume!)) {
      NSSound.beep()
      DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
        self.targetPop.performClick(self.targetPop)
      }
      return
    }
    let theme = self.nameBox.stringValue
    let themeDest = self.targetVolume!.addPath("EFI/CLOVER/themes").addPath(theme)
    self.spinner.startAnimation(nil)
    let sr = self.sidebar.selectedRow
    if sr >= 0 && sr < self.dataSource().count {
      self.manager?.download(theme: theme,
                             down: .complete,
                             completion: { (path) in
                              
                              if let themePath = path {
                                try? fm.removeItem(atPath: themeDest)
                                do {
                                  try fm.moveItem(atPath: themePath, toPath: themeDest)
                                  NSSound(named: "Glass")?.play()
                                } catch {
                                  NSSound(named: "Basso")?.play()
                                  DispatchQueue.main.async {
                                    let alert = NSAlert()
                                    alert.messageText = "Installation failed".locale
                                    alert.informativeText = "\(error.localizedDescription)"
                                    alert.addButton(withTitle: "OK")
                                    alert.runModal()
                                  }
                                }
                              } else {
                                NSSound(named: "Basso")?.play()
                                DispatchQueue.main.async {
                                  let alert = NSAlert()
                                  alert.messageText = "Installation failed".locale
                                  alert.informativeText = "Theme \"\(theme)\" not found."
                                  alert.addButton(withTitle: "OK")
                                  alert.runModal()
                                }
                              }
                              
                              DispatchQueue.main.async {
                                AppSD.isInstalling = false
                                self.spinner.stopAnimation(nil)
                              }
      })
    }

  }
  
  func addRow(for theme: String) {
    let index = AppSD.themes.count
    AppSD.themes.append(theme)
    //if self.installedThemesCheckBox.state == .off {
      self.sidebar.beginUpdates()
      self.sidebar.insertRows(at: IndexSet(integer: index), withAnimation: .slideUp)
      self.sidebar.endUpdates()
    //}
    self.nameBox.addItem(withObjectValue: theme)
  }
  
  func add(theme: String) {
    if AppSD.themes.contains(theme) {
      return
    }
    if let sha = self.manager?.getSha() {
      let themePath = self.manager!.basePath.addPath(sha).addPath(theme)
      if fm.fileExists(atPath: themePath.addPath("theme.svg")) {
        self.addRow(for: theme)
      } else if (fm.fileExists(atPath: themePath.addPath("screenshot.png"))
        && fm.fileExists(atPath: themePath.addPath("theme.plist"))) {
        self.addRow(for: theme)
      } else {
        self.manager?.download(theme: theme, down: .thumbnail, completion: { (path) in
          if  path != nil {
            DispatchQueue.main.async {
              self.addRow(for: theme)
            }
          }
        })
      }
    }
  }
  
  @IBAction func nameBoxSelected(_ sender: NSComboBox!) {
    let theme = sender.stringValue
    if theme.count > 0 && self.dataSource().contains(theme) {
      let index = self.dataSource().firstIndex(of: theme)
      self.sidebar.selectRowIndexes(IndexSet(integer: index!), byExtendingSelection: false)
      self.sidebar.scrollRowToVisible(index!)
    } else {
      NSSound.beep()
    }
  }
  
  func tableView(_ tableView: NSTableView, didAdd rowView: NSTableRowView, forRow row: Int) {
    if self.sidebar.selectedRow < 0 {
      self.sidebar.selectRowIndexes(IndexSet(integer: 0), byExtendingSelection: false)
    }
  }
  
  func numberOfRows(in tableView: NSTableView) -> Int {
    return self.dataSource().count
  }
  
  func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
    return 82
  }
  
  func tableViewSelectionDidChange(_ notification: Notification) {
    let sr = self.sidebar.selectedRow
    if sr >= 0 {
      if let v = self.sidebar.view(atColumn: 0, row: sr, makeIfNecessary: false) as? ThemeView {
        if v.isInstalled {
          self.unistallButton.animator().isHidden = false
          self.unistallButton.isEnabled = true
        } else {
          self.unistallButton.animator().isHidden = true
          self.unistallButton.isEnabled = false
        }
        
        if (self.manager?.exist(theme: v.name))! {
          self.installButton.isEnabled = true
        } else {
          self.installButton.isEnabled = false
        }
        
        guard let path = v.imagePath else {
          return
        }
        
        self.nameBox.stringValue = v.name
        self.authorField.stringValue = v.author ?? ""
        self.infoField.stringValue = v.info ?? ""
        
        self.webView.mainFrame.loadHTMLString("""
          <html>
          <body>
          <div align="center" style="position:relative; height: 100%; width: 100%; top:0;left 0;">
          <img src="\(URL(fileURLWithPath: path))" style="width: 100% ; max-width: 575px;top:0;"/>
          </div>
          </body>
          </html>
          """, baseURL: Bundle.main.bundleURL)
        
        if path.hasSuffix(".svg") {
          let svgStr = try? String(contentsOfFile: path)
          var author : String? = nil
          var version : String? = nil
          var description : String? = nil
          if let lines = svgStr?.components(separatedBy: .newlines) {
            /*
             Version="0.87"
             Year="2018-2019"
             Author="Blackosx"
             Description="Vector version of BGM (Based on Clovy theme file structure)"
             */
            
            for line in lines {
              if (line.range(of: "Version=\"") != nil) {
                version = line.components(separatedBy: "Version=\"")[1].components(separatedBy: "\"")[0]
              }
            }
            for line in lines {
              if (line.range(of: "Author=\"") != nil) {
                author = line.components(separatedBy: "Author=\"")[1].components(separatedBy: "\"")[0]
              }
            }
            for line in lines {
              if (line.range(of: "Description=\"") != nil) {
                description = line.components(separatedBy: "Description=\"")[1].components(separatedBy: "\"")[0]
              }
            }
          }
          
          self.authorField.stringValue = author ?? ""
          self.infoField.stringValue = "\(description ?? "?"), v\(version ?? "?")"
        } else {
          // get theme.plist
          let plistPath = "\((path as NSString).deletingLastPathComponent)/theme.plist"
          let plist = NSDictionary(contentsOfFile: plistPath)
          self.authorField.stringValue = (plist?.object(forKey: "Author") as? String) ?? ""
          self.infoField.stringValue = (plist?.object(forKey: "Description") as? String) ?? ""
        }
      }
    }
  }
  
  func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
    if let identifier = tableColumn?.identifier {
      if identifier.rawValue == "column1" {
        let tv = ThemeView(manager: self.manager!,
                           name: self.dataSource()[row],
                           row: row)
        tv.manager.delegate = self
        return tv
      }
    }
    
    return nil
  }
  
  func tableView(_ tableView: NSTableView, rowViewForRow row: Int) -> NSTableRowView? {
    return ThemeTableRowView()
  }
  
  // MARK: find suitable disks
  func populateTargets() {
    if AppSD.isInstalling {
      return
    }
    
    let selected : String? = self.targetPop.selectedItem?.representedObject as? String
    self.targetPop.removeAllItems()
    self.targetPop.addItem(withTitle: "Select a disk..".locale)
    let disks : NSDictionary = getAlldisks()
    let bootPartition = findBootPartitionDevice()
    let diskSorted : [String] = (disks.allKeys as! [String]).sorted()
    for d in diskSorted {
      let disk : String = d
      if isWritable(diskOrMtp: disk) && !kBannedMedia.contains(getVolumeName(from: disk) ?? "") {
        let fs : String = getFS(from: disk)?.lowercased() ?? kNotAvailable.locale
        let psm : String = getPartitionSchemeMap(from: disk) ?? kNotAvailable.locale
        let name : String = getVolumeName(from: disk) ?? kNotAvailable.locale
        let mp : String = getMountPoint(from: disk) ?? kNotAvailable.locale
        let parentDiskName : String = getMediaName(from: getBSDParent(of: disk) ?? "") ?? kNotAvailable.locale
        
        let supportedFS = ["msdos", "fat32", "exfat", "hfs"]
        
        if supportedFS.contains(fs) {
          self.targetPop.addItem(withTitle: "\(disk)\t\(name), \("mount point".locale): \(mp), \(fs.uppercased()), \(psm): (\(parentDiskName))")
          self.targetPop.invalidateIntrinsicContentSize()
          // get the image
          if disk == bootPartition {
            let image : NSImage = NSImage(named: "NSApplicationIcon")!.copy() as! NSImage
            image.size = NSMakeSize(16, 16)
            self.targetPop.lastItem?.image = image
          } else if let image : NSImage = getIconFor(volume: disk) {
            image.size = NSMakeSize(16, 16)
            self.targetPop.lastItem?.image = image
          }
          self.targetPop.lastItem?.representedObject = disk
        }
      }
    }
    
    if (selected != nil) {
      for item in self.targetPop.itemArray {
        if let d = item.representedObject as? String {
          if selected == d {
            self.targetPop.select(item)
            break
          }
        }
      }
    }
  }
  
  @IBAction func targetPopPressed(_ sender: FWPopUpButton!) {
    if let disk = sender?.selectedItem?.representedObject as? String {
      if !isMountPoint(path: disk) {
        //DispatchQueue.global(qos: .background).async {
        let cmd = "diskutil mount \(disk)"
        let msg = String(format: "Clover wants to mount %@", disk)
        let script = "do shell script \"\(cmd)\" with prompt \"\(msg)\" with administrator privileges"
        
        let task = Process()
        
        if #available(OSX 10.12, *) {
          task.launchPath = "/usr/bin/osascript"
          task.arguments = ["-e", script]
        } else {
          task.launchPath = "/usr/sbin/diskutil"
          task.arguments = ["mount", disk]
        }
        
        task.terminationHandler = { t in
          if t.terminationStatus == 0 {
            if isMountPoint(path: disk) {
              self.targetVolume = getMountPoint(from: disk)
              DispatchQueue.main.async {
                self.populateTargets()
                self.showInstalledThemes(self.installedThemesCheckBox)
              }
            }
            DispatchQueue.main.async { self.view.window?.makeKeyAndOrderFront(nil) }
          } else {
            NSSound.beep()
            self.showInstalledThemes(self.installedThemesCheckBox)
          }
        }
        task.launch()
        //}
      } else {
        self.targetVolume = getMountPoint(from: disk)
        self.populateTargets()
        self.showInstalledThemes(self.installedThemesCheckBox)
      }
    } else {
      self.targetVolume = nil
      self.unistallButton.isEnabled = false
      self.unistallButton.isHidden = true
      self.showInstalledThemes(self.installedThemesCheckBox)
    }
  }
}

// MARK: ThemeManager Window controller
final class ThemeManagerWC: NSWindowController, NSWindowDelegate {
  var viewController : NSViewController? = nil
  override var contentViewController: NSViewController? {
    get {
      self.viewController
    }
    set {
      self.viewController = newValue
    }
  }

  class func loadFromNib() -> ThemeManagerWC? {
    var topLevel: NSArray? = nil
    Bundle.main.loadNibNamed("ThemeManager", owner: self, topLevelObjects: &topLevel)
    if (topLevel != nil) {
      var wc : ThemeManagerWC? = nil
      for o in topLevel! {
        if o is ThemeManagerWC {
          wc = o as? ThemeManagerWC
        }
      }
      
      for o in topLevel! {
        if o is ThemeManagerVC {
          wc?.contentViewController = o as! ThemeManagerVC
        }
      }
      return wc
    }
    return nil
  }
  
  func windowShouldClose(_ sender: NSWindow) -> Bool {
    if AppSD.isInstalling  {
      return false
    }
    let settingVC = AppSD.settingsWC?.contentViewController as? SettingsViewController
    settingVC?.disksPopUp.isEnabled = true
    settingVC?.updateCloverButton.isEnabled = true
    settingVC?.searchESPDisks()
    AppSD.isInstallerOpen = false
    settingVC?.themeUserField.isEnabled = true
    settingVC?.themeRepoField.isEnabled = true
    self.window = nil
    self.close()
    AppSD.themeManagerWC = nil // remove a strong reference
    return true
  }
}

final class ThemeTableRowView: NSTableRowView {
  override func draw(_ dirtyRect: NSRect) {
    super.draw(dirtyRect)
    self.wantsLayer = true
    self.layer?.backgroundColor = NSColor.clear.cgColor
  }
  
  override var isEmphasized: Bool {
    set {}
    get {
      return false
    }
  }
  
  override var selectionHighlightStyle: NSTableView.SelectionHighlightStyle {
    set {}
    get {
      return .regular
    }
  }
  
  override func drawSelection(in dirtyRect: NSRect) {
    if self.selectionHighlightStyle != .none {
      let selectionRect = NSInsetRect(self.bounds, 2.5, 2.5)
      NSColor.green.setFill()
      //NSColor(calibratedWhite: 0.85, alpha: 0.6).setFill()
      let selectionPath = NSBezierPath.init(rect: selectionRect)
      selectionPath.fill()
    }
  }
}
