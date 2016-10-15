//********************************************************************
//	created:	29:9:2012   13:49
//	filename: 	AppleSMC.h
//	author:		tiamo
//	purpose:	apple smc
//********************************************************************

#ifndef _APPLE_SMC_H_
#define _APPLE_SMC_H_

#define APPLE_SMC_PROTOCOL_GUID	{0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}}

//EFI_FORWARD_DECLARATION(APPLE_SMC_PROTOCOL);
//typedef struct _##x x
typedef struct _APPLE_SMC_PROTOCOL APPLE_SMC_PROTOCOL;

typedef EFI_STATUS (EFIAPI* APPLE_SMC_READ_DATA)(IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, OUT VOID* DataBuffer);

typedef EFI_STATUS (EFIAPI* APPLE_SMC_WRITE_DATA)(IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer);
typedef EFI_STATUS (EFIAPI* APPLE_SMC_DUMP_DATA)(IN APPLE_SMC_PROTOCOL* This);

struct _APPLE_SMC_PROTOCOL
{
	UINT64																Signature;
	APPLE_SMC_READ_DATA										ReadData;
  APPLE_SMC_WRITE_DATA									WriteData;
  APPLE_SMC_DUMP_DATA										DumpData;
};

#define SMC_MAKE_IDENTIFIER(A, B, C, D)  \
(((A) << 24) | ((B) << 16) | ((C) << 8) | (D))
#define SMC_MAKE_KEY_TYPE(A, B, C, D) SMC_MAKE_IDENTIFIER ((A), (B), (C), (D))
#define SMC_KEY_NUM      SMC_MAKE_KEY ('$', 'N', 'u', 'm')
#define SMC_KEY_ADR      SMC_MAKE_KEY ('$', 'A', 'd', 'r')
#define SMC_KEY_NO_KEYS  SMC_MAKE_KEY ('#', 'K', 'e', 'y')
#define SMC_KEY_LDKN     SMC_MAKE_KEY ('L', 'D', 'K', 'N')

// Modes

#define SMC_MODE_APPCODE  'A'
#define SMC_MODE_UPDATE   'U'
#define SMC_MODE_BASE     'B'

// SMC_MODE
typedef CHAR8 *SMC_MODE;


// SMC_KEY_TYPE
typedef UINT32 SMC_KEY_TYPE;
typedef UINT32 SMC_KEY;
typedef UINT32 SMC_INDEX;
typedef UINT8 SMC_DATA;
typedef UINT8 SMC_DATA_SIZE;
typedef UINT8 SMC_RESULT;

typedef UINT8 SMC_KEY_ATTRIBUTES;
// Key Attributes

#define	SMC_KEY_ATTRIBUTE_PRIVATE   BIT (0)
#define	SMC_KEY_ATTRIBUTE_UKN_0x02  BIT (1)
#define	SMC_KEY_ATTRIBUTE_UKN_0x04  BIT (2)
#define	SMC_KEY_ATTRIBUTE_CONST     BIT (3)
#define	SMC_KEY_ATTRIBUTE_FUNCTION  BIT (4)
#define	SMC_KEY_ATTRIBUTE_UKN_0x20  BIT (5)
#define	SMC_KEY_ATTRIBUTE_WRITE     BIT (6)
#define	SMC_KEY_ATTRIBUTE_READ      BIT (7)


typedef struct _APPLE_SMC_IO_PROTOCOL APPLE_SMC_IO_PROTOCOL;

// SMC_IO_SMC_READ_VALUE
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_READ_VALUE)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
IN  SMC_KEY                Key,
IN  SMC_DATA_SIZE          Size,
OUT SMC_DATA               *Value
);

// SMC_IO_SMC_WRITE_VALUE
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_WRITE_VALUE)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
IN  SMC_KEY                Key,
IN  UINT32                 Size,
OUT SMC_DATA               *Value
);

// SMC_IO_SMC_GET_KEY_COUNT
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_GET_KEY_COUNT)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
OUT UINT32                 *Count
);

