/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "Platform.h"

struct p_state 
{
	union 
	{
		UINT16 Control;
		struct 
		{
			UINT8 VID;	// Voltage ID
			UINT8 FID;	// Frequency ID
		};
	};
	
	UINT8		CID;		// Compare ID
	UINT32	Frequency;
};

#endif /* !__LIBSAIO_ACPI_PATCHER_H */
