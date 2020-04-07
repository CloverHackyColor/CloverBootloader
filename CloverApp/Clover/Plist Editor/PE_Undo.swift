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

// MARK: Undo and redo support
@available(OSX 10.11, *)
extension PEOutlineView {
  // MARK: Undo Manager object
  
  /// Prepares PEOutlineView for the undo operation
  func prepareUndoObj() -> AnyObject {
    // nil is not expecte here. Be happy if throw an exception
    return self.undoManager?.prepare(withInvocationTarget: self) as AnyObject
  }
  
  // MARK: Undo drag Item
  /// Undo operation when an Item is dragged here an there in PEOutlineView
  @objc func undo_DragAndDrop(item: PENode,
                              fromParent: PENode,
                              fromIndex: Int,
                              toParent: PENode,
                              toIndex: Int) {
    self.prepareUndoObj().undo_DragAndDrop(item: item,
                                           fromParent: toParent,
                                           fromIndex: toIndex,
                                           toParent: fromParent,
                                           toIndex: fromIndex)
    
    self.undoManager?.setActionName(localizedUndoRedoMoveItem)
    let toParentCount = toParent.mutableChildren.count
    let new = item.copy() as! PENode
    if fromParent == toParent {
      var mutatingToIndex = toIndex
      if fromIndex < toIndex {
        mutatingToIndex -= 1
      }
      fromParent.mutableChildren.removeObject(at: fromIndex)
      fromParent.mutableChildren.insert(new, at: mutatingToIndex)
      
      DispatchQueue.main.async {
        self.reloadItem(fromParent, reloadChildren: true)
      }
    } else {
      self.editorVC?.asyncExpand(item: toParent, expandParentOnly: false)
      // drag and past from to another parent
      if toParentCount == 0 {
        toParent.mutableChildren.add(new)
        DispatchQueue.main.async {
          self.animator().moveItem(at: fromIndex,
                                   inParent: fromParent,
                                   to: toIndex,
                                   inParent: toParent)
        }
      } else {
        DispatchQueue.main.async {
          gDeduplicateKeyInParent(parent:toParent, newNode: new)
          
          toParent.mutableChildren.insert(new, at: toIndex)
          fromParent.mutableChildren.removeObject(at: fromIndex)
          
          
          self.animator().moveItem(at: fromIndex,
                                   inParent: fromParent,
                                   to: toIndex,
                                   inParent: toParent)
          self.reloadItem(toParent, reloadChildren: true)
          let row = self.row(forItem: new)
          if row >= 0 {
            self.scrollRowToVisible(row)
            self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
          }
        }
      }
    }
    
    // select the new dragged item
    DispatchQueue.main.async {
      let toBeSelected = self.row(forItem: new)
      if toBeSelected >= 0 {
        self.scrollRowToVisible(toBeSelected)
        self.selectRowIndexes(IndexSet(integer: toBeSelected), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo replace
  /// Undo operation for Search & Replace  in PEOutlineView
  @objc func undo_FindAndReplace(nodes: NSArray, oldTreeData: NSArray, newTreeData: NSArray) {
    self.prepareUndoObj().undo_FindAndReplace(nodes: nodes,
                                               oldTreeData: newTreeData,
                                               newTreeData: oldTreeData)
    // select and expand, one by one, rows in oldNodes
    self.undoManager?.setActionName(localizedReplace.lowercased())
    
    var index : Int = 0
    for n in nodes {
      let node = n as! PENode
      self.editorVC?.asyncExpand(item: node, expandParentOnly: true)

      let newData = newTreeData.object(at: index) as? TagData
      node.tagdata?.key = (newData?.key)!
      node.tagdata?.value = newData?.value
      DispatchQueue.main.async {
        let row = self.row(forItem: node)
        if row >= 0 {
          self.scrollRowToVisible(row)
          self.reloadData(forRowIndexes: IndexSet(integer: row), columnIndexes: IndexSet(integer: 0))
          self.reloadData(forRowIndexes: IndexSet(integer: row), columnIndexes: IndexSet(integer: 2))
          self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
        }
      }
      index+=1
    }
    
    self.editorVC?.performDelayedSearch()
  }
  
  // MARK: Undo set key string
  /// Undo operation for a key editing (Key column) in PEOutlineView
  @objc func undo_SetKey(node: PENode, newKey: String, oldKey: String) {
    // we are setting the key for a specific row and ther's no need to reload the entire group
    self.prepareUndoObj().undo_SetKey(node: node,
                                      newKey: oldKey,
                                      oldKey: newKey)
    
    self.undoManager?.setActionName(localizedUndoRedoTyping)
    node.tagdata?.key = newKey
    
    self.editorVC?.asyncExpand(item: node, expandParentOnly: true)
    //selecting the involved row, but we need to reload its view? isn't that already happened?
    DispatchQueue.main.async {
      self.reloadItem(node.parent, reloadChildren: false)
      let row = self.row(forItem: node)
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.reloadData(forRowIndexes: IndexSet(integer: row), columnIndexes: IndexSet(integer: 0))
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo Replace Existing key
  /// Undo operation for replacing a duplicate key-value pairs in PEOutlineView
  @objc func undo_ReplaceExisting(item: PENode,
                                  inParent: PENode,
                                  indexChild: Int,
                                  editedNode: PENode,
                                  oldKey: String,
                                  newKey: String) {
    
    self.prepareUndoObj().redo_ReplaceExisting(item: item,
                                               inParent: inParent,
                                               indexChild: indexChild,
                                               editedNode: editedNode,
                                               oldKey: newKey,
                                               newKey: oldKey)
    
    self.undoManager?.setActionName(localizedUndoRedoReplaceDuplicateKey)
    // expand the node if isn't. (an item can be added to a not expanded group??)
    self.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    inParent.mutableChildren.removeObject(at: indexChild)
    DispatchQueue.main.async {
      self.removeItems(at: IndexSet(integer: indexChild),
                       inParent: inParent,
                       withAnimation: [])
      
      editedNode.tagdata?.key = newKey
      
      // keep trak of the item because must be selected after the parent gets reloaded
      let row = self.row(forItem: editedNode)
      
      //reload the parent. In this case the parent it's the same
      //self.outline.reloadItem(inParent, reloadChildren: true)
      
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Redo Replace Existing key
  /// Reversed undo operation (Redo) for replacing a duplicate key-value pairs in PEOutlineView
  @objc func redo_ReplaceExisting(item: PENode,
                                                inParent: PENode,
                                                indexChild: Int,
                                                editedNode: PENode,
                                                oldKey: String,
                                                newKey: String) {
    
    self.prepareUndoObj().undo_ReplaceExisting(item: item,
                                               inParent: inParent,
                                               indexChild: indexChild,
                                               editedNode: editedNode,
                                               oldKey: newKey,
                                               newKey: oldKey)
    
    self.undoManager?.setActionName(localizedUndoRedoReplaceDuplicateKey)
    // expand the node if isn't. (an item can be added to a not expanded group??)
    self.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    
    inParent.mutableChildren.insert(item, at: indexChild)
    editedNode.tagdata?.key = newKey
    DispatchQueue.main.async {
      self.insertItems(at: IndexSet(integer: indexChild),
                       inParent: inParent,
                       withAnimation: [])
      
      
      // keep trak of the item because must be selected after the parent gets reloaded
      let row = self.row(forItem: editedNode)
      
      //reload the parent. In this case the parent it's the same
      self.reloadItem(inParent, reloadChildren: true)
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo set new value
  /// Undo operation (Redo) editing a value (Value Column) in PEOutlineView.
  /// See also undoableSetBoolValue(node: , sender:, originalValue:, actualValue:)
  @objc func undo_SetValue(node: PENode, newValue: Any, oldValue: Any) {
    let row = self.row(forItem: node)
    self.prepareUndoObj().undo_SetValue(node: node,
                                        newValue: oldValue,
                                        oldValue: newValue)
    
    self.editorVC?.asyncExpand(item: node, expandParentOnly: true)
    
    self.undoManager?.setActionName(localizedUndoRedoTyping)
    node.tagdata?.value = newValue
    DispatchQueue.main.async {
      self.reloadItem(node.parent, reloadChildren: false)
      
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.reloadData(forRowIndexes: IndexSet(integer: row), columnIndexes: IndexSet(integer: 2))
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo change to Bool tag
  /// Undo operation (Redo) editing a Bool value (Value Column) in PEOutlineView
  @objc func undo_SetBoolValue(node: PENode,
                                  sender: PEPopUpButton,
                                  originalValue: Bool,
                                  actualValue: Bool) {
    self.prepareUndoObj().undo_SetBoolValue(node: node,
                                            sender: sender,
                                            originalValue: actualValue,
                                            actualValue: originalValue)
    
    self.undoManager?.setActionName(localizedUndoRedoChangeBoolValue)
    node.tagdata?.value = actualValue
    
    // root node cannot have parent and is always visible
    if (node.parent != nil) {
      self.editorVC?.asyncExpand(item: node, expandParentOnly: true)
    }
    
    DispatchQueue.main.async {
      let row = self.row(forItem: node)
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
      
      sender.animator().selectItem(withTitle:
        actualValue ? localizedYes : localizedNo)
    }
  }
  
  // MARK: Undo change tag
  /// Undo operation changing a tag (Tag Column) in PEOutlineView.
  @objc func undo_ChangeType(node: PENode,
                                newData: TagData,
                                oldData: TagData,
                                newChilds: [Any]?,
                                oldChilds: [Any]?,
                                sender: PEPopUpButton) {
    
    self.prepareUndoObj().undo_ChangeType(node: node,
                                          newData: oldData,
                                          oldData: newData,
                                          newChilds: oldChilds,
                                          oldChilds: newChilds,
                                          sender: sender)
    self.undoManager?.setActionName(localizedUndoRedoChangeType)
    
    node.representedObject = newData
    node.mutableChildren.removeAllObjects()
    if let childrens = newChilds {
      node.mutableChildren.addObjects(from: childrens)
    }
    
    // root node cannot have parent and is always visible
    if (node.parent != nil) {
      self.editorVC?.asyncExpand(item: node, expandParentOnly: true)
    }
    
    DispatchQueue.main.async {
      self.reloadItem(node, reloadChildren: true)
      
      let row = self.row(forItem: node)
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo change contenitor (Dictionary to Array and viceversa)
  /// Undo operation changing a Dictionary to Array and viceversa (Tag Column) in PEOutlineView.
  @objc func undo_ChangeContenitor(node: PENode,
                                   sender: PEPopUpButton,
                                   originalType: String,
                                   actualType: String,
                                   oldKeys: PEArray,
                                   newKeys: PEArray) {
    /*
     We cannot leave old keys, but instead we should:
     - Dictionary, must have localized "New Item -x" for each children
     - Array, must be localized "Item x" for each children
     */
    
    self.prepareUndoObj().undo_ChangeContenitor(node: node,
                                                sender: sender,
                                                originalType: actualType,
                                                actualType: originalType,
                                                oldKeys: newKeys,
                                                newKeys: oldKeys)
    
    self.undoManager?.setActionName(localizedUndoRedoChangeType)
    
    // root node cannot have parent and is always visible
    if (node.parent != nil) {
      self.editorVC?.asyncExpand(item: node, expandParentOnly: false)
    }
    
    let actualTag = gPlistTag(from: actualType)

    if actualTag == .Dictionary {
      node.tagdata?.value = nil
      node.tagdata?.type = .Dictionary
    } else if actualTag == .Array {
      node.tagdata?.value = nil
      node.tagdata?.type = .Array
    }
    
    var index : Int = 0
    for n in node.mutableChildren {
      let modded : PENode = n as! PENode
      modded.tagdata?.key = newKeys[index] as! String
      index+=1
    }
    
    DispatchQueue.main.async {
      sender.animator().selectItem(withTitle: gPlistTagStr(tag: actualTag).locale)
    }
    
    self.editorVC?.asyncExpand(item: node, expandParentOnly: true)
    
    DispatchQueue.main.async {
      let row = self.row(forItem: node)
      if self.isItemExpanded(node) {
        self.reloadItem(node, reloadChildren: true)
      } else {
        self.reloadItem(node, reloadChildren: false)
      }
      
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo Paste
  /// Undo operation for "Paste" in PEOutlineView.
  /// See redo_PasteItem(item: , inParent: , indexChild: ) for the reversed operation.
  @objc func undo_Paste(item: PENode, inParent: PENode, indexChild: Int) {
    self.prepareUndoObj().redo_Paste(item: item,
                                     inParent: inParent,
                                     indexChild: indexChild)
    
    self.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    self.undoManager?.setActionName(localizedUndoRedoPasteItem)
    inParent.mutableChildren.insert(item, at: indexChild)
    
    DispatchQueue.main.async {
      self.insertItems(at: IndexSet(integer: indexChild), inParent: inParent, withAnimation: [])
      
      // basically we are adding a new Item. Reload the parent and the cildrens..and select new item
      self.reloadItem(inParent, reloadChildren: true)
      // find new item that should be there now..
      let row = self.row(forItem: item)
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo Paste (reverse)
  /// Redo operation for "Paste" in PEOutlineView.
  /// See undo_PasteItem(item: , inParent: , indexChild: ) for the reversed operation.
  @objc func redo_Paste(item: PENode, inParent: PENode, indexChild: Int) {
    self.prepareUndoObj().undo_Paste(item: item,
                                     inParent: inParent,
                                     indexChild: indexChild)
    
    self.undoManager?.setActionName(localizedUndoRedoPasteItem)
    // expand the node if isn't
    self.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    // find the row. After deleting it, it will be the row to select again (if exist, othewise select last row in the outline)
    DispatchQueue.main.async {
      let row = self.row(forItem: item)
      
      inParent.mutableChildren.removeObject(at: indexChild)
      self.removeItems(at: IndexSet(integer: indexChild), inParent: inParent, withAnimation: [])
      
      // basically we are removing a new Item just added. Reload the parent and the cildrens..
      self.reloadItem(inParent, reloadChildren: true)
      
      //..and select the next row
      if row >= 0 {
        if (self.item(atRow: row) != nil) {
          self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
        } else {
          // row it's out of bounds.. select last in the outline
          let lastRow = self.numberOfRows - 1
          if lastRow >= 0 {
            self.scrollRowToVisible(lastRow)
            self.selectRowIndexes(IndexSet(integer: lastRow), byExtendingSelection: false)
          }
        }
      }
    }
  }
  
  // MARK: Undo Cut
  /// Undo operation for "Cut" in PEOutlineView.
  /// See redo_Cut(item: , inParent: , indexChild: ) for the reversed operation.
  @objc func undo_Cut(item: PENode, inParent: PENode, indexChild: Int) {
    // find the row. After deleting it, it will be the row to select again (if exist, othewise select last row in the outline)
    self.prepareUndoObj().redo_Cut(item: item,
                                   inParent: inParent,
                                   indexChild: indexChild)
    self.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    self.undoManager?.setActionName(localizedUndoRedoCutItem)
    inParent.mutableChildren.removeObject(at: indexChild)
    self.removeItems(at: IndexSet(integer: indexChild), inParent: inParent, withAnimation: [])
    // basically we are removing a new Item just added. Reload the parent and the cildrens..
    
    
    DispatchQueue.main.async {
      self.reloadItem(inParent, reloadChildren: true)
      let row = self.row(forItem: item)
      //..and select the next row
      if row >= 0 {
        if (self.item(atRow: row) != nil) {
          self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
        } else {
          // row it's out of bounds.. select last in the outline
          let lastRow = self.numberOfRows - 1
          if lastRow >= 0 {
            self.scrollRowToVisible(lastRow)
            self.selectRowIndexes(IndexSet(integer: lastRow), byExtendingSelection: false)
          }
        }
      }
    }
    
  }
  
  // MARK: Undo Cut (reverse)
  /// Redo operation for "Cut" in PEOutlineView.
  /// See undo_Cut(item: , inParent: , indexChild: ) for the reversed operation.
  @objc func redo_Cut(item: PENode, inParent: PENode, indexChild: Int) {
    self.prepareUndoObj().undo_Cut(item: item,
                                   inParent: inParent,
                                   indexChild: indexChild)
    
    self.undoManager?.setActionName(localizedUndoRedoCutItem)
    // expand the node if isn't
    self.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    
    // we are adding an item..
    inParent.mutableChildren.insert(item, at: indexChild)
    DispatchQueue.main.async {
      self.insertItems(at: IndexSet(integer: indexChild), inParent: inParent, withAnimation: [])
      
      // select the new one after reloading the group
      self.reloadItem(inParent, reloadChildren: true)
      let row = self.row(forItem: item)
      if row >= 0 {
        self.scrollRowToVisible(row)
        self.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Undo add item
  /// Undo operation adding New items in PEOutlineView.
  /// See undo_Remove(item: , inParent: , indexChild: ) for the reverse operation.
  @objc func undo_Add(item: PENode,
                             inParent: PENode,
                             indexChild: Int,
                             target outlineView: PEOutlineView) {
    outlineView.prepareUndoObj().undo_Remove(item: item,
                                             inParent: inParent,
                                             indexChild: indexChild,
                                             target: outlineView)
    
    outlineView.undoManager?.setActionName(localizedUndoRedoAddNewItem)
    
    outlineView.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    
    inParent.mutableChildren.insert(item, at: indexChild)
    
    DispatchQueue.main.async {
      outlineView.insertItems(at: IndexSet(integer: indexChild), inParent: inParent, withAnimation: [])
      // select the new one after reloading the group
      outlineView.reloadItem(inParent, reloadChildren: true)
      let row = outlineView.row(forItem: item)
      if row >= 0 {
        outlineView.scrollRowToVisible(row)
        outlineView.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      }
    }
  }
  
  // MARK: Redo remove Item
  /// Redo operation adding New items in PEOutlineView.
  /// See undo_Add(item: , inParent: , indexChild: ) for the reverse operation.
  @objc func undo_Remove(item: PENode,
                         inParent: PENode,
                         indexChild: Int,
                         target outlineView: PEOutlineView) {
    // find the row. After deleting it, it will be the row to select again (if exist, othewise select last row in the outline)
    let row = outlineView.row(forItem: item)
    
    outlineView.prepareUndoObj().undo_Add(item: item,
                                          inParent: inParent,
                                          indexChild: indexChild,
                                          target: outlineView)
    
    outlineView.undoManager?.setActionName(localizedUndoRedoRemoveItem)
    // expand the node if isn't
    outlineView.editorVC?.asyncExpand(item: item, expandParentOnly: true)
    inParent.mutableChildren.removeObject(at: indexChild)
    DispatchQueue.main.async {
      outlineView.removeItems(at: IndexSet(integer: indexChild), inParent: inParent, withAnimation: [])
      
      // Reload the parent and the cildrens..
      outlineView.reloadItem(inParent, reloadChildren: true)
      
      //..and select the next row
      if row >= 0 {
        outlineView.scrollRowToVisible(row)
        if (outlineView.item(atRow: row) != nil) {
          outlineView.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
        } else {
          // row it's out of bounds.. select last in the outline
          let lastRow = outlineView.numberOfRows - 1
          if lastRow >= 0 {
            outlineView.selectRowIndexes(IndexSet(integer: lastRow), byExtendingSelection: false)
          }
        }
      }
    }
  }
}

