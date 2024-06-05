//*****************************************************************************
// Filename: ION_DRiverControlProgrammingScreen.cpp
// Description: This file supports the ION Driver Control Programming Main Screen.
//
// Date: July 26, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "ScreenSupport.h"
#include "QueueDefinition.h"

//*************************************************************************************

//*************************************************************************************
// Defines
//*************************************************************************************

typedef enum {ENABLE_BTN_ID = 120, PAD_DIRECTION_BTN_ID, DRV_MODE_BTN_ID, DIAGNOSTIC_BTN_ID} BTN_ENUM;

#define ION_DEVICE_PROGRAMMING_ITEMS_MAX (8)	// Max number of items in the ION Main Programming List

PROGRAMMING_SCREEN_INFO ION_Device_ProgramSettings_ScreenInfo[ION_DEVICE_PROGRAMMING_ITEMS_MAX];

//*************************************************************************************
// Local file variables
//*************************************************************************************

static char g_ModeSettingStr[32] = "MODE";

//*************************************************************************************
// Forward declarations
//*************************************************************************************

void Create_ION_Device_ProgrammingWidgets (GX_VERTICAL_LIST *list);
void PopulateION_Device_ProgrammingInfo (void);
VOID ION_Device_ProgrammingList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index);
UINT ION_DriverControlProgrammingScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);

//*************************************************************************************
// Initialize ION Device Programming Hierarchy.
//*************************************************************************************

void PopulateION_Device_ProgrammingInfo (void)
{
	int i;

	for (i=0; i < ION_DEVICE_PROGRAMMING_ITEMS_MAX; ++i)
	{
		ION_Device_ProgramSettings_ScreenInfo[i].m_Enabled = false;
		ION_Device_ProgramSettings_ScreenInfo[i].m_LargeDescriptionID = GX_STRING_ID_BLANK;
	}

	// Enable button
	ION_Device_ProgramSettings_ScreenInfo[0].m_Enabled = true;
	ION_Device_ProgramSettings_ScreenInfo[0].m_LargeDescriptionID = GX_STRING_ID_ENABLE;
	// DIRECTIONS
	ION_Device_ProgramSettings_ScreenInfo[1].m_Enabled = true;
	ION_Device_ProgramSettings_ScreenInfo[1].m_LargeDescriptionID = GX_STRING_ID_SET_DIRECTION;
	// MODE
	ION_Device_ProgramSettings_ScreenInfo[2].m_Enabled = true;
	ION_Device_ProgramSettings_ScreenInfo[2].m_LargeDescriptionID = GX_STRING_ID_BLANK;
	// DIAGNOSTICS
	ION_Device_ProgramSettings_ScreenInfo[3].m_Enabled = true;
	ION_Device_ProgramSettings_ScreenInfo[3].m_LargeDescriptionID = GX_STRING_ID_DIAGNOSTICS;
}

//*************************************************************************************
// This function creates and populates the ION Main Programming List.
//*************************************************************************************

void Create_ION_Device_ProgrammingWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < ION_DEVICE_PROGRAMMING_ITEMS_MAX; ++index)
	{
		if (ION_Device_ProgramSettings_ScreenInfo[index].m_Enabled)
		{
			ION_Device_ProgrammingList_callback (list, (GX_WIDGET*) &ION_Device_ProgramSettings_ScreenInfo[index], index);
			++activeFeatureCount;
		}
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;

}

//*************************************************************************************
// This function handles drawing and processing the List Items.
//*************************************************************************************

