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
final class PlistEditorVC: NSViewController,
  NSOutlineViewDelegate,
  NSOutlineViewDataSource,
  NSTextFieldDelegate,
NSSearchFieldDelegate, NSSplitViewDelegate {
  @IBOutlet var  findView: NSView!
  @IBOutlet var  editorView: NSView!
  @IBOutlet var scrollView : NSScrollView!
  @IBOutlet var outline : PEOutlineView!
  @IBOutlet var  doneBtn: NSButton?
  @IBOutlet var  showFindViewBtn: FindButton?
  @IBOutlet var  showReplaceBtn: ReplaceButton?
  @IBOutlet var  nextOrPreviousSegment: NSSegmentedControl?
  @IBOutlet var  replaceOneOrAllSegment: NSSegmentedControl?
  @IBOutlet var  searchField: PESearchField?
  @IBOutlet var  replaceField: PEReplaceField?
  
  @IBOutlet var findAndReplaceViewHeightConstraint: NSLayoutConstraint!
  
  var parser : PlistParser?
  var vcLoaded : Bool = false
  var isSearching : Bool = true
  var pbTreeNode : PENode?
  var edited : Bool = false
  var plistPath : URL?
  var searches : [PENode] = [PENode]()
  var isAddingNewItem : Bool = false
  var isEditable : Bool = true
  var rootNode : PENode?
  var searchTimer : Timer? = nil
  var doc : Document? = nil
  
  var document: Document? {
    get {
      self.doc = view.window?.windowController?.document as? Document
      return self.doc
    }
    set {
      self.doc = newValue
    }
  }
  
  deinit {
    NotificationCenter.default.removeObserver(self,
                                              name: PESearchFieldTextDidChange,
                                              object: nil)
  }
  

  override func viewDidLoad() {
    super.viewDidLoad()
    self.findAndReplaceViewHeightConstraint.constant = 0
    
    self.searchField?.placeholderString = localizedSearch
    self.searchField?.countLabel?.stringValue = ""
    
    self.doneBtn?.target = self
    self.doneBtn?.action = #selector(self.doneButtonPressed(_:))
    self.nextOrPreviousSegment?.target = self
    self.nextOrPreviousSegment?.action = #selector(self.segmentScrollerPressed(_:))
    self.replaceOneOrAllSegment?.target = self
    self.replaceOneOrAllSegment?.action = #selector(self.replaceOneOrAllSegmentPressed(_:))
    if self.replaceOneOrAllSegment != nil {
      for i in 0..<self.replaceOneOrAllSegment!.segmentCount {
        let label = self.replaceOneOrAllSegment!.label(forSegment: i)
        self.replaceOneOrAllSegment!.setLabel(label!.locale, forSegment: i)
      }
    }

    /*
     Interface Builder is a mess and doesn't want to constraints the scrollView:
     fu__!, who cares! ..I'll do by my self.
     */
    gAddConstraintsToFit(superView: self.editorView , subView: self.scrollView)
  }
  
  override func viewDidAppear() {
    super.viewDidAppear()
    self.searchField?.delegate = self
    
    if !self.vcLoaded {
      self.outline.target = self
      self.outline.editorVC = self
      self.outline.doubleAction = #selector(self.customDoubleClick)
      self.doc = self.document
      
      if ((self.doc?.fileURL) != nil) {
        self.view.window?.title = (self.doc?.fileURL?.path)!
      }
      // -----------------------------
      
      self.rootNode = self.parser!.root
      
      let type = self.rootNode!.tagdata!.type
      if type == .Dictionary {
        let ro = TagData(key: "Root", type: .Dictionary, value: NSDictionary())
        let root = PENode(representedObject: ro)
        root.mutableChildren.add(self.rootNode!)
        self.rootNode = root
      } else if type == .Array {
        let root = PENode(representedObject: TagData(key: "Root", type: .Array, value: NSArray()))
        root.mutableChildren.add(self.rootNode!)
        self.rootNode = root
      } else {
        let root = PENode(representedObject: TagData(key: "Root", type: .Dictionary, value: NSDictionary()))
        root.mutableChildren.add(self.rootNode!)
        self.rootNode!.tagdata?.key = "Root" // override
        self.rootNode = root
      }
      // -------------------------------------------------------------
      // What to do if the specified plist path is a directory or not exist or isn't valid?
      // open a new empty editor w/o specifying the real path!
      var isDir : ObjCBool = false
      
      if (plistPath != nil) && fm.fileExists(atPath: (plistPath?.path)!, isDirectory: &isDir) {
        self.plistPath = plistPath!
      } else {
        self.plistPath = nil
      }
      // -----------------------------
      self.outline.focusRingType = .exterior
      self.outline.selectionHighlightStyle = .regular
      self.outline.gridStyleMask = .solidHorizontalGridLineMask
      
      self.outline.tableColumns[0].headerCell.stringValue = "Key".locale
      self.outline.tableColumns[1].headerCell.stringValue = "Type".locale
      self.outline.tableColumns[2].headerCell.stringValue = "Value".locale
      
      self.outline.intercellSpacing = NSMakeSize(0, 0)
      self.outline.registerForDraggedTypes([kMyPBoardTypeXml, kMyPBoardTypeData])
      
      self.outline.reloadData()
      DispatchQueue.main.async {
        self.outline.expandItem(self.outline.item(atRow: 0))
        self.outline.selectRowIndexes(IndexSet(integer: 0), byExtendingSelection: false)
      }
      
      NotificationCenter.default.addObserver(self,
                                             selector: #selector(self.peSearchFieldTextDidChange(_:)),
                                             name: PESearchFieldTextDidChange,
                                             object: nil)
      
      self.document?.undoManager?.enableUndoRegistration()
      
      self.vcLoaded = true
      
      NSApp.setActivationPolicy(.regular)
      self.view.window?.makeFirstResponder(self.outline)
  
      if let mainMenu = NSApplication.shared.mainMenu {
        mainMenu.update()
        for i in mainMenu.items {
          if i.title == "Format" {
            mainMenu.removeItem(i)
            break
          }
        }
        mainMenu.translate()
      }
    }
  }
  
  override var representedObject: Any? {
    didSet {
      // Update the view, if already loaded.
    }
  }
  
  @objc func performFindPanelAction(_ sender: Any) {
    if let mItem = sender as? NSMenuItem {
      switch mItem.tag {
      case 0: /* Jump to selection */
        NSSound.beep()
        break
      case 1: /* Find */
        self.showFindView(sender: sender)
        break
      case 2: /* Find Next */
        self.showFindView(sender: sender)
        break
      case 3: /* Find Previous */
        self.showFindView(sender: sender)
        break
      case 7: /* Use selection for Find */
        NSSound.beep()
        break
      case 12: /* Find And Replace */
        self.showReplaceView(sender: sender)
        break
      default:
        break
      }
    }
  }
  
  @objc func showHelp(_ sender: Any?) {
    if Locale.current.languageCode?.lowercased() == "ru" {
      NSWorkspace.shared.open(URL(string: "https://applelife.ru/threads/clover.42089/")!)
    } else {
      NSWorkspace.shared.open(URL(string: "https://www.insanelymac.com/forum/topic/304530-clover-change-explanations/")!)
    }
  }

  func hideFindAndReplaceView() {
    self.findAndReplaceViewHeightConstraint.animator().constant = 0
    self.view.window?.makeFirstResponder(self.outline)
  }
  
  @IBAction func showFindView(sender: Any?) {
    self.findAndReplaceViewHeightConstraint.animator().constant = 28
    self.view.window?.makeFirstResponder(self.searchField)
  }
  
  @IBAction func showReplaceView(sender: Any?) {
    self.findAndReplaceViewHeightConstraint.animator().constant = 57
    self.view.window?.makeFirstResponder(self.replaceField)
  }
  
  @objc func customDoubleClick() {
    let clicked = self.outline.clickedRow
    if clicked > 0 {
      let selectedColumn = self.outline.clickedColumn
      switch selectedColumn {
      case 0:
        let petcv = self.outline.view(atColumn: selectedColumn,
                                      row: clicked,
                                      makeIfNecessary: false) as! PETableCellView?
        if (petcv != nil) {
          self.outline.window?.makeFirstResponder(petcv?.textField)
        }
        break
      case 2:
        if let nstcv = self.outline.view(atColumn: selectedColumn,
                                         row: clicked,
                                         makeIfNecessary: false) as! PETableCellView? {
          if (nstcv.field?.isEditable)! {
            self.outline.window?.makeFirstResponder(nstcv.textField)
          }
        }
        break
      default:
        break
      }
    }
  }
  // ----------------------------------------------------------------------
  
  // MARK: outline view delegate
  // -------------------------------------------------------------
  func childrenFor(item: Any?) -> [NSTreeNode] {
    if item != nil {
      return (item as! PENode).children!
    } else {
      return (self.rootNode!.children)!
    }
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   numberOfChildrenOfItem item: Any?) -> Int {
    if self.rootNode == nil {
      return 0
    }
    let count = childrenFor(item: item).count
    return count
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   heightOfRowByItem item: Any) -> CGFloat {
    return 18.0
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   child index: Int,
                   ofItem item: Any?) -> Any {
    return childrenFor(item: item)[index]
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   viewFor tableColumn: NSTableColumn?,
                   item: Any) -> NSView? {
    switch item {
    case let i as PENode:
      if tableColumn?.identifier.rawValue == "keyColumn" {
        var view : PETableCellView?
        view = outlineView.makeView(withIdentifier: "keyColumn".interfaceId(),
                                    owner: self) as? PETableCellView
        view?.setup(outline: self.outline, column: 0, type: .key)
 
        view?.node = i
        // hide buttonsView
        if ((self.outline.item(atRow: 0) as! PENode) == i) {
          view?.removeButton?.isHidden = true
          view?.removeButton?.isEnabled = false
          if i.tagdata?.type != .Dictionary && i.tagdata?.type != .Array {
            view?.addButton?.isHidden = true
            view?.addButton?.isEnabled = false
          } else {
            view?.addButton?.isHidden = false
            view?.addButton?.isEnabled = true
          }
        } else {
          view?.removeButton?.isHidden = false
          view?.removeButton?.isEnabled = true
          view?.addButton?.isHidden = false
          view?.addButton?.isEnabled = true
        }
        
        if let textField = view?.field {
          textField.node = i
          if ((self.outline.item(atRow: 0) as! PENode) == i) {
            textField.isEditable = false
            textField.isSelectable = false
            textField.stringValue = (i.tagdata?.key)!
          } else {
            if (i.peparent != nil) && i.peparent!.tagdata!.type == .Array {
              let childIndex : Int = i.peparent!.mutableChildren.index(of: i)
              textField.stringValue = "\(localizedItem) \(childIndex)"
              textField.isEditable = false
              textField.isSelectable = false
            } else {
              if (i.highLightPattern != nil) && (i.highLightPattern)!.count > 0 {
                textField.allowsEditingTextAttributes = true
                textField.attributedStringValue = self.setHiglight(substring: i.highLightPattern!,
                                                                   in: (i.tagdata?.key)!)
                textField.allowsEditingTextAttributes = false
              } else {
                //textField.allowsEditingTextAttributes = false
                textField.stringValue = (i.tagdata?.key)!
              }
              textField.isEditable = true
              textField.isSelectable = true
            }
          }
          
          if !self.isEditable {
            textField.isEditable = false
          }
        }
        
        return view
        
      } else if tableColumn?.identifier.rawValue == "typeColumn" {
        var view : PETableCellViewPop?
        view = outlineView.makeView(withIdentifier: "typeColumn".interfaceId(),
                                    owner: self) as? PETableCellViewPop
    
        view?.setup(with: .tags, outline: self.outline)
      
        view?.popup.node = i
        view?.popup.setAsAllType()
        
        view?.popup.selectItem(withTitle: gPlistTagStr(tag: i.tagdata!.type).locale)
        if !self.isEditable {
          view?.popup.isEnabled = false
        }
        return view
      } else  if tableColumn?.identifier.rawValue == "valueColumn" {
        if i.tagdata!.type == .Bool {
          var view = outlineView.makeView(withIdentifier: "valueBoolView".interfaceId(),
                                      owner: self) as? PETableCellViewPop
          if view == nil {
            view = outlineView.makeView(withIdentifier: "typeColumn".interfaceId(),
                                 owner: self) as? PETableCellViewPop
            view?.identifier = "valueBoolView".interfaceId()
            view?.popup.setAsBool()
          }
          
          view?.setup(with: .bool, outline: self.outline)
          
          view?.popup.node = i
          let result : String = i.tagdata!.value as! Bool
            ? localizedYes
            : localizedNo
          
          
          view?.popup.selectItem(withTitle: result)
          if !self.isEditable {
            view?.popup.isEnabled = false
          }
          return view
        } else {
          var view = outlineView.makeView(withIdentifier: "valueView".interfaceId(),
                                          owner: self) as? PETableCellView
          if view == nil {
            view = outlineView.makeView(withIdentifier: "keyColumn".interfaceId(),
                                            owner: self) as? PETableCellView
            view?.setup(outline: self.outline, column: 2, type: .value)
            view?.identifier = "valueView".interfaceId()
          }
          
          view?.field?.node = i
          if i.tagdata!.type ==  .Array || i.tagdata!.type ==  .Dictionary {
            view?.field?.isEditable = false
            view?.field?.isSelectable = false
            view?.field?.stringValue = ""
            let count = i.count
            let countString : String = "\(count)"
            view?.field?.placeholderString = countString + " " + ((count == 1) ? localizedItem : localizedItems)
          } else {
            view?.field?.placeholderString = ""
            view?.field?.isEditable = true
            view?.field?.isSelectable = true
            
            var str : String = ""
            switch i.tagdata!.type {
            case .Number:
              let num = i.tagdata?.value as! NSNumber
              str = localizedStringFrom(number: num)
              break
            case .Date:
              let date = i.tagdata?.value as! Date
              str = localizedDateToString(date)
              break
            case .Data:
              let data = i.tagdata?.value as! Data
              str = "<"
              let dataCount = data.count
              for i in 0..<dataCount {
                let byte = data[i]
                str += String(format: "%02x", byte)
                if i < (data.count - 1) {
                  str += " "
                }
              }
              str += ">"
              break
            default:
              str = "\((i.tagdata?.value) ?? "")"
            }
            
            if (i.highLightPattern != nil) && (i.highLightPattern)!.count > 0 {
              view?.field?.stringValue = str
              view?.field?.allowsEditingTextAttributes = true
              view?.field?.attributedStringValue = self.setHiglight(substring: i.highLightPattern!, in: str)
              view?.field?.allowsEditingTextAttributes = false
            } else {
              view?.field?.allowsEditingTextAttributes = false
              view?.field?.stringValue = str
            }
          }
            
          
          return view
        }
      }
    default:
      return nil
    }
    return nil
  }
  
  func setHiglight(substring: String, in string: String) -> NSAttributedString {
    var attributed : NSMutableAttributedString
    attributed = NSMutableAttributedString(string: string)
    
    let range : NSRange = (attributed.string.lowercased() as NSString).range(of: substring)
    if range.location != NSNotFound {
      let attributes = [
        NSAttributedString.Key.foregroundColor: NSColor.black,
        NSAttributedString.Key.backgroundColor: NSColor.yellow
      ]
      
      attributed.addAttributes(attributes, range: range)
    }
    
    return attributed
  }
  
  @objc func doneButtonPressed(_ sender: NSButton) {
    self.searchField?.stringValue = ""
    self.replaceField?.stringValue = ""
    self.performDelayedSearch()
    self.searchField?.countLabel?.placeholderString = ""
    self.hideFindAndReplaceView()
  }
  
  func outlineView(_ outlineView: NSOutlineView, rowViewForItem item: Any) -> NSTableRowView? {
    let rw = PETableRowView()
    rw.node = item as? PENode
    rw.outline = outlineView as? PEOutlineView
    return rw
  }
  
  func outlineView(_ outlineView: NSOutlineView, didAdd rowView: NSTableRowView, forRow row: Int) {
    if row >= 0  && self.isAddingNewItem {
      self.outline.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
      if let nstck = self.outline.view(atColumn: 0, row: row, makeIfNecessary: false) as! PETableCellView? {
        self.outline.window?.makeFirstResponder(nstck.textField)
      }
    }
  }
  
  // not used because no delegate is set)
  func controlTextDidChange(_ obj: Notification) {
    if (obj.object as AnyObject) as? NSObject == self.searchField {
      
    }
  }
  
  // called by NSSearchField subclass
  @objc func peSearchFieldTextDidChange(_ obj: Notification) {
    if (self.outline.window != nil) && (obj.object != nil) && (self.outline.window?.isKeyWindow)! {
      if (obj.object is PESearchField) &&  (obj.object as! PESearchField) == self.searchField {
        if (self.searchTimer != nil) && (self.searchTimer?.isValid)! {
          self.searchTimer?.invalidate()
        }
        
        // shedule a timer for the searches. This is to not call performDelayedSearch() so often
        self.searchTimer = Timer.scheduledTimer(timeInterval: 0.8,
                                                target: self,
                                                selector: #selector(self.performDelayedSearch),
                                                userInfo: nil,
                                                repeats: false)
      } else if (obj.object is PEReplaceField) && (obj.object as! PEReplaceField) == self.replaceField {
        // doing nothing
      }
    }
  }
  
  @objc func performDelayedSearch() {
    if (self.searchTimer != nil) {
      if self.searchTimer!.isValid {
        self.searchTimer!.invalidate()
      }
      self.searchTimer = nil
    }
    let pattern = self.searchField?.stringValue.lowercased()
    self.searches = self.findAllSearches(with: pattern!)
    self.searchField?.countLabel?.placeholderString = "\(self.searches.count)"

    for node in self.searches {
      self.asyncExpand(item: node, expandParentOnly: true)
    }
    
    /*
     Reload all visible rows to update our findings.
     Also will normalize previously highlighted fields
     */
    var maxIndex : Int = self.outline.numberOfRows - 1
    if maxIndex > 0 {
      while maxIndex >= 0 {
        self.outline.reloadData(forRowIndexes: IndexSet(integer: maxIndex),
                                columnIndexes: IndexSet(integer: 0))
        self.outline.reloadData(forRowIndexes: IndexSet(integer: maxIndex),
                                columnIndexes: IndexSet(integer: 2))
        
        maxIndex-=1
      }
    }
  }
  
  //FIXME: change "asyncExpand" to other as is no longer asynchronous
  func asyncExpand(item: PENode, expandParentOnly: Bool) {
    var parents : [PENode] = [PENode]()
    var parent : PENode? = item.peparent
    
    self.outline.expandItem(self.outline.item(atRow: 0))
    
    repeat {
      if (parent != nil) {
        parents.insert(parent!, at: 0)
      }
      parent = parent?.peparent
    } while parent != nil
    
    for n in parents {
      if !self.outline.isItemExpanded(n) {
        self.outline.expandItem(n)
      }
    }
    
    if !expandParentOnly {
      if !self.outline.isItemExpanded(item) {
        self.outline.expandItem(item)
      }
    }
  }
  
  func findAllSearches(with pattern: String) -> [PENode] {
    // where to start? --> from outline object at index 0
    var founds : [PENode] = [PENode]()
    let root : PENode? = self.outline.item(atRow: 0) as? PENode
    
    if (root != nil) &&
      (root?.tagdata?.type == .Dictionary || root?.tagdata?.type == .Array) {
      for n in root!.mutableChildren {
        self.populateSearches(pattern: pattern, in: n as! PENode, array: &founds)
      }
      
    }
    
    return founds
  }
  
  func populateSearches(pattern: String, in node: PENode, array: inout [PENode]) {
    // always search in the key if parent isn't array
    // always search in the value if node is not a contenitor (Dictionary or array)
    // Skip searching in boolean and Data values
    // problem: some value can have a localized result (Date and Number).. what to do here???
    
    let root : PENode? = self.outline.item(atRow: 0) as? PENode
    if node != root {
      var parentIsArray : Bool = false
      var found : Bool = false
      if node.peparent?.tagdata?.type == .Array {
        parentIsArray = true
      }
      
      // First: search a match in the key
      if !parentIsArray {
        if (node.tagdata?.key.lowercased().range(of: pattern) != nil) {
          node.highLightPattern = pattern
          array.append(node)
          found = true
        } else {
          node.highLightPattern = nil
        }
      }
      // Second: search a match in the value
      if !found {
        var localizedVal : String = ""
        
        switch node.tagdata!.type {
        case .String:
          localizedVal = node.tagdata?.value as! String
          break
        case .Number:
          let num = node.tagdata?.value as! NSNumber
          localizedVal = localizedStringFrom(number: num)
          break
        case .Date:
          let date = node.tagdata?.value as! Date
          localizedVal = localizedDateToString(date)
          break
        default:
          break
        }
        
        if (localizedVal.lowercased().range(of: pattern) != nil) {
          node.highLightPattern = pattern
          array.append(node)
        } else {
          node.highLightPattern = nil
        }
      }
    }
    
    
    if node.tagdata?.type == .Dictionary || node.tagdata?.type == .Array {
      for n in node.mutableChildren {
        self.populateSearches(pattern: pattern, in: n as! PENode, array: &array)
      }
    }
  }
  
  @objc func segmentScrollerPressed(_ sender: NSSegmentedControl?) {
    if (self.searchField?.stringValue.count)! > 0 {
      if self.searches.count > 0 {
        self.view.window?.makeFirstResponder(self.outline)
        var index : Int = 0
        /*
         check if we have a selected row and backup the object for later use:
         after expanding We may lose the selection
         */
        var selectedRow = self.outline.selectedRow
        var selectedNode: PENode? = nil
        if selectedRow >= 0 {
          selectedNode = self.outline.item(atRow: selectedRow) as? PENode
        }
        /*
         since we are calculating with rows indexes, expand now all involved nodes-parent before doing anything
         */
        for node in self.searches {
          self.asyncExpand(item: node, expandParentOnly: true)
        }
        
        /*
         (selectedNode != nil) means that user has starting scrolling already?
         sure, but can be nil if user has loses the selection by clicking on any
         If that happened, We know We have to start from self.searches[0].
         Otherwise (a row is selected but isn't in self.searches) we have to find
         its row index and start scrolling from there
         */
        
        if (selectedNode != nil) {
          //selectedRow was valid.. but its in our searches?
          if self.searches.contains(selectedNode!) {
            index = self.searches.firstIndex(of: selectedNode!)!
          } else {
            // nope, jump to the nearest one, but befor or after?
            selectedRow = self.outline.row(forItem: selectedNode)
            let nrowsIndex : Int = self.outline.numberOfRows - 1
            var i : Int = selectedRow
            if sender?.selectedSegment == 0 {
              // scan back
              while i > 0 {
                let scannedNode = self.outline.item(atRow: i) as! PENode
                if self.searches.contains(scannedNode)  {
                  index = self.searches.firstIndex(of: scannedNode)! + 1
                  selectedNode = scannedNode
                  break
                }
                i-=1
              }
            } else {
              // scan forward
              while i < nrowsIndex {
                let scannedNode = self.outline.item(atRow: i) as! PENode
                if self.searches.contains(scannedNode)  {
                  index = self.searches.firstIndex(of: scannedNode)! - 1
                  selectedNode = scannedNode
                  break
                }
                i+=1
              }
            }
          }
        } else {
          index = 0
          selectedNode = self.searches[0]
        }
        
        /* jump to previous or next search */
        var jumpTo : Int = index
        let lastIndex : Int = self.searches.count - 1
        if sender?.selectedSegment == 0 {
          // go back
          jumpTo = (jumpTo == 0) ? lastIndex : (jumpTo - 1)
          selectedNode = self.searches[jumpTo]
          let row = self.outline.row(forItem: selectedNode)
          if row >= 0 {
            self.outline.scrollRowToVisible(self.outline.row(forItem: selectedNode))
            self.outline.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
          }
        } else {
          // go ahead
          //let lastRowInOutLine : Int = self.outline.numberOfRows - 1
          let lastSearchInOutline : Int = self.outline.row(forItem: self.searches.last)
          
          if selectedRow > lastSearchInOutline {
            jumpTo = 0
          } else {
            jumpTo = (jumpTo == lastIndex) ? 0 : (jumpTo + 1)
          }
          
          selectedNode = self.searches[jumpTo]
          let row = self.outline.row(forItem: selectedNode)
          if row >= 0 {
            self.outline.scrollRowToVisible(self.outline.row(forItem: selectedNode))
            self.outline.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
          }
        }
      } else {
        NSSound.beep()
      }
    } else {
      NSSound.beep()
    }
  }
  
  @objc func replaceOneOrAllSegmentPressed(_ sender: NSSegmentedControl) {
    /* here we perform replacements.
     allow replace text field to be empty ("")
     */
    
    if (self.searchField?.stringValue.count)! > 0 && self.searches.count > 0 {
      self.view.window?.makeFirstResponder(self.outline)
      let nodes : NSMutableArray = NSMutableArray()
      let oldTreeData : NSMutableArray = NSMutableArray()
      let newTreeData : NSMutableArray = NSMutableArray()
      
      for n in ((sender.selectedSegment == 0) ? self.searches : [self.searches[0]]) {
        nodes.add(n)
        /*
         replace selected with Undo/Redo.
         To do that we should jump to the nearest search,
         apparently Xcode do that by always search from scratch everytime the segment is pressed
         and so it start always from index 0 and it looks scrolling ... but isn't!
         */
        let originalNode : PENode = n
        let parent : PENode = originalNode.peparent!
        let moddedNode : PENode = originalNode.copy() as! PENode
        
        /*
         Problem, we should ensure the node parent has not a key already present after the replace operation:
         in this case we must add a suffix (- 1, -2 etc.).
         */
        let replacement = self.replaceField?.stringValue ?? ""
        /* replacing on key */
        let poposedNewKeyName : String = (moddedNode.tagdata?.key.replacingOccurrencesOf(inSensitive: originalNode.highLightPattern!, withSensitive: replacement))!
        /* replacing on value:
         - Bool: cannot be replaced, but should not be there
         - Data: cannot be replaced,  but should not be there
         - Number: is localized formmatted, so return a value only if make sense
         - Date: is localized formmatted, so return a value only if make sense
         - String: no problem
         */
        var localizedVal : String = ""
        switch originalNode.tagdata!.type {
        case .String:
          localizedVal = moddedNode.tagdata?.value as! String
          localizedVal = localizedVal.replacingOccurrencesOf(inSensitive: originalNode.highLightPattern!,
                                                             withSensitive: replacement)
          moddedNode.tagdata?.value = localizedVal
          break
        case .Number:
          let num = originalNode.tagdata?.value as! NSNumber
          localizedVal = localizedStringFrom(number: num)
          
          localizedVal = localizedVal.replacingOccurrencesOf(inSensitive: originalNode.highLightPattern!,
                                                             withSensitive: replacement)
          // is still a valid Number??
          var nv : Int = 0
          localizedVal = localizedVal.components(separatedBy: CharacterSet.decimalDigits.inverted).joined()
          if localizedVal.count > 0 {
            nv = Int(localizedVal)!
          }
          moddedNode.tagdata?.value = nv
          break
        case .Date:
          let date = originalNode.tagdata?.value as! Date
          localizedVal = localizedDateToString(date)
          localizedVal = localizedVal.replacingOccurrencesOf(inSensitive: originalNode.highLightPattern!,
                                                             withSensitive: replacement)
          // is still a valid date??
          var dv : Date? = nil
          dv = localizedStringToDateS(localizedVal) // first try with "S" version that can be nil
          
          if (dv == nil) {
            // secondly try to detect it (NSDataDetector + NSTextCheckingResult)
            // This allow us to have the old date if after replacing is bad
            dv = funcyDateFromUser(localizedVal)
          }
          
          if (dv == nil) {
            dv = Date() // no way? init a new one...
          }
          
          moddedNode.tagdata?.value = dv! as NSDate
          break
        default:
          break
        }
        
        /* assing an univoque key name.
         Don't use deduplicateKeyInParent() because we are not adding a new key:
         parent must not contains duplicate keys, but this one is just the same as before,
         maybe that after replacing an occurences can be equal to another one
         */
        let childs = parent.mutableChildren
        if childs.count > 1 {
          // more than one item
          var actualKeys = [String]()
          for item in parent.mutableChildren {
            /* we're looking for duplicate keys, but since our node is already present
             remove it from actualKeys and see if poposedNewKeyName is already present
             */
            if (item as! PENode) != originalNode {
              actualKeys.append((item as! PENode).tagdata!.key)
            }
          }
          moddedNode.tagdata?.key = gProposedNewItem(with: poposedNewKeyName, in: actualKeys)
        } else {
          // replace the only available children's key
          moddedNode.tagdata?.key = poposedNewKeyName
        }
        
        
        /*
         remove highLightPattern from originalNode
         After a Undo/Redo user have to searcg again?
         */
        originalNode.highLightPattern = nil
        moddedNode.highLightPattern = nil
        
        oldTreeData.add(originalNode.tagdata!.copy())
        newTreeData.add(moddedNode.tagdata!.copy())
      }
      
      if sender.selectedSegment == 0 {
        self.searches.removeAll()
      } else {
        self.searches.remove(at: 0)
      }
      
      self.outline.undo_FindAndReplace(nodes: nodes,
                                       oldTreeData: oldTreeData,
                                       newTreeData: newTreeData)
    } else {
      NSSound.beep()
    }
  }
  
  func controlTextDidBeginEditing(_ obj: Notification) {
    if obj.object is PETextField {
      self.outline.wrongValue = true // here only indicate that we're editing
    }
  }
  
  func controlTextDidEndEditing(_ obj: Notification) {
    
    if obj.object is PETextField {
      self.outline.wrongValue = false // will be set to true again if value is wrong
      let textField = obj.object as! PETextField
      
      let ro = textField.node?.tagdata
      if textField.node?.parent == nil {
        return // usefull if user undo after adding a new row
      }
      let parent = textField.node!.peparent!
      
      switch textField.column {
      case 0:
        var canEdit : Bool = true
        let oldKey = ro?.key
        let newKey = textField.stringValue
        
        if oldKey != newKey {
          for i in parent.mutableChildren {
            let node = i as! PENode
            if node.tagdata?.key == newKey {
              self.outline.wrongValue = true
              canEdit = false
              NSSound.beep()
              textField.window?.makeFirstResponder(textField)
              let alert = PEAlert(in: (self.outline.window)!)
              alert.messageText = localizedDuplicateKeyMsgText
              
              let withFormat = localizedDuplicateKeyInfoText
              let question = String(format: withFormat, newKey)
              
              alert.informativeText = question
              alert.addButton(withTitle: localizedKeepEditing)
              alert.addButton(withTitle: localizedDuplicateKeyReplaceButton)
              
              alert.beginSheetModal(for: self.outline.window!, completionHandler: { (modalResponse) -> Void in
                if modalResponse == NSApplication.ModalResponse.alertFirstButtonReturn {
                  self.outline.window?.makeFirstResponder(textField)
                } else {
                  let indexChild : Int = parent.mutableChildren.index(of: node)
                  self.outline.undo_ReplaceExisting(item: node,
                                                    inParent: parent,
                                                    indexChild: indexChild,
                                                    editedNode: textField.node!,
                                                    oldKey: oldKey!,
                                                    newKey: newKey)
                }
              })
              break
            }
          }
          if canEdit {
            self.outline.undo_SetKey(node: textField.node!, newKey: newKey, oldKey: oldKey!)
          }
        }
        break
      case 2:
        switch ro!.type {
        case .String:
          self.outline.undo_SetValue(node: textField.node!,
                                     newValue: textField.stringValue,
                                     oldValue: ro!.value!)
          break
        case .Number:
          let newNum = numberFromLocalizedString(string: textField.stringValue)
          self.outline.undo_SetValue(node: textField.node!,
                                     newValue: (newNum is PEReal) ? newNum as! PEReal : newNum as! PEReal,
                                     oldValue: ro!.value!)
          break
        case .Data:
          let res = isHexStringValid(string: textField.stringValue.lowercased())
          if res != "HexSuccess" {
            self.outline.wrongValue = true
            NSSound.beep()
            textField.window?.makeFirstResponder(textField)
            let alert = PEAlert(in: self.outline.window)
            alert.messageText = localizedInvalidValueMsgText
            
            alert.informativeText = localizedInvalidValueInfoText + "\n\n(" + res + ")"
            
            alert.addButton(withTitle: localizedKeepEditing)
            alert.addButton(withTitle: localizedUndo)
            
            alert.beginSheetModal(for: self.outline.window!, completionHandler: { (modalResponse) -> Void in
              if modalResponse == NSApplication.ModalResponse.alertFirstButtonReturn {
                self.outline.window?.makeFirstResponder(textField)
              } else {
                self.outline.reloadItem(parent, reloadChildren: true)
              }
            })
          } else {
            let str : String = textField.stringValue.lowercased()
            self.outline.undo_SetValue(node: textField.node!,
                                       newValue: (str.hexadecimal() ?? Data()),
                                       oldValue: ro!.value!)
          }
          break
        case .Date:
          if (localizedStringToDateS(textField.stringValue) == nil) {
            self.outline.wrongValue = true
            NSSound.beep()
            textField.window?.makeFirstResponder(textField)
            let alert = PEAlert(in: (self.outline.window)!)
            alert.messageText = localizedInvalidValueMsgText
            
            alert.informativeText = localizedInvalidValueInfoText
            
            alert.addButton(withTitle: localizedKeepEditing)
            alert.addButton(withTitle: localizedUndo)
            
            alert.beginSheetModal(for: self.outline.window!, completionHandler: { (modalResponse) -> Void in
              if modalResponse == NSApplication.ModalResponse.alertFirstButtonReturn {
                self.outline.window?.makeFirstResponder(textField)
              } else {
                self.outline.reloadItem(parent, reloadChildren: true)
              }
            })
          } else {
            let str : String = textField.stringValue
            self.outline.undo_SetValue(node: textField.node!,
                                       newValue: localizedStringToDate(str),
                                       oldValue: ro!.value!)
          }
          break
        default:
          break
        }
        break
      default:
        break
      }
      self.outline.wrongValue = false
    }
  }
  
  func outlineViewItemDidExpand(_ notification: Notification) {
    let sr = self.outline.row(forItem: notification.userInfo?["NSObject"])
    if sr >= 0 {
      self.outline.selectRowIndexes(IndexSet(integer: sr), byExtendingSelection: false)
    }
  }
  
  func outlineViewItemDidCollapse(_ notification: Notification) {
    let sr = self.outline.row(forItem: notification.userInfo?["NSObject"])
    if sr >= 0 {
      self.outline.selectRowIndexes(IndexSet(integer: sr), byExtendingSelection: false)
    }
  }
  
  func outlineViewSelectionDidChange(_ notification: Notification) {
    self.highLightRow()
  }

  func highLightRow() {
    self.outline.enumerateAvailableRowViews { _, row in
      if let rv = self.outline.view(atColumn: 0,
                                    row: row,
                                    makeIfNecessary: false)?.superview as? PETableRowView {
        rv.setBorderType()
      }
    }
  }
  
  // MARK: determine if is a container
  func outlineView(_ outlineView: NSOutlineView, isGroupItem item: Any) -> Bool {
    return false
  }
  
  func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
    if let n = item as? PENode {
      return n.isExpandable
    }
    return false
  }
  
  func outlineView(_ outlineView: NSOutlineView, shouldExpandItem item: Any) -> Bool {
    if self.outline.wrongValue {
      NSSound.beep()
      return false
    }
    if let n = item as? PENode {
      return n.isExpandable
    }
    return false
  }
  
  
  func selectionShouldChange(in outlineView: NSOutlineView) -> Bool {
    return self.outline.wrongValue ? false : true
  }
  
  // MARK: disclosure triangle
  func outlineView(_ outlineView: NSOutlineView, shouldShowOutlineCellForItem item: Any) -> Bool {
    return true
  }
  
  // MARK: OutlineView drag and drop
  func outlineView(_ outlineView: NSOutlineView,
                   pasteboardWriterForItem item: Any) -> NSPasteboardWriting? {
    self.pbTreeNode = nil
    
    if (item as! PENode) != (self.outline.item(atRow: 0) as! PENode) {
      self.pbTreeNode = (item as! PENode)
      let pb = NSPasteboardItem()
      let a = NSKeyedArchiver.archivedData(withRootObject: self.pbTreeNode!)
      pb.setData(a, forType: kMyPBoardTypeData)
      return pb
    }
    
    return nil
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   draggingSession session: NSDraggingSession,
                   willBeginAt screenPoint: NSPoint,
                   forItems draggedItems: [Any]) {
    
  }
  func outlineView(_ outlineView: NSOutlineView,
                   draggingSession session: NSDraggingSession,
                   endedAt screenPoint: NSPoint,
                   operation: NSDragOperation) {
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   validateDrop info: NSDraggingInfo,
                   proposedItem item: Any?,
                   proposedChildIndex index: Int) -> NSDragOperation {

    if let dragOutline = info.draggingSource as? PEOutlineView {
      if dragOutline != self.outline {
        return NSDragOperation.move
      }
      if dragOutline != self.outline || item == nil {
        return (index > 0) ? NSDragOperation.move : []
      }
    }
    
    // item is the destination
    if let node = item as? PENode, let ol = outlineView as? PEOutlineView {
      // refuse if target is the same
      if node == ol.editorVC?.pbTreeNode { return [] }
      // all fine
      return NSDragOperation.move
    }
    return []
  }
  
  func outlineView(_ outlineView: NSOutlineView,
                   acceptDrop info: NSDraggingInfo,
                   item: Any?,
                   childIndex index: Int) -> Bool {
  
    if !self.isEditable || index < 0 {
      return false
    }
    
    // Accept drag & drop on self
    if (info.draggingSource as? PEOutlineView) == self.outline {
      if self.pbTreeNode == nil {
        return false
      }
      if let parent = (item as? PENode) {
        
        guard let fromIndex : Int = self.pbTreeNode?.indexPath.last else {
          return false
        }
        let maxIndex = parent.count
        
        if index > maxIndex {
          return false
        }
        
        self.outline.undo_DragAndDrop(item: self.pbTreeNode!,
                                      fromParent: self.pbTreeNode!.peparent!,
                                      fromIndex:  fromIndex,
                                      toParent: parent,
                                      toIndex: index)
        self.pbTreeNode = nil
      }
    } else {
      // Accept drag & drop from another document
      if let data = info.draggingPasteboard.data(forType: kMyPBoardTypeData),
        let node = NSKeyedUnarchiver.unarchiveObject(with: data) as? PENode,
        let otherOutline = info.draggingSource as? PEOutlineView, let parent = item as? PENode {
        
        gDeduplicateKeyInParent(parent:parent, newNode: node)
        otherOutline.undo_Add(item: node,
                              inParent: parent,
                              indexChild: index,
                              target: outlineView as! PEOutlineView)
        
        self.pbTreeNode = nil
        return true
      }
       
    }
    return false
  }
  
  // MARK: OutlineView save
  func save() -> Data {
    let root = self.outline.item(atRow: 0) as? PENode
    root?.isRootNode = true // mark row 0 as root
    let plist = gConvertPENodeToPlist(node: root)
    root?.isRootNode = false // restore
    return plist.data(using: .utf8)!
  }
  
  func convertToBinaryPlist() -> Data? {
    let data = save()
    var any : AnyObject
    
    var fmt =  PropertyListSerialization.PropertyListFormat.xml
    do {
      any = try PropertyListSerialization.propertyList(from: data,
                                                       options: .mutableContainersAndLeaves,
                                                       format: &fmt) as AnyObject
      
    } catch {
      return nil
    }
    
    do {
      
      try any = PropertyListSerialization.data(fromPropertyList: any,
                                               format: PropertyListSerialization.PropertyListFormat.binary,
                                               options: PropertyListSerialization.WriteOptions(0)) as NSData
      
    } catch  {
      return nil
    }
    
    if any is NSData {
      return any as? Data
    }
    return nil
  }
}

extension NSMenu {
  func translate() {
    for i in self.items {
      i.title = i.title.locale
      if i.submenu != nil {
        i.submenu!.title = i.submenu!.title.locale
        i.submenu!.translate()
      }
    }
  }
}
