#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "efidevp.h"

/* From utils.c */
extern void CatPrintf(char *target, const char *format, ...);
extern void *MallocCopy(unsigned int size, void *buf);

/*
 * Get parameter in a pair of parentheses follow the given node name.
 * For example, given the "Pci(0,1)" and NodeName "Pci", it returns "0,1".
 */
char *GetParamByNodeName (CHAR8 *Str, CHAR8 *NodeName)
{
	CHAR8  *ParamStr;
	CHAR8  *StrPointer;
	UINT32 NodeNameLength;
	UINT32 ParameterLength;

	// Check whether the node name matchs
	NodeNameLength = (UINT32)strlen (NodeName);
	if (strncasecmp(Str, NodeName, NodeNameLength) != 0) 
	{
		return NULL;
	}

	ParamStr = Str + NodeNameLength;
	if (!IS_LEFT_PARENTH (*ParamStr)) 
	{
		return NULL;
	}

	// Skip the found '(' and find first occurrence of ')'
	ParamStr++;
	ParameterLength = 0;
	StrPointer = ParamStr;
	
	while (!IS_NULL (*StrPointer)) 
	{
		if (IS_RIGHT_PARENTH (*StrPointer)) 
		{
			break;
		}
		StrPointer++;
		ParameterLength++;
	}
	
	if (IS_NULL (*StrPointer)) 
	{
		// ')' not found
		return NULL;
	}

	ParamStr = MallocCopy ((ParameterLength + 1), ParamStr);
	if (ParamStr == NULL) 
	{
		return NULL;
	}
	// Terminate the parameter string
	ParamStr[ParameterLength] = '\0';

	return ParamStr;
}

/* Get current sub-string from a string list, before return
 * the list header is moved to next sub-string. The sub-string is separated
 * by the specified character. For example, the separator is ',', the string
 * list is "2,0,3", it returns "2", the remain list move to "0,3"
 */
CHAR8 *SplitStr (CHAR8 **List, CHAR8 Separator)
{
  char  *Str;
  char  *ReturnStr;

  Str = *List;
  ReturnStr = Str;

  if (IS_NULL (*Str)) 
  {
    return ReturnStr;
  }

  // Find first occurrence of the separator
  while (!IS_NULL (*Str)) 
  {
    if (*Str == Separator) 
	{
      break;
    }
    Str++;
  }

  if (*Str == Separator) 
  {
    // Find a sub-string, terminate it
    *Str = '\0';
    Str++;
  }

  // Move to next sub-string
  *List = Str;

  return ReturnStr;
}

CHAR8 *GetNextParamStr (CHAR8 **List)
{
  // The separator is comma
  return SplitStr (List, ',');
}

// Get one device node from entire device path text.
CHAR8 *GetNextDeviceNodeStr (CHAR8 **DevicePath, BOOLEAN *IsInstanceEnd)
{
  CHAR8  *Str;
  CHAR8  *ReturnStr;
  UINT32  ParenthesesStack;

  Str = *DevicePath;
  if (IS_NULL (*Str)) 
  {
    return NULL;
  }

  // Skip the leading '/', '(', ')' and ','
  while (!IS_NULL (*Str)) 
  {
    if (!IS_SLASH (*Str) && !IS_COMMA (*Str) && !IS_LEFT_PARENTH (*Str) && !IS_RIGHT_PARENTH (*Str)) 
	{
      break;
    }
    Str++;
  }

  ReturnStr = Str;

  // Scan for the separator of this device node, '/' or ','
  ParenthesesStack = 0;
  while (!IS_NULL (*Str)) 
  {
    if ((IS_COMMA (*Str) || IS_SLASH (*Str)) && (ParenthesesStack == 0)) 
	{
      break;
    }

    if (IS_LEFT_PARENTH (*Str)) 
	{
      ParenthesesStack++;
    } 
	else if (IS_RIGHT_PARENTH (*Str)) 
	{
      ParenthesesStack--;
    }

    Str++;
  }

  if (ParenthesesStack != 0) 
  {
    // The '(' doesn't pair with ')', invalid device path text
    return NULL;
  }

  if (IS_COMMA (*Str)) 
  {
    *IsInstanceEnd = 1;
    *Str = '\0';
    Str++;
  } 
  else 
  {
    *IsInstanceEnd = 0;
    if (!IS_NULL (*Str)) 
	{
      *Str = '\0';
      Str++;
    }
  }

  *DevicePath = Str;

  return ReturnStr;
}

/*
 * Function unpacks a device path data structure so that all the nodes of a device path 
 * are naturally aligned.
 */
