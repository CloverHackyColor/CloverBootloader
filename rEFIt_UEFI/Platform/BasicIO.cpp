/*
 * BasicIO.cpp
 *
 *  Created on: 28 Mar 2020
 *
 */

#include <stdio.h>
#include "BasicIO.h"
//#include "EfiExternals.h"

extern "C" {
#include "Library/UefiBootServicesTableLib.h"
}


//
// Keyboard input
//

BOOLEAN ReadAllKeyStrokes(VOID)
{
    BOOLEAN       GotKeyStrokes;
    EFI_STATUS    Status;
    EFI_INPUT_KEY key;

    GotKeyStrokes = FALSE;
    for (;;) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
        if (Status == EFI_SUCCESS) {
            GotKeyStrokes = TRUE;
            continue;
        }
        break;
    }
    return GotKeyStrokes;
}

VOID PauseForKey(CONST CHAR16* msg)
{
    UINTN index;
    if (msg) {
      printf("\n %ls", msg);
    }
    printf("\n* Hit any key to continue *");

    if (ReadAllKeyStrokes()) {  // remove buffered key strokes
        gBS->Stall(5000000);     // 5 seconds delay
        ReadAllKeyStrokes();    // empty the buffer again
    }

    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    ReadAllKeyStrokes();        // empty the buffer to protect the menu

    printf("\n");
}

// Jief, TODO : not sure of the difference between this and PauseForKey. Looks like none. Can it be removed ?

VOID
WaitForKeyPress(
    CHAR16          *Message
    )
{
    EFI_STATUS      Status;
    UINTN           index;
    EFI_INPUT_KEY   key;

    printf("%ls", Message);
    do {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    } while(Status == EFI_SUCCESS);
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    do {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    } while(Status == EFI_SUCCESS);
}

//#if REFIT_DEBUG > 0
//VOID DebugPause(VOID)
//{
//    // show console and wait for key
//    SwitchToText(FALSE);
//    PauseForKey(L"");
//
//    // reset error flag
//    haveError = FALSE;
//}
//#endif

VOID EndlessIdleLoop(VOID)
{
    UINTN index;

    for (;;) {
        ReadAllKeyStrokes();
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    }
}
