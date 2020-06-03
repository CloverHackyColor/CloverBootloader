//
//  Extensions.swift
//  Clover
//
//  Created by vector sigma on 19/10/2019.
//  Copyright © 2019 CloverHackyColor. All rights reserved.
//

import Cocoa
import CommonCrypto

extension String {
  /// Create `Data` from hexadecimal string representation
  ///
  /// This takes a hexadecimal representation and creates a `Data` object. Note, if the string has any spaces or non-hex characters (e.g. starts with '<' and with a '>'), those are ignored and only hex characters are processed.
  ///
  /// - returns: Data represented by this hexadecimal string.
  
  func hexadecimal() -> Data? {
    var data = Data(capacity: self.count / 2)
    
    let regex = try! NSRegularExpression(pattern: "[0-9a-f]{1,2}", options: .caseInsensitive)
    regex.enumerateMatches(in: self, range: NSMakeRange(0, utf16.count)) { match, flags, stop in
      let byteString = (self as NSString).substring(with: match!.range)
      var num = UInt8(byteString, radix: 16)!
      data.append(&num, count: 1)
    }
    
    guard data.count > 0 else { return nil }
    
    return data
  }
  
  /// - returns: A  quoted string for nvram
  var nvramString: String {
    get {
      return "'\(self)'"
    }
  }
  
  
  /// - returns: Bool value indicating that our string start with the same prefix ignoring casing.
  func hasPrefixIgnoringCase(_ str: String) -> Bool {
    return self.lowercased().hasPrefix(str.lowercased())
  }
  
  /// - returns: only digits contained from the given string
  var keepNumericsOnly: String {
    get {
      return self.components(separatedBy: CharacterSet(charactersIn: "0123456789").inverted).joined(separator: "")
    }
  }
  
  //: ### Base64 encoding a string
  func base64Encoded() -> String? {
    if let data = self.data(using: .utf8) {
      return data.base64EncodedString()
    }
    return nil
  }
  
  //: ### Base64 decoding a string
  func base64Decoded() -> String? {
    if let data = Data(base64Encoded: self) {
      return String(data: data, encoding: .utf8)
    }
    return nil
  }
  
  //: ### Base64 encoding data
  func base64EncodedHex() -> String? {
    if let data = self.hexadecimal() {
      return data.base64EncodedString()
    }
    return nil
  }
  
  //: ### Base64 decoding data
  func base64DecodedHex() -> String? {
    if let data = Data(base64Encoded: self, options: Data.Base64DecodingOptions.ignoreUnknownCharacters) {
      return data.hexadecimal()
    }
    return nil
  }

  public func noSpaces() -> String {
    return self.trimmingCharacters(in: CharacterSet.whitespaces)
  }
  
  var lastPath: String {
    get {
      return (self as NSString).lastPathComponent
    }
  }
  
  var fileExtension: String {
    get {
      return (self as NSString).pathExtension
    }
  }
  
  var deletingLastPath: String {
    get {
      return (self as NSString).deletingLastPathComponent
    }
  }
  