EFI_DEVICE_PATH_P *UnpackDevicePath (EFI_DEVICE_PATH_P  *DevPath)
{
  EFI_DEVICE_PATH_P  *Src;
  EFI_DEVICE_PATH_P  *Dest;
  EFI_DEVICE_PATH_P  *NewPath;
  UINT32			Size;
  UINT32			Count;

  if (DevPath == NULL) 
  {
    return NULL;
  }
  
  // Walk device path and round sizes to valid boundries  
  Src   = DevPath;
  Size  = 0;
  for (Count = 0;;Count++) 
  {
	if(Count > MAX_DEVICE_PATH_LEN)
	{
		// BugBug: Code to catch bogus device path
		fprintf(stderr, "UnpackDevicePath: Cannot find device path end! Probably a bogus device path\n");
		return NULL;
	} 
    Size += DevicePathNodeLength (Src);
    Size += ALIGN_SIZE (Size);

    if (IsDevicePathEnd (Src)) 
	{
      break;
    }

    Src = (EFI_DEVICE_PATH_P *) NextDevicePathNode (Src);
  }

  // Allocate space for the unpacked path
  NewPath = (EFI_DEVICE_PATH_P *)(UINT8*)calloc(Size, sizeof(UINT8));
  
  if (NewPath != NULL) 
  {

    assert(((UINT32) NewPath) % MIN_ALIGNMENT_SIZE == 0);

    // Copy each node
    Src   = DevPath;
    Dest  = NewPath;
    for (;;) 
	{
      Size = DevicePathNodeLength (Src);
      memcpy(Dest, Src, Size);
      Size += ALIGN_SIZE (Size);
      SetDevicePathNodeLength (Dest, Size);
      Dest->Type |= EFI_DP_TYPE_UNPACKED;
      Dest = (EFI_DEVICE_PATH_P *) (((UINT8 *) Dest) + Size);

      if (IsDevicePathEnd (Src)) 
	  {
        break;
      }

      Src = (EFI_DEVICE_PATH_P *) NextDevicePathNode (Src);
    }
  }

  return NewPath;
}

// Returns the size of the device path, in bytes.
UINT32 DevicePathSize (const EFI_DEVICE_PATH_P  *DevicePath)
{
  const EFI_DEVICE_PATH_P*Start;
  UINT32 Count = 0;

  if (DevicePath == NULL) 
  {
    return 0;
  }

  // Search for the end of the device path structure
  Start = (EFI_DEVICE_PATH_P*) DevicePath;
  for (Count = 0;!IsDevicePathEnd(DevicePath);Count++)
  {
 	if(Count > MAX_DEVICE_PATH_LEN)
	{
		// BugBug: Code to catch bogus device path
		fprintf(stderr, "DevicePathSize: Cannot find device path end! Probably a bogus device path\n");
		return 0;
	}  
    DevicePath = NextDevicePathNode (DevicePath);
  }

  // Compute the size and add back in the size of the end device path structure
  return ((UINT32) DevicePath - (UINT32) Start) + sizeof (EFI_DEVICE_PATH_P);
}

// Creates a device node
EFI_DEVICE_PATH_P*CreateDeviceNode (UINT8 NodeType, UINT8 NodeSubType, UINT16 NodeLength)
{
  EFI_DEVICE_PATH_P*Node;

  if (NodeLength < sizeof (EFI_DEVICE_PATH_P)) 
  {
    return NULL;
  }

  Node = (EFI_DEVICE_PATH_P*) (UINT8*)calloc ((UINT32) NodeLength, sizeof(UINT8));
  if (Node != NULL) 
  {
    Node->Type    = NodeType;
    Node->SubType = NodeSubType;
    SetDevicePathNodeLength (Node, NodeLength);
  }

  return Node;
}

// Duplicate a device path structure.
EFI_DEVICE_PATH_P*DuplicateDevicePathP (EFI_DEVICE_PATH_P *DevicePath)
{
  EFI_DEVICE_PATH_P*NewDevicePath;
  UINT32 Size;

  if (DevicePath == NULL) 
  {
    return NULL;
  }

  // Compute the size
  Size = DevicePathSize (DevicePath);
  if (Size == 0) 
  {
    return NULL;
  }

  // Allocate space for duplicate device path
  NewDevicePath = MallocCopy(Size, DevicePath);

  return NewDevicePath;
}

