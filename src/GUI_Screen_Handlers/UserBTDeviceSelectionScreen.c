//*****************************************************************************
// Filename: UserBTDeviceSelectionScreen.cpp
// Description: This file supports the Idle Screen.
//
// Date: July 19, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "BluetoothDeviceInfo.h"

//*************************************************************************************
// Defines and Macros
//*************************************************************************************

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

void CreateUserBluetoothWidgets (GX_VERTICAL_LIST *list);
VOID UserBluetoothList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index);
void UpdateBTUserSelection (void);
//static void CleanUpWidgets (void);

//*************************************************************************************
// Global and Local variables
//*************************************************************************************

static int g_Selected_Button_Index = 0;
static int g_NumberOfPairedDevices = 0;

// The following is the Bluetooth Device information.
extern BLUETOOTH_DEVICE_DATA g_BluetoothDeviceSettings[];

// The following hold the Bluetooth Data for the User Bluetooth Screen.
BLUETOOTH_SCREEN_DATA g_BT_ScreenInfo[MAX_BLUETOOTH_DEVICES+1];

//*************************************************************************************
// We copy the Bluetooth Device Setup information into the Locally Accessible Bluetooth
// structure which also contain created widget information.
//*************************************************************************************

static void InitializeUserBluetoothDeviceInformation(void)
{
	int sourceArrayIndex;

	g_NumberOfPairedDevices = 1;	// First line is "BACK", start at Array Index "1"
	for (sourceArrayIndex = 0; sourceArrayIndex < (MAX_BLUETOOTH_DEVICES - 1); ++sourceArrayIndex)	// "-1" cuz we don't want to show the BT Attendant
	{
		if (g_BluetoothDeviceSettings[sourceArrayIndex].m_Status == BT_PAIRED)
		{
			g_BT_ScreenInfo[g_NumberOfPairedDevices].m_DeviceSettings = &(g_BluetoothDeviceSettings[sourceArrayIndex]);
			++g_NumberOfPairedDevices;
		}
	}
}

//*************************************************************************************
// This function creates and populates the Bluetooth List for the USER.
//*************************************************************************************

void CreateUserBluetoothWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < g_NumberOfPairedDevices; ++index)
	{
		if (index == 0)	// Index 0 is the "BACK" button.
		{
			UserBluetoothList_callback (list, (GX_WIDGET*) &g_BT_ScreenInfo[index], index);
			++activeFeatureCount;
		}
		else if (g_BT_ScreenInfo[index].m_DeviceSettings->m_Status == BT_PAIRED)
		{
			UserBluetoothList_callback (list, (GX_WIDGET*) &g_BT_ScreenInfo[index], index);
			++activeFeatureCount;
		}
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;
}

//*************************************************************************************

