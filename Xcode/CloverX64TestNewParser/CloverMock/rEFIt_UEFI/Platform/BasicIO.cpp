/*
 * BasicIO.cpp
 *
 *  Created on: 28 Mar 2020
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include <stdio.h>
#include "../../../../../rEFIt_UEFI/Platform/BasicIO.h"
////#include "EfiExternals.h"
//
//extern "C" {
//#include "Library/UefiBootServicesTableLib.h"
//}


//
// Keyboard input
//

BOOLEAN ReadAllKeyStrokes(void)
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


void PauseForKey(const wchar_t* msg)
{
  printf("%ls", msg);
  getchar();
}

// Jief, TODO : not sure of the difference between this and PauseForKey. Looks like none. Can it be removed ?

void
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
//void DebugPause(void)
//{
//    // show console and wait for key
//    SwitchToText(FALSE);
//    PauseForKey(L"");
//
//    // reset error flag
//    haveError = FALSE;
//}
//#endif

void EndlessIdleLoop(void)
{
    UINTN index;

    for (;;) {
        ReadAllKeyStrokes();
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    }
}




