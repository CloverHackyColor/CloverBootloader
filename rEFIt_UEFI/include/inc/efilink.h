#ifndef _EFI_LINK_H
#define _EFI_LINK_H

/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    link.h (renamed efilink.h to avoid conflicts)

Abstract:

    EFI link list macro's



Revision History

--*/

#ifndef EFI_NT_EMUL

//
// List entry - doubly linked list
//

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY  *Flink;
    struct _LIST_ENTRY  *Blink;
} LIST_ENTRY;

#endif 


//
//  VOID
//  InitializeListHead(
//      LIST_ENTRY *ListHead
//      );
//

#define InitializeListHead(ListHead) \
    (ListHead)->Flink = ListHead;    \
    (ListHead)->Blink = ListHead;

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define _RemoveEntryList(Entry) {       \
        LIST_ENTRY *_Blink, *_Flink;    \
        _Flink = (Entry)->Flink;        \
        _Blink = (Entry)->Blink;        \
        _Blink->Flink = _Flink;         \
        _Flink->Blink = _Blink;         \
        }

#if EFI_DEBUG
    #define RemoveEntryList(Entry)                      \
        _RemoveEntryList(Entry);                        \
        (Entry)->Flink = (LIST_ENTRY *) BAD_POINTER;    \
        (Entry)->Blink = (LIST_ENTRY *) BAD_POINTER; 
#else
    #define RemoveEntryList(Entry)      \
        _RemoveEntryList(Entry);
#endif

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    LIST_ENTRY *_ListHead, *_Blink;     \
    _ListHead = (ListHead);             \
    _Blink = _ListHead->Blink;          \
    (Entry)->Flink = _ListHead;         \
    (Entry)->Blink = _Blink;            \
    _Blink->Flink = (Entry);            \
    _ListHead->Blink = (Entry);         \
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    LIST_ENTRY *_ListHead, *_Flink;     \
    _ListHead = (ListHead);             \
    _Flink = _ListHead->Flink;          \
    (Entry)->Flink = _Flink;            \
    (Entry)->Blink = _ListHead;         \
    _Flink->Blink = (Entry);            \
    _ListHead->Flink = (Entry);         \
    }

//  VOID
//  SwapListEntries(
//      PLIST_ENTRY Entry1,
//      PLIST_ENTRY Entry2
//      );
//
// Put Entry2 before Entry1
//
#define SwapListEntries(Entry1,Entry2) {\
    LIST_ENTRY *Entry1Flink, *Entry1Blink;     \
    LIST_ENTRY *Entry2Flink, *Entry2Blink;     \
    Entry2Flink = (Entry2)->Flink;             \
    Entry2Blink = (Entry2)->Blink;             \
    Entry1Flink = (Entry1)->Flink;             \
    Entry1Blink = (Entry1)->Blink;             \
    Entry2Blink->Flink = Entry2Flink;       \
    Entry2Flink->Blink = Entry2Blink;        \
    (Entry2)->Flink = Entry1;               \
    (Entry2)->Blink = Entry1Blink;          \
    Entry1Blink->Flink = (Entry2);            \
    (Entry1)->Blink = (Entry2);             \
    }

//
//  EFI_FIELD_OFFSET - returns the byte offset to a field within a structure
//

#define EFI_FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))

//
//  CONTAINING_RECORD - returns a pointer to the structure
//      from one of it's elements.
//

#define _CR(Record, TYPE, Field)  \
    ((TYPE *) ( (CHAR8 *)(Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#if EFI_DEBUG
    #define CR(Record, TYPE, Field, Sig)     \
        _CR(Record, TYPE, Field)->Signature != Sig ?        \
            (TYPE *) ASSERT_STRUCT(_CR(Record, TYPE, Field), Record) : \
            _CR(Record, TYPE, Field)
#else
    #define CR(Record, TYPE, Field, Signature)   \
        _CR(Record, TYPE, Field)                           
#endif


//
// A lock structure
//

typedef struct _FLOCK {
    EFI_TPL     Tpl;
    EFI_TPL     OwnerTpl;
    UINTN       Lock;
} FLOCK;

#endif

