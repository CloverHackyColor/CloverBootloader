#ifndef _DEVPATH_H
#define _DEVPATH_H

#define MAX_PATH_LEN 4096
#define MAX_DEVICE_PATH_LEN 1000

#define MIN_ALIGNMENT_SIZE  4
#define ALIGN_SIZE(a) ((a % MIN_ALIGNMENT_SIZE) ? MIN_ALIGNMENT_SIZE - (a % MIN_ALIGNMENT_SIZE) : 0)

#ifndef IS_COMMA
#define IS_COMMA(a)          ((a) == ',')
#define IS_HYPHEN(a)         ((a) == '-')
#define IS_DOT(a)            ((a) == '.')
#define IS_LEFT_PARENTH(a)   ((a) == '(')
#define IS_RIGHT_PARENTH(a)  ((a) == ')')
#define IS_SLASH(a)          ((a) == '/')
#define IS_NULL(a)           ((a) == '\0')

typedef signed char      INT8;
typedef unsigned char    UINT8;
typedef UINT8           BOOLEAN;
typedef char            CHAR8;
typedef short            INT16;
typedef unsigned short  UINT16;
typedef int              INT32;
typedef unsigned int    UINT32;
typedef long long        INT64;
typedef unsigned long long  UINT64;
#endif

typedef struct _EFI_DEVICE_PATH_P_TAG
{
	UINT8 Type;
	UINT8 SubType;
	UINT8 Length[2];
	
} EFI_DEVICE_PATH_P;


typedef struct 
{
	UINT8 Type;
	UINT8 SubType;
	void (*Function) (CHAR8 *, void *, BOOLEAN, BOOLEAN);
} DEVICE_PATH_TO_TEXT_TABLE;

typedef struct 
{
	CHAR8 *DevicePathNodeText;
	EFI_DEVICE_PATH_P * (*Function) (CHAR8 *);
} DEVICE_PATH_FROM_TEXT_TABLE;

#define EFI_DP_TYPE_MASK                    0x7F
#define EFI_DP_TYPE_UNPACKED                0x80
#define END_DEVICE_PATH_TYPE                0x7f
#ifndef END_ENTIRE_DEVICE_PATH_SUBTYPE
#define END_ENTIRE_DEVICE_PATH_SUBTYPE      0xff
#endif
#define END_INSTANCE_DEVICE_PATH_SUBTYPE    0x01
#ifndef END_DEVICE_PATH_LENGTH
#define END_DEVICE_PATH_LENGTH              (sizeof(EFI_DEVICE_PATH_P))
#endif
#define DP_IS_END_TYPE(a)
#define DP_IS_END_SUBTYPE(a)        ( ((a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE )

#define DevicePathType(a)           ( ((a)->Type) & EFI_DP_TYPE_MASK )
#define DevicePathSubType(a)        ( (a)->SubType )
#define DevicePathNodeLength(a)     ( ((a)->Length[0]) | ((a)->Length[1] << 8) )
#define NextDevicePathNode(a)       ( (EFI_DEVICE_PATH_P *) ( ((unsigned char *) (a)) + DevicePathNodeLength(a)))
#define IsDevicePathEndType(a)      ( DevicePathType(a) == END_DEVICE_PATH_TYPE )
#define IsDevicePathEndSubType(a)   ( (a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE )
#define IsDevicePathEnd(a)          ( IsDevicePathEndType(a) && IsDevicePathEndSubType(a) )
#define IsDevicePathUnpacked(a)     ( (a)->Type & EFI_DP_TYPE_UNPACKED )


#define SetDevicePathNodeLength(a,l) {				\
            (a)->Length[0] = (UINT8) (l);			\
            (a)->Length[1] = (UINT8) ((l) >> 8);	\
            }

#define SetDevicePathEndNode(a)  {							\
            (a)->Type = END_DEVICE_PATH_TYPE;				\
            (a)->SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;  \
            (a)->Length[0] = sizeof(EFI_DEVICE_PATH_P);		\
            (a)->Length[1] = 0;								\
            }

// ****** PCI *******
#define HARDWARE_DEVICE_PATH 0x01

#define HW_PCI_DP 0x01

typedef struct _PCI_DEVICE_PATH
{
	EFI_DEVICE_PATH_P Header;
	unsigned char Function;
	unsigned char  Device;
} PCI_DEVICE_PATH_P;

// ****** ACPI ******* 
#define ACPI_DEVICE_PATH 0x02

#define ACPI_DP 0x01

typedef struct _ACPI_HID_DEVICE_PATH 
{
	EFI_DEVICE_PATH_P	Header;
	UINT32	HID;
	UINT32	UID;
} ACPI_HID_DEVICE_PATH_P;

// 
//  EISA ID Macro
//  EISA ID Definition 32-bits
//   bits[15:0] - three character compressed ASCII EISA ID.
//   bits[31:16] - binary number
//    Compressed ASCII is 5 bits per character 0b00001 = 'A' 0b11010 = 'Z'
//
#define PNP_EISA_ID_CONST         0x41d0    
//#define EISA_ID(_Name, _Num)      ((UINT32) ((_Name) | (_Num) << 16))
#define EISA_PNP_ID(_PNPId)       (EISA_ID(PNP_EISA_ID_CONST, (_PNPId)))
//#define EFI_PNP_ID(_PNPId)        (EISA_ID(PNP_EISA_ID_CONST, (_PNPId)))

#define PNP_EISA_ID_MASK          0xffff
#define EISA_ID_TO_NUM(_Id)       ((_Id) >> 16)

void EisaIdFromText (CHAR8 *Text, UINT32 *EisaId);

// Convert a device node to its text representation.
//CHAR8 *ConvertDeviceNodeToText (const EFI_DEVICE_PATH_P *DeviceNode, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts);

// Convert a device path to its text representation.
CHAR8 *ConvertDevicePathToAscii (const EFI_DEVICE_PATH_P  *DeviceNode, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts);

// Convert text to the binary representation of a device node.
//EFI_DEVICE_PATH_P*ConvertTextToDeviceNode (const CHAR8 *TextDeviceNode);

// Convert text to the binary representation of a device path.
//EFI_DEVICE_PATH_P*ConvertTextToDevicePath (const CHAR8 *TextDevicePath);

// Returns the size of the device path, in bytes.
UINT32 DevicePathSize (const EFI_DEVICE_PATH_P *DevicePath);

// Function is used to append a device path node to the end of another device path.
//EFI_DEVICE_PATH_P*AppendDevicePathNode (EFI_DEVICE_PATH_P *Src1, EFI_DEVICE_PATH_P *Node);

// Function is used to insert a device path node to the start of another device path.
//EFI_DEVICE_PATH_P*InsertDevicePathNode (EFI_DEVICE_PATH_P *Src1, EFI_DEVICE_PATH_P *Node);

//io_iterator_t RecursiveFindDevicePath(io_iterator_t iterator, const io_string_t search, const io_name_t plane, EFI_DEVICE_PATH_P**DevicePath, BOOLEAN *match);

#endif
