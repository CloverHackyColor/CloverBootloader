//
//  Extensions.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa

extension String {

  public func noSpaces() -> String {
    return self.trimmingCharacters(in: CharacterSet.whitespaces)
  }
  
  var lastPath: String {
    return (self as NSString).lastPathComponent
  }
  var fileExtension: String {
    return (self as NSString).pathExtension
  }
  var deletingLastPath: String {
    return (self as NSString).deletingLastPathComponent
  }
  var deletingFileExtension: String {
    return (self as NSString).deletingPathExtension
  }
  var componentsPath: [String] {
    return (self as NSString).pathComponents
  }
  func addPath(_ path: String) -> String {
    let nsSt = self as NSString
    return nsSt.appendingPathComponent(path)
  }
  func appendingFileExtension(ext: String) -> String? {
    let nsSt = self as NSString
    return nsSt.appendingPathExtension(ext)
  }
  
  // https://stackoverflow.com/questions/25554986/how-to-convert-string-to-unsafepointeruint8-and-length#
  func toPointer() -> UnsafePointer<UInt8>? {
    guard let data = self.data(using: String.Encoding.utf8) else { return nil }
    
    let buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: data.count)
    let stream = OutputStream(toBuffer: buffer, capacity: data.count)
    
    stream.open()
    let value = data.withUnsafeBytes {
        $0.baseAddress?.assumingMemoryBound(to: UInt8.self)
    }
    guard let val = value else {
        return nil
    }
    
    stream.write(val, maxLength: data.count)
    
    stream.close()
    
    return UnsafePointer<UInt8>(buffer)
  }
}
