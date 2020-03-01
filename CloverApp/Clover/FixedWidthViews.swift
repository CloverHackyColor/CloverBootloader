//
//  FixedWidthViews.swift
//  Clover
//
//  Created by vector sigma on 05/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

final class FWPopUpButton: NSPopUpButton {
  private var pfixedWidth : CGFloat = 50
  @IBInspectable var fixedWidth: CGFloat {
    get {
      return self.pfixedWidth
    } set {
      self.pfixedWidth = newValue
    }
  }
  override var intrinsicContentSize: NSSize {
    return NSMakeSize(self.fixedWidth, super.intrinsicContentSize.height)
  }
}

final class FWButton: NSButton {
  private var pfixedWidth : CGFloat = 50
  @IBInspectable var fixedWidth: CGFloat {
    get {
      return self.pfixedWidth
    } set {
      self.pfixedWidth = newValue
    }
  }
  override var intrinsicContentSize: NSSize {
    return NSMakeSize(self.fixedWidth, super.intrinsicContentSize.height)
  }
}

final class FWTextField: NSTextField {
  private var pfixedWidth : CGFloat = 50
  @IBInspectable var fixedWidth: CGFloat {
    get {
      return self.pfixedWidth
    } set {
      self.pfixedWidth = newValue
    }
  }
  override var intrinsicContentSize: NSSize {
    return NSMakeSize(self.fixedWidth, super.intrinsicContentSize.height)
  }
}
