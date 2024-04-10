//*****************************************************************************
// Filename: ION_BT_ProgDevSelectScreen.cpp
//
// Date: Sept 5, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"
#include "BluetoothDeviceInfo.h"

#define BT_SELECTED_BTN_ID (20)

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

// The following hold the Bluetooth Data for the User Bluetooth Screen.
BLUETOOTH_SCREEN_DATA g_BTDeviceSetup_ScreenInfo[MAX_BLUETOOTH_DEVICES];

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

void CreateBluetoothWidgets (GX_VERTICAL_LIST *list);
VOID BluetoothList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index);

//*************************************************************************************
// We copy the Bluetooth Device Setup information into the Locally Accessible Bluetooth
// structure which also contain created widget information.
//*************************************************************************************

static void Initialize_BT_SelectionScreenInfo(BLUETOOTH_SCREEN_DATA *screenData)
{
	int sourceArrayIndex;

	for (sourceArrayIndex = 0; sourceArrayIndex < MAX_BLUETOOTH_DEVICES; ++sourceArrayIndex)
	{
		screenData[sourceArrayIndex].m_DeviceSettings = &g_BluetoothDeviceSettings[sourceArrayIndex];
	}
}

//*************************************************************************************
// Function Name: FeatureList_callback
//
// Description: This function handles drawing and processing the List Items.
//
//*************************************************************************************

VOID BluetoothList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
    BLUETOOTH_SCREEN_DATA *feature = (BLUETOOTH_SCREEN_DATA *)widget;
    GX_BOOL result;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
    if (!result)
    {
        gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
        gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /*| GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

        gx_utility_rectangle_define(&childsize, 0, 2, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14), 62);

		// Create a button
		gx_text_button_create (&feature->m_ButtonWidget, "MYBUTTON", &feature->m_ItemWidget, GX_STRING_ID_BLUETOOTH, GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, (USHORT) (BT_SELECTED_BTN_ID +  index), &childsize);
		gx_text_button_text_id_set (&feature->m_ButtonWidget, g_BluetoothDeviceSettings[index].m_DescriptionID);
		feature->m_ButtonWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_TEXT;
		feature->m_ButtonWidget.gx_widget_normal_fill_color = GX_COLOR_ID_TEXT_INPUT_FILL;
		feature->m_ButtonWidget.gx_widget_selected_fill_color = GX_COLOR_ID_TEXT_INPUT_TEXT;
	}

}

//*************************************************************************************
// This function populates the Feature List.
//*************************************************************************************

void CreateBluetoothWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < MAX_BLUETOOTH_DEVICES; ++index)
	{
		BluetoothList_callback (list, (GX_WIDGET*) &g_BTDeviceSetup_ScreenInfo[index], index);
		++activeFeatureCount;
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;
}

//*************************************************************************************
// Function Name: FeatureSettingsScreen_event_process
//
// Description: This handles the Feature Settings Screen messages
//
//*************************************************************************************

UINT ION_BluetoothDeviceSelectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	int btnID;

	ION_BT_DEVICESELECTIONSCREEN_CONTROL_BLOCK *BluetoothWindowPtr = (ION_BT_DEVICESELECTIONSCREEN_CONTROL_BLOCK*) window;

	//GX_EVENT myEvent;

	switch (event_ptr->gx_event_type)
	{
        case GX_EVENT_SHOW:
            Initialize_BT_SelectionScreenInfo(g_BTDeviceSetup_ScreenInfo);
            BT_Screen_Widget_Cleanup (g_BTDeviceSetup_ScreenInfo, MAX_BLUETOOTH_DEVICES);
            BluetoothWindowPtr->ION_BT_DeviceSelectionScreen_BluetoothDeviceListBox.gx_vertical_list_child_count = 0;
            CreateBluetoothWidgets (&BluetoothWindowPtr->ION_BT_DeviceSelectionScreen_BluetoothDeviceListBox);
            break;

        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            PushWindow (window);
            screen_toggle((GX_WINDOW *)&ION_MainProgrammingScreen, window);
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            BT_Screen_Widget_Cleanup (g_BTDeviceSetup_ScreenInfo, MAX_BLUETOOTH_DEVICES);
            BluetoothWindowPtr->ION_BT_DeviceSelectionScreen_BluetoothDeviceListBox.gx_vertical_list_child_count = 0;
            screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            break;

        case GX_SIGNAL (BT_SELECTED_BTN_ID, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+1, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+2, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+3, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+4, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+5, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+6, GX_EVENT_CLICKED):
        case GX_SIGNAL (BT_SELECTED_BTN_ID+7, GX_EVENT_CLICKED):
            btnID = (int)(event_ptr->gx_event_type) >> 8;	// This isolates the Button ID
            g_SelectedBTDevice_ToProgram = (uint8_t) (btnID - BT_SELECTED_BTN_ID);
            screen_toggle((GX_WINDOW *)&ION_BT_SetupScreen, window);
            break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}













