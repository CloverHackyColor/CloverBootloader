/*
 * vector sigma (https://github.com/vectorsigma72)
 * Copyright 2020 vector sigma All Rights Reserved.
 *
 * The source code contained or described herein and all documents related
 * to the source code ("Material") are owned by vector sigma.
 * Title to the Material remains with vector sigma or its suppliers and licensors.
 * The Material is proprietary of vector sigma and is protected by worldwide copyright.
 * No part of the Material may be used, copied, reproduced, modified, published,
 * uploaded, posted, transmitted, distributed, or disclosed in any way without
 * vector sigma's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by vector sigma in writing.
 *
 * Unless otherwise agreed by vector sigma in writing, you may not remove or alter
 * this notice or any other notice embedded in Materials by vector sigma in any way.
 *
 * The license is granted for the CloverBootloader project (i.e. https://github.com/CloverHackyColor/CloverBootloader)
 * and all the users as long as the Material is used only within the
 * source code and for the exclusive use of CloverBootloader, which must
 * be free from any type of payment or commercial service for the license to be valid.
 */

import Cocoa

@available(OSX 10.11, *)
class PlistEditorWC: NSWindowController, NSWindowDelegate {
  var subview: PlistEditorWC?
  var fixed: Bool = false
  
  override func windowDidLoad() {
    super.windowDidLoad()
    
    self.window?.titlebarAppearsTransparent = false
    self.window?.isMovableByWindowBackground = true
    
    self.window?.styleMask = [.titled, .miniaturizable, .closable, .resizable]
    
    self.window?.collectionBehavior = [.fullScreenAuxiliary, .fullScreenPrimary]
    
    if #available(OSX 10.12, *) {
      self.window?.tabbingMode = NSWindow.TabbingMode.preferred
    } else {
      self.shouldCascadeWindows = true
    }
  }
  
  
  func windowWillClose(_ notification: Notification) { }
  
  func windowDidBecomeMain(_ notification: Notification) { }
  
  func windowDidBecomeKey(_ notification: Notification) { }
  
  func windowDidResignKey(_ notification: Notification) { }
  
  class func loadFromNib(parser: PlistParser) -> PlistEditorWC {
    let id = NSStoryboard.SceneIdentifier("Document Window Controller")
    let wc = NSStoryboard(name: NSStoryboard.Name("PlistEditor"),
                          bundle: nil).instantiateController(withIdentifier: id) as! PlistEditorWC
    (wc.contentViewController as? PlistEditorVC)?.parser = parser
    return wc
  }
  
  override func newWindowForTab(_ sender: Any?) {
    let dc = NSDocumentController.shared
    do {
      let doc = try dc.makeUntitledDocument(ofType: "XML PropertyList v1") as? Document
      
      doc?.makeWindowControllers()
      dc.addDocument(doc!) // <--- very important! w/o this call the document is not connected to main menu
      if #available(OSX 10.12, *) {
        self.window!.addTabbedWindow((doc?.windowController?.window)!, ordered: .above)
      }
      doc?.windowController?.window?.makeKeyAndOrderFront(self)
    } catch let error as NSError {
      let alert = NSAlert()
      alert.messageText = error.localizedFailureReason ?? error.localizedDescription
      alert.informativeText = error.localizedRecoverySuggestion ?? ""
      alert.runModal()
    }
    
  }
}

