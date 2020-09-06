//********************************************************************
//	created:	27:8:2012   21:30
//	filename: 	AppleKeyState.h
//	author:		tiamo
//	purpose:	apple key state
//********************************************************************

#ifndef _APPLE_KEY_STATE_H_
#define _APPLE_KEY_STATE_H_

#define APPLE_KEY_STATE_PROTOCOL_GUID					{0x5b213447, 0x6e73, 0x4901, {0xa4, 0xf1, 0xb8, 0x64, 0xf3, 0xb7, 0xa1, 0x72}}

typedef UINT16 APPLE_KEY_CODE;
// APPLE_MODIFIER_MAP
typedef UINT16 APPLE_MODIFIER_MAP;


#define APPLE_KEY_STATE_MODIFY_LEFT_CONTROL									0x01
#define APPLE_KEY_STATE_MODIFY_LEFT_SHIFT									  0x02
#define APPLE_KEY_STATE_MODIFY_LEFT_OPTION									0x04
#define APPLE_KEY_STATE_MODIFY_LEFT_COMMAND									0x08
#define APPLE_KEY_STATE_MODIFY_RIGHT_CONTROL								0x10
#define APPLE_KEY_STATE_MODIFY_RIGHT_SHIFT									0x20
#define APPLE_KEY_STATE_MODIFY_RIGHT_OPTION									0x40
#define APPLE_KEY_STATE_MODIFY_RIGHT_COMMAND								0x80

