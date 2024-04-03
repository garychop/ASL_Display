//*****************************************************************************
// Filename: ION_BT_ActiveScreen.cpp
//
// Date: July 19, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"
#include "BluetoothDeviceInfo.h"

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

//*************************************************************************************
// Function Name: ION_BT_ActiveScreen_event_process
//
// Description: This handles the Bluetooth Active Screen messages
//
//*************************************************************************************

UINT ION_BT_ActiveScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	ION_BT_ACTIVESCREEN_CONTROL_BLOCK *windowPtr = (ION_BT_ACTIVESCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		// Now show the Device Information.
		gx_prompt_text_id_set((GX_PROMPT*)&windowPtr->ION_BT_ActiveScreen_ActiveDeviceName, g_BluetoothDeviceSettings[g_SelectedBluetoothDeviceIndex].m_DescriptionID);
		gx_pixelmap_prompt_pixelmap_set (&windowPtr->ION_BT_ActiveScreen_ActiveDeviceName, GX_ID_NONE, GX_ID_NONE, g_BluetoothDeviceSettings[g_SelectedBluetoothDeviceIndex].m_BT_Icon_Selected, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
		gx_widget_fill_color_set ((GX_WIDGET*) &windowPtr->ION_BT_ActiveScreen_ActiveDeviceName, g_BluetoothDeviceSettings[g_SelectedBluetoothDeviceIndex].m_NormalFillColor, g_BluetoothDeviceSettings[g_SelectedBluetoothDeviceIndex].m_NormalFillColor);
        //gx_prompt_text_color_set (&windowPtr->ION_BT_ActiveScreen_ActiveDeviceName, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
		// Set the Status to "Connecting..." and make it yellow.
		gx_prompt_text_id_set ((GX_PROMPT*) &windowPtr->ION_BT_ActiveScreen_ActiveStatusPrompt, GX_STRING_ID_CONNECTING);
		gx_prompt_text_color_set (&windowPtr->ION_BT_ActiveScreen_ActiveStatusPrompt, GX_COLOR_ID_YELLOW, GX_COLOR_ID_YELLOW);
//		gx_system_timer_start(window, BT_CONNECTING_TIMER_ID, 80, 0);	// "x4" seems to make the time real.
        g_ActiveScreen = (GX_WIDGET*) window;
		break;

    case GX_SIGNAL (BT_SUBMENU_CHANGED_ID, GX_EVENT_CLICKED): // This event is triggered by a change in the Bluetooth SubIndex message from ION Hub
        if ((g_BluetoothSubIndex & 0x01) == 0x00)  // Did we stop processing the Device?
        {
            screen_toggle((GX_WINDOW *)&ION_BT_UserSelectionScreen, window);
        }
        break;

//	case GX_EVENT_PEN_DOWN:
//		if (event_ptr->gx_event_target->gx_widget_id == DRIVE_CONTROL_USERPORT_ID)
//		{
//			gx_system_timer_start(window, USER_PORT_TIMER_ID, g_TimeoutValue * 4, 0);	// "x4" seems to make the time real.
//		}
//		break;
//
//	case GX_EVENT_PEN_UP:
//		gx_system_timer_stop(window, USER_PORT_TIMER_ID);
//		if (g_ChangeScreen_WIP)
//		{
//			g_ChangeScreen_WIP = FALSE;
//			break;
//		}
//		screen_toggle((GX_WINDOW *)&ION_BT_UserSelectionScreen, window);
//		break;

//    case GX_EVENT_TIMER:
//		if (event_ptr->gx_event_payload.gx_event_timer_id == BT_CONNECTING_TIMER_ID)
//		{
//			gx_prompt_text_id_set ((GX_PROMPT*) &windowPtr->ION_BT_ActiveScreen_ActiveStatusPrompt, GX_STRING_ID_OK_TO_USE);
//			gx_prompt_text_color_set (&windowPtr->ION_BT_ActiveScreen_ActiveStatusPrompt, GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN);
//		}
//        else if (event_ptr->gx_event_payload.gx_event_timer_id == USER_PORT_TIMER_ID)
//		{
//			screen_toggle((GX_WINDOW *)&ION_BT_UserSelectionScreen, window);
//			g_ChangeScreen_WIP = TRUE;
//		}
//		break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}

