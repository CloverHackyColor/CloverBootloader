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

//MARK: PEOutlineView (NSOutlineView)
@available(OSX 10.11, *)
final class PEOutlineView: NSOutlineView, NSMenuDelegate {
  var wrongValue : Bool = false
  var scrollTimer : Timer? = nil
  var editorVC : PlistEditorVC?

  override func drawGrid(inClipRect clipRect: NSRect) {
    let nRow = self.numberOfRows
    if nRow > 0 {
      let lastRowRect = self.rect(ofRow: nRow - 1)
      let myClipRect = NSMakeRect(0, 0, lastRowRect.size.width, lastRowRect.size.height)
      let finalClipRect : NSRect = NSIntersectionRect(clipRect, myClipRect)
      super.drawGrid(inClipRect: finalClipRect)
    }
  }
  
  override func scrollWheel(with event: NSEvent) {
    super.scrollWheel(with: event)
    NotificationCenter.default.post(name: PEOutLineViewEndScrolling, object: nil)
    
    if (self.scrollTimer != nil) {
      if self.scrollTimer!.isValid {
        self.scrollTimer?.invalidate()
      }
      self.scrollTimer = nil
    }
    
    self.scrollTimer = Timer.scheduledTimer(timeInterval: 0.2,
                                            target: self,
                                            selector: #selector(endScrolling),
                                            userInfo: nil,
                                            repeats: false)
    
  }
  
  @objc func endScrolling() {
    self.enumerateAvailableRowViews { _, row in
      if let cv = self.view(atColumn: 0,
                            row: row,
                            makeIfNecessary: false) as? PETableCellView {
        cv.hideButtons()
        if let rv = cv.superview as? PETableRowView {
          rv.setBorderType()
        }
      }
    }
  }
  
  override func menu(for event: NSEvent) -> NSMenu? {
    let point = self.convert(event.locationInWindow, from: nil)
    let row = self.row(at: point)
    let item = self.item(atRow: row)
    
    if (item == nil) {
      return nil
    }
    
    self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
    return self.getBaseMenu(outlineView: self, for: item!)
  }
  
  func getBaseMenu(outlineView: PEOutlineView, for item: Any) -> NSMenu {
    let node: PENode = item as! PENode
    // get a key path for the node! (like Boot->Arguments-> etc)
    // to do that create an array..
    let keyPaths : NSMutableArray = NSMutableArray()
    var parent: PENode? = node
    
    while (parent != nil) && (parent != outlineView.item(atRow: 0) as? PENode) {
      var key : String = (parent?.ro as! TagData).key
      let type = ((parent?.parent as! PENode).ro as! TagData).type
      if type == .Array {
        // we need the index in this case!
        let index : Int = (parent?.parent as! PENode).mutableChildren.index(of: parent!)
        //key = arrayPrefixId + String(index) + arraySuffixId
        key = localizedItem + " " + String(index)
      }
      if keyPaths.count == 0 {
        keyPaths.add(key)
      } else {
        keyPaths.insert(key, at: 0)
      }
      parent = parent?.parent as? PENode
    }
    
    
    let menu = NSMenu(title: "Contextual Menu")
    let row = outlineView.row(forItem: item)
    if row > 0 && (outlineView.editorVC?.isEditable)! {
      menu.addItem(withTitle: "Cut".locale,
                   action: #selector(outlineView.cut(_:)),
                   keyEquivalent: "X")
    }
    
    menu.addItem(withTitle: "Copy".locale,
                 action: #selector(outlineView.copy(_:)),
                 keyEquivalent: "C")
    
    if (outlineView.editorVC?.isEditable)! {
      menu.addItem(withTitle: "Paste".locale,
                   action: #selector(outlineView.paste(_:)),
                   keyEquivalent: "V")
    } else {
      return menu
    }
    
    return menu
  }
  
  func menu(_ menu: NSMenu, willHighlight item: NSMenuItem?) {
    print("menu(_ menu: NSMenu, willHighlight item: NSMenuItem?)")
  }
  
  /*
   Override the validation for the textfield:
   Don't accept first responder on textfield if the event is called with a menu (left mouse down)
   if the row is not selected don't become first responder!
   if already selected 1 click its enough to begin editing!
   */
  override func validateProposedFirstResponder(_ responder: NSResponder, for event: NSEvent?) -> Bool {
    if responder is PETextField {
      //let field = responder as! PETextField
      if (event != nil) {
        if event?.type == NSEvent.EventType.rightMouseDown {
          return false
        } else if event?.type == NSEvent.EventType.leftMouseDown {
          let selected = self.selectedRow
          if selected > 0 {
            let point = self.convert((event?.locationInWindow)!, from: nil)
            let row = self.row(at: point)
            if self.selectedRow == row {
              return true
            }
            self.selectRowIndexes(IndexSet(integer: selected), byExtendingSelection: false)
          }
          return false
        }
      }
    }
    return super.validateProposedFirstResponder(responder, for: event)
  }
  
