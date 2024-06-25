//*****************************************************************************
// Filename: ION_SIPNPUFF_ProgrammingScreen.cpp
// Description: This file supports the ION Programming Main Screen.
//
// Date: Oct 16, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "DeviceInfo.h"
#include "ScreenSupport.h"
#include "QueueDefinition.h"

//*************************************************************************************
//	- Sip-N-Puff
//		- Enable
//		- Calibrate
//		- Mode Port
//		- Diagnostics?

//*************************************************************************************
// Defines, Macros
//*************************************************************************************

#define ION_SIPNPUFF_PROGRAMMING_BUTTON_ID_BASE (100)
typedef enum {ENABLE_BTN, CALIBRATE_BTN, DIRECTIONS_BTN, DIAGNOSTIC_BTN, INVALID_BTN} SNP_BUTTON_ENUM;

//*************************************************************************************
// Forward declarations
//*************************************************************************************

void PopulateION_SIPNPUFF_ProgrammingInfo (void);
void Create_ION_SIPNPUFF_ProgramminghWidgets (GX_VERTICAL_LIST *list);
VOID ION_SIPNPUFF_ProgrammingList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index);

//*************************************************************************************
// Initialize ION Programming Hierarchy.
//*************************************************************************************

void PopulateION_SIPNPUFF_ProgrammingInfo (void)
{
    CleanupInfoStruct(&g_ProgrammingScreenInfoStruct[0], &ION_SIPnPuffProgrammingScreen.ION_SIPnPuffProgrammingScreen_ListBox);
	// Enable button
	g_ProgrammingScreenInfoStruct[ENABLE_BTN].m_Enabled = true;
	g_ProgrammingScreenInfoStruct[ENABLE_BTN].m_LargeDescriptionID = GX_STRING_ID_ENABLE;
	// CALIBRATE
	g_ProgrammingScreenInfoStruct[CALIBRATE_BTN].m_Enabled = true;
	g_ProgrammingScreenInfoStruct[CALIBRATE_BTN].m_LargeDescriptionID = GX_STRING_ID_CALIBRATE;
	// DIRECTIONS
	g_ProgrammingScreenInfoStruct[DIRECTIONS_BTN].m_Enabled = true;
	g_ProgrammingScreenInfoStruct[DIRECTIONS_BTN].m_LargeDescriptionID = GX_STRING_ID_SET_DIRECTION;
	// DIAGNOSTICS
	g_ProgrammingScreenInfoStruct[DIAGNOSTIC_BTN].m_Enabled = true;
	g_ProgrammingScreenInfoStruct[DIAGNOSTIC_BTN].m_LargeDescriptionID = GX_STRING_ID_DIAGNOSTICS;
}

//*************************************************************************************
// This function creates and populates the ION Main Programming List.
//*************************************************************************************

void Create_ION_SIPNPUFF_ProgramminghWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < MAX_PROGRAMMING_SCREEN_STRUCTURES; ++index)
	{
		if (g_ProgrammingScreenInfoStruct[index].m_Enabled)
		{
			ION_SIPNPUFF_ProgrammingList_callback (list, (GX_WIDGET*) &g_ProgrammingScreenInfoStruct[index], index);
			++activeFeatureCount;
		}
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;

}

//*************************************************************************************
// This function handles drawing and processing the List Items.
//*************************************************************************************

