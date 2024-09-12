//*****************************************************************************
//
// (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
//
// All rights reserved. This file is the intellectual property of ASL and it may
// not be disclosed to others or used for any purposes without the written consent of ASL.
//
//*****************************************************************************
// Filename: ION_MainProgrammingScreen.cpp
// Description: This file supports the ION Programming Main Screen.
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
// Defines, Macros
//*************************************************************************************

typedef enum ION_PROG_INDEX {DRIVER_SELECT_BTN = 20, HUB_BTN, ATTENDANT_BTN, SOUND_BTN, AUDIO_PHRASE_BTN, USER_SETTINGS_BTN, FEATURE_LIST_BTN, STANDBY_SELECT_BTN, RESET_BTN, INVALID_BTN} MAIN_PROGRAMMING_IDX;
typedef enum {DRIVER_SELECT_IDX, HUB_IDX, ATTENDANT_IDX, SOUND_IDX, AUDIO_PHRASE_IDX, USER_SETTINGS_IDX, FEATURE_LIST_IDX, STANDBY_SELECT_IDX, RESET_IDX, INVALID_IDX} PROGRAMMING_DEVICE_INDEX;

BUTTON_WIDGET_SCREEN_INFO ION_ProgramSettings_ScreenInfo[INVALID_IDX+1];

//*************************************************************************************
// Forward Declarations

static void PopulateIONProgrammingInfo (void);
static void Create_ION_ProgramminghWidgets (GX_VERTICAL_LIST *list);

//*************************************************************************************
// Initialize ION Programming Hierarchy.
//*************************************************************************************

