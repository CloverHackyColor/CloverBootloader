
/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef _PEXPERT_DEVICE_TREE_H_
#define _PEXPERT_DEVICE_TREE_H_

/*
-------------------------------------------------------------------------------
 Foundation Types
-------------------------------------------------------------------------------
*/
enum {
	kDTPathNameSeparator	= '/'				/* 0x2F */
};


/* Property Name Definitions (Property Names are C-Strings)*/
enum {
	kDTMaxPropertyNameLength=31	/* Max length of Property Name (terminator not included) */
};

typedef char DTPropertyNameBuf[32];


/* Entry Name Definitions (Entry Names are C-Strings)*/
enum {
	kDTMaxEntryNameLength		= 31	/* Max length of a C-String Entry Name (terminator not included) */
};

/* length of DTEntryNameBuf = kDTMaxEntryNameLength +1*/
typedef char DTEntryNameBuf[32];


/* Entry*/
typedef struct OpaqueDTEntry* DTEntry;

/* Entry Iterator*/
typedef struct OpaqueDTEntryIterator* DTEntryIterator;

/* Property Iterator*/
typedef struct OpaqueDTPropertyIterator* DTPropertyIterator;


/* status values*/
enum {
		kError = -1,
		kIterationDone = 0,
		kSuccess = 1
};

/*

Structures for a Flattened Device Tree
 */

//These definitions show the primitivity of C-language where there is no possibility to
//explain the structure of DT

#define kPropNameLength	32

typedef struct DTProperty {
    char		name[kPropNameLength];	// NUL terminated property name
    UINT32		length;		// Length (bytes) of following prop value
//  unsigned long	value[1];	// Variable length value of property
					// Padded to a multiple of a longword?
} DTProperty;

typedef struct OpaqueDTEntry {
    UINT32		nProperties;	// Number of props[] elements (0 => end)
    UINT32		nChildren;	// Number of children[] elements
//  DTProperty	props[];// array size == nProperties
//  DeviceTreeNode	children[];	// array size == nChildren
} DeviceTreeNode;

typedef DeviceTreeNode *RealDTEntry;

typedef struct DTSavedScope {
	struct DTSavedScope * nextScope;
	RealDTEntry scope;
	RealDTEntry entry;
	unsigned long index;		
} *DTSavedScopePtr;

/* Entry Iterator*/
typedef struct OpaqueDTEntryIterator {
	RealDTEntry outerScope;
	RealDTEntry currentScope;
	RealDTEntry currentEntry;
	DTSavedScopePtr savedScope;
	UINT32 currentIndex;		
} *RealDTEntryIterator;

/* Property Iterator*/
typedef struct OpaqueDTPropertyIterator {
	RealDTEntry entry;
	DTProperty *currentProperty;
	UINT32 currentIndex;
} *RealDTPropertyIterator;

/*
-------------------------------------------------------------------------------
 Device Tree Calls
-------------------------------------------------------------------------------
*/

/* Used to initalize the device tree functions. */
/* base is the base address of the flatened device tree */
VOID DTInit(VOID *base);

/*
-------------------------------------------------------------------------------
 Entry Handling
-------------------------------------------------------------------------------
*/
/* Compare two Entry's for equality. */
extern INTN DTEntryIsEqual(CONST DTEntry ref1, CONST DTEntry ref2);

/*
-------------------------------------------------------------------------------
 LookUp Entry by Name
-------------------------------------------------------------------------------
*/
/*
 DTFindEntry:
 Find the device tree entry that contains propName=propValue.
 It currently  searches the entire
 tree.  This function should eventually go in DeviceTree.c.
 Returns:    kSuccess = entry was found.  Entry is in entryH.
             kError   = entry was not found
*/
extern INTN DTFindEntry(const CHAR8 *propName, const CHAR8 *propValue, DTEntry *entryH);

/*
 Lookup Entry
 Locates an entry given a specified subroot (searchPoint) and path name.  If the
 searchPoint pointer is NULL, the path name is assumed to be an absolute path
 name rooted to the root of the device tree.
*/
extern INTN DTLookupEntry(const DTEntry searchPoint, const CHAR8 *pathName, DTEntry *foundEntry);