VOID ION_SIPNPUFF_ProgrammingList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
	PROGRAMMING_SCREEN_INFO*feature = (PROGRAMMING_SCREEN_INFO*)widget;
    GX_BOOL result;
	int SNP_Enabled;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
    if (!result)
    {
		if (index == ENABLE_BTN)	// Draw the ENABLE toggle button
		{
			// left, top, right, bottom
	        gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
			gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT | GX_STYLE_BORDER_NONE /* | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

			childsize.gx_rectangle_left = (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 74); // This locates the Checkbox to the left of the Scroll bar but inside the List box.
			childsize.gx_rectangle_top = (68-33)/2; // "68" is the widget hieght, "33" is the Background height
			childsize.gx_rectangle_right = (GX_VALUE) (childsize.gx_rectangle_left + 58);	// "58" is width of background pic
			childsize.gx_rectangle_bottom = (GX_VALUE) (childsize.gx_rectangle_top + 33); // 47;
			SNP_Enabled = gp_ProgrammingDevice->m_Enabled;
			custom_checkbox_create(&feature->m_Checkbox, &feature->m_ItemWidget, &DefaultCheckboxInfo, &childsize, SNP_Enabled);

			gx_utility_rectangle_define(&childsize, 0, 0, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 74 - 14), 68);
			gx_prompt_create(&feature->m_PromptWidget, NULL, &feature->m_ItemWidget, 0, GX_STYLE_TRANSPARENT | GX_STYLE_BORDER_NONE | GX_STYLE_ENABLED, 0, &childsize);
			//gx_widget_fill_color_set(&feature->m_PromptWidget, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY);
			gx_prompt_text_color_set(&feature->m_PromptWidget, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
			gx_prompt_text_id_set(&feature->m_PromptWidget, feature->m_LargeDescriptionID);
		}
		else // It must CALIBRATE, DIRECTION or DIAGNOSTIC button to create
		{
			gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
			gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /*| GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

			gx_utility_rectangle_define(&childsize, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2), 2, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14), 62);

			// Create a button and set colors
			gx_text_button_create (&feature->m_ButtonWidget, "MYBUTTON", &feature->m_ItemWidget, feature->m_LargeDescriptionID, GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, (USHORT) (ION_SIPNPUFF_PROGRAMMING_BUTTON_ID_BASE + index), &childsize);
			feature->m_ButtonWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_TEXT;
			feature->m_ButtonWidget.gx_widget_normal_fill_color = GX_COLOR_ID_TEXT_INPUT_FILL;
			feature->m_ButtonWidget.gx_widget_selected_fill_color = GX_COLOR_ID_TEXT_INPUT_TEXT;
		}
	}

}

//*************************************************************************************
// Event processing of ION Main Programmng Screen.
//*************************************************************************************

UINT ION_SIPNPUFF_ProgrammingScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	UINT myErr = GX_SUCCESS;
    ION_SIPNPUFFPROGRAMMINGSCREEN_CONTROL_BLOCK *SNP_Window = (ION_SIPNPUFFPROGRAMMINGSCREEN_CONTROL_BLOCK *)window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		PopulateION_SIPNPUFF_ProgrammingInfo();
		Create_ION_SIPNPUFF_ProgramminghWidgets(&ION_SIPnPuffProgrammingScreen.ION_SIPnPuffProgrammingScreen_ListBox);
		if (SNP_Window->ION_SIPnPuffProgrammingScreen_ListBox.gx_vertical_list_total_rows <= 3)
			gx_widget_hide ((GX_WIDGET*) &SNP_Window->ION_SIPnPuffProgrammingScreen_Vertical_scroll);
		if (gp_ProgrammingDevice->m_DriverConfiguration == SIP_N_PUFF_DEVICE_IDX)
			gx_prompt_text_id_set(&SNP_Window->ION_SIPnPuffProgrammingScreen_DriverName_Prompt, GX_STRING_ID_SIP_N_PUFF);
		else if (gp_ProgrammingDevice->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
			gx_prompt_text_id_set(&SNP_Window->ION_SIPnPuffProgrammingScreen_DriverName_Prompt, GX_STRING_ID_SNP_HEAD_ARRAY);
		break;

	case GX_SIGNAL(ION_SIPNPUFF_PROGRAMMING_BUTTON_ID_BASE + CALIBRATE_BTN, GX_EVENT_CLICKED):
//		PushWindow (window);
//        screen_toggle((GX_WINDOW *)&ION_SNP_Calibrate_Screen, window);
		break;

	case GX_SIGNAL(ION_SIPNPUFF_PROGRAMMING_BUTTON_ID_BASE + DIRECTIONS_BTN, GX_EVENT_CLICKED):
		PushWindow (window);
        screen_toggle((GX_WINDOW *)&ION_PadDirectionScreen, window);
		break;

	case GX_SIGNAL (ION_SIPNPUFF_PROGRAMMING_BUTTON_ID_BASE + DIAGNOSTIC_BTN, GX_EVENT_CLICKED):
//		PushWindow (window);
//		if (gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_4_QUADRANT)
//			screen_toggle((GX_WINDOW *)&Diag_4Quad_Screen, window);
//		else
//			screen_toggle((GX_WINDOW *)&DiagnosticScreen, window);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
		CleanupInfoStruct(&g_ProgrammingScreenInfoStruct[0], &SNP_Window->ION_SIPnPuffProgrammingScreen_ListBox);
		if (g_ProgrammingScreenInfoStruct[0].m_Checkbox.gx_widget_style & GX_STYLE_BUTTON_PUSHED)
			gp_ProgrammingDevice->m_Enabled = ENABLED;
		else
			gp_ProgrammingDevice->m_Enabled = DISABLED;
	    SendDriverEnable (gp_ProgrammingDevice->m_DriverConfiguration, gp_ProgrammingDevice->m_Enabled);    // Send to HUB
		screen_toggle(PopPushedWindow(), window);
		break;
	}

    myErr = gx_window_event_process(window, event_ptr);

	return myErr;
}

