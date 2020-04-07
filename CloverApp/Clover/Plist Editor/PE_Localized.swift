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

//MARK: Undo and Redo
let localizedUndo = "Undo".locale
let localizedRedo = "Redo".locale

let localizedUndoRedoTyping = "typing".locale
let localizedUndoRedoChangeType = "change type".locale
let localizedUndoRedoChangeBoolValue = "change bool value".locale
let localizedUndoRedoReplaceDuplicateKey = "replace duplicate key".locale
let localizedUndoRedoMoveItem = "move item".locale
let localizedUndoRedoPasteItem = "paste Item".locale
let localizedUndoRedoRemoveItem = "remove Item".locale
let localizedUndoRedoCutItem = "cut Item".locale
let localizedUndoRedoAddNewItem = "add new Item".locale

//MARK: localized words for the outline
let localizedHeaderKey = "Key".locale
let localizedHeaderType = "Type".locale
let localizedHeaderValue = "Value".locale

//MARK: Plist tags
let localizedUnsupported = "Unsupported".locale
let localizedDictionary = "Dictionary".locale
let localizedArray = "Array".locale
let localizedString = "String".locale
let localizedNumber = "Number".locale
let localizedBool = "Bool".locale
let localizedDate = "Date".locale
let localizedData = "Data".locale
let localizedYes = "YES".locale
let localizedNo  = "NO".locale

//MARK: Placheholders
let localizedItem  = "Item".locale
let localizedItems = "Items".locale
let localizedUntiteled = "Untiteled".locale
let localizedNewItem = "New Item".locale
let localizedBytes = "bytes".locale

//MARK: Wrong editing
let DataEmptyString  = "No data".locale
let DataMissingOpenTag  = "missing '<' at the beginning".locale
let DataMissingEndTag  = "missing '>' at the end".locale
let DataIllegalHex  = "Your data contains illegal characters".locale
let DataOddBytes  = "bytes count is odd, must be even".locale
let localizedKeepEditing = "Keep editing".locale

let localizedInvalidValueMsgText = "Invalid value detected!".locale
let localizedInvalidValueInfoText = "Your edit is not valid. Do you want to restore last valid value or keep editing?".locale

//MARK: Search & Replace
let localizedSearch  = "Search".locale
let localizedReplace = "Replace".locale

// the 3 string below is for an alert that indicate you that you are attempting to have a duplicate key in the same Dictionary that is not allowed
let localizedDuplicateKeyMsgText = "Duplicate key in the Dictionary!".locale
let localizedDuplicateKeyInfoText = "'%@' is already present in the Dictionary. Do you want to undo the editing or replace the existing key?".locale
// text for the buttons of this alert
let localizedDuplicateKeyReplaceButton = "Replace existing key".locale