  var deletingFileExtension: String {
    get {
      return (self as NSString).deletingPathExtension
    }
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
  
  /// Replace any occurences of a string (word) with a new one (newWord)
  ///
  /// - returns: String where word search is case insensitive while newWord replacing is case sensitive..
  
  func replacingOccurrencesOf(inSensitive word: String, withSensitive newWord: String) -> String {
    //print("word = \(word)")
    //print("newWord = \(newWord)")
    var newText = self
    if let range = newText.lowercased().range(of: word.lowercased()) {
      newText = newText.replacingOccurrences(of: word,
                                             with: newWord,
                                             options: String.CompareOptions.caseInsensitive,
                                             range: range)
    }
    // check again
    let r = newText.lowercased().range(of: word.lowercased())
    if (r != nil) {
      newText = newText.replacingOccurrencesOf(inSensitive: word, withSensitive: newWord)
    }
    return newText
  }
  
  // NSUserInterfaceItemIdentifier
  func interfaceId() -> NSUserInterfaceItemIdentifier {
    return NSUserInterfaceItemIdentifier(self)
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
  
  /// Encode XML special characthers such:
  /// & as &amp, \ as &quot, ' as &apos,  < as &lt, and > as &gt
  var encodingXMLCharacters: String {
    get {
      /*
       "   &quot;
       '   &apos;
       <   &lt;
       >   &gt;
       &   &amp;
       */
      
      var s = self
      s = s.replacingOccurrences(of: "&",  with: "&amp;")
      s = s.replacingOccurrences(of: "\"", with: "&quot;")
      s = s.replacingOccurrences(of: "'",  with: "&apos;")
      s = s.replacingOccurrences(of: "<",  with: "&lt;")
      s = s.replacingOccurrences(of: ">",  with: "&gt;")
      print(s)
      return s
    }
  }
  
  /// Decode XML  characters  such:
  /// &amp to &, &quot to \ , ' to &apos,  &lt to <, and &gt to >
  var decodingXMLCharacters: String {
    get {
      /*
       "   &quot;
       '   &apos;
       <   &lt;
       >   &gt;
       &   &amp;
       */
      
      var s = self
      s = s.replacingOccurrences(of: "&amp;",  with: "&")
      s = s.replacingOccurrences(of: "&quot;", with: "\"")
      s = s.replacingOccurrences(of: "&apos;", with: "'")
      s = s.replacingOccurrences(of: "&lt;",   with: "<")
      s = s.replacingOccurrences(of: "&gt;",   with: ">")
      return s
    }
  }
}

extension NSNumber {
  var hexString: String {
    get {
      if (self.intValue > 0xFFFF) {
        return String(format: "0x%08llx", self.intValue)
      } else {
        return String(format: "0x%04llx", self.intValue)
      }
    }
  }
}

extension Int {
  var data: Data {
    get {
      var num = self
      return Data(bytes: &num, count: MemoryLayout<Int>.size)
    }
  }
}

extension UInt8 {
  var data: Data {
    get {
      var num = self
      return Data(bytes: &num, count: MemoryLayout<UInt8>.size)
    }
  }
}
extension UInt16 {
  var data: Data {
    get {
      var num = self
      return Data(bytes: &num, count: MemoryLayout<UInt16>.size)
    }
  }
}

extension UInt32 {
  var data: Data {
    get {
      var num = self
      return Data(bytes: &num, count: MemoryLayout<UInt32>.size)
    }
  }
}

extension Data {
  /// Create hexadecimal string representation of `Data` object.
  ///
  /// - returns: `String` representation of this `Data` object.
  
  func hexadecimal() -> String {
    return map { String(format: "%02x", $0) }
      .joined(separator: "")
  }
  
  var sha1: String {
    get {
      var h = [UInt8](repeating: 0, count: Int(CC_SHA1_DIGEST_LENGTH))
      
      self.withUnsafeBytes {
        _ = CC_SHA1($0.baseAddress, CC_LONG(self.count), &h)
      }
      return Data(h).hexadecimal()
    }
  }
  
  func castToCPointer<T>() -> T {
    let mem = UnsafeMutablePointer<T>.allocate(capacity: 1)
    _ = self.copyBytes(to: UnsafeMutableBufferPointer(start: mem, count: 1))
    let val =  mem.move()
    mem.deallocate()
    return val
  }
  
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

extension NSImage {
  func resize(to newSize: NSSize) -> NSImage {
    let frame = NSRect(x: 0, y: 0, width: newSize.width, height: newSize.height)
    if let representation = self.bestRepresentation(for: frame, context: nil, hints: nil) {
      let image = NSImage(size: newSize, flipped: false, drawingHandler: { (_) -> Bool in
        return representation.draw(in: frame)
      })
      
      return image
    }
    return self
  }
}

extension NSBitmapImageRep {
  var png: Data? {
    get {
      representation(using: .png, properties: [:])
    }
  }
}

extension NSView {
  func removeAllConstraints() {
    self.removeConstraints(self.constraints)
    for view in self.subviews {
      view.removeAllConstraints()
    }
  }
  
  @IBInspectable var cornerRadius: CGFloat {
    get {
      return self.layer?.cornerRadius ?? 0
    } set {
      self.wantsLayer = true
      self.layer?.masksToBounds = true
      self.layer?.cornerRadius = CGFloat(Int(newValue * 100)) / 100
    }
  }
}