void EisaIdToText (UINT32 EisaId, CHAR8 *Text)
{
  CHAR8 PnpIdStr[17];

  //SPrint ("%X", 0x0a03) => "0000000000000A03"
  snprintf(PnpIdStr, 17, "%X", EisaId >> 16);
  snprintf(Text,0,"%c%c%c%s",'@' + ((EisaId >> 10) & 0x1f),'@' + ((EisaId >>  5) & 0x1f),'@' + ((EisaId >>  0) & 0x1f), PnpIdStr + (16 - 4));
}

void DevPathToTextPci (CHAR8  *Str, void  *DevPath, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts)
{
  PCI_DEVICE_PATH_P *Pci;

  Pci = DevPath;
  CatPrintf(Str, "Pci(0x%x,0x%x)", Pci->Device, Pci->Function);
}

void DevPathToTextAcpi (CHAR8 *Str, void *DevPath, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts)
{
  ACPI_HID_DEVICE_PATH_P  *Acpi;

  Acpi = DevPath;
  if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) 
  {
    switch (EISA_ID_TO_NUM (Acpi->HID)) 
	{
		case 0x0a03:
			CatPrintf(Str, "PciRoot(0x%x)", Acpi->UID);
		break;
		default:
			CatPrintf(Str, "Acpi(PNP%04x,0x%x)", EISA_ID_TO_NUM (Acpi->HID), Acpi->UID);
		break;
    }
  } 
  else 
  {
    CatPrintf(Str, "Acpi(0x%08x,0x%x)", Acpi->HID, Acpi->UID);
  }
}

void DevPathToTextEndInstance (CHAR8 *Str, void *DevPath, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts)
{
  CatPrintf(Str, ",");
}

void DevPathToTextNodeUnknown (CHAR8 *Str, void *DevPath, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts)
{
  CatPrintf(Str, "?");
}

DEVICE_PATH_TO_TEXT_TABLE DevPathToTextTable[] = 
{
  HARDWARE_DEVICE_PATH,
  HW_PCI_DP,
  DevPathToTextPci,
  ACPI_DEVICE_PATH,
  ACPI_DP,
  DevPathToTextAcpi,
  END_DEVICE_PATH_TYPE,
  END_INSTANCE_DEVICE_PATH_SUBTYPE,
  DevPathToTextEndInstance,
  0,
  0,
  NULL
};

// Convert a device path to its text representation.
CHAR8 *ConvertDevicePathToAscii (const EFI_DEVICE_PATH_P*DevicePath, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts)
{
	CHAR8				*Str;
	EFI_DEVICE_PATH_P		*DevPathNode;
	EFI_DEVICE_PATH_P		*UnpackDevPath;
	UINT32				Index;
	UINT32				NewSize;
	void (*DumpNode) (CHAR8 *, void *, BOOLEAN, BOOLEAN);

	if (DevicePath == NULL) 
	{
		return NULL;
	}

	Str = (CHAR8 *)calloc(MAX_PATH_LEN, sizeof(CHAR8));

	// Unpacked the device path
	UnpackDevPath = UnpackDevicePath ((EFI_DEVICE_PATH_P*) DevicePath);
	assert(UnpackDevPath != NULL);

	// Process each device path node
	DevPathNode = UnpackDevPath;
	while (!IsDevicePathEnd (DevPathNode)) 
	{
		// Find the handler to dump this device path node
		DumpNode = NULL;
		for (Index = 0; DevPathToTextTable[Index].Function; Index += 1) 
		{
			if (DevicePathType (DevPathNode) == DevPathToTextTable[Index].Type &&
				DevicePathSubType (DevPathNode) == DevPathToTextTable[Index].SubType) 
				{
					DumpNode = DevPathToTextTable[Index].Function;
					break;
				}
		}
		// If not found, use a generic function
		if (!DumpNode) 
		{
			DumpNode = DevPathToTextNodeUnknown;
		}
		
		//  Put a path seperator in if needed
		if (strlen(Str) && DumpNode != DevPathToTextEndInstance) 
		{
			if (*(Str + strlen(Str) - 1) != ',') 
			{
				CatPrintf(Str, "/");
			}
		}
		// Print this node of the device path
		DumpNode (Str, DevPathNode, DisplayOnly, AllowShortcuts);

		// Next device path node
		DevPathNode = NextDevicePathNode (DevPathNode);
	}
	
	// Shrink pool used for string allocation
	free(UnpackDevPath);
	NewSize = ((UINT32)strlen(Str) + 1);
	Str = realloc(Str, NewSize);
	assert(Str != NULL);
	Str[strlen(Str)] = 0;
	return Str;
}

