/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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

#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include "device_tree.h"

#define round_long(x)	(((x) + 3UL) & ~(3UL))
#define next_prop(x)	((DeviceTreeNodeProperty *) (((UINT8*)x) + sizeof(DeviceTreeNodeProperty) + round_long(x->length)))

/* Entry*/

INTN DTInitialized;
RealDTEntry DTRootNode;

/*
 * Support Routines
 */
RealDTEntry
skipProperties(RealDTEntry entry)
{
	DeviceTreeNodeProperty *prop;
	UINTN k;

	if (entry == NULL || entry->nProperties == 0) {
		return NULL;
	} else {
		prop = (DeviceTreeNodeProperty *) (entry + 1);
		for (k = 0; k < entry->nProperties; k++) {
			prop = next_prop(prop);
		}
	}
	return ((RealDTEntry) prop);
}

RealDTEntry
skipTree(RealDTEntry root)
{
	RealDTEntry entry;
	UINTN k;

	entry = skipProperties(root);
	if (entry == NULL) {
		return NULL;
	}
	for (k = 0; k < root->nChildren; k++) {
		entry = skipTree(entry);
	}
	return entry;
}

RealDTEntry
GetFirstChild(RealDTEntry parent)
{
	return skipProperties(parent);
}

RealDTEntry
GetNextChild(RealDTEntry sibling)
{
	return skipTree(sibling);
}

CONST CHAR8*
GetNextComponent(CONST CHAR8 *cp, CHAR8 *bp)
{
	while (*cp != 0) {
		if (*cp == kDTPathNameSeparator) {
			cp++;
			break;
		}
		*bp++ = *cp++;
	}
	*bp = 0;
	return cp;
}

RealDTEntry
FindChild(RealDTEntry cur, CHAR8 *buf)
{
	RealDTEntry	child;
	UINT32      index;
	CHAR8*			str;
	UINTN      dummy;

	if (cur->nChildren == 0) {
		return NULL;
	}
	index = 1;
	child = GetFirstChild(cur);
	while (1) {
		if (DTGetProperty(child, "name", (VOID **)&str, &dummy) != kSuccess) {
			break;
		}
		if (AsciiStrCmp((CHAR8*)str, (CHAR8*)buf) == 0) {
			return child;
		}
		if (index >= cur->nChildren) {
			break;
		}
		child = GetNextChild(child);
		index++;
	}
	return NULL;
}


/*
 * External Routines
 */
VOID
DTInit(VOID *base)
{
	DTRootNode = (RealDTEntry) base;
	DTInitialized = (DTRootNode != 0);
}

INTN
DTEntryIsEqual(CONST DTEntry ref1, CONST DTEntry ref2)
{
	/* equality of pointers */
	return (ref1 == ref2);
}

CHAR8 *startingP;		// needed for find_entry
INTN find_entry(CONST CHAR8 *propName, CONST CHAR8 *propValue, DTEntry *entryH);

INTN DTFindEntry(CONST CHAR8 *propName, CONST CHAR8 *propValue, DTEntry *entryH)
{
	if (!DTInitialized) {
		return kError;
	}

	startingP = (CHAR8 *)DTRootNode;
	return(find_entry(propName, propValue, entryH));
}

INTN find_entry(CONST CHAR8 *propName, CONST CHAR8 *propValue, DTEntry *entryH)
{
	DeviceTreeNode *nodeP = (DeviceTreeNode *) (VOID *) startingP;
	UINTN k;

	if (nodeP->nProperties == 0) return(kError);	// End of the list of nodes
	startingP = (CHAR8 *) (nodeP + 1);

	// Search current entry
	for (k = 0; k < nodeP->nProperties; ++k) {
		DeviceTreeNodeProperty *propP = (DeviceTreeNodeProperty *) (VOID *) startingP;

		startingP += sizeof (*propP) + ((propP->length + 3) & -4);

		if (AsciiStrCmp((CHAR8*)propP->name, (CHAR8*)propName) == 0) {
			if (propValue == NULL || AsciiStrCmp( (CHAR8*)(propP + 1), (CHAR8*)propValue) == 0)
			{
				*entryH = (DTEntry)nodeP;
				return(kSuccess);
			}
		}
	}

	// Search child nodes
	for (k = 0; k < nodeP->nChildren; ++k)
	{
		if (find_entry(propName, propValue, entryH) == kSuccess)
			return(kSuccess);
	}
	return(kError);
}