/*
-------------------------------------------------------------------------------
 Entry Iteration
-------------------------------------------------------------------------------
*/
/*
 An Entry Iterator maintains three variables that are of interest to clients.
 First is an "OutermostScope" which defines the outer boundry of the iteration.
 This is defined by the starting entry and includes that entry plus all of it's
 embedded entries. Second is a "currentScope" which is the entry the iterator is
 currently in. And third is a "currentPosition" which is the last entry returned
 during an iteration.

 Create Entry Iterator
 Create the iterator structure. The outermostScope and currentScope of the iterator
 are set to "startEntry".  If "startEntry" = NULL, the outermostScope and
 currentScope are set to the root entry.  The currentPosition for the iterator is
 set to "nil".
*/
extern INTN DTCreateEntryIterator(const DTEntry startEntry, DTEntryIterator *iterator);

/* Dispose Entry Iterator*/
extern INTN DTDisposeEntryIterator(DTEntryIterator iterator);

/*
 Enter Child Entry
 Move an Entry Iterator into the scope of a specified child entry.  The
 currentScope of the iterator is set to the entry specified in "childEntry".  If
 "childEntry" is nil, the currentScope is set to the entry specified by the
 currentPosition of the iterator.
*/
extern INTN DTEnterEntry(DTEntryIterator iterator, DTEntry childEntry);

/*
 Exit to Parent Entry
 Move an Entry Iterator out of the current entry back into the scope of it's parent
 entry. The currentPosition of the iterator is reset to the current entry (the
 previous currentScope), so the next iteration call will continue where it left off.
 This position is returned in parameter "currentPosition".
*/
extern INTN DTExitEntry(DTEntryIterator iterator, DTEntry *currentPosition);

/*
 Iterate Entries 
 Iterate and return entries contained within the entry defined by the current
 scope of the iterator.  Entries are returned one at a time. When
INTN== kIterationDone, all entries have been exhausted, and the
 value of nextEntry will be Nil. 
*/
extern INTN DTIterateEntries(DTEntryIterator iterator, DTEntry *nextEntry);

/*
 Restart Entry Iteration
 Restart an iteration within the current scope.  The iterator is reset such that
 iteration of the contents of the currentScope entry can be restarted. The
 outermostScope and currentScope of the iterator are unchanged. The currentPosition
 for the iterator is set to "nil".
*/
extern INTN DTRestartEntryIteration(DTEntryIterator iterator);

/*
-------------------------------------------------------------------------------
 Get Property Values
-------------------------------------------------------------------------------
*/
/*
 Get the value of the specified property for the specified entry.  

 Get Property
*/
extern INTN DTGetProperty(const DTEntry entry, const CHAR8 *propertyName, void **propertyValue, UINTN *propertySize);

/*
-------------------------------------------------------------------------------
 Iterating Properties
-------------------------------------------------------------------------------
*/
/*
 Create Property Iterator
 Create the property iterator structure. The target entry is defined by entry.
*/

extern INTN DTCreatePropertyIterator(const DTEntry entry,
					DTPropertyIterator *iterator);

/* Dispose Property Iterator*/
extern INTN DTDisposePropertyIterator(DTPropertyIterator iterator);

/*
 Iterate Properites
 Iterate and return properties for given entry.  
 WhenINTN== kIterationDone, all properties have been exhausted.
*/

extern INTN DTIterateProperties(DTPropertyIterator iterator,
						CHAR8 **foundProperty);

/*
 Restart Property Iteration
 Used to re-iterate over a list of properties.  The Property Iterator is
 reset to the beginning of the list of properties for an entry.
*/

extern INTN DTRestartPropertyIteration(DTPropertyIterator iterator);



// dmazar: do not have boot services when fixing dev tree in BootFixes,
// so need one version without AllocPool.
// caller should not call DTDisposePropertyIterator when using this version .
extern INTN DTCreatePropertyIteratorNoAlloc(CONST DTEntry entry, DTPropertyIterator iterator);

#endif /* _PEXPERT_DEVICE_TREE_H_ */