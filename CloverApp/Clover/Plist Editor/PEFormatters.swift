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

//MARK: UTC Date formatters
func utcDateFormatter() -> DateFormatter {
  let formatter = DateFormatter()
  formatter.timeZone = TimeZone(abbreviation: "UTC")
  formatter.locale = Locale(identifier: "en_US_POSIX")
  formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss'Z'"
  return formatter
}

func utcStringToDate(_ str: String) -> Date {
  return utcDateFormatter().date(from: str)!
}

func utcDateToString(_ str: Date) -> String {
  return utcDateFormatter().string(from: str)
}

//MARK: localized Date formatters to return current timezone and custom date style
func localizedDateFormatter() -> DateFormatter {
  let formatter = DateFormatter()
  formatter.timeZone = TimeZone.current
  formatter.locale = Locale.current
  formatter.dateStyle = .medium
  formatter.timeStyle = .medium
  return formatter
}

func localizedStringToDate(_ str: String) -> Date {
  return localizedDateFormatter().date(from: str)!
}

func localizedStringToDateS(_ str: String) -> Date? { /* safe version that can safely be nil */
  return localizedDateFormatter().date(from: str)
}

func localizedDateToString(_ date: Date) -> String {
  return localizedDateFormatter().string(from: date)
}

func funcyDateFromUser(_ str: String) -> Date? {
  let detector = try? NSDataDetector(types: NSTextCheckingResult.CheckingType.date.rawValue)
  let result: NSTextCheckingResult? = detector?.firstMatch(in: str, options: [], range: NSRange(location: 0, length: str.count))
  if result?.resultType == .date {
    let date: Date? = result?.date
    if date != nil {
      return date!
    }
  }
  return nil
}


//MARK: Number formatters
func localizedNumberFormatter() -> NumberFormatter {
  let formatter = NumberFormatter()
  formatter.locale = Locale.current
  formatter.numberStyle = .decimal
  //formatter.groupingSeparator = formatter.locale.groupingSeparator
  formatter.minimumFractionDigits = 0
  formatter.maximumFractionDigits = 8
  return formatter
}

func localizedStringFrom(number: NSNumber) -> String {
  return localizedNumberFormatter().string(from: number)!
}

/*
 what will goes in the Outlineview cell is a localized string that use custom decimal/thousand separator
 */
func numberFromLocalizedString(string: String) -> NSNumber {
  // check if is an hex string
  if string.hasPrefix("0x") || string.hasPrefix("0X") {
    //get string after 0x
    let comp = string.lowercased().components(separatedBy: "0x")
    if comp.count == 2 && stringContainsOnlyHexChars(string: comp[1]) {
      let scanner = Scanner(string: string)
      var value: UInt64 = 0
      if scanner.scanHexInt64(&value) {
        return NSNumber(value: value)
      }
    }
  }
  
  let nf = localizedNumberFormatter()
  let separator = nf.decimalSeparator
  var isNegative : Bool = false
  var str : String = string
  
  if str.count == 0 {
    return NSNumber(value: 0) // length is 0 .. number is 0
  }
  
  // check if is a negative number
  if str.hasPrefix("-") {
    isNegative = true
  }
  
  // if the first char is the decimal separator insert a 0 (e.g. ",2" to "0,2")
  if str.hasPrefix(separator!) {
    str = "0" + str
  }
  
  // clean the string from non numerical part (but keep the decimal separator)
  let setToKeep = CharacterSet.init(charactersIn: "0123456789" + separator!).inverted
  str = str.components(separatedBy: setToKeep).joined()
  // check the lenght again..
  if str.count == 0 {
    return NSNumber(value: 0)
  }
  // remove the separator if is the last char
  if str.hasSuffix(separator!) {
    str = String(str.dropLast())
  }
  // check the lenght again..
  if str.count == 0 {
    return NSNumber(value: 0)
  }
  /*
   check if we have 0 or at least one separator
   (if more than one truncate before the second!)
   */
  if (str.range(of: separator!) != nil) {
    let arr = str.components(separatedBy: separator!)
    if arr.count > 1 {
      str = arr[0] + "." + arr[1]
    }
  }
  
  // readd the subtraction operator if was there
  if str != "0" && isNegative {
    str = "-" + str
  }
  
  //determine if is integer or double
  var num : NSNumber? = nil
  if (str.range(of: ".") != nil) {
    num = NSNumber(value: (str as NSString).doubleValue)
  } else {
    num = NSNumber(value: (str as NSString).integerValue)
  }
  
  if (num != nil) {
    return num!
  }
  return NSNumber(value: 0)
}

//MARK: Data to string/string to Data
/*
 We expect the data enclosed in "<>", must have valid hex characters,
 must be multiple of 2
 */
func isHexStringValid(string: String) -> String {
  var hex = string.lowercased()
  if hex.count == 0 {
    return DataEmptyString
  }
  
  let hexSet : CharacterSet = CharacterSet(charactersIn: "0123456789abcdef")
  
  // remove blank spaces
  hex = hex.trimmingCharacters(in: CharacterSet.whitespacesAndNewlines)
  hex = hex.replacingOccurrences(of: " ", with: "")
  if hex.count == 0 {
    return DataEmptyString
  }
  
  // first char must be '<'
  if !hex.hasPrefix("<") {
    return DataMissingOpenTag
  }
  
  // well, is still >=2? (like "<>")
  if hex.count < 2 {
    return DataMissingEndTag
  }
  // last char must be '>'
  if !hex.hasSuffix(">") {
    return DataMissingEndTag
  }
  
  // contains only valid hex characters? testw/o "<>"
  var copy : String = String(hex.dropFirst(1))
  copy = String(copy.dropLast(1))
  
  if copy.trimmingCharacters(in: hexSet).count > 0 {
    return DataIllegalHex
  }
  
  if copy.count % 2 != 0 { // ----> w/o considering "<>"
    return DataOddBytes
  }
  
  return "HexSuccess"
}

func stringContainsOnlyHexChars(string: String) -> Bool {
  var hex = string.lowercased()
  if hex.count == 0 {
    return false
  }
  
  let hexSet : CharacterSet = CharacterSet(charactersIn: "0123456789abcdef")
  
  // remove blanck spaces
  hex = hex.trimmingCharacters(in: CharacterSet.whitespacesAndNewlines)
  if hex.count == 0 {
    return false
  }
  
  // contains only valid hex characters?
  if (hex.rangeOfCharacter(from: hexSet) == nil) {
    return false
  }
  
  return true
}

