/** @file

Module Name:

  EmuVariableControl.h

  Protocol for controlling EmuVariableUefi driver - installing or uninstalling RT var services emulation.

  initial version - dmazar

**/

#ifndef __EmuVariableControl_H__
#define __EmuVariableControl_H__

typedef struct _EMU_VARIABLE_CONTROL_PROTOCOL EMU_VARIABLE_CONTROL_PROTOCOL;

/**
 * EMU_VARIABLE_CONTROL_PROTOCOL.InstallEmulation() type definition
 */
typedef EFI_STATUS (EFIAPI * EMU_VARIABLE_CONTROL_INSTALL_EMULATION) (
    IN  EMU_VARIABLE_CONTROL_PROTOCOL     *This
);

/**
 * EMU_VARIABLE_CONTROL_PROTOCOL.UninstallEmulation() type definition
 */
typedef EFI_STATUS (EFIAPI * EMU_VARIABLE_CONTROL_UNINSTALL_EMULATION) (
    IN  EMU_VARIABLE_CONTROL_PROTOCOL     *This
);


/**
 * EMU_VARIABLE_CONTROL_PROTOCOL
 */
struct _EMU_VARIABLE_CONTROL_PROTOCOL {
    ///
    /// Installs RT var services emulation, replaces original RT var services.
    ///
	EMU_VARIABLE_CONTROL_INSTALL_EMULATION		InstallEmulation;
    ///
    /// Uninstalls RT var services emulation, returns original RT var services back.
    ///
	EMU_VARIABLE_CONTROL_INSTALL_EMULATION		UninstallEmulation;
};


#define EMU_VARIABLE_CONTROL_PROTOCOL_GUID \
  { \
    0x21f41e73, 0xd214, 0x4fcd, {0x85, 0x50, 0x0d, 0x11, 0x51, 0xcf, 0x8e, 0xfb } \
  }

/** EMU_VARIABLE_CONTROL_PROTOCOL GUID */
extern EFI_GUID gEmuVariableControlProtocolGuid;


#endif