// SMC_IO_SMC_MAKE_KEY
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_MAKE_KEY)(
IN  CHAR8    *Name,
OUT SMC_KEY  *Key
);

// SMC_IO_SMC_GET_KEY_FROM_INDEX
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_GET_KEY_FROM_INDEX)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
IN  SMC_INDEX              Index,
OUT SMC_KEY                *Key
);

// SMC_IO_SMC_GET_KEY_INFO
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_GET_KEY_INFO)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
IN  SMC_KEY                Key,
OUT SMC_DATA_SIZE          *Size,
OUT SMC_KEY_TYPE           *Type,
OUT SMC_KEY_ATTRIBUTES     *Attributes
);

// SMC_IO_SMC_RESET
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_RESET)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINT32                 Mode
);

// SMC_IO_SMC_FLASH_TYPE
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_FLASH_TYPE)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINT32                 Type
);

// SMC_IO_SMC_FLASH_WRITE
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_FLASH_WRITE)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINT32                 Unknown,
IN UINT32                 Size,
IN SMC_DATA               *Data
);

// SMC_IO_SMC_FLASH_AUTH
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_FLASH_AUTH)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINT32                 Size,
IN SMC_DATA               *Data
);

// SMC_IO_SMC_UNSUPPORTED
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_UNSUPPORTED)(
VOID
);

// SMC_IO_SMC_UNKNOWN_1
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_UNKNOWN_1)(
VOID
);

// SMC_IO_SMC_UNKNOWN_2
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_UNKNOWN_2)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINTN                  Ukn1,
IN UINTN                  Ukn2
);

// SMC_IO_SMC_UNKNOWN_3
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_UNKNOWN_3)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINTN                  Ukn1,
IN UINTN                  Ukn2
);

// SMC_IO_SMC_UNKNOWN_4
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_UNKNOWN_4)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINTN                  Ukn1
);

// SMC_IO_SMC_UNKNOWN_5
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_UNKNOWN_5)(
IN APPLE_SMC_IO_PROTOCOL  *This,
IN UINTN                  Ukn1
);

// APPLE_SMC_IO_PROTOCOL
struct _APPLE_SMC_IO_PROTOCOL {
  UINT64                        Revision;            ///<
  SMC_IO_SMC_READ_VALUE         SmcReadValue;        ///<
  SMC_IO_SMC_WRITE_VALUE        SmcWriteValue;       ///<
  SMC_IO_SMC_GET_KEY_COUNT      SmcGetKeyCount;      ///<
  SMC_IO_SMC_MAKE_KEY           SmcMakeKey;          ///<
  SMC_IO_SMC_GET_KEY_FROM_INDEX SmcGetKeyFromIndex;  ///<
  SMC_IO_SMC_GET_KEY_INFO       SmcGetKeyInfo;       ///<
  SMC_IO_SMC_RESET              SmcReset;            ///<
  SMC_IO_SMC_FLASH_TYPE         SmcFlashType;        ///<
  SMC_IO_SMC_UNSUPPORTED        SmcUnsupported;      ///<
  SMC_IO_SMC_FLASH_WRITE        SmcFlashWrite;       ///<
  SMC_IO_SMC_FLASH_AUTH         SmcFlashAuth;        ///<
  SMC_INDEX                     Index;               ///<
  SMC_ADDRESS                   Address;             ///<
  BOOLEAN                       Mmio;                ///<
  SMC_IO_SMC_UNKNOWN_1          SmcUnknown1;         ///<
  SMC_IO_SMC_UNKNOWN_2          SmcUnknown2;         ///<
  SMC_IO_SMC_UNKNOWN_3          SmcUnknown3;         ///<
  SMC_IO_SMC_UNKNOWN_4          SmcUnknown4;         ///<
  SMC_IO_SMC_UNKNOWN_5          SmcUnknown5;         ///<
};


extern EFI_GUID gAppleSMCProtocolGuid;

#endif
