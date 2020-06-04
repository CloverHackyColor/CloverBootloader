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

/*
 This is the minimum plist file (binary format) as a boolean true as root (the shorter value that can hold), and is 42 bytes:
 62 70 6C 69 73 74 30 30 09 08 00 00 00 00 00 00 01 01 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09
 
 Instead, the xml version have a length in bytes (xml1Header + "<true/>" + xml1Footer) of 179.
 Line feeds are present only in the header: this means that at least the header should be well formatted
 */
let minBinary : Int = 42
let minXml : Int = 179


final class PlistParser: NSObject, XMLParserDelegate {
  var root : PENode?
  var plistPath : String? = nil
  var error : String? = nil
  var isBinary : Bool = false
  var rootType : PlistTag = .String
  private var currentValue : String? = nil
  private var lastElement : String? = nil
  private var currentNode: PENode?
  
  //MARK: initialization
  override init() {
    super.init()
    self.root = nil
  }
  
  /// You already know the root node for the document.
  init(withNode node: PENode) {
    super.init()
    self.root = node
  }
  
  /// PlistParser initialization from Data (either string or binary data).
  /// Usually fileType and data comes from NSDocument
  convenience init(fromData data: Data, fileType: PEFileType) {
    var node : PENode? = nil
    var any : Any?
    
    var plistType : PEFileType = fileType
    
    // don't trust NSDocument: check our self if this is a binary plist
    if data.count >= minBinary {
      let binaryHeaderData: Data = binaryPlistHeader.data(using: String.Encoding.utf8)!
      let fileheader = data.subdata(in: 0..<binaryHeaderData.count)
      
      if fileheader == binaryHeaderData {
        plistType = .binaryPlistv1 // override!
      }
    }
    
    if plistType == .binaryPlistv1 {
      any = serialize(data: data)
      if (any != nil) {
        node = PlistParser.populateTreeWith(plist: any!)
      }
    } else {
      any = serialize(data: data)
      // don't fail!
      if any == nil {
        any = PEDictionary()
      }
    }
    
    /*
     determine if this is a stupid plist (root is not a Dictionary nor Array),
     otherwise init with node.
     Why? "node" is used only with binary plist where its Dictionaries aren't sorted as happen in a xml plist..
     ..but no need to use the parser if a plist is stupid
     */
    
    if (any != nil) {
      if any is NSString {
        node = PENode(representedObject: TagData(key: "", type: .String, value: any as! String))
      } else if any is NSNumber {
        let numberType = CFNumberGetType(any as! NSNumber)
        if numberType == .charType {
          // Bool
          node = PENode(representedObject: TagData(key: "", type: .Bool, value: any as! Bool))
        } else {
          // Number of any kind, but int or double..
          if any is PEInt {
            node = PENode(representedObject: TagData(key: "Root", type: .Number, value: any as! PEInt))
          } else {
            node = PENode(representedObject: TagData(key: "Root", type: .Number, value: any as! PEReal))
          }
        }
      } else if any is NSData {
        node = PENode(representedObject: TagData(key: "", type: .Data, value: any as! Data))
      } else if any is NSDate {
        node = PENode(representedObject: TagData(key: "", type: .Date, value: any as! Date))
      }
    }
    
    if (node != nil) {
      self.init(withNode: node!)
      self.isBinary = true
    } else  {
      self.init()
      let parser : XMLParser = XMLParser(data: data)
      self.currentValue = ""
      parser.delegate = self
      parser.parse()
    }
  }
  