static void PopulateIONProgrammingInfo (void)
{
    DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);

	// Driver Control Select Button
    ION_ProgramSettings_ScreenInfo[DRIVER_SELECT_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[DRIVER_SELECT_IDX].m_LargeDescriptionID = GX_STRING_ID_DRIVER_CONTROL;
	// Hub Ports
    ION_ProgramSettings_ScreenInfo[HUB_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[HUB_IDX].m_LargeDescriptionID = GX_STRING_ID_HUB;
	// Attendant Settings
    ION_ProgramSettings_ScreenInfo[ATTENDANT_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[ATTENDANT_IDX].m_LargeDescriptionID = GX_STRING_ID_ATTENDANT_SETTING;
	// Auditory Cues
    ION_ProgramSettings_ScreenInfo[SOUND_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[SOUND_IDX].m_LargeDescriptionID = GX_STRING_ID_AUDITORY_CUES;
    // Audio Phrase
    ION_ProgramSettings_ScreenInfo[AUDIO_PHRASE_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[AUDIO_PHRASE_IDX].m_LargeDescriptionID = GX_STRING_ID_TALK;
	// User Settings
    ION_ProgramSettings_ScreenInfo[USER_SETTINGS_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[USER_SETTINGS_IDX].m_LargeDescriptionID = GX_STRING_ID_USER_SETTINGS;
	// Standby Select Settings
    ION_ProgramSettings_ScreenInfo[STANDBY_SELECT_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[STANDBY_SELECT_IDX].m_LargeDescriptionID = GX_STRING_ID_STANDBY_SELECT;
	// Feature Settings
    ION_ProgramSettings_ScreenInfo[FEATURE_LIST_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[FEATURE_LIST_IDX].m_LargeDescriptionID = GX_STRING_ID_FEATURE_LIST;
	// Reset Settings
    ION_ProgramSettings_ScreenInfo[RESET_IDX].m_Enabled = true;
    ION_ProgramSettings_ScreenInfo[RESET_IDX].m_LargeDescriptionID = GX_STRING_ID_RESET_SETTINGS;
}

//*************************************************************************************
// This function creates and populates the ION Main Programming List.
//*************************************************************************************

static void Create_ION_ProgramminghWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < MAX_PROGRAMMING_SCREEN_STRUCTURES; ++index)
	{
		if (ION_ProgramSettings_ScreenInfo[index].m_Enabled)
		{
			ION_MainProgrammingList_callback (list, (GX_WIDGET*) &ION_ProgramSettings_ScreenInfo[index], index);
			++activeFeatureCount;
		}
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;
}

//*************************************************************************************
// Function Name: User ION_MainProgrammingList_callback
//
// Description: This function handles drawing and processing the List Items.
//
//*************************************************************************************

VOID ION_MainProgrammingList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
    BUTTON_WIDGET_SCREEN_INFO*feature = (BUTTON_WIDGET_SCREEN_INFO*)widget;
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
		gx_text_button_create (&feature->m_ButtonWidget, "MYBUTTON", &feature->m_ItemWidget, feature->m_LargeDescriptionID, GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED, (USHORT) (DRIVER_SELECT_BTN + index), &childsize);
		feature->m_ButtonWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_TEXT;
		feature->m_ButtonWidget.gx_widget_normal_fill_color = GX_COLOR_ID_TEXT_INPUT_FILL;
		feature->m_ButtonWidget.gx_widget_selected_fill_color = GX_COLOR_ID_TEXT_INPUT_TEXT;
	}
}

//*************************************************************************************
// Event processing of ION Main Programmng Screen.
//*************************************************************************************

UINT ION_MainProgrammingScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	ION_MAINPROGRAMMINGSCREEN_CONTROL_BLOCK *windowPtr = (ION_MAINPROGRAMMINGSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		PopulateIONProgrammingInfo();
		Create_ION_ProgramminghWidgets(&windowPtr->ION_MainProgrammingScreen_ION_MainProgrammingListBox);
		break;

	// Process the Head Array.
	case GX_SIGNAL(DRIVER_SELECT_BTN, GX_EVENT_CLICKED):
        DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		PushWindow (window);
		screen_toggle((GX_WINDOW *)&ION_DriverSelectScreen, window);
		break;

		// Do the HUB Port Setup stuff
	case GX_SIGNAL (HUB_BTN, GX_EVENT_CLICKED):
        DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		//PushWindow (window);
		//screen_toggle((GX_WINDOW *)&ION_HUB_Setup_Screen, window);
		break;

	// Do Attendant Setup stuff
	case GX_SIGNAL (ATTENDANT_BTN, GX_EVENT_CLICKED):
        //DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		//PushWindow (window);
		//screen_toggle((GX_WINDOW *)&ION_AttendantSetup_Screen, window);
		break;

	// Do Auditory (Sound) Setup stuff
	case GX_SIGNAL(SOUND_BTN, GX_EVENT_CLICKED):
        DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		PushWindow(window);
		screen_toggle((GX_WINDOW*)&ION_AuditorySettingsScreen, window);
		break;

    // Do "Say Something" Audio Phrase setup
    case GX_SIGNAL(AUDIO_PHRASE_BTN, GX_EVENT_CLICKED):
        DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
        PushWindow(window);
        screen_toggle((GX_WINDOW*)&ION_AudibleString_Settings_Screen, window);
        break;

	case GX_SIGNAL (USER_SETTINGS_BTN, GX_EVENT_CLICKED):
        //DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		//PushWindow (window);
		//screen_toggle((GX_WINDOW *)&UserSettingsScreen, window);
		break;

	case GX_SIGNAL (FEATURE_LIST_BTN, GX_EVENT_CLICKED):
        //DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		//PushWindow (window);
		//screen_toggle((GX_WINDOW *)&FeatureSettingsScreen, window);
		break;

	case GX_SIGNAL(STANDBY_SELECT_BTN, GX_EVENT_CLICKED):
        //DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
        //CleanupInfoStruct(&g_ProgrammingScreenInfoStruct[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox);
		//PushWindow(window);
		//screen_toggle((GX_WINDOW*)&StandbySelectSettings_Screen, window);
		break;

	case GX_SIGNAL (RESET_BTN, GX_EVENT_CLICKED):
        DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		PushWindow (window);
		screen_toggle((GX_WINDOW *)&ResetScreen, window);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
	    DeleteButtonScreenWidgets(&ION_ProgramSettings_ScreenInfo[0], &ION_MainProgrammingScreen.ION_MainProgrammingScreen_ION_MainProgrammingListBox, INVALID_IDX);
		screen_toggle(PopPushedWindow(), window);
		break;
	}

    gx_window_event_process(window, event_ptr);

	return GX_SUCCESS;
}

