//*****************************************************************************
// Filename: BluetoothDeviceInfo.h
// Description: This file declares function, typedefs and macros to support
//		the Bluetooth devices
//
// Date: July 19, 2023.
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#ifndef BLUETOOTH_DEVICE_INFORMATON_H
#define BLUETOOTH_DEVICE_INFORMATON_H

//#include "asl104E_display_demo_resources.h"

#define MAX_BLUETOOTH_DEVICES (8)
#define BUTTON_ID_BASE (100)
#define DESCRIPTION_STRING_WIDTH (64)
#define BT_COLOR_STRING_WIDTH (16)

typedef enum BT_COLOR_ENUM {BT_WHITE, BT_BLUE, BT_ORANGE, BT_RED, BT_GREEN, BT_YELLOW, BT_COLOR_END} BT_COLOR;

// "BT_TYPE_END" must be last.
typedef enum BT_DEVICE_TYPE_ENUM {BT_MOUSE_TYPE, BT_3_SWITCH_TYPE, BT_4_SWITCH_TYPE, BT_AUX_COMM_TYPE, BT_FEEDER_TYPE, BT_PHONE_TYPE, BT_TABLET_TYPE, BT_LEGACY_TYPE, BT_SEATING_TYPE, BT_GENERIC_TYPE_1, BT_GENERIC_TYPE_2, BT_GENERIC_TYPE_3, BT_GENERIC_TYPE_4, BT_ACU_TYPE, BT_TYPE_END} BT_DEVICE_TYPE;

// The following describes the possible BT device Statii
typedef enum BT_STATUS_ENUM {BT_DISABLED, BT_PAIRING_WIP, BT_PAIRED, BT_STATUS_END} BT_STATUS;

// This holds the Global Bluetooth Device Data but not the Screen Information.
typedef struct BLUETOOTH_DEVICE_DATA_STRUCT
{
    BT_STATUS m_Status;      // Indicates the status of this bluetooth device.
	BT_DEVICE_TYPE m_BT_Type;
	GX_RESOURCE_ID m_DescriptionID;
	GX_RESOURCE_ID m_BT_Icon_Selected;
	GX_RESOURCE_ID m_BT_Icon_Normal;
	BT_COLOR m_BT_Color;
	GX_COLOR m_NormalFillColor;
	GX_COLOR m_SelectedFillColor;
} BLUETOOTH_DEVICE_DATA;

// This structure is used by each screen.
typedef struct BLUETOOTH_SCREEN_DATA_STRUCT
{
	GX_WIDGET m_ItemWidget;
	GX_PROMPT m_PromptWidget;
	GX_TEXT_BUTTON m_ButtonWidget;
	GX_PIXELMAP_PROMPT m_PixelPromptWidget;
	GX_MULTI_LINE_TEXT_BUTTON m_MultilineBtnWidget;
	GX_ICON m_IconWidget;
	BLUETOOTH_DEVICE_DATA *m_DeviceSettings;
} BLUETOOTH_SCREEN_DATA;


//*****************************************************************************
// Global Variables
//*****************************************************************************

extern BLUETOOTH_DEVICE_DATA g_BluetoothDeviceSettings[MAX_BLUETOOTH_DEVICES];
extern const char g_BT_DESCRIPTION_STRINGS [BT_TYPE_END][DESCRIPTION_STRING_WIDTH+1];
extern const char g_BT_COLOR_TEXT [][BT_COLOR_STRING_WIDTH];
extern int g_SelectedBluetoothDeviceIndex;
extern int g_SelectedBTDevice_ToProgram;	// This contains the array index into the Bluetooth setup information.

//*****************************************************************************
// Function Prototypes
//*****************************************************************************

void InitializeBluetoothDeviceInformation(void);
void BT_SetDeviceTypeInformation (UINT index, BT_DEVICE_TYPE deviceType);
void BT_SetDeviceColor (UINT index, BT_COLOR color);
void BT_SetDeviceStatus (UINT index, BT_STATUS newStatus);
BT_STATUS BT_GetDeviceStatus (UINT index);
VOID BT_Screen_Widget_Cleanup (BLUETOOTH_SCREEN_DATA *bt_screen_info, INT arraySize);

#endif // BLUETOOTH_DEVICE_INFORMATON_H


