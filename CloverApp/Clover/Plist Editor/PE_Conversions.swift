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

//MARK: PENode to Plist
///Conver the given PENode to a XML Property List v1
func gConvertPENodeToPlist(node: PENode?) -> String {
  var plist : String = ""
  if (node == nil) {
    return plist
  }
  
  let ro = node!.tagdata!
  let type = node!.tagdata!.type
  if type == .Dictionary {
    if node!.count == 0 {
      plist = xml1Header + "\n" + "<dict/>" + "\n" + xml1Footer
    } else {
      plist = xml1Header + "\n" + gSerializeNodeToPlist(node: node!, file: "", indentation: 0) + xml1Footer
    }
  } else if type == .Array {
    if node!.count == 0  {
      plist = xml1Header + "\n" + "<array/>" + "\n" + xml1Footer
    } else {
      plist = xml1Header + "\n" + gSerializeNodeToPlist(node: node!, file: "", indentation: 0) + xml1Footer
    }
  } else if type == .String {
    plist = xml1Header + "\n" + "<string>" + (ro.value as! String) + "</string>" + "\n" + xml1Footer
  } else if type == .Data {
    let data = ro.value as! NSData
    let strBase64 : String = (data as Data).base64EncodedString(options: .endLineWithLineFeed)
    plist = xml1Header + "\n" + "<data>" + strBase64 + "</data>" + "\n" + xml1Footer
  } else if type == .Date {
    plist = xml1Header + "\n" + "<date>" + utcDateToString(ro.value as! Date) + "</date>" + "\n" + xml1Footer
  } else if type == .Number {
    let strNum = "\(node!.tagdata!.value!)"
    if (strNum as NSString).range(of: ".").location != NSNotFound {
      plist = xml1Header + "\n" + "<real>" + strNum + "</real>" + "\n" + xml1Footer
    } else {
      plist = xml1Header + "\n" + "<integer>" + strNum + "</integer>" + "\n" + xml1Footer
    }
  } else if type == .Bool {
    let n = node!.tagdata!.value as! NSNumber
    plist = xml1Header + "\n" + (n.boolValue ? "<true/>" : "<false/>") + "\n" + xml1Footer
  }
  
  return plist
}

///This func is meant to be called only by the gConvertPENodeToPlist(node: ) function
func gSerializeNodeToPlist(node: PENode, file: String, indentation: Int) -> String {
  var str : String = ""
  var indent : String = ""
  // isRoot means that is still empty and we need to add the open/close tag for the Dictionary (or array)
  let isRoot : Bool = (node.isRootNode == nil) ? false : node.isRootNode!
  var i : Int = indentation <= 0 ? 1 : indentation
  let indentChild : Int = isRoot ? i : i + 1
  while i != 0 {
    indent = indent + "\t"
    i -= 1
  }
  
  let type = node.tagdata!.type
  if type == .Dictionary {
    if !isRoot {
      if node.peparent != nil && node.peparent!.tagdata!.type != .Array {
        str += indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
      }
    } else {
      str = "<dict>\n"
    }
    if node.mutableChildren.count > 0 {
      /*
       <key>New item</key>
       <dict>
       <key>test</key>
       <string>hi</string>
       </dict>
       */
      if !isRoot {
        str = str + indent + "<dict>" + "\n"
      }
      for child in node.mutableChildren {
        str = str + gSerializeNodeToPlist(node: child as! PENode, file: str, indentation: indentChild)
      }
      if !isRoot {
        str = str + indent + "</dict>" + "\n"
      }
    } else {
      /*
       <key>New item</key>
       <dict/>
       */
      str = str + indent + "<dict/>" + "\n"
    }
  } else if type == .Array {
    if !isRoot {
      str += indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
    } else {
      str = "<array>\n"
    }
    if node.mutableChildren.count > 0 {
      /*
       <key>New item</key>
       <array>
       <string>hi</string>
       </array>
       */
      if !isRoot {
        str = str + indent + "<array>" + "\n"
      }
      for child in node.mutableChildren {
        str = str + gSerializeNodeToPlist(node: child as! PENode, file: str, indentation: indentChild)
      }
      if !isRoot {
        str = str + indent + "</array>" + "\n"
      }
    } else {
      /*
       <key>New item</key>
       <array/>
       */
      str = str + indent + "<array/>" + "\n"
    }
  } else if type == .String {
    /*
     <key>test</key>
     <string>hi</string>
     */
    if node.peparent!.tagdata!.type != .Array {
      str = str + indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
    } else if isRoot {
      str = str + indent + "<key>" + localizedNewItem + "</key>" + "\n"
    }
    str = str + indent + "<string>" + (node.tagdata!.value as! String) + "</string>" + "\n"
  } else if type == .Data {
    if node.peparent!.tagdata!.type != .Array {
      str = str + indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
    } else if isRoot {
      str = str + indent + "<key>" + localizedNewItem + "</key>" + "\n"
    }
    let data = node.tagdata!.value as! NSData
    let strBase64 : String = (data as Data).base64EncodedString(options: .endLineWithLineFeed)
    str = str + indent + "<data>" + strBase64 + "</data>" + "\n"
  } else if type == .Date {
    if node.peparent!.tagdata!.type != .Array {
      str = str + indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
    } else if isRoot {
      str = str + indent + "<key>" + localizedNewItem + "</key>" + "\n"
    }
    str = str + indent + "<date>" + utcDateToString(node.tagdata!.value as! Date) + "</date>" + "\n"
  } else if type == .Number {
    if node.peparent!.tagdata!.type != .Array {
      str = str + indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
    } else if isRoot {
      str = str + indent + "<key>" + localizedNewItem + "</key>" + "\n"
    }
    let strNum = "\(node.tagdata!.value!)"
    if (strNum as NSString).range(of: ".").location != NSNotFound {
      // <real>1.2</real>
      str = str + indent + "<real>" + strNum + "</real>" + "\n"
    } else {
      // <integer>1</integer>
      str = str + indent + "<integer>" + strNum + "</integer>" + "\n"
    }
  } else if type == .Bool {
    if node.peparent!.tagdata!.type != .Array {
      str = str + indent + "<key>" + node.tagdata!.key + "</key>" + "\n"
    } else if isRoot {
      str = str + indent + "<key>" + localizedNewItem + "</key>" + "\n"
    }
    let n = node.tagdata!.value as! NSNumber
    str += indent + (n.boolValue ? "<true/>" : "<false/>") + "\n"
  }
  
  if isRoot {
    if node.tagdata!.type == .Array {
      str =  str + "</array>\n"
    } else {
      str = str + "</dict>\n"
    }
  }
  
  return str
}