#define APPLE_KEY_STATE_A													0x7004
#define APPLE_KEY_STATE_B													0x7005
#define APPLE_KEY_STATE_C													0x7006
#define APPLE_KEY_STATE_D													0x7007
#define APPLE_KEY_STATE_E													0x7008
#define APPLE_KEY_STATE_F													0x7009
#define APPLE_KEY_STATE_G													0x700a
#define APPLE_KEY_STATE_H													0x700b
#define APPLE_KEY_STATE_I													0x700c
#define APPLE_KEY_STATE_J													0x700d
#define APPLE_KEY_STATE_K													0x700e
#define APPLE_KEY_STATE_L													0x700f
#define APPLE_KEY_STATE_M													0x7010
#define APPLE_KEY_STATE_N													0x7011
#define APPLE_KEY_STATE_O													0x7012
#define APPLE_KEY_STATE_P													0x7013
#define APPLE_KEY_STATE_Q													0x7014
#define APPLE_KEY_STATE_R													0x7015
#define APPLE_KEY_STATE_S													0x7016
#define APPLE_KEY_STATE_T													0x7017
#define APPLE_KEY_STATE_U													0x7018
#define APPLE_KEY_STATE_V													0x7019
#define APPLE_KEY_STATE_W													0x701a
#define APPLE_KEY_STATE_X													0x701b
#define APPLE_KEY_STATE_Y													0x701c
#define APPLE_KEY_STATE_Z													0x701d
#define APPLE_KEY_STATE_1													0x701e
#define APPLE_KEY_STATE_2													0x701f
#define APPLE_KEY_STATE_3													0x7020
#define APPLE_KEY_STATE_4													0x7021
#define APPLE_KEY_STATE_5													0x7022
#define APPLE_KEY_STATE_6													0x7023
#define APPLE_KEY_STATE_7													0x7024
#define APPLE_KEY_STATE_8													0x7025
#define APPLE_KEY_STATE_9													0x7026
#define APPLE_KEY_STATE_0													0x7027
#define APPLE_KEY_STATE_ENTER											0x7028
#define APPLE_KEY_STATE_ESCAPE										0x7029
#define APPLE_KEY_STATE_BACKSPACE									0x702a
#define APPLE_KEY_STATE_TAB												0x702b
//SPACE =?
#define APPLE_KEY_STATE_MINUS											0x702d
#define APPLE_KEY_STATE_EQUAL											0x702e
#define APPLE_KEY_STATE_LEFT_BRACKET							0x702f
#define APPLE_KEY_STATE_RIGHT_BRACKET							0x7030
#define APPLE_KEY_STATE_BACKSLASH									0x7031
#define APPLE_KEY_STATE_SEMICOLON									0x7033
#define APPLE_KEY_STATE_QUOTATION									0x7034
#define APPLE_KEY_STATE_ACUTE											0x7035
#define APPLE_KEY_STATE_COMMA											0x7036
#define APPLE_KEY_STATE_PERIOD 										0x7037
#define APPLE_KEY_STATE_SLASH											0x7038
#define APPLE_KEY_STATE_CAPS_LOCK									0x7039
#define APPLE_KEY_STATE_F1												0x703a
#define APPLE_KEY_STATE_F2												0x703b
#define APPLE_KEY_STATE_F3												0x703c
#define APPLE_KEY_STATE_F4												0x703d
#define APPLE_KEY_STATE_F5												0x703e
#define APPLE_KEY_STATE_F6												0x703f
#define APPLE_KEY_STATE_F7												0x7040
#define APPLE_KEY_STATE_F8												0x7041
#define APPLE_KEY_STATE_F9												0x7043
#define APPLE_KEY_STATE_F10												0x7043
#define APPLE_KEY_STATE_F11												0x7044
#define APPLE_KEY_STATE_F12												0x7045
#define APPLE_KEY_STATE_F14												0x7046
#define APPLE_KEY_STATE_F15												0x7047
#define APPLE_KEY_STATE_F16												0x7048
#define APPLE_KEY_STATE_INSERT										0x7049
#define APPLE_KEY_STATE_HOME											0x704a
#define APPLE_KEY_STATE_PAGE_UP										0x704b
#define APPLE_KEY_STATE_DELETE										0x704c
#define APPLE_KEY_STATE_END												0x704d
#define APPLE_KEY_STATE_PAGE_DOWN									0x704e
#define APPLE_KEY_STATE_RIGHT_ARROW								0x704f
#define APPLE_KEY_STATE_LEFT_ARROW								0x7050
#define APPLE_KEY_STATE_DOWN_ARROW								0x7051
#define APPLE_KEY_STATE_UP_ARROW									0x7052
#define APPLE_KEY_STATE_NUM_PAD_CLEAR							0x7053
#define APPLE_KEY_STATE_NUM_PAD_DIV								0x7054
#define APPLE_KEY_STATE_NUM_PAD_MUL								0x7055
#define APPLE_KEY_STATE_NUM_PAD_MINUS							0x7056
#define APPLE_KEY_STATE_NUM_PAD_PLUS							0x7057
#define APPLE_KEY_STATE_NUM_PAD_ENTER							0x7058
#define APPLE_KEY_STATE_NUM_PAD_1									0x7059
#define APPLE_KEY_STATE_NUM_PAD_2									0x705a
#define APPLE_KEY_STATE_NUM_PAD_3									0x705b
#define APPLE_KEY_STATE_NUM_PAD_4									0x705c
#define APPLE_KEY_STATE_NUM_PAD_5									0x705d
#define APPLE_KEY_STATE_NUM_PAD_6									0x705e
#define APPLE_KEY_STATE_NUM_PAD_7									0x705f
#define APPLE_KEY_STATE_NUM_PAD_8									0x7060
#define APPLE_KEY_STATE_NUM_PAD_9									0x7061
#define APPLE_KEY_STATE_NUM_PAD_INSERT						0x7062
#define APPLE_KEY_STATE_NUM_PAD_DELETE						0x7063
#define APPLE_KEY_STATE_APPLICATION								0x7065

typedef struct _APPLE_KEY_STATE_PROTOCOL APPLE_KEY_STATE_PROTOCOL;

typedef EFI_STATUS (EFIAPI *READ_KEY_STATE)(IN APPLE_KEY_STATE_PROTOCOL *This, OUT UINT16 *ModifyFlags, OUT UINTN *PressedKeyStatesCount, OUT APPLE_KEY_CODE *PressedKeyStates);
//it seems key stroke assumed <8 AppleKeys
typedef EFI_STATUS (EFIAPI *SEARCH_KEY_STROKE)(IN APPLE_KEY_STATE_PROTOCOL *This, IN UINT16 ModifyFlags, IN UINTN PressedKeyStatesCount, IN OUT APPLE_KEY_CODE *PressedKeyStates, IN BOOLEAN ExactMatch);

struct _APPLE_KEY_STATE_PROTOCOL
{
	UINTN 																	Signature;
	READ_KEY_STATE													ReadKeyState;
  SEARCH_KEY_STROKE                       SearchKeyStroke;
};

extern EFI_GUID gAppleKeyStateProtocolGuid;

#endif
