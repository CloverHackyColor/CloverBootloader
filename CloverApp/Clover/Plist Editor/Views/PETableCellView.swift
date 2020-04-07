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
import CoreData

//MARK: PETableCellView (NSTableCellView)
@available(OSX 10.11, *)
final class PETableCellView: NSTableCellView {
  @IBOutlet var addButton : NSButton!
  @IBOutlet var removeButton : NSButton!
  @IBOutlet var buttonsView: NSView!
  @IBOutlet var buttonsViewWidthConstraint: NSLayoutConstraint!
  
  var trackingArea: NSTrackingArea?
  var scroller : NSScrollView? = nil
  var outline : PEOutlineView?
  var node : PENode?
  var type : PETableCellViewType = .key
  
  var field: PETextField? {
    get {
      return self.textField as? PETextField
    }
    set {
      self.textField = newValue
    }
  }
  
  
  func setup(outline: PEOutlineView, column: Int, type: PETableCellViewType) {
    // actions
    if type == .key {
      self.addButton.ignoresMultiClick = true
      self.addButton.target = self
      self.addButton.action = #selector(self.addNewItem)
      
      self.removeButton.ignoresMultiClick = true
      self.removeButton.target = self
      self.removeButton.action = #selector(self.removeItem)
    }
    
    self.type = type
    self.outline = outline
    self.field?.column = column
    self.field?.delegate = self.outline?.editorVC
    self.field?.outline = self.outline
    self.scroller = outline.enclosingScrollView
  }
  
  @objc func reloadItemInParent(_ aNotification: Notification) {
    let parent = self.node?.peparent
    let ro : TagData? = parent?.tagdata
    
    if (parent != nil) && ((ro != nil)) && (ro!.type == .Array) {
      let childIndex : Int = parent!.mutableChildren.index(of: self.node!)
      DispatchQueue.main.async {
        self.textField?.animator().stringValue = "Item \(childIndex)"
      }
    }
  }
  
  override var backgroundStyle: NSView.BackgroundStyle {
    didSet {
       if #available(OSX 10.13, *) { /* do nothing */ } else {
        if self.backgroundStyle == .light {
          self.textField?.textColor = NSColor.controlTextColor
        } else if self.backgroundStyle == .dark {
          self.textField?.textColor = NSColor.alternateSelectedControlTextColor
        }
       }
    }
  }
  
  @objc func addNewItem() {
    if (self.node != nil) && (self.node?.parent != nil) && !(self.outline?.wrongValue)! {
      self.outline?.addNewItemFromCell(node: self.node!, parent: self.node?.parent as! PENode)
      self.hideButtons()
    } else {
      //
      NSSound.beep()
      self.window?.makeFirstResponder(self.outline)
    }
  }
  
  @objc func removeItem() {
    let parent  = self.node?.parent
    if (self.node != nil) && (parent != nil) && !(self.outline?.wrongValue)!  {
      self.outline?.removeItemFromCell(node: self.node!, parent: parent as! PENode)
    } else {
      NSSound.beep()
      self.window?.makeFirstResponder(self.outline)
    }
  }
  
  @objc func hideButtons() {
    self.buttonsViewWidthConstraint.constant = 0
  }
  
  override func mouseDown(with theEvent: NSEvent) { }
  
  override func mouseUp(with theEvent: NSEvent) { }

  override func mouseEntered(with theEvent: NSEvent) {
    if self.type == .key &&
      self.outline?.editorVC != nil &&
      self.outline!.editorVC!.isEditable &&
      self.window!.isKeyWindow {
      self.buttonsViewWidthConstraint.constant = 46
    }
  }
  
  override func mouseExited(with event: NSEvent) {
    self.hideButtons()
  }
  
  override func rightMouseDown(with event: NSEvent) { }

  override func updateTrackingAreas() {
    if self.type == .key {
      if self.trackingArea != nil {
        self.removeTrackingArea(self.trackingArea!)
      }
      
      self.trackingArea = NSTrackingArea(rect: self.bounds,
                                         options: [.activeAlways,/*.activeInKeyWindow, */.mouseEnteredAndExited] ,
                                         owner: self, userInfo: nil)
      
      self.addTrackingArea(self.trackingArea!)
    }
  }
}

