/*
 * (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
 *
 * All rights reserved. This file is the intellectual property of ASL and it may
 * not be disclosed to others or used for any purposes without the written consent of ASL.
 *
*******************************************************************************
 * Filename: ION_AudibleUserStringSelectionScreen.cpp
 * Description: This file supports the AUdible String Selection Screen.
 *
 * Date: August 21, 2024
 *
 * Author: G. Chopcinski, Kg Solutions, LLC
 * 
 ******************************************************************************
*/

/*****************************************************************************
 * Includes
 */

#include "ASL165_System.h"
//#include "BluetoothDeviceInfo.h"
//#include <mmsystem.h>

/*************************************************************************************
* Defines and Macros
*/

// This structure is used by this screen.
typedef struct
{
	int m_AV_Index;
	GX_WIDGET m_ItemWidget;
	GX_PROMPT m_PromptWidget;
} AV_USER_SCREEN_INFO;

/*************************************************************************************
* Forward Declarations
*/

VOID AudibleUserSelectionString_List_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index);
//void SetNextSelection(ION_AUDIBLESTRINGSELECTIONSCREEN_CONTROL_BLOCK *windowPtr);

/*************************************************************************************
* Global and Local variables
*/

//static int g_Selected_Button_Index = 0;
static int g_NumberOfButtons;

// The following hold the Data for the User Audioble Phrase Screen.
AV_USER_SCREEN_INFO g_ScreenInfo[AUDIO_PHRASE_MAX_NUMBER];

/*************************************************************************************
 * Description: This function handles drawing and processing the List Items.
 */

VOID AudibleUserSelectionString_List_callback(GX_VERTICAL_LIST* list, GX_WIDGET* widget, INT index)
{
	GX_RECTANGLE childsize;
	AV_USER_SCREEN_INFO* feature = (AV_USER_SCREEN_INFO*)widget;
	GX_BOOL result;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
	if (!result)
	{
		// Create the list item, this creates the space for the List Item.
		gx_utility_rectangle_define(&childsize, 0, 0, 270, 68);	// Left, Top, Right, Botton; 0,0,x,75 is too high
		gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET*)list, GX_STYLE_TRANSPARENT /* | GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

		// Create the Prompt that contains the String Description plus the Icon.
		gx_utility_rectangle_define(&childsize, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_left + 2), 4, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 24), (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_bottom - 4));
		gx_prompt_create(&feature->m_PromptWidget, "Prompt", &feature->m_ItemWidget, GX_ID_NONE, 
			/*GX_STYLE_BORDER_THIN | */GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT, (USHORT) index, &childsize);
		
		gx_prompt_font_set(&feature->m_PromptWidget, GX_FONT_ID_ASC24PT);
		// Put the text into the button and into the screen structure.
		if (index == 0)
		{
			gx_prompt_text_id_set(&feature->m_PromptWidget, GX_STRING_ID_BACK);
		}
		else
		{
			gx_prompt_text_id_set(&feature->m_PromptWidget, g_AudioPhraseSettings[index - 1].m_Description); // "-1" because we put "BACK" in the first slot.
		}
	}

}


/*************************************************************************************
* This function creates and populates the Bluetooth List for the USER.
*/

static void CreateWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	activeFeatureCount = 0;
	for (index = 0; index < AUDIO_PHRASE_MAX_NUMBER; ++index)
	{
		if (index == 0)	// Index 0 is the "BACK" button.
		{
			g_ScreenInfo[index].m_AV_Index = 0;
			AudibleUserSelectionString_List_callback(list, (GX_WIDGET*) &g_ScreenInfo[activeFeatureCount], index);
			++activeFeatureCount;
		}
		else if (g_AudioPhraseSettings[index - 1].m_Enabled)
		{
			g_ScreenInfo[activeFeatureCount].m_AV_Index = index - 1;
			AudibleUserSelectionString_List_callback(list, (GX_WIDGET*) &g_ScreenInfo[activeFeatureCount], index);
			++activeFeatureCount;
		}
	}
	g_NumberOfButtons = activeFeatureCount;
	list->gx_vertical_list_total_rows = activeFeatureCount;
}

/*************************************************************************************
*
*/

