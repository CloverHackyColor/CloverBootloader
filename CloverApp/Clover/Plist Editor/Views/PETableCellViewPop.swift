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
//MARK: PETableCellViewPop (NSTableCellView)

@available(OSX 10.11, *)
final class PETableCellViewPop: NSTableCellView {
  var type : PETableCellViewType = .tags
  @IBOutlet var popup : PEPopUpButton!
  
  override init(frame frameRect: NSRect) {
    super.init(frame:frameRect)
    self.wantsLayer = true
    self.canDrawSubviewsIntoLayer = true
    self.canDrawConcurrently = AppSD.canDrawConcurrently
  }
  
  required init(coder: NSCoder) {
    super.init(coder: coder)!
  }
 
  override func draw(_ dirtyRect: NSRect) {
    super.draw(dirtyRect)
  }
  
  func setup(with type: PETableCellViewType, outline: PEOutlineView) {
    self.type = type
    self.popup.outline = outline
    self.popup.target = self.popup?.outline?.editorVC
    switch self.type {
    case .bool:
      self.popup.action = #selector(self.popup.outline!.editorVC!.boolPopUpPressed)
    case .tags:
      self.popup.action = #selector(self.popup.outline!.editorVC!.typePopUpPressed)
    case .value: fallthrough
    case .key:
      break // not here
    }
  }
}