void UpdateBTUserSelection (void)
{
	int index;

	for (index = 0; index < g_NumberOfPairedDevices; ++index)
	{
		if (g_Selected_Button_Index == index)
		{
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_FILL;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_normal_fill_color = GX_COLOR_ID_BTN_LOWER;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_selected_fill_color = GX_COLOR_ID_SELECTED_FILL;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_prompt_disabled_text_color = GX_COLOR_ID_DISABLED_TEXT;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_prompt_normal_text_color = GX_COLOR_ID_TEXT;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_prompt_selected_text_color = GX_COLOR_ID_WHITE;
			//gx_prompt_font_set ((GX_PROMPT*) &g_BT_ScreenInfo[index].m_PixelPromptWidget, GX_FONT_ID_ASC24PT);
			if (index == 0)
				gx_pixelmap_prompt_pixelmap_set (&g_BT_ScreenInfo[index].m_PixelPromptWidget, GX_ID_NONE, GX_ID_NONE, GX_PIXELMAP_ID_BT_ICON_BACK_BLACK_60X60, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
			else
			{
				gx_pixelmap_prompt_pixelmap_set (&g_BT_ScreenInfo[index].m_PixelPromptWidget, GX_ID_NONE, GX_ID_NONE, g_BT_ScreenInfo[index].m_DeviceSettings->m_BT_Icon_Selected, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
				g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_normal_fill_color = g_BT_ScreenInfo[index].m_DeviceSettings->m_NormalFillColor;
			}
		}
		else
		{
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_SHINE;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_normal_fill_color = GX_COLOR_ID_SHINE;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_widget_selected_fill_color = GX_COLOR_ID_SHINE;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_prompt_disabled_text_color = GX_COLOR_ID_DISABLED_TEXT;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_prompt_normal_text_color = GX_COLOR_ID_SELECTED_TEXT;
			g_BT_ScreenInfo[index].m_PixelPromptWidget.gx_prompt_selected_text_color = GX_COLOR_ID_SELECTED_TEXT;
			//gx_prompt_font_set ((GX_PROMPT*) &g_BT_ScreenInfo[index].m_PixelPromptWidget, GX_FONT_ID_PROMPT); 
			if (index == 0)
				gx_pixelmap_prompt_pixelmap_set (&g_BT_ScreenInfo[index].m_PixelPromptWidget, GX_ID_NONE, GX_ID_NONE, GX_PIXELMAP_ID_BT_ICON_BACK_WHITE_60X60, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
			else
				gx_pixelmap_prompt_pixelmap_set (&g_BT_ScreenInfo[index].m_PixelPromptWidget, GX_ID_NONE, GX_ID_NONE, g_BT_ScreenInfo[index].m_DeviceSettings->m_BT_Icon_Normal, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
		}
	}
}

//*************************************************************************************
// Function Name: User BluetoothList_callback
//
// Description: This function handles drawing and processing the List Items.
//
//*************************************************************************************

VOID UserBluetoothList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
    BLUETOOTH_SCREEN_DATA *feature = (BLUETOOTH_SCREEN_DATA *)widget;
    GX_BOOL result;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
    if (!result)
    {
		// Create the list item, this creates the space for the List Item.
        gx_utility_rectangle_define(&childsize, 0, 0, 270, 68);	// Left, Top, Right, Botton; 0,0,x,75 is too high
        gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /* | GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

		// Create the Prompt that contains the String Description plus the Icon.
		gx_utility_rectangle_define(&childsize, (GX_VALUE)(feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2), 4, (GX_VALUE)(feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 24), (GX_VALUE)(feature->m_ItemWidget.gx_widget_size.gx_rectangle_bottom - 4));
		gx_pixelmap_prompt_create (&feature->m_PixelPromptWidget, "PixelPrompt", &feature->m_ItemWidget, GX_ID_NONE, GX_ID_NONE,
		        /*GX_STYLE_BORDER_THIN | */GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT, (USHORT) (BUTTON_ID_BASE + index), &childsize);

		gx_prompt_font_set ((GX_PROMPT*) &feature->m_PixelPromptWidget, GX_FONT_ID_ASC24PT);
		// Put the text into the button and into the screen structure.
		if (index == 0)
		{
			gx_prompt_text_id_set ((GX_PROMPT *) &feature->m_PixelPromptWidget, GX_STRING_ID_BACK);
			gx_pixelmap_prompt_pixelmap_set (&feature->m_PixelPromptWidget, GX_ID_NONE, GX_ID_NONE, GX_PIXELMAP_ID_DEG_ARROW090_30X30, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
		}
		else
		{
			gx_prompt_text_id_set ((GX_PROMPT *) &feature->m_PixelPromptWidget, g_BT_ScreenInfo[index].m_DeviceSettings->m_DescriptionID);
			gx_pixelmap_prompt_pixelmap_set (&feature->m_PixelPromptWidget, GX_ID_NONE, GX_ID_NONE, g_BT_ScreenInfo[index].m_DeviceSettings->m_BT_Icon_Selected, GX_ID_NONE, GX_ID_NONE, GX_ID_NONE);
		}
	}

}


//*************************************************************************************
// Function Name: UserBTDeviceSelectionScreen_event_handler
//*************************************************************************************

UINT ION_BT_UserSelectionScreen_event_handler (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	ION_BT_USERSELECTIONSCREEN_CONTROL_BLOCK *UserBluetoothWindowPtr = (ION_BT_USERSELECTIONSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
	    // Words of Wisdom:
	    // Do not attempt to clean up the creation of widgets here.
	    // It causes bad things to happen. I'm guessing that the GUIX library does NOT
	    // actually perform 'deleting widgets' until the call at the end of this
	    // event handler is executed. Thus, it really doesn't happen and it
	    // does bad creation which are not captured.
	    // If you need to clean up (delete) created widgets, do it as this
	    // screen is being dismissed or toggled to a different screen.
	    //BT_Screen_Widget_Cleanup(g_BT_ScreenInfo, MAX_BLUETOOTH_DEVICES);

	    // This functions sets up the "container" for the widgets that make up the Bluetooth Device list for the user.
		InitializeUserBluetoothDeviceInformation();
		// Create the List and populate the items with Text, Color and Icons.
		CreateUserBluetoothWidgets (&UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox);
		g_Selected_Button_Index = 0;	// Start with the "BACK" item.
		UpdateBTUserSelection();		// Adjust the items colors, font and icons.
		gx_vertical_list_selected_set (&UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox, g_Selected_Button_Index);
        g_ChangeScreen_WIP = false;
        g_ActiveScreen = (GX_WIDGET*) window;
		break;

	case GX_SIGNAL (BT_SUBMENU_CHANGED_ID, GX_EVENT_CLICKED): // This event is triggered by a change in the Bluetooth SubIndex message from ION Hub
	    if (g_BluetoothSubIndex == 0x00)        // "00" = Main Menu with Bluetooth feature selected.
	    {
//	        BT_Screen_Widget_Cleanup(g_BT_ScreenInfo, MAX_BLUETOOTH_DEVICES);
	        screen_toggle((GX_WINDOW *)&MainUserScreen, window);
	    }
        else if (g_BluetoothSubIndex == 0x01)   // Are we back at "BACK"?
        {
            g_Selected_Button_Index = 0;
            UpdateBTUserSelection();
            gx_vertical_list_selected_set (&UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox, g_Selected_Button_Index);
        }
	    else if ((g_BluetoothSubIndex & 0x01) == 0x01)  // Are we trying to connect?
	    {
//	        BT_Screen_Widget_Cleanup(g_BT_ScreenInfo, MAX_BLUETOOTH_DEVICES);
	        g_SelectedBluetoothDeviceIndex = (g_BluetoothSubIndex >> 4) - 1;  // The device is in the upper nibble and make 0-based
	        screen_toggle((GX_WINDOW *)&ION_BT_ActiveScreen, window);
	    }
	    else
	    {
	        g_Selected_Button_Index = g_BluetoothSubIndex >> 4;      // The 1-based index is in the upper nibble
	        if (g_Selected_Button_Index >= MAX_BLUETOOTH_DEVICES)
	            g_Selected_Button_Index = 0;
	        UpdateBTUserSelection();
	        gx_vertical_list_selected_set (&UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox, g_Selected_Button_Index);
	    }
	    break;

    case GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
		// Select the next item, rolling to the first item.
//		++g_Selected_Button_Index;
//		if (g_Selected_Button_Index >= g_NumberOfPairedDevices)
//			g_Selected_Button_Index = 0;
//		UpdateBTUserSelection();
//		gx_vertical_list_selected_set (&UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox, g_Selected_Button_Index);
        break;

    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
		// Select the Previous Item, rolling to the last item in the list.
 		break;

//	case GX_EVENT_PEN_DOWN:	// We are going to determine if the Up or Down arrow buttons have been held for a
//							// ... long time (2 seconds) and goto advancing the next item in the User's Bluetooth List.
//
//		if (event_ptr->gx_event_target->gx_widget_id == DRIVE_CONTROL_USERPORT_ID)
//		{
//			gx_system_timer_start(window, USER_PORT_TIMER_ID, g_TimeoutValue * 4, 0);	// "x4" seems to make the time real.
//		}
//		break;

//	case GX_EVENT_PEN_UP:
//		gx_system_timer_stop(window, USER_PORT_TIMER_ID);
//		if (g_ChangeScreen_WIP)
//		{
//			g_ChangeScreen_WIP = false;
//			break;
//		}
//		if (event_ptr->gx_event_target->gx_widget_id == DRIVE_CONTROL_USERPORT_ID)
//		{
//			// Return back tot he previous screen.
//			if (g_Selected_Button_Index == 0)
//			{
//				for (i = 0; i < MAX_BLUETOOTH_DEVICES + 1; ++i)
//				{
//					if (&g_BT_ScreenInfo[i].m_ButtonWidget != NULL)
//						gx_widget_delete ((GX_WIDGET*) &g_BT_ScreenInfo[i].m_ButtonWidget);
//					if (&g_BT_ScreenInfo[i].m_ItemWidget != NULL)
//						gx_widget_delete ((GX_WIDGET*) &g_BT_ScreenInfo[i].m_ItemWidget);
//				}
//				UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox.gx_vertical_list_child_count = 0;
//				screen_toggle((GX_WINDOW *)&MainUserScreen, window);
//			}
//			else
//			{
//				g_SelectedBluetoothDeviceIndex = g_Selected_Button_Index - 1;
//				screen_toggle((GX_WINDOW *)&ION_BT_ActiveScreen, window);
//			}
//		}
//		break;

//    case GX_EVENT_TIMER:
//		// This is initiated by a PEN_DOWN process of the USER PORT BUTTON.
//		//gx_button_deselect ((GX_BUTTON*) &MainUserScreen.base.PrimaryTemplate_UpArrowButton, GX_EVENT_TOGGLE_OFF);
//		//gx_button_deselect ((GX_BUTTON*) &MainUserScreen.base.PrimaryTemplate_DownArrowButton, GX_EVENT_TOGGLE_OFF);
//		g_ChangeScreen_WIP = true;	// Need to suppress PEN_UP Handling.
//		if (event_ptr->gx_event_payload.gx_event_timer_id == USER_PORT_TIMER_ID)
//		{
//			++g_Selected_Button_Index;
//			if (g_Selected_Button_Index >= g_NumberOfPairedDevices)
//				g_Selected_Button_Index = 0;
//			UpdateBTUserSelection();
//			gx_vertical_list_selected_set (&UserBluetoothWindowPtr->ION_BT_UserSelectionScreen_BluetoothDeviceListBox, g_Selected_Button_Index);
//		}
//		break;

	} // end switch on event_ptr

	gx_window_event_process(window, event_ptr);
	
	return GX_SUCCESS;
}



