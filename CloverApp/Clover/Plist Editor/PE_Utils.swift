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

import Foundation

/**
 Assign a unique name for newNode if parent already contains it.
 - Parameter parent: the parent node in which newNode will be inserted.
 - Parameter newNode: the new node for which it's key must be unique.
 - Discussion:  Call this method before insert new nodes in the parent node.
 */
func gDeduplicateKeyInParent(parent: PENode, newNode: PENode) {
  /* making an array of existing keys */
  var actualKeys = [String]()
  for item in parent.mutableChildren {
    actualKeys.append(((item as! PENode).tagdata?.key)!)
  }
  
  /* new way */
  let newKey : String = newNode.tagdata!.key
  newNode.tagdata!.key = gProposedNewItem(with: newKey, in: actualKeys)
}

/**
 Propose a new key name for a node Dictionary's keys.
 - Parameter key: the actual key of the node or the desired one.
 - Parameter array: an array of String containing all the keys for the target node's childrens.
 - Returns: a `String` with a unique name for the given array. Will return the same string if the given array doesn't contains it.
 - Discussion:  A dictioanry cannot have duplicate keys. This method ensure that a given key will be unique by adding a numeric suffix (key - n).
 */
func gProposedNewItem(with key: String, in array: [String]) -> String {
  var i : Int = 0
  var proposed : String = key
  
  repeat {
    if !array.contains(proposed) {
      break
    }
    proposed = "\(key) - \(i)"
    i+=1
  } while true
  
  return proposed
}

/**
 A string representation for a given `PlistTag`.
 - Parameter tag: a `PlistTag`.
 - Returns: a `String` object.
 - Discussion:  the returned string can be localized which is the main purpose.
 */
func gPlistTagStr(tag: PlistTag) -> String {
  switch tag {
  case .Dictionary:
    return "Dictionary"
  case .Array:
    return "Array"
  case .String:
    return "String"
  case .Number:
    return "Number"
  case .Bool:
    return "Bool"
  case .Date:
    return "Date"
  case .Data:
    return "Data"
  }
}

/**
 A `PlistTag` representation for a given `PlistTag`.
 - Parameter str: a `String` which must be no other than `Dictionary`, `Array`, `String`, `Number`, `Bool`,
 `Date` or `Data`.
 - Returns: a `String` object.
 - Discussion:  A `fatalError` occours if the given str is not avalid `PlistTag`.
 */
func gPlistTag(from str: String) -> PlistTag {
  switch str {
  case "Dictionary":
    return .Dictionary
  case "Array":
    return .Array
  case "String":
    return .String
  case "Number":
    return .Number
  case "Bool":
    return .Bool
  case "Date":
    return .Date
  case "Data":
    return .Data
  default:
    fatalError("plistTag(from str: String): unsupported str parameter \(str)")
  }
}

/**
 Loading a plist file.
 - Parameter path: a `String` representing the path to a loadable plist file.
 - Discussion:  The insternal Plist Editor is used for loading plist files while in 10.9/10.10, in order, PlistEdit Pro or Xcode are used.
 In 10.9/10.10 if mentioned programs aren't present the directory containing the desired file will be opened by the Finder.
 */
func loadPlist(at path: String) {
  if #available(OSX 10.10, *) {
    let dc = NSDocumentController.shared
    dc.openDocument(withContentsOf: URL(fileURLWithPath: path), display: true) {
      (document, documentWasAlreadyOpen, error) in
      if error != nil {
        AppSD.setActivationPolicy()
        print(error!.localizedDescription)
        NSSound.beep()
      } else {
        if (document != nil) {
          dc.addDocument(document!)
        }
      }
    }
  } else {
    // Use reccomended programs (hope) to avoid Text Edit
    var success = NSWorkspace.shared.openFile(path, withApplication: "PlistEdit Pro")
    if !success {
      success = NSWorkspace.shared.openFile(path, withApplication: "Xcode")
    }
    /* will it be Text Edit???
     if !success {
     success = NSWorkspace.shared.openFile(path)
     }*/
    
    if !success { // open the directory path
      success = NSWorkspace.shared.openFile(path.deletingLastPath)
    }
    AppSD.setActivationPolicy()
  }
}
