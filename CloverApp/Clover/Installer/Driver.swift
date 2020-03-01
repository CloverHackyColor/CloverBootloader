//
//  Driver.swift
//  Clover
//
//  Created by vector sigma on 22/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

enum EFIkind: String {
  case uefi = "UEFI"
  case bios = "BIOS"
}

final class EFIDriver {
  private var internalState : NSControl.StateValue = .off
  var dest  : String
  var src   : String
  var kind  : EFIkind
  var sectionName : String
  
  var state : NSControl.StateValue {
    get {
      self.itemView?.checkBox.state = self.internalState
      return self.internalState
    }
    set {
      self.internalState = newValue
      self.itemView?.checkBox.state = self.internalState
    }
  }
  
  var isFromClover : Bool
  var itemView : CollectionViewItem? = nil
  
  init(dest: String, src: String,
       kind: EFIkind,
       sectionName: String,
       state: NSControl.StateValue,
       isFromClover: Bool) {
    self.dest = dest
    self.src = src
    self.kind = kind
    self.sectionName = sectionName
    self.isFromClover = isFromClover
    self.state = state
  }
}

