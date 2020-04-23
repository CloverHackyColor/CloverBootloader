///*
// * refit/list.c
// * list manager functions
// *
// * Copyright (c) 2013 Cadet-petit Armel
// * All rights reserved.
// *
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions are
// * met:
// *
// *  * Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// *
// *  * Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the
// *    distribution.
// *
// *  * Neither the name of Cadet-petit Armel nor the names of the
// *    contributors may be used to endorse or promote products derived
// *    from this software without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// */
//
//#include "Platform.h"
//#include "list.h"
//
//
//#ifndef DEBUG_ALL
//#define DEBUG_LIST 1
//#else
//#define DEBUG_LIST DEBUG_ALL
//#endif
//
//#if DEBUG_LIST == 0
//#define DBG(...)
//#else
//#define DBG(...) DebugLog(DEBUG_LIST, __VA_ARGS__)
//#endif
//
//
//EFI_STATUS
//CreateListElement(IN OUT LIST_ENTRY   *List)
//{
//
//  //
//  // Check for invalid input parameters
//  //
//  if (List == NULL) {
//    return EFI_INVALID_PARAMETER;
//  }
//
//  //
//  // Initializes the List
//  //
//  InitializeListHead (List);
//
//  return EFI_SUCCESS;
//}
//
//#ifdef RELEASE_LIST
//EFI_STATUS
//AddListElement(
//                IN OUT LIST_ENTRY               *List,
//                IN VOID                         *Element,
//                IN OUT UINTN                    *CurrentHandle,
//                IN LIST_ELEMENT_CONSTRUCTOR     Constructor            OPTIONAL,
//                IN LIST_ELEMENT_DESTRUCTOR      Destructor             OPTIONAL,
//                IN VOID                         *ConstructorParameters OPTIONAL
//
//                )
//#else
//EFI_STATUS
//Add_ListElement(
//               IN OUT LIST_ENTRY               *List,
//               IN VOID                         *Element,
//               IN OUT UINTN                    *CurrentHandle,
//               IN LIST_ELEMENT_CONSTRUCTOR     Constructor            OPTIONAL,
//               IN LIST_ELEMENT_DESTRUCTOR      Destructor             OPTIONAL,
//               IN VOID                         *ConstructorParameters OPTIONAL
//
//               )
//#endif
//{
//
//  REFIT_LIST       *CurrentList;
//  EFI_STATUS       Status;
//  UINTN            Handle;
//
//
//
//  //
//  // Check for invalid input parameters
//  //
//  if (List == NULL) {
//    return EFI_INVALID_PARAMETER;
//  }
//
//  if (CurrentHandle == NULL) {
//    return EFI_INVALID_PARAMETER;
//  }
//
//  Handle = *CurrentHandle;
//
//  //
//  // Initializes the element if the Constructor is not NULL
//  //
//  if (Constructor != NULL) {
//
//    Status = Constructor(&Element, ConstructorParameters);
//
//    if (EFI_ERROR(Status)) {
//      return EFI_ABORTED;
//    }
//#ifdef DEBUG_LIST
//    if (Destructor == NULL) {
//      DBG("Warning: The element have a constructor but no destructor, possible memory leak !!!\n");
//    }
//#endif
//  }
//#ifdef DEBUG_LIST
//  else {
//
//    if (Element == NULL) {
//      return EFI_INVALID_PARAMETER;
//    }
//
//    if (Destructor != NULL) {
//      DBG("Warning: The element have a destructor but no constructor, an unpredictable bug may occur (or not) !!!\n");
//    }
//  }
//#endif
//
//  //
//  // Create a new list entry
//  //
//  CurrentList = (__typeof__(CurrentList))AllocateZeroPool (sizeof (REFIT_LIST));
//
//  if (!CurrentList) {
//    return EFI_OUT_OF_RESOURCES;
//  }
//
//  //
//  // Increment the handle
//  //
//  Handle++;
//
//  //
//  // Initialize the list entry contents
//  //
//  CurrentList->Element     = Element;
//  CurrentList->Signature   = REFIT_LIST_SIGNATURE;
//  CurrentList->Destructor  = Destructor;
//  CurrentList->Handle      = Handle;
//
//  //
//  // NOTE: It should be useless to store this but let store it anyway
//  //
//  CurrentList->Constructor = Constructor;
//  CurrentList->ConstructorParameters = ConstructorParameters;
//
//  //
//  // Return the handle
//  //
//  *CurrentHandle = Handle;
//
//  //
//  // Insert the list entry at the end of the list
//  //
//  InsertTailList (List, &CurrentList->Link);
//
//  return EFI_SUCCESS;
//}
//
//LIST_ENTRY *
//FindElementByHandle(
//                    IN      LIST_ENTRY                *List,
//                    IN      CONST UINTN               Handle
//                    )
//{
//
//  LIST_ENTRY		             *Link;
//	REFIT_LIST                 *Entry;
//
//  //
//  // Check for invalid input parameters
//  //
//  if (List == NULL) {
//    return NULL;
//  }
//
//  if (Handle == 0) {
//    //
//    // The first handle of the list is 1, if 0 is passed in parameters it's useless to go further
//    // and we return the list pointer
//    //
//    return List;
//  }
//
//  //
//  // Find the entry in the list with the given handle (if any)
//  //
//  if (IsListEmpty(List) == FALSE) {
//		for (Link = List->ForwardLink; Link != List; Link = Link->ForwardLink) {
//
//      if (Link != NULL) {
//
//        if (Link != List) {
//
//          Entry = CR(Link, REFIT_LIST, Link, REFIT_LIST_SIGNATURE);
//
//          if (Entry  != NULL) {
//
//            if (Handle == Entry->Handle) {
//              return Link;
//
//            }
//          }
//        }
//
//      }
//	}
//  }
//
//  return List;
//}
//
//EFI_STATUS
//RemoveElementByHandle(
//                      IN OUT  LIST_ENTRY                *List,
//                      IN      CONST UINTN               Handle
//                      )
//{
//
//  LIST_ENTRY		         *Link;
//  REFIT_LIST                 *Entry;
//  EFI_STATUS                 Status;
//  VOID                       *Element;
//
//  //
//  // Check for invalid input parameters
//  //
//  if (List == NULL) {
//    return EFI_INVALID_PARAMETER;
//  }
//
//  if (Handle == 0) {
//    //
//    // The first handle of the list is 1, if 0 is passed in parameters it's useless to go further
//    // and we return the list pointer
//    //
//    return EFI_INVALID_PARAMETER;
//  }
//
//  //
//  // Remove and Free the entry of the list with the given handle
//  // (additionally destroy the associated element if the destructor is not NULL)
//  //
//  if ((Link = FindElementByHandle(List,Handle)) != List) {
//
//    if (Link != NULL) {
//
//      Entry = CR(Link, REFIT_LIST, Link, REFIT_LIST_SIGNATURE);
//
//      if (Entry != NULL) {
//        Element = Entry->Element;
//
//        if (Element != NULL) {
//          if (Entry->Destructor != NULL) {
//
//            //
//            // Destroy the element with the given destructor
//            //
//            Status = Entry->Destructor(&Element);
//
//            if (EFI_ERROR(Status)) {
//              return EFI_ABORTED;
//            }
//
//          }
//        }
//
//        RemoveEntryList (Link);
//
//        FreePool(Entry);
//      }
//
//    }
//
//  } else {
//    return EFI_NOT_FOUND;
//  }
//
//  return EFI_SUCCESS;
//}
//
//EFI_STATUS
//FreeListElement(IN OUT LIST_ENTRY *List)
//{
//  LIST_ENTRY		             *Link;
//	REFIT_LIST                 *Entry;
//  VOID                       *Element;
//  EFI_STATUS                 Status;
//
//  //
//  // Check for invalid input parameters
//  //
//  if (List == NULL) {
//    return EFI_INVALID_PARAMETER;
//  }
//
//  //
//  // Remove and Free all entrys of the list (additionally destroy all elements that have a destructor)
//  //
//  if (IsListEmpty(List) == FALSE) {
//		for (Link = List->ForwardLink; Link != List; Link = Link->ForwardLink) {
//
//      if (Link != NULL) {
//
//        if (Link != List) {
//          Entry = CR(Link, REFIT_LIST, Link, REFIT_LIST_SIGNATURE);
//
//          if (Entry != NULL) {
//            Element = Entry->Element;
//
//            if (Element != NULL) {
//              if (Entry->Destructor != NULL) {
//
//                //
//                // Destroy the element with the given destructor
//                //
//                Status = Entry->Destructor(&Element);
//
//                if (EFI_ERROR(Status)) {
//                  return EFI_ABORTED;
//                }
//
//              }
//            }
//
//            RemoveEntryList (Link);
//
//            FreePool(Entry);
//
//          }
//        }
//      }
//		}
//	} else {
//    return EFI_NOT_FOUND;
//  }
//
//  return EFI_SUCCESS;
//
//}
//
//// EOF
