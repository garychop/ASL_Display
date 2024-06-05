//*****************************************************************************
// Filename: ION_DriverSelectScreen.c
// Description: This file supports the ION Driver Select Screen.
//
// Date: Feb 1, 2024
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "ScreenSupport.h"

//*************************************************************************************
// Defines, Macros
//*************************************************************************************

typedef enum ION_PROG_INDEX {QUAD3_BTN = 2000, QUAD_4_BTN, SIP_N_PUFF_BTN, TWO_SWITCH_BTN, SNP_HEAD_ARRAY_BTN, HUB_BTN, ATTENDANT_BTN, SOUND_BTN, USER_SETTINGS_BTN, FEATURE_LIST_BTN, STANDBY_SELECT_BTN, RESET_BTN, INVALID_BTN} MAIN_PROGRAMMING_IDX;
typedef enum {QUAD3_IDX, QUAD_4_IDX, SIP_N_PUFF_IDX, TWO_SWITCH_IDX, SNP_HEAD_ARRAY_IDX, HUB_IDX, ATTENDANT_IDX, SOUND_IDX, USER_SETTINGS_IDX, FEATURE_LIST_IDX, STANDBY_SELECT_IDX, RESET_IDX, INVALID_IDX} PROGRAMMING_DEVICE_INDEX;
#define ION_DRIVER_CONTROL_MAX (8)	// Max number of items in the ION Main Programming List

PROGRAMMING_SCREEN_INFO ION_DriverSelect_ScreenInfo[ION_DRIVER_CONTROL_MAX];

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

static void PopulateIONDriverSelectInfo(void);
static VOID ION_DriverSelectList_callback(GX_VERTICAL_LIST* list, GX_WIDGET* widget, INT index);
static void Create_ION_DriverSelectWidgets(GX_VERTICAL_LIST* list);

//*************************************************************************************
// Initialize ION Programming Hierarchy.
//*************************************************************************************

static void PopulateIONDriverSelectInfo (void)
{
	int i;

	for (i=0; i < ION_DRIVER_CONTROL_MAX; ++i)
	{
		ION_DriverSelect_ScreenInfo[i].m_Enabled = false;
		ION_DriverSelect_ScreenInfo[i].m_LargeDescriptionID = GX_STRING_ID_BLANK;
	}

	// Driver Control
	ION_DriverSelect_ScreenInfo[QUAD3_IDX].m_Enabled = true;
	ION_DriverSelect_ScreenInfo[QUAD3_IDX].m_LargeDescriptionID = GX_STRING_ID_QUADRANT_3;
	// Driver Control
	ION_DriverSelect_ScreenInfo[QUAD_4_IDX].m_Enabled = true;
	ION_DriverSelect_ScreenInfo[QUAD_4_IDX].m_LargeDescriptionID = GX_STRING_ID_QUADRANT_4;
	// SIP-N-PUFF
	ION_DriverSelect_ScreenInfo[SIP_N_PUFF_IDX].m_Enabled = true;
	ION_DriverSelect_ScreenInfo[SIP_N_PUFF_IDX].m_LargeDescriptionID = GX_STRING_ID_SIP_N_PUFF;
	// SNP Head Array
	ION_DriverSelect_ScreenInfo[SNP_HEAD_ARRAY_IDX].m_Enabled = true;
	ION_DriverSelect_ScreenInfo[SNP_HEAD_ARRAY_IDX].m_LargeDescriptionID = GX_STRING_ID_SNP_HEAD_ARRAY;
	// 2-SWITCH
	ION_DriverSelect_ScreenInfo[TWO_SWITCH_IDX].m_Enabled = true;
	ION_DriverSelect_ScreenInfo[TWO_SWITCH_IDX].m_LargeDescriptionID = GX_STRING_ID_TWO_SWITCH;
}

//*************************************************************************************
// Function Name: User ION_MainProgrammingList_callback
//
// Description: This function handles drawing and processing the List Items.
//
//*************************************************************************************

