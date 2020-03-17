//
//  ThemeView.swift
//  ThemeManager
//
//  Created by vector sigma on 07/01/2020.
//  Copyright © 2020 vectorsigma. All rights reserved.
//

import Cocoa
import WebKit

class ThemeView: NSView, WebFrameLoadDelegate, WebUIDelegate {
  var manager : ThemeManager
  var imagePath : String? 
  var name : String
  var author : String?
  var info : String?
  var row : Int
  var isInstalled = false
  
  public let webView: WebView = {
    let wv = WebView(frame: NSMakeRect(0, 0, 147, 82))
    wv.mainFrame.frameView.allowsScrolling = false
    wv.drawsBackground = false
    return wv
  } ()

  deinit {
    self.webView.stopLoading(nil)
  }
  
  func webView(_ sender: WebView!, makeFirstResponder responder: NSResponder!) {
    if self.row >= 0 {
      self.manager.delegate?.sidebar.selectRowIndexes(IndexSet(integer: self.row),
                                                      byExtendingSelection: false)
    }
  }
  
  // MARK: - designated initializer
  init(manager: ThemeManager, name: String, row: Int) {
    self.manager = manager
    self.name = name
    self.row = row
    super.init(frame: NSMakeRect(0, 0, 147, 82))
    self.setup()
    self.load()
  }
  
  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }
  
  private func setup() {
    self.addSubview(self.webView)
    
    self.webView.translatesAutoresizingMaskIntoConstraints = false
    let left = NSLayoutConstraint(item: self.webView,
                                  attribute: .left,
                                  relatedBy: .equal,
                                  toItem: self,
                                  attribute: .left,
                                  multiplier: 1.0,
                                  constant: 0.0)
    let bottom = NSLayoutConstraint(item: self.webView,
                                    attribute: .bottom,
                                    relatedBy: .equal,
                                    toItem: self,
                                    attribute: .bottom,
                                    multiplier: 1.0,
                                    constant: 0.0)
    let width = NSLayoutConstraint(item: self.webView,
                                   attribute: .width,
                                   relatedBy: .equal,
                                   toItem: self,
                                   attribute: .width,
                                   multiplier: 1.0,
                                   constant: 0.0)
    let height = NSLayoutConstraint(item: self.webView,
                                    attribute: .height,
                                    relatedBy: .equal,
                                    toItem: self,
                                    attribute: .height,
                                    multiplier: 1.0,
                                    constant: 0.0)
    self.addConstraints([left, bottom, height, width])
    
    
    self.webView.uiDelegate = self
    self.webView.frameLoadDelegate = self
  }

  func load() {
    if self.manager.exist(theme: self.name) {
      self.manager.getImageUrl(for: self.name, completion: { (ipath) in
        if let path = ipath {
          self.imagePath = path
        } else {
          DispatchQueue.main.async { [weak self] in
            self?.load()
          }
        }
        self.setInstalledStatus()
      })
    } else {
      self.setInstalledStatus()
    }
  }
  
  @objc func targetVolumeDidChange() {
    self.setInstalledStatus()
  }
  
  func setInstalledStatus() {
    if let vol = self.manager.delegate?.targetVolume {
      self.isInstalled = fm.fileExists(atPath: vol.addPath("EFI/CLOVER/themes").addPath(self.name))
      if (self.imagePath == nil) {
        if fm.fileExists(atPath: vol.addPath("EFI/CLOVER/themes").addPath(self.name).addPath("theme.svg")) {
          self.imagePath = vol.addPath("EFI/CLOVER/themes").addPath(self.name).addPath("theme.svg")
          self.setInstalledStatus()
        } else if fm.fileExists(atPath: vol.addPath("EFI/CLOVER/themes").addPath(self.name).addPath("screenshot.png")) {
          self.imagePath = vol.addPath("EFI/CLOVER/themes").addPath(self.name).addPath("screenshot.png")
          self.setInstalledStatus()
        }
      }
    }
    
    if let path = self.imagePath {
      if self.isInstalled {
        let icon = Bundle.main.path(forResource: "check", ofType: "png")
        self.webView.mainFrame.loadHTMLString("""
          <html>
          <body>
          <div align="center" style="position:relative; height: 100%; width: 100%; top:0;left 0;">
          <img src="\(URL(fileURLWithPath: path))" style="width: 100% ; max-width: 128px;"/>
          <img src="\(URL(fileURLWithPath: icon!))" style="position:absolute; top:8; left:8;"/>
          </div>
          </body>
          </html>
          """, baseURL: Bundle.main.bundleURL)
      } else {
        self.webView.mainFrame.loadHTMLString("""
          <html>
          <body>
          <div align="center" style="position:relative; height: 100%; width: 100%; top:0;left 0;">
          <img src="\(URL(fileURLWithPath: path))" style="width: 100% ; max-width: 128px;top:0;"/>
          </div>
          </body>
          </html>
          """, baseURL: Bundle.main.bundleURL)
      }
    }
  }
}