static void UpdateSelection (void)
{
	int index;

	for (index = 0; index < AUDIO_PHRASE_MAX_NUMBER; ++index)
	{
		if ((g_SaySomething_HHP_Index-1) == index)
		{
			g_ScreenInfo[index].m_PromptWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_DISABLED_FILL;
			g_ScreenInfo[index].m_PromptWidget.gx_widget_normal_fill_color = GX_COLOR_ID_BTN_LOWER;
			g_ScreenInfo[index].m_PromptWidget.gx_widget_selected_fill_color = GX_COLOR_ID_SELECTED_FILL;
			g_ScreenInfo[index].m_PromptWidget.gx_prompt_disabled_text_color = GX_COLOR_ID_DISABLED_TEXT;
			g_ScreenInfo[index].m_PromptWidget.gx_prompt_normal_text_color = GX_COLOR_ID_TEXT;
			g_ScreenInfo[index].m_PromptWidget.gx_prompt_selected_text_color = GX_COLOR_ID_WHITE;
		}
		else
		{
			g_ScreenInfo[index].m_PromptWidget.gx_widget_disabled_fill_color = GX_COLOR_ID_SHINE;
			g_ScreenInfo[index].m_PromptWidget.gx_widget_normal_fill_color = GX_COLOR_ID_SHINE;
			g_ScreenInfo[index].m_PromptWidget.gx_widget_selected_fill_color = GX_COLOR_ID_SHINE;
			g_ScreenInfo[index].m_PromptWidget.gx_prompt_disabled_text_color = GX_COLOR_ID_DISABLED_TEXT;
			g_ScreenInfo[index].m_PromptWidget.gx_prompt_normal_text_color = GX_COLOR_ID_SELECTED_TEXT;
			g_ScreenInfo[index].m_PromptWidget.gx_prompt_selected_text_color = GX_COLOR_ID_SELECTED_TEXT;
		}
	}
}

/*************************************************************************************
* This function highligths the next selection in the list.
*/
//void SetNextSelection(ION_AUDIBLESTRINGSELECTIONSCREEN_CONTROL_BLOCK *windowPtr)
//{
//	int maxItems;
//	int selectedIndex;
//
//	maxItems = windowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox.gx_vertical_list_child_count;
//	selectedIndex = windowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox.gx_vertical_list_selected;
//	if (selectedIndex >= maxItems - 1)
//		selectedIndex = 0;
//	else
//		++selectedIndex;
//
//	g_Selected_Button_Index = selectedIndex;
//
//}

/*************************************************************************************
 * Function Name: UserBTDeviceSelectionScreen_event_handler
*/

UINT ION_AudibleStringSelectionScreen_event_handler(GX_WINDOW *window, GX_EVENT *event_ptr)
{
	//UINT i;
	ION_AUDIBLESTRINGSELECTIONSCREEN_CONTROL_BLOCK *WindowPtr = (ION_AUDIBLESTRINGSELECTIONSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		CreateWidgets(&WindowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox);
		//g_Selected_Button_Index = 0;	// Start with the "BACK" item.
		//g_SaySomethingIndex = 0;        // Start with Main Menu select, this should be changed to 0x01 "BACK" index
		UpdateSelection();		// Adjust the items colors, font and icons.
		gx_vertical_list_selected_set (&WindowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox, g_SaySomething_HHP_Index-1);
        g_ChangeScreen_WIP = false;
		break;

//    case GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
//		// Select the next item, rolling to the first item.
//		SetNextSelection(WindowPtr);
//		UpdateSelection();
//		gx_vertical_list_selected_set (&WindowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox, g_Selected_Button_Index);
//		break;

//    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
//		// Select the Previous Item, rolling to the last item in the list.
//		if (g_Selected_Button_Index == 0)
//			g_Selected_Button_Index = g_NumberOfButtons - 1; // "-1" makes it an Array Index
//		else
//			--g_Selected_Button_Index;
//		UpdateSelection();
//		gx_vertical_list_selected_set (&WindowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox, g_Selected_Button_Index);
//		break;

    case GX_SIGNAL (SAY_SOMETHING_BTN, GX_EVENT_CLICKED): // This event is triggered by a change in the Bluetooth SubIndex message from ION Hub
        if (g_SaySomething_HHP_Index == 0x00)        // "00" = Main Menu with Bluetooth feature selected.
        {
//          BT_Screen_Widget_Cleanup(g_BT_ScreenInfo, MAX_BLUETOOTH_DEVICES);
            screen_toggle((GX_WINDOW *)&MainUserScreen, window);
        }
//        else if (g_SaySomething_HHP_Index == 0x01)   // Are we back at "BACK"?
//       {
//            g_Selected_Button_Index = 0;
//            UpdateSelection();
//            gx_vertical_list_selected_set (&WindowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox, g_SaySomething_HHP_Index);
//        }
        else
        {
//            g_Selected_Button_Index = g_BluetoothSubIndex >> 4;      // The 1-based index is in the upper nibble
//            if (g_Selected_Button_Index >= AV_LIST_END)
//                g_Selected_Button_Index = 0;
//            UpdateBTUserSelection();
            UpdateSelection();      // Adjust the items colors, font and icons.
            gx_vertical_list_selected_set (&WindowPtr->ION_AudibleStringSelectionScreen_AudibleStringListBox, g_SaySomething_HHP_Index-1);
        }
        break;


	} // end switch on event_ptr

	
	gx_window_event_process(window, event_ptr);
	
	return GX_SUCCESS;
}



