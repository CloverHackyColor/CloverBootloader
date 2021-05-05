/*
 * BasicIO.cpp
 *
 *  Created on: 28 Mar 2020
 *
 */

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include <stdio.h>
#include "../../rEFIt_UEFI/Platform/BasicIO.h"
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
  panic("not yet");
}


void PauseForKey(const XString8& msg)
{
  printf("%s", msg.c_str());
  printf("Press any key ");
  getchar();
  printf("\n");
}

// Jief, TODO : not sure of the difference between this and PauseForKey. Looks like none. Can it be removed ?

void
WaitForKeyPress(
    CHAR16          *Message
    )
{
    panic("not yet");
}


void EndlessIdleLoop(void)
{
    panic("not yet");
}


BOOLEAN CheckFatalError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
    panic("not yet");
}

BOOLEAN CheckError(IN EFI_STATUS Status, IN CONST CHAR16 *where)
{
    panic("not yet");
}