//if(DTLookupEntry(NULL,"/",&efiPlatform)==kSuccess)

INTN
DTLookupEntry(CONST DTEntry searchPoint, CONST CHAR8 *pathName, DTEntry *foundEntry)
{
	DTEntryNameBuf	buf;
	RealDTEntry	cur;
	CONST CHAR8*	cp;

	if (!DTInitialized) {
		return kError;
	}
	if (searchPoint == NULL)
	{
		cur = DTRootNode;
	} 
	else
	{
		cur = searchPoint;
	}
	cp = pathName;
	if (*cp == kDTPathNameSeparator) {
		cp++;
		if (*cp == 0) {
			*foundEntry = cur;
			return kSuccess;
		}
	}
	do {
		cp = GetNextComponent(cp, buf);

		/* Check for done */
		if (*buf == 0) {
			if (*cp == 0) {
				*foundEntry = cur;
				return kSuccess;
			}
			break;
		}

		cur = FindChild(cur, buf);

	} while (cur != NULL);

	return kError;
}

INTN
DTCreateEntryIterator(CONST DTEntry startEntry, DTEntryIterator *iterator)
{
	RealDTEntryIterator iter;

	if (!DTInitialized) {
		return kError;
	}

	iter = (RealDTEntryIterator) AllocatePool(sizeof(struct OpaqueDTEntryIterator));
	if (startEntry != NULL) {
		iter->outerScope = (RealDTEntry) startEntry;
		iter->currentScope = (RealDTEntry) startEntry;
	} else {
		iter->outerScope = DTRootNode;
		iter->currentScope = DTRootNode;
	}
	iter->currentEntry = NULL;
	iter->savedScope = NULL;
	iter->currentIndex = 0;

	*iterator = iter;
	return kSuccess;
}

INTN
DTDisposeEntryIterator(DTEntryIterator iterator)
{
	RealDTEntryIterator iter = iterator;
	DTSavedScopePtr scope;

	while ((scope = iter->savedScope) != NULL) {
		iter->savedScope = scope->nextScope;
		FreePool(scope);
	}
	FreePool(iterator);
	return kSuccess;
}

INTN
DTEnterEntry(DTEntryIterator iterator, DTEntry childEntry)
{
	RealDTEntryIterator iter = iterator;
	DTSavedScopePtr newScope;

	if (childEntry == NULL) {
		return kError;
	}
	newScope = (DTSavedScopePtr) AllocatePool(sizeof(struct DTSavedScope));
	newScope->nextScope = iter->savedScope;
	newScope->scope = iter->currentScope;
	newScope->entry = iter->currentEntry;
	newScope->index = iter->currentIndex;		

	iter->currentScope = childEntry;
	iter->currentEntry = NULL;
	iter->savedScope = newScope;
	iter->currentIndex = 0;

	return kSuccess;
}

INTN
DTExitEntry(DTEntryIterator iterator, DTEntry *currentPosition)
{
	RealDTEntryIterator iter = iterator;
	DTSavedScopePtr newScope;

	newScope = iter->savedScope;
	if (newScope == NULL) {
		return kError;
	}
	iter->savedScope = newScope->nextScope;
	iter->currentScope = newScope->scope;
	iter->currentEntry = newScope->entry;
	iter->currentIndex = newScope->index;
	*currentPosition = iter->currentEntry;

	FreePool(newScope);

	return kSuccess;
}

