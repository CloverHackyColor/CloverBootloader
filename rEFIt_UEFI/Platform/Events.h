/*
 * Console.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_EVENTS_H_
#define PLATFORM_EVENTS_H_

extern EFI_EVENT                       mVirtualAddressChangeEvent;
extern EFI_EVENT                       OnReadyToBootEvent;
extern EFI_EVENT                       ExitBootServiceEvent;
extern EFI_EVENT                       mSimpleFileSystemChangeEvent;


EFI_STATUS
EventsInitialize (
  IN LOADER_ENTRY *Entry
  );


EFI_STATUS
GuiEventsInitialize (VOID);


// timeout will be in ms here, as small as 1ms and up
EFI_STATUS
WaitFor2EventWithTsc (
                      IN EFI_EVENT        Event1,
                      IN EFI_EVENT        Event2,
                      IN UINT64           Timeout OPTIONAL
                    );


EFI_STATUS
EjectVolume (
  IN REFIT_VOLUME *Volume
  );

#endif /* PLATFORM_EVENTS_H_ */
