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
final class Document: NSDocument {
  var data : Data?
  var windowController : PlistEditorWC?
  var contentViewController: PlistEditorVC!
  
  override init() {
    super.init()
    self.undoManager?.disableUndoRegistration()
  }
  
  override class var autosavesInPlace: Bool {
    return UDs.bool(forKey: kAutoSavePlistsKey)
  }
  
  override func canAsynchronouslyWrite(to url: URL,
                                       ofType typeName: String,
                                       for saveOperation: NSDocument.SaveOperationType) -> Bool {
    return true
  }
  
  override class func canConcurrentlyReadDocuments(ofType: String) -> Bool {
    return (ofType == "XML PropertyList v1" ||
      ofType == "Binary PropertyList v1" ||
      ofType == "XML")
  }
  
  override func removeWindowController(_ windowController: NSWindowController) {
    super.removeWindowController(windowController)
    let documents = NSDocumentController.shared.documents
    if documents.count == 1 {
      NSApp.setActivationPolicy(.accessory)
    }
  }
  
  override func makeWindowControllers() {
    var parser : PlistParser?
    if self.data != nil && self.fileType != nil { // loading a file
      parser = PlistParser(fromData: self.data!, fileType: PEFileType(rawValue: self.fileType!)!)
      if (parser?.root == nil) {
        Swift.print("Document " + ((self.fileURL?.absoluteString) ?? "Untitled".locale) + " was ureadable")
        parser = nil // will beep
      }
    } else if self.data == nil && self.fileType != nil { // loading an empty document
      // Empty document??
      parser = PlistParser(fromPath: "")
    }
    
    if parser != nil {
      if parser!.isBinary {
        self.fileType = PEFileType.binaryPlistv1.rawValue
      }
      
      self.windowController = PlistEditorWC.loadFromNib(parser: parser!)
      addWindowController(self.windowController!)
      if (self.fileURL != nil) {
        self.windowController?.window?.representedFilename = (self.fileURL?.path)!
        self.windowController?.window?.setFrameAutosaveName(NSWindow.FrameAutosaveName((self.fileURL?.path)!))
      }
      if !NSDocumentController.shared.documents.contains(self) {
        NSDocumentController.shared.addDocument(self)
      }
      
      if let vc = self.windowController?.contentViewController as? PlistEditorVC {
        contentViewController = vc
        vc.rootNode = parser?.root
      }
    } else {
      NSSound.beep()
    }
  }
  
  override func write(to url: URL, ofType typeName: String) throws {
    if let vc = self.windowController?.contentViewController as? PlistEditorVC {
      switch typeName {
      case PEFileType.xmlPlistv1.rawValue:
        do {
          try vc.save().write(to: url, options: [])
        } catch  {
          throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
        }
        break
      case PEFileType.xml.rawValue:
        do {
          try vc.save().write(to: url, options: [])
          
        } catch  {
          throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
        }
        break
      case PEFileType.binaryPlistv1.rawValue:
        let data = vc.convertToBinaryPlist()
        if (data == nil) {
          throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
        }
        do {
          try data?.write(to: url, options: [])
        } catch  {
          throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
        }
        break
      default:
        throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
      }
    }
  }
  
  override func read(from data: Data, ofType typeName: String) throws {
    if typeName == PEFileType.xmlPlistv1.rawValue ||
      typeName == PEFileType.binaryPlistv1.rawValue ||
      typeName == PEFileType.xml.rawValue {
      self.data = data
    } else {
      throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
    }
  }
}


