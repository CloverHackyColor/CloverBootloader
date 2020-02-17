/*
 * refit/list.c
 * list manager functions
 *
 * Copyright (c) 2013 Cadet-petit Armel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Cadet-petit Armel nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __REFITLIST_STANDARD_H__
#define __REFITLIST_STANDARD_H__


#include "libeg.h"

//
// NOTE: To avoid a duplicate symbol error, with the lib.c AddListElement
// do not remove this quote !!!
//
//#define RELEASE_LIST

//
// Refit Elements Linked List Signature.
//
#define REFIT_LIST_SIGNATURE SIGNATURE_32 ('R', 'E', 'L', 'L')

//
// Refit Elements Linked List Constructor Type.
//
typedef
EFI_STATUS
(EFIAPI *LIST_ELEMENT_CONSTRUCTOR) (
  IN OUT VOID   **Element,
  IN OUT VOID   *Parameters
  );

//
// Refit Elements Linked List Destructor Type.
//
typedef
EFI_STATUS
(EFIAPI *LIST_ELEMENT_DESTRUCTOR) (
  IN VOID   **Element
  );

//
// Refit Elements Linked List Entry definition.
//
//  Signature must be set to REFIT_LIST_SIGNATURE
//  Link is the linked list data.
//  Element is a pointer to the Element.
//  Constuctor is a pointer to the Element's constructor function.
//  Destructor is a pointer to the Element's destructor function.
//  ConstructorParameters is the to pass to the constructor.
//  Handle is used to identify a particular Element.
//
typedef struct {
  UINT32                       Signature;
  LIST_ENTRY                   Link;
  VOID                         *Element;
  LIST_ELEMENT_CONSTRUCTOR     Constructor;
  LIST_ELEMENT_DESTRUCTOR      Destructor;
  VOID                         *ConstructorParameters;
  UINTN                        Handle;
} REFIT_LIST;


EFI_STATUS
CreateListElement(IN OUT LIST_ENTRY   *List);

#ifdef RELEASE_LIST
EFI_STATUS
AddListElement(
                IN OUT LIST_ENTRY               *List,
                IN VOID                         *Element,
                IN OUT UINTN                    *CurrentHandle,
                IN LIST_ELEMENT_CONSTRUCTOR     Constructor            OPTIONAL,
                IN LIST_ELEMENT_DESTRUCTOR      Destructor             OPTIONAL,
                IN VOID                         *ConstructorParameters OPTIONAL
                );
#else
EFI_STATUS
Add_ListElement(
                IN OUT LIST_ENTRY               *List,
                IN VOID                         *Element,
                IN OUT UINTN                    *CurrentHandle,
                IN LIST_ELEMENT_CONSTRUCTOR     Constructor            OPTIONAL,
                IN LIST_ELEMENT_DESTRUCTOR      Destructor             OPTIONAL,
                IN VOID                         *ConstructorParameters OPTIONAL
                
                );
#endif

LIST_ENTRY *
FindElementByHandle(
                    IN      LIST_ENTRY                *List,
                    IN      CONST UINTN               Handle
                    );


EFI_STATUS
RemoveElementByHandle(
                      IN OUT  LIST_ENTRY                *List,
                      IN      CONST UINTN               Handle
                      );


EFI_STATUS
FreeListElement(IN OUT LIST_ENTRY *List);


#endif
/* EOF */
