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
extension PlistEditorVC {
  @objc func boolPopUpPressed(sender: PEPopUpButton) {
    let originalValue : Bool = sender.node!.tagdata!.value as! Bool
    let actualValue = (sender.title == localizedYes) ? true : false
    
    if originalValue != actualValue {
      self.outline.undo_SetBoolValue(node: sender.node!,
                                     sender: sender,
                                     originalValue: originalValue,
                                     actualValue: actualValue)
    }
  }

  @objc func typePopUpPressed(sender: PEPopUpButton) {
    let originalType = sender.node!.tagdata!.type
    let actualType = sender.selectedItem?.representedObject as! PlistTag
    if originalType != actualType {
      let newTree : TagData? = sender.node?.tagdata?.copy() as? TagData

      newTree?.type = actualType
      /*
       switching between contenitors types (Array/Dictionary) can save some memory
       since we need don't need to backup the childrens.
       Reloading the view it's enough to show children correctly
       */

      switch originalType {
      case .Dictionary:
        if actualType == .Array {
          var oldKeys : PEArray = PEArray()
          var newKeys : PEArray = PEArray()
          
          var index : Int = 0
          for n in sender.node!.mutableChildren {
            let ex : PENode = n as! PENode
            oldKeys.append(ex.tagdata!.key)
            newKeys.append("\(localizedItem) \(index)")
            index+=1
          }
          
          self.outline.undo_ChangeContenitor(node: sender.node!,
                                             sender: sender,
                                             originalType: gPlistTagStr(tag: originalType),
                                             actualType: gPlistTagStr(tag: actualType),
                                             oldKeys: oldKeys,
                                             newKeys: newKeys)
        } else {
          if actualType == .String {
            newTree?.value = ""
          } else if actualType == .Number {
            newTree?.value = 0
          } else if actualType == .Bool {
            newTree?.value = false
          } else if actualType == .Data {
            newTree?.value = Data()
          } else if actualType == .Date {
            newTree?.value = Date()
          }
          self.outline.undo_ChangeType(node: sender.node!,
                                       newData: newTree!,
                                       oldData: sender.node!.tagdata!,
                                       newChilds: nil,
                                       oldChilds: sender.node!.children,
                                       sender: sender)
        }
        break
      case .Array:
        if actualType == .Dictionary {
          var oldKeys : PEArray = PEArray()
          var newKeys : PEArray = PEArray()
          
          var index : Int = 0
          for n in sender.node!.mutableChildren {
            let _ : PENode = n as! PENode
            oldKeys.append("\(localizedItem) \(index)")
            if index == 0 {
              newKeys.append(localizedNewItem)
            } else {
              newKeys.append("\(localizedNewItem) - \(index)")
            }
            
            index+=1
          }
          self.outline.undo_ChangeContenitor(node: sender.node!,
                                             sender: sender,
                                             originalType: gPlistTagStr(tag: originalType),
                                             actualType: gPlistTagStr(tag: actualType),
                                             oldKeys: oldKeys,
                                             newKeys: newKeys)
        } else {
          if actualType == .String {
            newTree?.value = ""
          } else if actualType == .Number {
            newTree?.value = 0
          } else if actualType == .Bool {
            newTree?.value = false
          } else if actualType == .Data {
            newTree?.value = Data()
          } else if actualType == .Date {
            newTree?.value = Date()
          }
          
          self.outline.undo_ChangeType(node: sender.node!,
                                       newData: newTree!,
                                       oldData: sender.node!.tagdata!,
                                       newChilds: nil,
                                       oldChilds: sender.node!.children,
                                       sender: sender)
        }
        break
      case .String:
        let oldString = sender.node?.tagdata?.value as! String
        if actualType == .Number {
          let nv = localizedNumberFormatter().number(from: oldString) ?? 0
          newTree?.value = nv
        } else if actualType == .Bool {
          // check if old string is compatible with a bool value
          var bv : Bool = false
          let localized = localizedYes
          let firstChar = localized[localized.startIndex]
          if oldString.hasPrefix(String(firstChar).lowercased())
            || oldString.hasPrefix(String(firstChar).uppercased()) {
            bv = true
          }
          newTree?.value = bv
        } else if actualType == .Data {
          // da migliorare (se la stringa era un hex valido usare quella!!)
          // check if old string is compatible with bytes
          var dv : Data? = nil
          
          if isHexStringValid(string: oldString) == "HexSuccess" {
            dv = oldString.hexadecimal()
          }
          
          if (dv == nil) {
            dv = oldString.data(using: String.Encoding.utf8)
          }
          
          if (dv == nil) {
            dv = Data()
          }
          newTree?.value = dv!
        } else if actualType == .Date {
          // check if old string is compatible with date
          var dv : Date? = nil
          dv = localizedStringToDateS(oldString) // first try with "S" version that can be nil
          
          if (dv == nil) {
            dv = funcyDateFromUser(oldString) // secondly try to detect it (NSDataDetector + NSTextCheckingResult)
          }
          if (dv == nil) {
            dv = Date() // no way, init a new one..
          }
          newTree?.value = dv!
        } else if actualType == .Dictionary {
          newTree?.value = nil
        } else if actualType == .Array {
          newTree?.value = nil
        }
        self.outline.undo_ChangeType(node: sender.node!,
                                     newData: newTree!,
                                     oldData: sender.node!.tagdata!,
                                     newChilds: nil,
                                     oldChilds: sender.node!.children,
                                     sender: sender)
        break
      case .Bool:
        if actualType == .String {
          var ns : String = "0"
          if (sender.node?.tagdata?.value as! NSNumber).boolValue {
            ns = "1"
          }
          newTree?.value = ns
        } else if actualType == .Number {
          var nn : Int = 0
          if (sender.node?.tagdata?.value as! NSNumber).boolValue {
            nn = 1
          }
          newTree?.value = nn
        } else if actualType == .Data {
          newTree?.value = Data()
        } else if actualType == .Date {
          newTree?.value = Date()
        } else if actualType == .Dictionary {
          newTree?.value = nil
        } else if actualType == .Array {
          newTree?.value = nil
        }
        self.outline.undo_ChangeType(node: sender.node!,
                                     newData: newTree!,
                                     oldData: sender.node!.tagdata!,
                                     newChilds: nil,
                                     oldChilds: sender.node!.children,
                                     sender: sender)
        break
      case .Number:
        if actualType == .String {
          let str = localizedNumberFormatter().string(from: sender.node!.tagdata!.value as! NSNumber)
          newTree?.value = str ?? ""
        } else if actualType == .Bool {
          var nb : Bool = false
          if (sender.node?.tagdata?.value as! NSNumber).boolValue {
            nb = true
          }
          newTree?.value = nb
        } else if actualType == .Data {
          newTree?.value = Data()
        } else if actualType == .Date {
          newTree?.value = Date()
        } else if actualType == .Dictionary {
          newTree?.value = nil
        } else if actualType == .Array {
          newTree?.value = nil
        }
        self.outline.undo_ChangeType(node: sender.node!,
                                     newData: newTree!,
                                     oldData: sender.node!.tagdata!,
                                     newChilds: nil,
                                     oldChilds: sender.node!.children,
                                     sender: sender)
        break
      case .Data:
        if actualType == .String {
          newTree?.value = "\(sender.node!.tagdata!.value!)"
        } else if actualType == .Bool {
          newTree?.value = false
        } else if actualType == .Number {
          newTree?.value = 0
        } else if actualType == .Date {
          newTree?.value = Date()
        } else if actualType == .Dictionary {
          newTree?.value = nil
        } else if actualType == .Array {
          newTree?.value = nil
        }
        self.outline.undo_ChangeType(node: sender.node!,
                                     newData: newTree!,
                                     oldData: sender.node!.tagdata!,
                                     newChilds: nil,
                                     oldChilds: sender.node!.children,
                                     sender: sender)
        break
      case .Date:
        if actualType == .String {
          let dateStr = localizedDateToString(sender.node!.tagdata!.value as! Date)
          newTree?.value = dateStr
        } else if actualType == .Bool {
          newTree?.value = false
        } else if actualType == .Number {
          newTree?.value = 0
        } else if actualType == .Data {
          newTree?.value = Data()
        } else if actualType == .Dictionary {
          newTree?.value = nil
        } else if actualType == .Array {
          newTree?.value = nil
        }
        self.outline.undo_ChangeType(node: sender.node!,
                                     newData: newTree!,
                                     oldData: sender.node!.tagdata!,
                                     newChilds: nil,
                                     oldChilds: sender.node!.children,
                                     sender: sender)
        break
      }
    }
  }
}

