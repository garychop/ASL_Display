//*****************************************************************************
// Filename: ION_BT_SetupScreen.cpp
//
// Date: July 28, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"
#include "BluetoothDeviceInfo.h"

//*************************************************************************************
// Macros
//*************************************************************************************

#define BT_SETUP_BUTTON_ID_BASE (5)
#define MAX_SETUP_BUTTONS (4)

#define DESCRIPTION_BUTTON_ID (151)
#define COLOR_BUTTON_ID (DESCRIPTION_BUTTON_ID + 1)
#define PAIRING_BUTTON_ID (COLOR_BUTTON_ID + 1)

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

//extern BLUETOOTH_DEVICE_DATA g_BluetoothDeviceSettings[];
//extern uint8_t g_SelectedBTDevice_ToProgram;	// This contains the array index into the Bluetooth setup information.

static char g_PromptStr[32];	// Used for BT prompt #

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

static void Set_BT_Setup_ButtonInfo (ION_BT_SETUPSCREEN_CONTROL_BLOCK *window);

//*************************************************************************************
// This function set the Bluetooth Device and Color buttons.
//*************************************************************************************

static void Set_BT_Setup_ButtonInfo (ION_BT_SETUPSCREEN_CONTROL_BLOCK *window)
{
	// Set Device Type Button
	gx_text_button_text_id_set(&window->ION_BT_SetupScreen_BT_Type_Button, g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_DescriptionID);
	// Set Color Button
	window->ION_BT_SetupScreen_BT_Color_Button.gx_widget_normal_fill_color = g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_NormalFillColor;
	window->ION_BT_SetupScreen_BT_Color_Button.gx_widget_selected_fill_color = g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_SelectedFillColor;
	gx_text_button_text_set (&window->ION_BT_SetupScreen_BT_Color_Button, GetColorString(g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Color));
	// Set the Pairing Button
	if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status == BT_DISABLED)
		gx_text_button_text_id_set ((GX_TEXT_BUTTON*) &window->ION_BT_SetupScreen_BT_Pairing_Button, GX_STRING_ID_BT_START_PAIR);
//	else if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status == BT_PAIRED)
//		gx_text_button_text_id_set ((GX_TEXT_BUTTON*) &window->ION_BT_SetupScreen_BT_Pairing_Button, GX_STRING_ID_BT_FORGET);
}

//*************************************************************************************
// Description: This handles the ION Bluetooth Device Setup Screen messages
//*************************************************************************************

UINT ION_BT_Setup_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	ION_BT_SETUPSCREEN_CONTROL_BLOCK *BT_SetupWindowPtr = (ION_BT_SETUPSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		// Set the Button Information
		Set_BT_Setup_ButtonInfo(BT_SetupWindowPtr);
		// Set the Device Number "#1"
		sprintf(g_PromptStr, "#%d", (UINT)(g_SelectedBTDevice_ToProgram + 1));
		gx_prompt_text_set(&ION_BT_SetupScreen.ION_BT_SetupScreen_BT_Number_Prompt, g_PromptStr);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
		screen_toggle((GX_WINDOW*)&ION_BT_DeviceSelectionScreen, window);
		break;

	case GX_SIGNAL(TYPE_BTN_ID, GX_EVENT_CLICKED):
		if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Type != BT_ACU_TYPE)	// Don't change the BT ACU type.
		{
			// Set the next Bluetooth Device Type
			++g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Type;
			if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Type >= BT_ACU_TYPE)	// Don't allow ACU either
				g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Type = BT_MOUSE_TYPE;
			// Set the Devices information.
			BT_SetDeviceTypeInformation(g_SelectedBTDevice_ToProgram, g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Type);
			Set_BT_Setup_ButtonInfo(BT_SetupWindowPtr);
		}
		break;

	case GX_SIGNAL (COLOR_BTN_ID, GX_EVENT_CLICKED):
		if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Type != BT_ACU_TYPE)	// Don't change the BT ACU type.
		{
			// Set to the next color
			++g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Color;
			if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Color >= BT_COLOR_END)
				g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Color = (BT_COLOR)0;
			// Set the device's new color.
			BT_SetDeviceColor(g_SelectedBTDevice_ToProgram, g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_BT_Color);
			Set_BT_Setup_ButtonInfo(BT_SetupWindowPtr);
		}
		break;

	case GX_SIGNAL (BT_PAIRING_BTN_ID, GX_EVENT_CLICKED):
		if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status == BT_DISABLED)
		{
		    g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status = BT_ENABLED;

		}
// Chop add this code back in
//			g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status = BT_PAIRING_WIP;
//			// Switch to Pairing Screen.
//			screen_toggle((GX_WINDOW *)&ION_BT_PairingScreen, window);
//		}
//		else if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status == BT_ENABLED)
//		{
//			g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status = BT_DISABLED;
//		}
//		else if (g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status == BT_PAIRED)
//		{
//			g_BluetoothDeviceSettings[g_SelectedBTDevice_ToProgram].m_Status = BT_DISABLED;
//		}
		Set_BT_Setup_ButtonInfo (BT_SetupWindowPtr);
		break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}