  /// PlistParser initialization from path. NSDocument uses data, so this is for custom initializations.
  convenience init(fromPath path: String) {
    var realOrNilPath : String? = path // this will be nil if the loaded file is not a plist (will avoid to overwrite it)
    var data : Data? = nil
    var isPlist : Bool = false
    var node : PENode? = nil
    var PropertyListSerializationError : String? = nil
    var any : Any?
    data = FileManager.default.contents(atPath: path)
    
    // test if is a binary plist
    if (data != nil) {
      if (data?.count)! >= minBinary {
        let binaryHeaderData: Data = binaryPlistHeader.data(using: String.Encoding.utf8)!
        let presumedHeader = data?.subdata(in: 0 ..< binaryHeaderData.count)
        if presumedHeader == binaryHeaderData {
          isPlist = true
          any = serialize(data: data!)
          
          // don't fail!
          if any == nil {
            any = PEDictionary()
            PropertyListSerializationError = "PlistParser: Unknown error loading binary plist file"
          }
          node = PlistParser.populateTreeWith(plist: any!)
        }
      }
      
      // test if is a xml plist
      if !isPlist {
        if (data?.count)! >= minXml {
          let xmlHeaderData: Data = xml1Header.data(using: String.Encoding.utf8)!
          let presumedHeader = data?.subdata(in: 0 ..< xmlHeaderData.count)
          if presumedHeader == xmlHeaderData {
            isPlist = true
            any = serialize(data: data!)
          }
        }
      }
    }
    
    /*
     determine if this is a stupid plist (root is not a Dictionary nor Array),
     otherwise init with node.
     Why? "node" is used only with binary plist where its Dictionaries aren't sorted as happen in a xml plist..
     ..but no need to use the parser if a plist is stupid
     */
    
    if (any != nil) {
      if any is NSString {
        node = PENode(representedObject: TagData(key: "", type: .String, value: any as! String))
      } else if any is NSNumber {
        let numberType = CFNumberGetType(any as! NSNumber)
        if numberType == .charType {
          node = PENode(representedObject: TagData(key: "Root", type: .Bool, value: any as! Bool))
        } else {
          // Number of any kind, but int or double..
          if any is PEInt {
            node = PENode(representedObject: TagData(key: "Root", type: .Number, value: any as! PEInt))
          } else {
            node = PENode(representedObject: TagData(key: "Root", type: .Number, value: any as! PEReal))
          }
        }
      } else if any is NSData {
        node = PENode(representedObject: TagData(key: "", type: .Data, value: any as! Data))
      } else if any is NSDate {
        node = PENode(representedObject: TagData(key: "", type: .Date, value: any as! Date))
      }
    }
    
    
    if (node != nil) {
      self.init(withNode: node!)
      self.error = PropertyListSerializationError
      self.isBinary = true
    } else  {
      self.init(fromData: (data ?? (xml1Header + "\n<dict/>\n" + xml1Footer).data(using: String.Encoding.utf8))!,
                fileType: PEFileType.xmlPlistv1)
    }
    
    
    if isPlist {
      realOrNilPath = path
    }
    self.plistPath = realOrNilPath
  }
  
