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

//MARK: TagData
final class TagData: NSObject, NSCoding, NSCopying {
  var key : String
  var type : PlistTag
  var value : Any?
  var placeHolder : String
  
  required init(key: String, type: PlistTag, value: Any?) {
    self.key   = key
    self.type  = type
    self.value = value
    self.placeHolder = ""
    super.init()
    
    self.type  = type
    
    // don't hold the value of contenitors, ....save the memory!!!!
    if self.type == .Array {
      self.value = nil
    } else if self.type == .Dictionary {
      self.value = nil
    }
  }
  
  // Welocome to NSCoding!
  required init(coder decoder: NSCoder) {
    self.key = decoder.decodeObject(forKey: "key") as? String ?? ""
    self.type = PlistTag(rawValue: (decoder.decodeObject(forKey: "type") as! NSNumber).intValue) ?? .String
    
    let val = decoder.decodeObject(forKey: "value")
    switch self.type {
    case .String:
      self.value = val as! String
    case .Dictionary:
      self.value = nil
    case .Array:
      self.value = nil
    case .Number:
      if val is PEInt {
        self.value = val as! PEInt
      } else {
        self.value = val as! PEReal
      }
    case .Bool:
      self.value = val as! Bool
    case .Date:
      self.value = val as! Date
    case .Data:
      self.value = val as! Data
    }
    
    self.placeHolder = ""
  }
  
  func encode(with coder: NSCoder) {
    coder.encode(self.key, forKey: "key")
    coder.encode(NSNumber(value: self.type.rawValue), forKey: "type")
    coder.encode(self.value, forKey: "value")
  }
  
  func copy(with zone: NSZone? = nil) -> Any {
    let keyCopy = "\(self.key)"
    let typeCopy = PlistTag.init(rawValue: self.type.rawValue)
    let valCopy = (self.value as AnyObject).copy()
 
    return TagData(key: keyCopy, type: typeCopy!, value: valCopy)
  }
}