static VOID ION_DriverSelectList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
	PROGRAMMING_SCREEN_INFO*feature = (PROGRAMMING_SCREEN_INFO*)widget;
    GX_BOOL result;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
    if (!result)
    {
        gx_utility_rectangle_define(&childsize, 0, 0, 204, 68);	// 0,0,x,75 is too high
        //ASFOUND gx_utility_rectangle_define(&childsize, 0, 0, 215, 60);	// 0,0,x,75 is too high
        gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /*| GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

        //gx_utility_rectangle_define(&childsize, 0, 2, feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14, 62);
		//ASFOUND gx_utility_rectangle_define(&childsize, feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2, 4, feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14, 56);
		gx_utility_rectangle_define(&childsize, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2), 2, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 14), 62);

		// Create a button and set colors
		gx_text_button_create (&feature->m_ButtonWidget, "MYBUTTON", &feature->m_ItemWidget, feature->m_LargeDescriptionID, GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, (USHORT) (QUAD3_BTN + index), &childsize);
		feature->m_ButtonWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_TEXT;
		feature->m_ButtonWidget.gx_widget_normal_fill_color = GX_COLOR_ID_TEXT_INPUT_FILL;
		feature->m_ButtonWidget.gx_widget_selected_fill_color = GX_COLOR_ID_TEXT_INPUT_TEXT;
	}
}

//*************************************************************************************
// This function creates and populates the ION Main Programming List.
//*************************************************************************************

static void Create_ION_DriverSelectWidgets(GX_VERTICAL_LIST* list)
{
	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < ION_DRIVER_CONTROL_MAX; ++index)
	{
		if (ION_DriverSelect_ScreenInfo[index].m_Enabled)
		{
			ION_DriverSelectList_callback(list, (GX_WIDGET*)&ION_DriverSelect_ScreenInfo[index], index);
			++activeFeatureCount;
		}
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;
}

//*************************************************************************************
// Event processing of ION Main Programmng Screen.
//*************************************************************************************

UINT ION_DriverSelectScreen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
//	ION_DRIVERSELECTSCREEN_CONTROL_BLOCK *windowPtr = (ION_DRIVERSELECTSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		PopulateIONDriverSelectInfo();
		Create_ION_DriverSelectWidgets(&ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox);
		break;

	// Process the Head Array.
	case GX_SIGNAL(QUAD3_BTN, GX_EVENT_CLICKED):
		SetProgrammingDriverControl (&g_DeviceSettings[HEAD_ARRY_DEVICE_IDX]);
//		CleanupInfoStruct(&ION_DriverSelect_ScreenInfo[0], &ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox, ION_DRIVER_CONTROL_MAX);
		PushWindow (window);
        screen_toggle((GX_WINDOW *)&ION_DriverControlProgrammingScreen, window);
		break;

	// Process the 4-Quadrant Drive Control
	case GX_SIGNAL(QUAD_4_BTN, GX_EVENT_CLICKED):
		SetProgrammingDriverControl (&g_DeviceSettings[DRIVER_4_QUAD_IDX]);
//		CleanupInfoStruct(&ION_DriverSelect_ScreenInfo[0], &ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox, ION_DRIVER_CONTROL_MAX);
		PushWindow (window);
        screen_toggle((GX_WINDOW *)&ION_DriverControlProgrammingScreen, window);
		break;

	// Process the Sip-N-Puff button
	case GX_SIGNAL(SIP_N_PUFF_BTN, GX_EVENT_CLICKED):
		SetProgrammingDriverControl (&g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX]);
//		CleanupInfoStruct(&ION_DriverSelect_ScreenInfo[0], &ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox, ION_DRIVER_CONTROL_MAX);
		PushWindow (window);
        screen_toggle((GX_WINDOW *)&ION_SIPnPuffProgrammingScreen, window);
		break;

		// Process the 2-Switch button
	case GX_SIGNAL(TWO_SWITCH_BTN, GX_EVENT_CLICKED):
		SetProgrammingDriverControl(&g_DeviceSettings[TWO_SWITCH_IDX]);
//		CleanupInfoStruct(&ION_DriverSelect_ScreenInfo[0], &ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox, ION_DRIVER_CONTROL_MAX);
		PushWindow(window);
		screen_toggle((GX_WINDOW*)&ION_DriverControlProgrammingScreen, window);
		break;

		// Process the SNP Head Array button
	case GX_SIGNAL(SNP_HEAD_ARRAY_BTN, GX_EVENT_CLICKED):
//		SetProgrammingDriverControl(&g_DeviceSettings[SNP_HEAD_ARRAY_IDX]);
//		CleanupInfoStruct(&ION_DriverSelect_ScreenInfo[0], &ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox, ION_DRIVER_CONTROL_MAX);
		PushWindow(window);
		screen_toggle((GX_WINDOW*)&ION_SIPnPuffProgrammingScreen, window);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
//		CleanupInfoStruct(&ION_DriverSelect_ScreenInfo[0], &ION_DriverSelectScreen.ION_DriverSelectScreen_ListBox, ION_DRIVER_CONTROL_MAX);
		screen_toggle(PopPushedWindow(), window);
		break;
	}

    gx_window_event_process(window, event_ptr);

	return 0;
}

