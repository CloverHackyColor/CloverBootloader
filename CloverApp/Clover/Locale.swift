//
//  Locale.swift
//  Clover
//
//  Created by vector sigma on 17/11/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

extension String {
  var locale: String {
    get {
      let preferred : [String] = Bundle.main.preferredLocalizations
      var table : String = "en"
      if preferred.count > 0 {
        table = preferred[0]
      }
      if localeBundle == nil {
        print("localeBundle is nil")
        return self
      }
      var result = localeBundle?.localizedString(forKey: self, value: nil, table: table)
      
      if result == self {
        result = localeBundle?.localizedString(forKey: self, value: nil, table: "en")
      }
      return (result != nil) ? result! : self
    }
  }
  
  public func locale(_ localized: Bool) -> String {
    return (localized ? self.locale : self)
  }
}

// MARK: localize view and sub views
func localize(view: NSView) {
  for o in view.subviews {
    if o is NSButton {
      let x = (o as! NSButton)
      if x.title.count > 0 {
        x.title = x.title.locale
      }
    } else if o is NSTextField {
      let x = (o as! NSTextField)
      if x.stringValue.count > 0 {
        x.stringValue = x.stringValue.locale
      }
    } else if o is NSBox {
      let x = (o as! NSBox)
      if x.title.count > 0 {
        x.title = x.title.locale
      }
    } else if o is NSTabView {
      let x = (o as! NSTabView)
      for i in x.tabViewItems {
        i.label = i.label.locale
        if let v = i.view {
          localize(view: v)
        }
      }
    }
    
    if o.subviews.count > 0 {
      for v in o.subviews {
        localize(view: v)
      }
    }
  }
}
