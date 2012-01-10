/*++

Copyright (c) 1998  Intel Corporation

Module Name:

Abstract:



Revision History

--*/



//
// The variable store protocol interface is specific to the reference
// implementation.  The initialization code adds variable store devices
// to the system, and the FW connects to the devices to provide the
// variable store interfaces through these devices.
//

//
// Variable Store Device protocol
//

#define VARIABLE_STORE_PROTOCOL    \
    { 0xf088cd91, 0xa046, 0x11d2, {0x8e, 0x42, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

INTERFACE_DECL(_EFI_VARIABLE_STORE);

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_CLEAR) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN OUT VOID                     *Scratch
    );


typedef
EFI_STATUS
(EFIAPI *EFI_STORE_READ) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN UINTN                        Offset,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_UPDATE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN UINTN                        Offset,
    IN UINTN                        BufferSize,
    IN VOID                         *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_SIZE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        NoBanks
    );

typedef
EFI_STATUS
(EFIAPI *EFI_TRANSACTION_UPDATE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN VOID                         *NewContents
    );

typedef struct _EFI_VARIABLE_STORE {

    //
    // Number of banks and bank size
    //

    UINT32                      Attributes;
    UINT32                      BankSize;
    UINT32                      NoBanks;

    //
    // Functions to access the storage banks
    //

    EFI_STORE_CLEAR             ClearStore;
    EFI_STORE_READ              ReadStore;
    EFI_STORE_UPDATE            UpdateStore;
    EFI_STORE_SIZE              SizeStore OPTIONAL;
    EFI_TRANSACTION_UPDATE      TransactionUpdate OPTIONAL;

} EFI_VARIABLE_STORE;


//
//
// ClearStore()     - A function to clear the requested storage bank.  A cleared
//      bank contains all "on" bits.
//
// ReadStore()      - Read data from the requested store.
//
// UpdateStore()    - Updates data on the requested store. The FW will only
//      ever issue updates to clear bits in the store. Updates must be
//      performed in LSb to MSb order of the update buffer.
//
// SizeStore()      - An optional function for non-runtime stores that can be
//      dynamically sized.  The FW will only ever increase or decrease the store
//      by 1 banksize at a time, and it is always adding or removing a bank from 
//      the end of the store.
//
// By default the FW will update variables and storage banks in an
// "atomic" manner by keeping 1 old copy of the data during an update,
// and recovering appropiately if the power is lost during the middle
// of an operation.  To do this the FW needs to have multiple banks
// of storage dedicated to its use. If that's not possible, the driver 
// can implement an atomic bank update function and the FW will allow 
// 1 bank in this case.  (It will allow any number of banks,
// but it won't require an "extra" bank to provide its bank transaction 
// function).
//
// TransactionUpdate()  - An optional function that can clear & update an 
//      entire bank in an "atomic" fashion.  If the operation fails in the 
//      middle the driver is responsible for having either the previous copy 
//      of the bank's data or the new copy.  A copy that's partially written
//      is not valid as internal data settings may get lost.  Supply this
//      function only when needed.
//

