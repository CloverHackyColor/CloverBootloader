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

@available(OSX 10.10, *)
final class PEFindVC: NSViewController, NSSearchFieldDelegate, NSTextViewDelegate {
  var editor : PlistEditorVC?
  @IBOutlet var  doneBtn: NSButton!
  @IBOutlet var  showReplaceBtn: ReplaceButton!
  @IBOutlet var  nextOrPreviousSegment: NSSegmentedControl!
  @IBOutlet var  searchField: PESearchField!
  
  class func loadFromNib(editor: PlistEditorVC) -> PEFindVC? {
    let id = NSStoryboard.SceneIdentifier("FindView")
    let vc = NSStoryboard(name: NSStoryboard.Name("PlistEditor"),
                          bundle: nil).instantiateController(withIdentifier: id) as! PEFindVC
    vc.editor = editor
    vc.searchField?.countLabel?.placeholderString = ""
    if (editor.searchField != nil) {
      editor.searchField?.superview?.removeFromSuperview()
    }
    return vc
  }
  
  override func viewDidLoad() {
    super.viewDidLoad()
    // set up actions
    self.editor?.doneBtn = self.doneBtn
    self.doneBtn.target = self.editor
    self.doneBtn.action = #selector(self.editor?.doneButtonPressed(_:))
    
    self.editor?.showReplaceBtn = self.showReplaceBtn
    self.showReplaceBtn.target = self.editor
    self.showReplaceBtn.action = #selector(self.editor?.showReplaceView(sender:))
    
    self.editor?.nextOrPreviousSegment = self.nextOrPreviousSegment
    self.nextOrPreviousSegment.target = self.editor
    self.nextOrPreviousSegment.action = #selector(self.editor?.segmentScrollerPressed(_:))

    self.editor?.searchField = self.searchField
    self.searchField.placeholderString = localizedSearch
    self.searchField.countLabel?.stringValue = ""
    
    if let search = NSPasteboard.init(name: .findPboard).string(forType: .string) {
      self.searchField.stringValue = search
      if search != self.editor?.currentSearchString {
        self.editor?.performDelayedSearch()
      }
    } else {
      self.searchField.stringValue = self.editor?.currentSearchString ?? ""
      if let count = self.editor?.searches.count {
        self.searchField.countLabel.placeholderString = (count > 0) ? "\(count)" : ""
      } else {
        self.searchField.countLabel.placeholderString = ""
      }
    }
  }
}

@available(OSX 10.10, *)
final class PEFindAndReplaceVC: NSViewController, NSSearchFieldDelegate {
  var editor : PlistEditorVC?
  @IBOutlet var  doneBtn: NSButton!
  @IBOutlet var  showFindViewBtn: FindButton!
  @IBOutlet var  showReplaceBtn: ReplaceButton!
  @IBOutlet var  nextOrPreviousSegment: NSSegmentedControl!
  @IBOutlet var  replaceOneOrAllSegment: NSSegmentedControl!
  @IBOutlet var  searchField: PESearchField!
  @IBOutlet var  replaceField: PEReplaceField!
  
  class func loadFromNib(editor: PlistEditorVC) -> PEFindAndReplaceVC? {
    let id = NSStoryboard.SceneIdentifier("FindAndReplaceView")
    let vc = NSStoryboard(name: NSStoryboard.Name("PlistEditor"),
                          bundle: nil).instantiateController(withIdentifier: id) as! PEFindAndReplaceVC
    vc.editor = editor
    if (editor.searchField != nil) {
      editor.searchField?.superview?.removeFromSuperview()
    }
    return vc
  }
  
  override func viewDidLoad() {
    super.viewDidLoad()
    // set up actions
    self.editor?.doneBtn = self.doneBtn
    self.doneBtn.target = self.editor
    self.doneBtn.action = #selector(self.editor?.doneButtonPressed(_:))
    
    self.editor?.showFindViewBtn = self.showFindViewBtn
    self.showFindViewBtn.target = self.editor
    self.showFindViewBtn.action = #selector(self.editor?.showFindView(sender:))
    
    self.editor?.showReplaceBtn = self.showReplaceBtn
    self.showReplaceBtn.target = self.editor
    self.showReplaceBtn.action = #selector(self.editor?.showReplaceView(sender:))
    
    self.editor?.nextOrPreviousSegment = self.nextOrPreviousSegment
    self.nextOrPreviousSegment.target = self.editor
    self.nextOrPreviousSegment.action = #selector(self.editor?.segmentScrollerPressed(_:))
    
    self.editor?.searchField = self.searchField
    self.searchField.placeholderString = localizedSearch
    self.searchField.countLabel?.stringValue = ""
    self.editor?.replaceField = self.replaceField
    if let search = NSPasteboard.init(name: .findPboard).string(forType: .string) {
      self.searchField.stringValue = search
      if search != self.editor?.currentSearchString {
        self.editor?.performDelayedSearch()
      }
    } else {
      self.searchField.stringValue = self.editor?.currentSearchString ?? ""
      if let count = self.editor?.searches.count {
        self.searchField.countLabel.placeholderString = (count > 0) ? "\(count)" : ""
      } else {
        self.searchField.countLabel.placeholderString = ""
      }
    }
    
    self.replaceField.placeholderString = localizedReplace
    self.editor?.replaceField = self.replaceField
    self.replaceField.stringValue = self.editor?.currentReplaceString ?? ""
    
    self.editor?.replaceOneOrAllSegment = self.replaceOneOrAllSegment
    self.replaceOneOrAllSegment.target = self.editor
    self.replaceOneOrAllSegment.action = #selector(self.editor?.replaceOneOrAllSegmentPressed(_:))
    for i in 0..<self.replaceOneOrAllSegment.segmentCount {
      let label = self.replaceOneOrAllSegment.label(forSegment: i)
      self.replaceOneOrAllSegment.setLabel(label!.locale, forSegment: i)
    }
  }
}
