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

#define SMC_PORT_BASE                   0x0300
#define SMC_PORT_LENGTH                 0x0020
#define APPLE_SMC_IO_PROTOCOL_REVISION  0x33

#define SMC_MAKE_IDENTIFIER(A, B, C, D)  \
(((A) << 24) | ((B) << 16) | ((C) << 8) | (D))
#define SMC_MAKE_KEY(A, B, C, D)      SMC_MAKE_IDENTIFIER ((A), (B), (C), (D))
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
enum {
  SmcKeyTypeCh8    = SMC_MAKE_KEY_TYPE ('c', 'h', '8', '*'),
  SmcKeyTypeChar   = SMC_MAKE_KEY_TYPE ('c', 'h', 'a', 'r'),
  SmcKeyTypeFlag   = SMC_MAKE_KEY_TYPE ('f', 'l', 'a', 'g'),
  SmcKeyTypeFp1f   = SMC_MAKE_KEY_TYPE ('f', 'p', '1', 'f'),
  SmcKeyTypeFp4c   = SMC_MAKE_KEY_TYPE ('f', 'p', '4', 'c'),
  SmcKeyTypeFp5b   = SMC_MAKE_KEY_TYPE ('f', 'p', '5', 'b'),
  SmcKeyTypeFp6a5b = SMC_MAKE_KEY_TYPE ('f', 'p', '6', 'a'),
  SmcKeyTypeFp79   = SMC_MAKE_KEY_TYPE ('f', 'p', '7', '9'),
  SmcKeyTypeFpa6   = SMC_MAKE_KEY_TYPE ('f', 'p', 'a', '6'),
  SmcKeyTypeFpc4   = SMC_MAKE_KEY_TYPE ('f', 'p', 'c', '4'),
  SmcKeyTypeFpe2   = SMC_MAKE_KEY_TYPE ('f', 'p', 'e', '2'),
  SmcKeyTypeSint8  = SMC_MAKE_KEY_TYPE ('s', 'i', '8', ' '),
  SmcKeyTypeSint16 = SMC_MAKE_KEY_TYPE ('s', 'i', '1', '6'),
  SmcKeyTypeSp1e   = SMC_MAKE_KEY_TYPE ('s', 'p', '1', 'e'),
  SmcKeyTypeSp3c   = SMC_MAKE_KEY_TYPE ('s', 'p', '3', 'c'),
  SmcKeyTypeSp4b   = SMC_MAKE_KEY_TYPE ('s', 'p', '4', 'b'),
  SmcKeyTypeSp5a   = SMC_MAKE_KEY_TYPE ('s', 'p', '5', 'a'),
  SmcKeyTypeSp69   = SMC_MAKE_KEY_TYPE ('s', 'p', '6', '9'),
  SmcKeyTypeSp78   = SMC_MAKE_KEY_TYPE ('s', 'p', '7', '8'),
  SmcKeyTypeSp87   = SMC_MAKE_KEY_TYPE ('s', 'p', '8', '7'),
  SmcKeyTypeSp96   = SMC_MAKE_KEY_TYPE ('s', 'p', '9', '6'),
  SmcKeyTypeSpb4   = SMC_MAKE_KEY_TYPE ('s', 'p', 'b', '4'),
  SmcKeyTypeSpf0   = SMC_MAKE_KEY_TYPE ('s', 'p', 'f', '0'),
  SmcKeyTypeUint8  = SMC_MAKE_KEY_TYPE ('u', 'i', '8', ' '),
  SmcKeyTypeUint16 = SMC_MAKE_KEY_TYPE ('u', 'i', '1', '6'),
  SmcKeyTypeUint32 = SMC_MAKE_KEY_TYPE ('u', 'i', '3', '2'),
  SmcKeyTypeLim    = SMC_MAKE_KEY_TYPE ('{', 'l', 'i', 'm'),
  SmcKeyTypePwm    = SMC_MAKE_KEY_TYPE ('{', 'p', 'w', 'm'),
  SmcKeyTypeAla    = SMC_MAKE_KEY_TYPE ('{', 'a', 'l', 'a'),
  SmcKeyTypeAlc    = SMC_MAKE_KEY_TYPE ('{', 'a', 'l', 'c'),
  SmcKeyTypeAli    = SMC_MAKE_KEY_TYPE ('{', 'a', 'l', 'i'),
  SmcKeyTypeAlr    = SMC_MAKE_KEY_TYPE ('{', 'a', 'l', 'r'),
  SmcKeyTypeAlt    = SMC_MAKE_KEY_TYPE ('{', 'a', 'l', 't')
};

typedef UINT32 SMC_KEY;
typedef UINT32 SMC_INDEX;
typedef UINT8  SMC_DATA;
typedef UINT8  SMC_DATA_SIZE;
typedef UINT8  SMC_RESULT;
typedef UINT32 SMC_ADDRESS;

typedef UINT8 SMC_KEY_ATTRIBUTES;
// Key Attributes

#define BIT(n)		            	(1u << (n))

