/*
 * BasicIO.cpp
 *
 *  Created on: 28 Mar 2020
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include <stdio.h>
#include "../Platform/BasicIO.h"
//#include "EfiExternals.h"

extern "C" {
#include "Library/UefiBootServicesTableLib.h"
}


//
// Keyboard input
//

XBool ReadAllKeyStrokes(void)
{
    XBool         GotKeyStrokes;
    EFI_STATUS    Status;
    EFI_INPUT_KEY key;

    GotKeyStrokes = false;
    for (;;) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
        if (Status == EFI_SUCCESS) {
            GotKeyStrokes = true;
            continue;
        }
        break;
    }
    return GotKeyStrokes;
}

void PauseForKey(const XString8& msg)
{
    UINTN index;
    if ( msg.notEmpty() ) {
      printf("%s", msg.c_str());
    }
    if ( msg.lastChar() != '\n' ) printf(" ");
    printf("Hit any key to continue...\n");

    if (ReadAllKeyStrokes()) {  // remove buffered key strokes
        gBS->Stall(5000000);     // 5 seconds delay
        ReadAllKeyStrokes();    // empty the buffer again
    }

    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &index);
    ReadAllKeyStrokes();        // empty the buffer to protect the menu

    printf("\n");
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
//    SwitchToText(false);
//    PauseForKey(L"");
//
//    // reset error flag
//    haveError = false;
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


XBool CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];

    if (!EFI_ERROR(Status))
        return false;

    MsgLog("Fatal Error: %s %ls\n", efiStrError(Status), where);

//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
    printf("Fatal Error: %s %ls\n", efiStrError(Status), where);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    haveError = true;

    //gBS->Exit(ImageHandle, ExitStatus, ExitDataSize, ExitData);

    return true;
}

XBool CheckError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
//    CHAR16 ErrorName[64];

    if (!EFI_ERROR(Status))
        return false;

    MsgLog("Fatal Error: %s %ls\n", efiStrError(Status), where);

//    StatusToString(ErrorName, Status);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
    printf("Error: %s %ls\n", efiStrError(Status), where);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    haveError = true;

    return true;
}


