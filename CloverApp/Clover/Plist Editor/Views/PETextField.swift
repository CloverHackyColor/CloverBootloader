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

//MARK: PETextField (NSTextField)
@available(OSX 10.11, *)
final class PETextField: NSTextField, NSTextViewDelegate {
  var outline : PEOutlineView? = nil
  var node : PENode? = nil
  var column : Int = -1
  
  override var intrinsicContentSize: NSSize {
    get {
      return NSMakeSize(self.frame.size.width, super.intrinsicContentSize.height)
    }
  }
  
  override var lineBreakMode: NSLineBreakMode {
    get {
      return .byTruncatingTail
    }
    set {
      super.lineBreakMode = .byTruncatingTail
    }
  }
  
  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    self.wantsLayer = true
    self.canDrawConcurrently = AppSD.canDrawConcurrently
    self.cell = PETextFieldCell()
    
  }
  
  required init?(coder: NSCoder) {
    super.init(coder: coder)
  }
  
  override func mouseDown(with event: NSEvent) {
    // doing nothing
  }
  
  override func textShouldBeginEditing(_ textObject: NSText) -> Bool {
    return (self.outline?.editorVC?.isEditable)!
  }
  
  override func textShouldEndEditing(_ textObject: NSText) -> Bool {
    return true
  }
  
  func textViewDidChangeSelection(_ notification: Notification) {
    if let cur = self.currentEditor() {
      if cur.selectedRange.length > 0 {
        self.textColor = NSColor.controlTextColor
      }
    }
  }
  
  override func updateLayer() {
    super.updateLayer()
    //if Thread.isMainThread {
    //  self.invalidateIntrinsicContentSize()
    //}
  }
}

@available(OSX 10.11, *)
final class PETextFieldCell : NSTextFieldCell {
 
  override func drawingRect(forBounds rect: NSRect) -> NSRect {
    let height : CGFloat = 14//rect.size.height
    let y = ((rect.size.height - height) / 2)
    let newRect = NSRect(x: 0, y: y, width: rect.size.width, height: height)
    return super.drawingRect(forBounds: newRect)
  }
}