INTN
DTIterateEntries(DTEntryIterator iterator, DTEntry *nextEntry)
{
	RealDTEntryIterator iter = iterator;

	if (iter->currentIndex >= iter->currentScope->nChildren) {
		*nextEntry = NULL;
		return kIterationDone;
	} else {
		iter->currentIndex++;
		if (iter->currentIndex == 1) {
			iter->currentEntry = GetFirstChild(iter->currentScope);
		} else {
			iter->currentEntry = GetNextChild(iter->currentEntry);
		}
		*nextEntry = iter->currentEntry;
		return kSuccess;
	}
}

INTN
DTRestartEntryIteration(DTEntryIterator iterator)
{
	RealDTEntryIterator iter = iterator;
#if 0
	// This commented out code allows a second argument (outer)
	// which (if TRUE) causes restarting at the outer scope
	// rather than the current scope.
	DTSavedScopePtr scope;

	if (outer) {
		while ((scope = iter->savedScope) != NULL) {
			iter->savedScope = scope->nextScope;
			FreePool(scope);
		}
		iter->currentScope = iter->outerScope;
	}
#endif
	iter->currentEntry = NULL;
	iter->currentIndex = 0;
	return kSuccess;
}

INTN
DTGetProperty(CONST DTEntry entry, CONST char *propertyName, VOID **propertyValue, UINTN *propertySize)
{
	DeviceTreeNodeProperty *prop;
	UINTN k;

	if (entry == NULL || entry->nProperties == 0) {
		return kError;
	} else {
		prop = (DeviceTreeNodeProperty *) (entry + 1);
		for (k = 0; k < entry->nProperties; k++) {
			if (AsciiStrCmp((CHAR8*)prop->name, (CHAR8*)propertyName) == 0) {
				*propertyValue = (VOID *) (((UINT8*)prop)
						+ sizeof(DeviceTreeNodeProperty));
				*propertySize = prop->length;
				return kSuccess;
			}
			prop = next_prop(prop);
		}
	}
	return kError;
}

INTN
DTCreatePropertyIterator(CONST DTEntry entry, DTPropertyIterator *iterator)
{
	RealDTPropertyIterator iter;

	iter = (RealDTPropertyIterator) AllocatePool(sizeof(struct OpaqueDTPropertyIterator));
	iter->entry = entry;
	iter->currentProperty = NULL;
	iter->currentIndex = 0;

	*iterator = iter;
	return kSuccess;
}

// dmazar: do not have boot services when fixing dev tree in BootFixes,
// so need one version without AllocPool.
// caller should not call DTDisposePropertyIterator when using this version .
INTN
DTCreatePropertyIteratorNoAlloc(CONST DTEntry entry, DTPropertyIterator iterator)
{
	RealDTPropertyIterator iter = iterator;

	iter->entry = entry;
	iter->currentProperty = NULL;
	iter->currentIndex = 0;

	return kSuccess;
}

INTN
DTDisposePropertyIterator(DTPropertyIterator iterator)
{
	FreePool(iterator);
	return kSuccess;
}

INTN
DTIterateProperties(DTPropertyIterator iterator, CHAR8 **foundProperty)
{
	RealDTPropertyIterator iter = iterator;

	if (iter->currentIndex >= iter->entry->nProperties) {
		*foundProperty = NULL;
		return kIterationDone;
	} else {
		iter->currentIndex++;
		if (iter->currentIndex == 1) {
			iter->currentProperty = (DeviceTreeNodeProperty *) (iter->entry + 1);
		} else {
			iter->currentProperty = next_prop(iter->currentProperty);
		}
		*foundProperty = iter->currentProperty->name;
		return kSuccess;
	}
}

INTN
DTRestartPropertyIteration(DTPropertyIterator iterator)
{
	RealDTPropertyIterator iter = iterator;

	iter->currentProperty = NULL;
	iter->currentIndex = 0;
	return kSuccess;
}

