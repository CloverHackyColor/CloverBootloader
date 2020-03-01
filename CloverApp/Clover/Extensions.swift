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

extension NSNumber {
  var hexString: String {
    if (self.intValue > 0xFFFF) {
      return String(format: "0x%08llx", self.intValue)
    } else {
      return String(format: "0x%04llx", self.intValue)
    }
  }
}

extension Int {
  var data: Data {
    var num = self
    return Data(bytes: &num, count: MemoryLayout<Int>.size)
  }
}

extension UInt8 {
  var data: Data {
    var num = self
    return Data(bytes: &num, count: MemoryLayout<UInt8>.size)
  }
}
extension UInt16 {
  var data: Data {
    var num = self
    return Data(bytes: &num, count: MemoryLayout<UInt16>.size)
  }
}

extension UInt32 {
  var data: Data {
    var num = self
    return Data(bytes: &num, count: MemoryLayout<UInt32>.size)
  }
}

extension Data {
  func toPointer() -> UnsafePointer<UInt8>? {
    
    let buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: self.count)
    let stream = OutputStream(toBuffer: buffer, capacity: self.count)
    
    stream.open()
    let value = self.withUnsafeBytes {
      $0.baseAddress?.assumingMemoryBound(to: UInt8.self)
    }
    guard let val = value else {
      return nil
    }
    
    stream.write(val, maxLength: self.count)
    
    stream.close()
    
    return UnsafePointer<UInt8>(buffer)
  }
}

extension URL {
  // https://stackoverflow.com/questions/38343186/write-extend-file-attributes-swift-example?answertab=active#tab-top
  /// Get extended attribute.
  func extendedAttribute(forName name: String) throws -> Data  {
    
    let data = try self.withUnsafeFileSystemRepresentation { fileSystemPath -> Data in
      
      // Determine attribute size:
      let length = getxattr(fileSystemPath, name, nil, 0, 0, 0)
      guard length >= 0 else { throw URL.posixError(errno) }
      
      // Create buffer with required size:
      var data = Data(count: length)
      
      // Retrieve attribute:
      let result =  data.withUnsafeMutableBytes { [count = data.count] in
        getxattr(fileSystemPath, name, $0.baseAddress, count, 0, 0)
      }
      guard result >= 0 else { throw URL.posixError(errno) }
      return data
    }
    return data
  }
  
  /// Set extended attribute.
  func setExtendedAttribute(data: Data, forName name: String) throws {
    
    try self.withUnsafeFileSystemRepresentation { fileSystemPath in
      let result = data.withUnsafeBytes {
        setxattr(fileSystemPath, name, $0.baseAddress, data.count, 0, 0)
      }
      guard result >= 0 else { throw URL.posixError(errno) }
    }
  }
  
  /// Remove extended attribute.
  func removeExtendedAttribute(forName name: String) throws {
    
    try self.withUnsafeFileSystemRepresentation { fileSystemPath in
      let result = removexattr(fileSystemPath, name, 0)
      guard result >= 0 else { throw URL.posixError(errno) }
    }
  }
  
  /// Get list of all extended attributes.
  func listExtendedAttributes() throws -> [String] {
    
    let list = try self.withUnsafeFileSystemRepresentation { fileSystemPath -> [String] in
      let length = listxattr(fileSystemPath, nil, 0, 0)
      guard length >= 0 else { throw URL.posixError(errno) }
      
      // Create buffer with required size:
      var namebuf = Array<CChar>(repeating: 0, count: length)
      
      // Retrieve attribute list:
      let result = listxattr(fileSystemPath, &namebuf, namebuf.count, 0)
      guard result >= 0 else { throw URL.posixError(errno) }
      
      // Extract attribute names:
      let list = namebuf.split(separator: 0).compactMap {
        $0.withUnsafeBufferPointer {
          $0.withMemoryRebound(to: UInt8.self) {
            String(bytes: $0, encoding: .utf8)
          }
        }
      }
      return list
    }
    return list
  }
  
  /// Helper function to create an NSError from a Unix errno.
  private static func posixError(_ err: Int32) -> NSError {
    return NSError(domain: NSPOSIXErrorDomain, code: Int(err),
                   userInfo: [NSLocalizedDescriptionKey: String(cString: strerror(err))])
  }
}

extension io_object_t {
  /// - Returns: The device's name.
  func name() -> String? {
    let buf = UnsafeMutablePointer<io_name_t>.allocate(capacity: 1)
    defer { buf.deallocate() }
    return buf.withMemoryRebound(to: CChar.self, capacity: MemoryLayout<io_name_t>.size) {
      if IORegistryEntryGetName(self, $0) == KERN_SUCCESS {
        return String(cString: $0)
      }
      return nil
    }
  }
  
  func info() -> NSDictionary? {
    var serviceDictionary : Unmanaged<CFMutableDictionary>?
    if IORegistryEntryCreateCFProperties(self, &serviceDictionary, kCFAllocatorDefault, 0) == KERN_SUCCESS {
      if let info : NSDictionary = serviceDictionary?.takeRetainedValue() {
        return info
      }
    }
    return nil
  }
}