  func parser(_ parser: XMLParser,
              didStartElement elementName: String,
              namespaceURI: String?,
              qualifiedName qName: String?,
              attributes attributeDict: [String : String] = [:]) {
    switch elementName {
    case "dict":
      if (self.root == nil) {
        self.rootType = .Dictionary
        self.root = self.getNewDictNode()
        self.currentNode = self.root
      } else {
        if lastElement == "array" || self.currentNode!.tagdata!.type == .Array {
          let new = self.getNewDictNode()
          self.currentNode?.mutableChildren.add(new)
          self.currentNode = new
          break
        } else if self.lastElement == "key" {
          self.currentNode?.tagdata?.type = .Dictionary
          self.currentNode?.tagdata?.value = nil
        } else {
          let new = self.getNewDictNode()
          self.currentNode?.parent?.mutableChildren.add(new)
          self.currentNode = new
        }
      }
      break
    case "array":
      if (self.root == nil) {
        self.rootType = .Array
        self.root = self.getNewArrayNode()
        self.currentNode = self.root
      } else {
        if self.lastElement == "key" {
          self.currentNode?.tagdata?.type = .Array
          self.currentNode?.tagdata?.value = nil
        } else {
          let new = self.getNewArrayNode()
          self.currentNode?.parent?.mutableChildren.add(new)
          self.currentNode = new
        }
      }
      break
    case "key":
      let new = self.getNewStringNode()
      self.currentNode?.mutableChildren.add(new)
      self.currentNode = new
      break
    case "string":
      if self.lastElement != "key" {
        let new = self.getNewStringNode()
        self.addNewChildren(newNode: new)
      }
      break
    case "date":
      if self.lastElement != "key" {
        let new = self.getNewDateNode()
        self.addNewChildren(newNode: new)
      }
      break
    case "data":
      if self.lastElement != "key" {
        let new = self.getNewDataNode()
        self.addNewChildren(newNode: new)
      }
      break
    case "real":
      if self.lastElement != "key" {
        let new = self.getNewNumberNode()
        self.addNewChildren(newNode: new)
      }
      break
    case "integer":
      if self.lastElement != "key" {
        let new = self.getNewNumberNode()
        self.addNewChildren(newNode: new)
      }
      break
    case "true":
      if self.lastElement != "key" {
        let new = self.getNewBoolNode()
        self.addNewChildren(newNode: new)
      }
      break
    case "false":
      if self.lastElement != "key" {
        let new = self.getNewBoolNode()
        self.addNewChildren(newNode: new)
      }
      break
    default:
      break
    }
    self.lastElement = elementName
  }
  
  //MARK: XMLParser functions
  func parser(_ parser: XMLParser,
              didEndElement elementName: String,
              namespaceURI: String?,
              qualifiedName qName: String?) {
    if (self.root != nil) {
      let value : String? = self.currentValue?.trimmingCharacters(in: CharacterSet.whitespacesAndNewlines)
      switch elementName {
      case "dict":
        self.goback()
        break
      case "array":
        self.goback()
        break
      case "key":
        self.currentNode?.tagdata?.key = value?.decodingXMLCharacters ?? ""
        break
      case "string":
        self.currentNode?.tagdata?.value =  value!.decodingXMLCharacters
        self.currentNode?.tagdata?.type = .String
        self.goback()
        break
      case "date":
        self.currentNode?.tagdata?.value = utcStringToDate(value!)
        self.currentNode?.tagdata?.type = .Date
        self.goback()
        break
      case "data":
        let data : Data = Data(base64Encoded: value!, options: Data.Base64DecodingOptions.ignoreUnknownCharacters)!
        self.currentNode?.tagdata?.value = data
        self.currentNode?.tagdata?.type = .Data
        self.goback()
        break
      case "real":
        self.currentNode?.tagdata?.value = PEReal(value!)!
        self.currentNode?.tagdata?.type = .Number
        self.goback()
        break
      case "integer":
        self.currentNode?.tagdata?.value = PEInt(value!)!
        self.currentNode?.tagdata?.type = .Number
        self.goback()
        break
      case "true":
        self.currentNode?.tagdata?.value = true
        self.currentNode?.tagdata?.type = .Bool
        self.goback()
        break
      case "false":
        self.currentNode?.tagdata?.value = false
        self.currentNode?.tagdata?.type = .Bool
        self.goback()
        break
      default:
        break
      }
    }
    self.lastElement = elementName
    self.currentValue = ""
  }
  
  func parser(_ parser: XMLParser, foundCharacters string: String) {
    self.currentValue = self.currentValue?.appending(string)
  }
  
  func parserDidStartDocument(_ parser: XMLParser) {
  }
  
  func parserDidEndDocument(_ parser: XMLParser) {
    if self.rootType == .Dictionary || self.rootType == .Array {
      self.root?.tagdata?.key = "Root"
    }
  }
  
  func parser(_ parser: XMLParser, foundIgnorableWhitespace whitespaceString: String) {
    
  }
  
  func parser(_ parser: XMLParser, parseErrorOccurred parseError: Error) {
    self.root = nil
    self.error = "\(parseError)".replacingOccurrences(of: "NSXML", with: "Plist")
  }
  
