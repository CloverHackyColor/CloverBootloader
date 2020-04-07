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

import Foundation

//MARK: Plist Editor custom types

typealias PEArray = [Any]
typealias PEDictionary = [String : Any]
typealias PEInt  = Int
typealias PEReal = Double

//MARK: Plist Editor file types
enum PEFileType : String {
  case xmlPlistv1 = "XML PropertyList v1"
  case binaryPlistv1 = "Binary PropertyList v1"
  case xml = "XML"
}

//MARK: supported plist object type
enum PlistTag : Int {
  case Dictionary = 0
  case Array      = 1
  case String     = 2
  case Number     = 3
  case Bool       = 4
  case Date       = 5
  case Data       = 6
}

//MARK: PETableCellView behaviour
enum PETableCellViewType: Int {
  case key
  case value
  case tags
  case bool
}

//MARK: Autosave
let kAutoSavePlistsKey : String  = "autoSavePlists"

//MARK: Paste board types
let kMyPBoardTypeXml  = NSPasteboard.PasteboardType(rawValue: "public.xml")
let kMyPBoardTypeData =  NSPasteboard.PasteboardType(rawValue: "PBoardType.data")

//MARK: Header and footer
let xml1Header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n<plist version=\"1.0\">"
let xml1Footer = "</plist>"

let binaryPlistHeader = "bplist00"