VOID ION_Device_ProgrammingList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
	PROGRAMMING_SCREEN_INFO*feature = (PROGRAMMING_SCREEN_INFO*)widget;
    GX_BOOL result;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
    if (!result)
    {
		if (index == 0)	// This is the Enable Checkbox
		{
			// left, top, right, bottom
	        gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
			//gx_utility_rectangle_define(&childsize, 0, 0, 215, 52);	// 0,0,x,75 is too high
			gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT | GX_STYLE_BORDER_NONE /* | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

			childsize.gx_rectangle_left = (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 74); // This locates the Checkbox to the left of the Scroll bar but inside the List box.
			childsize.gx_rectangle_top = (68-33)/2; // "68" is the widget hieght, "33" is the Background height
			childsize.gx_rectangle_right = (GX_VALUE) (childsize.gx_rectangle_left + 58);	// "58" is width of background pic
			childsize.gx_rectangle_bottom = (GX_VALUE) (childsize.gx_rectangle_top + 33); // 47;
			custom_checkbox_create(&feature->m_Checkbox, &feature->m_ItemWidget, &DefaultCheckboxInfo, &childsize, gp_ProgrammingDevice->m_Enabled);

			gx_utility_rectangle_define(&childsize, 0, 0, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 74 - 14), 68);
			gx_prompt_create(&feature->m_PromptWidget, NULL, &feature->m_ItemWidget, 0, GX_STYLE_TRANSPARENT | GX_STYLE_BORDER_NONE | GX_STYLE_ENABLED, 0, &childsize);
			//gx_widget_fill_color_set(&feature->m_PromptWidget, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY);
			gx_prompt_text_color_set(&feature->m_PromptWidget, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE); // , GX_COLOR_ID_WHITE);
			gx_prompt_text_id_set(&feature->m_PromptWidget, feature->m_LargeDescriptionID);
		}
		else if (index == 2)	// This is the Mode Port setting
		{
			gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
			gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /*| GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

			gx_utility_rectangle_define(&childsize, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2), 2, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14), 62);

			// Create a multiline text button and set colors
			gx_multi_line_text_button_create (&feature->m_MultiLineButtonWidget, "MYBUTTON", &feature->m_ItemWidget, feature->m_LargeDescriptionID, GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, (USHORT) (ENABLE_BTN_ID + index), &childsize);
			feature->m_MultiLineButtonWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_TEXT;
			feature->m_MultiLineButtonWidget.gx_widget_normal_fill_color = GX_COLOR_ID_TEXT_INPUT_FILL;
			feature->m_MultiLineButtonWidget.gx_widget_selected_fill_color = GX_COLOR_ID_TEXT_INPUT_TEXT;
		}
		else // This the Diagnostic Button.
		{
			gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
			gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /*| GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

			gx_utility_rectangle_define(&childsize, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2), 2, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14), 62);

			// Create a button and set colors
			gx_text_button_create (&feature->m_ButtonWidget, "MYBUTTON", &feature->m_ItemWidget, feature->m_LargeDescriptionID, GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, (USHORT) (ENABLE_BTN_ID + index), &childsize);
			feature->m_ButtonWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_TEXT;
			feature->m_ButtonWidget.gx_widget_normal_fill_color = GX_COLOR_ID_TEXT_INPUT_FILL;
			feature->m_ButtonWidget.gx_widget_selected_fill_color = GX_COLOR_ID_TEXT_INPUT_TEXT;
		}
	}
}

//*************************************************************************************
// This function sets the text in the button. This must use the Global Variable
// to strore the string.
//*************************************************************************************

static void setModeButtonString (GX_MULTI_LINE_TEXT_BUTTON *button, int setting)
{
	GX_CONST GX_CHAR *modeID_Ptr, *modeSettingID_Ptr;
	int targetStrID;

	switch (setting)
	{
	case DRV_MODE_SWITCH_PIN5:
		targetStrID = GX_STRING_ID_MODE_RESET;
		break;
	case DRV_MODE_SWITCH_REVERSE:
		targetStrID = GX_STRING_ID_DIR_REVERSE;
		break;
	case DRV_MODE_SWITCH_FORWARD:
		targetStrID = GX_STRING_ID_DIR_FORWARD;
		break;
	case DRV_MODE_SWITCH_LEFT:
		targetStrID = GX_STRING_ID_DIR_LEFT;
		break;
	case DRV_MODE_SWITCH_RIGHT:
		targetStrID = GX_STRING_ID_DIR_RIGHT;
		break;
	case DRV_MODE_USER_PORT:
		targetStrID = GX_STRING_ID_USER_PORT;
		break;
	case DRV_MODE_SWITCH_SINGLE_TAP:
		targetStrID = GX_STRING_ID_SINGLE_TAP;
		break;
	case DRV_MODE_SWITCH_DOUBLE_TAP:
		targetStrID = GX_STRING_ID_DOUBLE_TAP;
		break;
	default:
		targetStrID = GX_STRING_ID_NONE;
		break;
	} // end switch
	gx_system_string_get (GX_STRING_ID_MODE_PORT, &modeID_Ptr);
	gx_system_string_get ((GX_RESOURCE_ID) (targetStrID), &modeSettingID_Ptr);
	sprintf (g_ModeSettingStr, "%s\r<%s>", modeID_Ptr, modeSettingID_Ptr);
	gx_multi_line_text_button_text_set (button, g_ModeSettingStr);
}


//*************************************************************************************
// Function Name: ION_DriverControlProgrammingScreen_event_process
//
// Description: This handles the Startup Screen messages
//
//*************************************************************************************

UINT ION_DriverControlProgrammingScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	UINT myErr = GX_SUCCESS;
	ION_DRIVERCONTROLPROGRAMMINGSCREEN_CONTROL_BLOCK *windowPtr = (ION_DRIVERCONTROLPROGRAMMINGSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		// Display the Device's Name
		gx_prompt_text_id_set (&windowPtr->ION_DriverControlProgrammingScreen_DriverName_Prompt, gp_ProgrammingDevice->m_DeviceNameStringID);
        CleanupInfoStruct(&ION_Device_ProgramSettings_ScreenInfo[0], &ION_DriverControlProgrammingScreen.ION_DriverControlProgrammingScreen_ListBox, ION_DEVICE_PROGRAMMING_ITEMS_MAX);
		// Set up the Buttons.
		PopulateION_Device_ProgrammingInfo();
		Create_ION_Device_ProgrammingWidgets(&ION_DriverControlProgrammingScreen.ION_DriverControlProgrammingScreen_ListBox);
		// Show or Hide the List Box scrool Button.
		if (windowPtr->ION_DriverControlProgrammingScreen_ListBox.gx_vertical_list_total_rows <= 3)
			gx_widget_hide ((GX_WIDGET*) &windowPtr->ION_DriverControlProgrammingScreen_Vertical_scroll);
		// Populate the Mode Button verbiage
		setModeButtonString (&ION_Device_ProgramSettings_ScreenInfo[2].m_MultiLineButtonWidget, gp_ProgrammingDevice->m_Mode_Switch_Schema);
		break;

	case GX_SIGNAL(PAD_DIRECTION_BTN_ID, GX_EVENT_CLICKED):	// This is PAD DIRECTION.
//		PushWindow (window);
//        screen_toggle((GX_WINDOW *)&ION_PadDirectionScreen, window);
		break;

	case GX_SIGNAL(DRV_MODE_BTN_ID, GX_EVENT_CLICKED):	// This is USER PORT button
		++gp_ProgrammingDevice->m_Mode_Switch_Schema;
		if (gp_ProgrammingDevice->m_Mode_Switch_Schema >= DRV_MODE_SWITCH_END)
			gp_ProgrammingDevice->m_Mode_Switch_Schema = (DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM) 0;

		setModeButtonString (&ION_Device_ProgramSettings_ScreenInfo[2].m_MultiLineButtonWidget, gp_ProgrammingDevice->m_Mode_Switch_Schema);
		break;

	case GX_SIGNAL (DIAGNOSTIC_BTN_ID, GX_EVENT_CLICKED):
//		PushWindow (window);
//		if (gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_2_QUADRANT)
//			screen_toggle((GX_WINDOW *)&Diag_4Quad_Screen, window);
//		else if (gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_4_QUADRANT)
//			screen_toggle((GX_WINDOW*)&Diag_4Quad_Screen, window);
//		else
//			screen_toggle((GX_WINDOW *)&DiagnosticScreen, window);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
		if (ION_Device_ProgramSettings_ScreenInfo[0].m_Checkbox.gx_widget_style & GX_STYLE_BUTTON_PUSHED)
			gp_ProgrammingDevice->m_Enabled = ENABLED;
		else
			gp_ProgrammingDevice->m_Enabled = DISABLED;
	    SendDriverEnable (gp_ProgrammingDevice->m_DriverConfiguration, gp_ProgrammingDevice->m_Enabled);    // Send to HUB
		CleanupInfoStruct(&ION_Device_ProgramSettings_ScreenInfo[0], &ION_DriverControlProgrammingScreen.ION_DriverControlProgrammingScreen_ListBox, ION_DEVICE_PROGRAMMING_ITEMS_MAX);
        screen_toggle(PopPushedWindow(), window);
		break;
	}

	myErr = gx_window_event_process(window, event_ptr);

	return myErr;
}

