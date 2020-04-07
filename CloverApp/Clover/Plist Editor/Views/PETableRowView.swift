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

enum RowViewBorderType : Int {
  case none               = 0
  case verticalOnly       = 2
  case verticalAndBottom  = 3
}

@available(OSX 10.11, *)
final class PETableRowView: NSTableRowView {
  internal var n: PENode?
  internal var o: PEOutlineView? = nil
  
  override init(frame frameRect: NSRect) {
    super.init(frame: frameRect)
    self.wantsLayer = true
    self.canDrawSubviewsIntoLayer = true
    self.canDrawConcurrently = AppSD.canDrawConcurrently
    self.isEmphasized = true
    //self.postsBoundsChangedNotifications = true
  }

  required init?(coder decoder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }
  
  var outline: PEOutlineView? {
    get {
      return self.o
    } set {
      self.o = newValue
    }
  }
  
  var node: PENode? {
    get {
      return self.n
    } set {
      self.n = newValue
    }
  }
  
  func setBorderType() {
    super.setNeedsDisplay(self.bounds)
  }

  override func draw(_ dirtyRect: NSRect) {
    super.draw(dirtyRect)
    var bt : RowViewBorderType = .none
    if self.isSelected {
      self.highLightAllBorders(in: dirtyRect)
    } else {
      let row : Int = self.outline?.selectedRow ?? -1
      if row >= 0 {
        if let selNode = self.outline?.item(atRow: row) as? PENode {
          // determine if self.node is a child (or sub child of selfNode)
          let ipsc = "\(self.node!.indexPath)".trimmingCharacters(in: CharacterSet(arrayLiteral: "]"))
          let ips = "\(selNode.indexPath)".trimmingCharacters(in: CharacterSet(arrayLiteral: "]")) + ","
          if ipsc.hasPrefix(ips) {
            bt = .verticalOnly
            let childs = selNode.mutableChildren
            if childs.count > 0 && (childs.lastObject as! PENode) == self.node {
              bt = .verticalAndBottom
            }
          }
        }
      }
    }
    
    switch bt {
    case .none: break // taken into account previously
    case.verticalAndBottom:
      self.highLightLeftBorder(in: dirtyRect)
      self.highLightRightBorder(in: dirtyRect)
      self.highLightBottomBorder(in: dirtyRect)
    case .verticalOnly:
      self.highLightLeftBorder(in: dirtyRect)
      self.highLightRightBorder(in: dirtyRect)
    }
  }
  
  private func borderColor() -> NSColor {
    return NSColor.alternateSelectedControlColor
  }
  
  private func lineWidth() -> CGFloat {
    return 2.5
  }
  
  private func highLightAllBorders(in dirtyRect: NSRect) {
    let origin = NSMakePoint(dirtyRect.origin.x, dirtyRect.origin.y)
    var rect = NSRect.zero
    rect.origin = origin
    rect.size.width = dirtyRect.size.width
    rect.size.height = dirtyRect.size.height
    let path: NSBezierPath = NSBezierPath(rect: rect)
    path.lineWidth = self.lineWidth()
    self.borderColor().set()
    path.stroke()
    
  }
  
  private func highLightRightBorder(in dirtyRect: NSRect) {
    let origin = NSMakePoint(dirtyRect.origin.x, dirtyRect.origin.y)
    var rect = NSRect.zero
    rect.origin = origin
    rect.size.width = dirtyRect.size.width
    rect.size.height = dirtyRect.size.height
    let path: NSBezierPath = NSBezierPath()
    path.lineWidth = self.lineWidth()
    path.move(to: NSMakePoint(rect.size.width - 1, 0))
    // draw right vertical side
    path.line(to: NSMakePoint(rect.size.width - 1, rect.size.height))
    self.borderColor().set()
    path.stroke()
  }
  
  private func highLightLeftBorder(in dirtyRect: NSRect) {
    let origin = NSMakePoint(dirtyRect.origin.x, dirtyRect.origin.y)
    var rect = NSRect.zero
    rect.origin = origin
    rect.size.width = dirtyRect.size.width
    rect.size.height = dirtyRect.size.height
    let path: NSBezierPath = NSBezierPath()
    path.lineWidth = self.lineWidth()
    path.move(to: NSMakePoint(1, 0))
    // draw right vertical side
    path.line(to: NSMakePoint(1, rect.size.width))
    self.borderColor().set()
    path.stroke()
  }
  
  private func highLightBottomBorder(in dirtyRect: NSRect) {
    let origin = NSMakePoint(dirtyRect.origin.x, dirtyRect.origin.y)
    var rect = NSRect.zero
    rect.origin = origin
    rect.size.width = dirtyRect.size.width
    rect.size.height = dirtyRect.size.height
    let path: NSBezierPath = NSBezierPath()
    path.lineWidth = self.lineWidth()
    path.move(to: NSMakePoint(0, rect.size.height))
    // draw orizzontal-bottom side
    path.line(to: NSMakePoint(rect.size.width, rect.size.height))
    self.borderColor().set()
    path.stroke()
  }
  
  private func isParent() -> Bool {
    let type : PlistTag = self.node!.tagdata!.type
    return type == .Array || type == .Dictionary
  }
  
  private func getParent(of node: PENode?) -> PENode? {
    if let parent : PENode = node?.parent as? PENode {
      let root : PENode? = self.outline?.item(atRow: 0) as? PENode
      if parent != root {
        return parent
      }
    }
    return nil
  }
}

