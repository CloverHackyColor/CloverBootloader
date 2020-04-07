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
final class PEPopUpButton: NSPopUpButton {
  internal var n: PENode?
  internal var o: PEOutlineView?
  
  var node: PENode? {
    get {
      return self.n
    } set {
      self.n = newValue
    }
  }
  
  override public var alignmentRectInsets: NSEdgeInsets {
    return NSEdgeInsetsMake(0, 0, 0, 0)
  }
  
  override func willOpenMenu(_ menu: NSMenu, with event: NSEvent) {
    if (self.outline != nil) {
      /*
       the following is needed when user click on the type column:
       He can have ongoing edits on some rows, so that changing first responder
       We can aesily inform the outline that We're playing with something else
       and have to force ask the user in case edits are wrong.
       This also prevent a crash.
       */
      if (self.outline?.wrongValue)! {
        menu.cancelTrackingWithoutAnimation() // this grant the popup menu to not showup
      }
      self.window?.makeFirstResponder(self)
      self.window?.makeFirstResponder(self.outline)
      /*
       Also consider that PlistEditor() will resign first responder the PETextField involved,
       just to ensure that user will not be able to leave fields with invalid datas
       */
    }
  }
  
  var outline: PEOutlineView? {
    get {
      return self.o
    } set {
      self.o = newValue
    }
  }
  
  func setAsBool() {
    self.removeAllItems()
    self.addItems(withTitles: [localizedNo, localizedYes])
  }
  
  func setAsRoot() {
    self.removeAllItems()
    self.addItem(withTitle: gPlistTagStr(tag: .Dictionary).locale)
    self.lastItem?.representedObject = PlistTag.Dictionary
    self.addItem(withTitle: gPlistTagStr(tag: .Dictionary).locale)
    self.lastItem?.representedObject = PlistTag.Dictionary
  }
  
  func setAsAllType() {
    self.removeAllItems()
    self.addItem(withTitle: gPlistTagStr(tag: .Dictionary).locale)
    self.lastItem?.representedObject = PlistTag.Dictionary
    self.addItem(withTitle: gPlistTagStr(tag: .Array).locale)
    self.lastItem?.representedObject = PlistTag.Array
    self.addItem(withTitle: gPlistTagStr(tag: .String).locale)
    self.lastItem?.representedObject = PlistTag.String
    self.addItem(withTitle: gPlistTagStr(tag: .Number).locale)
    self.lastItem?.representedObject = PlistTag.Number
    self.addItem(withTitle: gPlistTagStr(tag: .Bool).locale)
    self.lastItem?.representedObject = PlistTag.Bool
    self.addItem(withTitle: gPlistTagStr(tag: .Data).locale)
    self.lastItem?.representedObject = PlistTag.Data
    self.addItem(withTitle: gPlistTagStr(tag: .Date).locale)
    self.lastItem?.representedObject = PlistTag.Date
  }
}

class PEPopUpButtonCell: NSPopUpButtonCell {
  override func drawTitle(_ title: NSAttributedString,
                          withFrame frame: NSRect,
                          in controlView: NSView) -> NSRect {
    
    /*
     overriding this method give to the title color the right white color
     instead of gray under dark mode. Can change in future so, again, the following method
     will help adjusting the title color. For now just returning super!
     */
    return super.drawTitle(title, withFrame:frame, in:controlView)
    /*
    var attributedTitle = title
    
    if let popUpButton = self.controlView as? PEPopUpButton {
      if let object = popUpButton.selectedItem?.representedObject as? Dictionary<String, String> {
        if let shortTitle = object["shortTitle"] {
          attributedTitle = NSAttributedString(string:shortTitle, attributes:title.attributes(at:0, effectiveRange:nil))
        }
      }
    }
    return super.drawTitle(attributedTitle, withFrame:frame, in:controlView)
    */
  }
}