  @objc func cut(_: Any) {
    if !(self.editorVC?.isEditable)! {
      NSSound.beep()
      return
    }
    
    let selected = self.selectedRow
    if selected >= 0 {
      // don't cut Root!
      if selected == 0 {
        NSSound.beep()
        return
      }
      if let item = self.item(atRow: selected) {
        let root = PENode(representedObject: TagData(key: "Root",
                                                     type: .Dictionary,
                                                     value: nil))
        let parent = (item as! PENode).parent
        if parent == nil {
          NSSound.beep()
          return
        }
        root.mutableChildren.add((item as! PENode).copy())
        let plist = gConvertPENodeToPlist(node: root)
        
        let pasteboard = NSPasteboard.general
        pasteboard.declareTypes([NSPasteboard.PasteboardType.string], owner: nil)
        pasteboard.setString(plist, forType: NSPasteboard.PasteboardType.string)
        
        // cut
        
        let indexChild = parent?.children?.firstIndex(of: item as! PENode)
        
        self.undo_Cut(item: item as! PENode,
                      inParent:parent as! PENode,
                      indexChild: indexChild!)
      } else {
        NSSound.beep()
      }
    }
    else
    {
      NSSound.beep()
    }
  }
  
  @objc func copy(_: Any) {
    let selected = self.selectedRow
    if selected > 0 {
      let item = self.item(atRow: selected)
      
      let root = PENode(representedObject: TagData(key: "Root",
                                                   type: .Dictionary,
                                                   value: nil))
      root.mutableChildren.add((item! as! PENode).copy())
      let plist = gConvertPENodeToPlist(node: root)
      
      let pasteboard = NSPasteboard.general
      pasteboard.declareTypes([NSPasteboard.PasteboardType.string], owner: nil)
      pasteboard.setString(plist, forType: NSPasteboard.PasteboardType.string)
    } else {
      NSSound.beep()
    }
  }
  
  @objc func paste(_: Any) {
    // paste the NSPasteboardTypeString content, if is a plist and if a row is selected.
    //FIXME: check paste op on the root
    
    if !(self.editorVC?.isEditable)! {
      NSSound.beep()
      return
    }
    let selected = self.selectedRow
    if selected >= 0 {
      let plistType = (self.item(atRow: 0) as! PENode).tagdata!.type
      if plistType == .Dictionary || plistType == .Array {
        let node = self.item(atRow: selected) as! PENode
        let pasteboard = NSPasteboard.general
        
        var newNode : PENode
        let data = pasteboard.string(forType: NSPasteboard.PasteboardType.string)!.data(using: .utf8)!
        let parser = PlistParser(fromData: data, fileType: .xmlPlistv1)
        
        if (parser.root != nil) {
          newNode = parser.root!.mutableChildren[0] as! PENode
        } else {
          let data = TagData(key: localizedNewItem,
                             type: .String,
                             value: pasteboard.string(forType: NSPasteboard.PasteboardType.string)! as NSString)
          newNode = PENode(representedObject: data)
        }
        
        // Are we pasting on Root and is a contenitor?
        if (node == self.item(atRow: 0) as! PENode) {
          if !self.isItemExpanded(node) {
            self.expandItem(node)
          }
          gDeduplicateKeyInParent(parent: node, newNode: newNode)
          self.undo_Paste(item: newNode, inParent: node, indexChild: 0)
        } else {
          let parent = node.parent
          var indexChild = (parent?.children?.firstIndex(of: node))! as Int
          indexChild += 1
          
          // isLeaf strictly means "no children", but Dictionaries and Array are contenitors anyway!
          if node.tagdata!.type == .Array || node.tagdata!.type == .Dictionary {
            let isExpanded = self.isItemExpanded(node)
            if isExpanded {
              gDeduplicateKeyInParent(parent: node, newNode: newNode)
              self.undo_Paste(item: newNode, inParent: node, indexChild: 0)
            } else {
              gDeduplicateKeyInParent(parent: parent as! PENode, newNode: newNode)
              self.undo_Paste(item: newNode, inParent: parent! as! PENode, indexChild: indexChild)
            }
          } else {
            gDeduplicateKeyInParent(parent: parent as! PENode, newNode: newNode)
            self.undo_Paste(item: newNode, inParent: parent! as! PENode, indexChild: indexChild)
          }
        }
      } else {
        // plist is not a dictionary or array
        NSSound.beep()
      }
    } else {
      // no selected row
      NSSound.beep()
    }
  }
  
  //MARK: add/remove functions (called from PETableCellViewKey)
  @objc func addNewItemFromCell(node: PENode, parent: PENode) {
    self.editorVC?.isAddingNewItem = true
    
    let newItemName = localizedNewItem
    let newNode = PENode(representedObject: TagData(key: newItemName,
                                                    type: .String,
                                                    value: "" as NSString))
    var indexChild = (parent.children?.firstIndex(of: node))! as Int
    indexChild += 1
    
    // isLeaf strictly means "no children", but Dictionaries and Array are contenitors anyway!
    if node.tagdata!.type == .Array || node.tagdata!.type == .Dictionary {
      if self.isItemExpanded(node) {
        gDeduplicateKeyInParent(parent: node, newNode: newNode)
        self.undo_Add(item: newNode, inParent: node, indexChild: 0, target: self)
      } else {
        gDeduplicateKeyInParent(parent: parent, newNode: newNode)
        self.undo_Add(item: newNode, inParent: parent, indexChild: indexChild, target: self)
      }
    } else {
      gDeduplicateKeyInParent(parent: parent, newNode: newNode)
      self.undo_Add(item: newNode, inParent: parent, indexChild: indexChild, target: self)
    }
    self.editorVC?.isAddingNewItem = false
  }
  
  func removeItemFromCell(node: PENode, parent: PENode) {
    let indexChild = parent.mutableChildren.index(of: node)
    self.undo_Remove(item: node, inParent: parent, indexChild: indexChild, target: self)
  }
}


