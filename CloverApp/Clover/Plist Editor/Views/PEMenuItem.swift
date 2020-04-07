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

final class PEMenuItem: NSMenuItem {
  internal var sep : Bool = false
  internal var d : String?
  public var isKey : Bool = false
  public var keyPath : String = ""
  public var node : PENode? = nil
  
  public var useEditor : Bool = false
  
  override var isSeparatorItem: Bool {
    get {
      return self.sep
    } set {
      self.sep = newValue
    }
  }
  
  var explanation: String? {
    get {
      return self.d
    } set {
      self.d = newValue
    }
  }
  
  override func copy(with zone: NSZone? = nil) -> Any {
    let copy = PEMenuItem()
    copy.menu = nil // nil allow to reuse this Item somewhere else..
    copy.title = self.title
    copy.keyEquivalent = self.keyEquivalent
    copy.image = self.image?.copy() as? NSImage
    copy.onStateImage = self.onStateImage.copy() as? NSImage
    copy.offStateImage = self.offStateImage?.copy() as? NSImage
    copy.mixedStateImage = self.mixedStateImage.copy() as? NSImage
    copy.representedObject = self.representedObject
    copy.submenu = self.submenu?.copy() as? NSMenu;
    copy.toolTip = self.toolTip;
    copy.target = self.target;
    copy.explanation = self.explanation
    copy.isKey = self.isKey
    copy.keyPath = self.keyPath
    copy.node = self.node
    copy.useEditor = self.useEditor
    copy.action = self.action
    return copy
  }
}


