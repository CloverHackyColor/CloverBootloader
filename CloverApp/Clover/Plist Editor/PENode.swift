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

final class PENode: NSTreeNode, NSCopying, NSCoding {
  var pattern : String? = nil // -> this isn't copied with .copy()
  internal var ro: Any?
  var isRootNode : Bool? = nil
  
  var highLightPattern: String? {
    get {
      return self.pattern
    }
    set {
      self.pattern = newValue
    }
  }
  
  var count: Int {
    get {
      return (self.children == nil) ? 0 : self.children!.count
    }
  }
  
  override var representedObject: Any? {
    get {
      return self.ro
    } set {
      self.ro = newValue
    }
  }
  
  var isExpandable: Bool {
    get {
      return (self.tagdata?.type == .Dictionary || self.tagdata?.type == .Array)
    }
  }
  
  weak var peparent: PENode? {
    get {
      return self.parent as? PENode
    }
  }
  
  var tagdata: TagData? {
    get {
      return (self.representedObject as! TagData)
    } set {
      self.representedObject = newValue
    }
  }
  
  required convenience init(coder decoder: NSCoder) {
    self.init(representedObject:decoder.decodeObject(forKey: "TagData"))
    self.mutableChildren.addObjects(from: decoder.decodeObject(forKey: "childrens") as! [Any])
  }
  
  func encode(with coder: NSCoder) {
    coder.encode(self.representedObject, forKey: "TagData")
    coder.encode(self.mutableChildren, forKey: "childrens")
  }
  
  required override init(representedObject modelObject: Any?) {
    self.ro = modelObject
    super.init(representedObject: self.ro)
  }
  
  func copy(with zone: NSZone? = nil) -> Any {
    let nodeCopy = PENode(representedObject: self.tagdata!.copy())
    self.reAddChildToParent(newNode: nodeCopy, origNode: self)
    return nodeCopy
  }
  
  private func reAddChildToParent(newNode: PENode, origNode: PENode)  {
    for item in origNode.mutableChildren {
      let origChild = item as! PENode
      let nodeCopy = PENode(representedObject: origChild.tagdata!.copy())
      newNode.mutableChildren.add(nodeCopy)
      self.reAddChildToParent(newNode: nodeCopy, origNode: origChild)
    }
  }
}