  //MARK: Helper functions for XMLParser
  private func addNewChildren(newNode: PENode) {
    if (self.root == nil) {
      self.rootType = (newNode.tagdata?.type)!
      self.root = newNode
      self.currentNode = self.root
    } else {
      self.currentNode?.mutableChildren.add(newNode)
      self.currentNode = newNode
    }
  }
  
  func isChildTag(tag: String) -> Bool {
    return tag == "string" ||
      tag == "date" ||
      tag == "data" ||
      tag == "real" ||
      tag == "integer" ||
      tag == "true" ||
      tag == "false"
  }
  
  private func goback() {
    if ((self.currentNode?.peparent) == nil) {
      self.currentNode = self.root
    } else {
      self.currentNode = self.currentNode?.peparent
    }
  }
  
  //MARK: Get empty nodes
  private func getNewDictNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .Dictionary,
                                             value: nil))
  }
  
  private func getNewArrayNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .Array,
                                             value: nil))
  }
  
  private func getNewStringNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .String,
                                             value: ""))
  }
  
  private func getNewNumberNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .Number,
                                             value: 0))
  }
  
  private func getNewDataNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .Data,
                                             value: Data()))
  }
  
  private func getNewDateNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .Date,
                                             value: Date()))
  }
  
  private func getNewBoolNode() -> PENode {
    return PENode(representedObject: TagData(key: localizedNewItem,
                                             type: .Bool,
                                             value: false))
  }
  
  //MARK: class Helper functions for binary plist
  
  /// Populate a PENode with the contents of a plist
  class func populateTreeWith(plist: Any) -> PENode {
    let data: TagData = TagData(key: "Root", type: .Dictionary, value: nil) // initialization
    let node: PENode = PENode(representedObject: data)
    add(child: plist, parent: node)
    return node
  }
  
  /// Adds a new PENode with the contents of a child value.
  /// This function recursively add nodes to the initial parent node
  class func add(child: Any, parent: PENode) {
    if child is PEDictionary {
      let dict = child as! PEDictionary
      for key in dict.keys {
        let curVal = dict[key]
        var type : PlistTag
        if curVal is PEDictionary {
          type = .Dictionary
        } else if curVal is PEArray {
          type = .Array
        } else if (curVal is PEInt || curVal is PEReal) {
          type = .Number
        } else if curVal is Bool {
          type = .Bool
        } else if curVal is String {
          type = .String
        } else if curVal is Data {
          type = .Data
        } else if curVal is Date {
          type = .Date
        } else {
          break
          //fatalError("add(child: Any, parent: PENode) unsupported tag for \(curVal ?? "Unknown")")
        }
        
        let d = TagData(key: key,
                        type: type,
                        value: (type == .Array || type == .Dictionary) ? nil : curVal)
        let node = PENode(representedObject: d)
        parent.mutableChildren.add(node)
        add(child: curVal!, parent: node)
      }
    } else if child is PEArray {
      let arr = child as! PEArray
      for i in 0..<arr.count {
        let curVal = arr[i]
        var type : PlistTag
        if curVal is PEDictionary {
          type = .Dictionary
        } else if curVal is PEArray {
          type = .Array
        } else if (curVal is PEInt || curVal is PEReal) {
          type = .Number
        } else if curVal is Bool {
          type = .Bool
        } else if curVal is String {
          type = .String
        } else if curVal is Data {
          type = .Data
        } else if curVal is Date {
          type = .Date
        } else {
          break
          //fatalError("add(child: Any, parent: PENode) unsupported tag for \(curVal)")
        }
        
        let d = TagData(key: "\(localizedItem) \(i)",
          type: type,
          value: (type == .Array || type == .Dictionary) ? nil : curVal)
        let node = PENode(representedObject: d)
        parent.mutableChildren.add(node)
        add(child: curVal, parent: node)
      }
    }
  }
}