#define	SMC_KEY_ATTRIBUTE_PRIVATE   BIT (0)
#define	SMC_KEY_ATTRIBUTE_UKN_0x02  BIT (1)
#define	SMC_KEY_ATTRIBUTE_UKN_0x04  BIT (2)
#define	SMC_KEY_ATTRIBUTE_CONST     BIT (3)
#define	SMC_KEY_ATTRIBUTE_FUNCTION  BIT (4)
#define	SMC_KEY_ATTRIBUTE_UKN_0x20  BIT (5)
#define	SMC_KEY_ATTRIBUTE_WRITE     BIT (6)
#define	SMC_KEY_ATTRIBUTE_READ      BIT (7)


typedef struct _APPLE_SMC_IO_PROTOCOL    APPLE_SMC_IO_PROTOCOL;
typedef struct _APPLE_SMC_STATE_PROTOCOL APPLE_SMC_STATE_PROTOCOL;

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
IN  SMC_DATA_SIZE          Size,
OUT SMC_DATA               *Value
);

// SMC_IO_SMC_GET_KEY_COUNT
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_GET_KEY_COUNT)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
OUT SMC_DATA               *Count
);

// SMC_IO_SMC_MAKE_KEY
typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_MAKE_KEY)(
IN  APPLE_SMC_IO_PROTOCOL  *This,
IN  CHAR8    *Name,
OUT SMC_KEY  *Key
);

typedef
EFI_STATUS
(EFIAPI *SMC_IO_SMC_ADD_KEY)(
IN   APPLE_SMC_IO_PROTOCOL  *This,
IN   SMC_KEY                Key,
IN   SMC_DATA_SIZE          Size,
IN   SMC_KEY_TYPE           Type,
IN   SMC_KEY_ATTRIBUTES     Attributes
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
IN  APPLE_SMC_IO_PROTOCOL  *This
);

// SMC_IO_SMC_UNKNOWN_1
typedef
EFI_STATUS
(EFIAPI *SMC_STATE_UNKNOWN_1)(
IN  APPLE_SMC_STATE_PROTOCOL  *This
);

// SMC_IO_SMC_UNKNOWN_2
typedef
EFI_STATUS
(EFIAPI *SMC_STATE_UNKNOWN_2)(
IN APPLE_SMC_STATE_PROTOCOL  *This,
IN UINTN                  Ukn1,
IN UINTN                  Ukn2
);

// SMC_IO_SMC_UNKNOWN_3
typedef
EFI_STATUS
(EFIAPI *SMC_STATE_UNKNOWN_3)(
IN APPLE_SMC_STATE_PROTOCOL  *This,
IN UINTN                  Ukn1,
IN UINTN                  Ukn2
);

// SMC_IO_SMC_UNKNOWN_4
typedef
EFI_STATUS
(EFIAPI *SMC_STATE_GET_STATE)(
IN APPLE_SMC_STATE_PROTOCOL  *This,
OUT UINTN                  *Ukn1
);

// SMC_IO_SMC_UNKNOWN_5
typedef
EFI_STATUS
(EFIAPI *SMC_STATE_UNKNOWN_5)(
IN APPLE_SMC_STATE_PROTOCOL  *This,
IN UINTN                  Ukn1
);

// APPLE_SMC_IO_PROTOCOL
struct _APPLE_SMC_IO_PROTOCOL {
  UINT64                        Signature; //Revision;  
  SMC_IO_SMC_READ_VALUE         SmcReadValue;        ///< 0x08
  SMC_IO_SMC_WRITE_VALUE        SmcWriteValue;       ///< 0x10
  SMC_IO_SMC_GET_KEY_COUNT      SmcGetKeyCount;      ///<
  //SMC_IO_SMC_MAKE_KEY           SmcMakeKey;          ///<
  SMC_IO_SMC_ADD_KEY            SmcAddKey;          ///<
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
/*  SMC_IO_SMC_UNKNOWN_1          SmcUnknown1;         ///<
  SMC_IO_SMC_UNKNOWN_2          SmcUnknown2;         ///<
  SMC_IO_SMC_UNKNOWN_3          SmcUnknown3;         ///<
  SMC_IO_SMC_UNKNOWN_4          SmcUnknown4;         ///<
  SMC_IO_SMC_UNKNOWN_5          SmcUnknown5;         ///< */
};



struct _APPLE_SMC_STATE_PROTOCOL {
  UINT64                       Revision;            ///<
  SMC_STATE_UNKNOWN_1          SmcUnknown1;         ///< 0x08
  SMC_STATE_UNKNOWN_2          SmcUnknown2;         ///< 0x10
  SMC_STATE_UNKNOWN_3          SmcUnknown3;         ///< 0x18
  SMC_STATE_GET_STATE          SmcUnknown4;         ///< 0x20 - really called
  SMC_STATE_UNKNOWN_5          SmcUnknown5;         ///<
};



extern EFI_GUID gAppleSMCProtocolGuid;
extern EFI_GUID gAppleSMCStateProtocolGuid;

#endif
