//*****************************************************************************
// Filename: FeatureSettingsScreen.cpp
//
// Date: Aug 28, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"
#include "QueueDefinition.h"

CUSTOM_CHECKBOX_INFO checkbox_info =
{
    TOGGLE_BUTTON_ID,
    GX_PIXELMAP_ID_SWITCH_BG,
    GX_PIXELMAP_ID_SWITCH_ACTIVE,
    GX_PIXELMAP_ID_SWITCH_DISACTIVE,
    4,
    24 // 24
};


//ALARM_LIST_ROW alarm_list_row_memory[NUM_FEATURES + 1];

//extern CUSTOM_CHECKBOX_INFO checkbox_info;

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

int g_SettingsChanged;

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

void CreateFeatureWidgets (GX_VERTICAL_LIST *list);
void UpdateFeatureSettings (void);

//*************************************************************************************
// Function Name: FeatureList_callback
//
// Description: This function handles drawing and processing the List Items.
//
//*************************************************************************************

VOID FeatureList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
    MAIN_SCREEN_FEATURE *feature = (MAIN_SCREEN_FEATURE *)widget;
    GX_BOOL result;

	gx_widget_created_test(&feature->m_ItemWidget, &result);
    if (!result)
    {
        gx_utility_rectangle_define(&childsize, 0, 0, 215, 52);	// 0,0,x,75 is too high
        gx_widget_create(&feature->m_ItemWidget, NULL, (GX_WIDGET *)list, GX_STYLE_TRANSPARENT /*| GX_STYLE_BORDER_THIN | GX_STYLE_ENABLED | GX_STYLE_TEXT_LEFT */, GX_ID_NONE, &childsize);

        //gx_utility_rectangle_define(&childsize, 130, (52-27)/2, 188, 47); // Left, Top, Right, Botton
		childsize.gx_rectangle_left = (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 58 - 14); // "58" is size of pixel  140;
		childsize.gx_rectangle_top = (52-27)/2;
		childsize.gx_rectangle_right = (GX_VALUE) (childsize.gx_rectangle_left + 58);	// "58" is width of background pic
		childsize.gx_rectangle_bottom = 47;
		custom_checkbox_create(&feature->m_Checkbox, &feature->m_ItemWidget, &checkbox_info, &childsize, feature->m_Enabled);

        gx_utility_rectangle_define(&childsize, 0, 0, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 58 - 14), 52);
		gx_prompt_create(&feature->m_PromptWidget, NULL, &feature->m_ItemWidget, 0, GX_STYLE_TRANSPARENT | GX_STYLE_BORDER_NONE | GX_STYLE_ENABLED, 0, &childsize);
        //gx_widget_fill_color_set(&feature->m_PromptWidget, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY);
        gx_prompt_text_color_set(&feature->m_PromptWidget, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE); // , GX_COLOR_ID_WHITE);
		gx_prompt_text_id_set(&feature->m_PromptWidget, feature->m_LargeDescriptionID);
	}

}

//*************************************************************************************
// This function gets the settings from the Feature List and populate the Feature
// settings.
//
//*************************************************************************************

void UpdateFeatureSettings ()
{
	int feature;
	int numActive;

	numActive = 0;
	for (feature = 0; feature < NUM_FEATURES; ++feature)
	{
		if (g_MainScreenFeatureInfo[feature].m_Available)
		{
			if (g_MainScreenFeatureInfo[feature].m_Checkbox.gx_widget_style & GX_STYLE_BUTTON_PUSHED)
			{
				g_MainScreenFeatureInfo[feature].m_Enabled = true;
				g_MainScreenFeatureInfo[feature].m_Location = numActive;
				++numActive;
			}
			else
			{
				g_MainScreenFeatureInfo[feature].m_Enabled = false;
			}
		}
	}
}

//*************************************************************************************
// Function Name: FeatureSettingsScreen_event_process
//
// Description: This handles the Feature Settings Screen messages
//
//*************************************************************************************

UINT FeatureSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	//UINT myErr = 0;
	int feature;
    uint8_t myActiveFeatures;
	FEATURESETTINGSSCREEN_CONTROL_BLOCK *FeatureWindowPtr = (FEATURESETTINGSSCREEN_CONTROL_BLOCK*) window;

	//GX_EVENT myEvent;

	switch (event_ptr->gx_event_type)
	{
		case GX_EVENT_SHOW:
			CreateFeatureWidgets (&FeatureWindowPtr->FeatureSettingsScreen_FeatureListBox);
			if (FeatureWindowPtr->FeatureSettingsScreen_FeatureListBox.gx_vertical_list_total_rows < 5)
				gx_widget_hide ((GX_WIDGET*) &FeatureWindowPtr->FeatureSettingsScreen_FeatureList_vertical_scroll);
			// "Showing" the scrool bar doesn't work here. I put it in the processing of the OK button.
			//else
			//	gx_widget_show ((GX_WIDGET*) &FeatureWindowPtr->FeatureSettingsScreen_FeatureList_vertical_scroll);
			break;

		case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
			gx_widget_show ((GX_WIDGET*) &FeatureWindowPtr->FeatureSettingsScreen_FeatureList_vertical_scroll);
			screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
			UpdateFeatureSettings();
			// Delete all widgets so each time this screen gets accessed, we must re-establish the list becuase it might change
			// due to RNet Enabled/Disabled.
			for (feature = 0; feature < NUM_FEATURES; ++feature)
			{
				if (&g_MainScreenFeatureInfo[feature].m_PromptWidget != NULL)
				{
					gx_widget_delete((GX_WIDGET*) &g_MainScreenFeatureInfo[feature].m_PromptWidget);
				}
				if (&g_MainScreenFeatureInfo[feature].m_Checkbox != NULL)
				{
					gx_widget_delete((GX_WIDGET*) &g_MainScreenFeatureInfo[feature].m_Checkbox);
				}
				if (&g_MainScreenFeatureInfo[feature].m_ItemWidget != NULL)
				{
					gx_widget_delete((GX_WIDGET*) &g_MainScreenFeatureInfo[feature].m_ItemWidget);
				}
			}
			FeatureWindowPtr->FeatureSettingsScreen_FeatureListBox.gx_vertical_list_child_count = 0;
			//{
			//	//myChildWidget = &FeatureWindowPtr->FeatureSettingsScreen_FeatureListBox.gx_widget_first_child;
			//	myErr = gx_widget_delete ((GX_WIDGET*) FeatureWindowPtr->FeatureSettingsScreen_FeatureListBox.gx_widget_first_child);
			//}
			// THe following did nothing for my cause.
			//myEvent.gx_event_type = GX_EVENT_DELETE;
			//myErr = gx_vertical_list_event_process (&FeatureSettingsScreen.FeatureSettingsScreen_FeatureListBox, &myEvent);

	        CreateEnabledFeatureStatus(&myActiveFeatures);
			SendFeatureSetting (myActiveFeatures, g_TimeoutValue);
	        SendFeatureGetCommand();                // Send command to get the current users settings.
			break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return GX_SUCCESS;
}

//*************************************************************************************
// This function populates the Feature List.
//*************************************************************************************

void CreateFeatureWidgets (GX_VERTICAL_LIST *list)
{
 	int index;
	int activeFeatureCount;

	if (g_RNet_Active)
	{
		// Display as "RNet TOGGLE"
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_TOGGLE;
		// Display as "RNET USER MENU"
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_MENU;
	}
	else
	{
		// Display as NEXT FUNCTION
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_FUNCTION;
		// Display as NEXT PROFILE
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_PROFILE;
	}

	activeFeatureCount = 0;
	for (index = 0; index < NUM_FEATURES; ++index)
	{
		if (g_MainScreenFeatureInfo[index].m_Available)
		{
			FeatureList_callback (list, (GX_WIDGET*) &g_MainScreenFeatureInfo[index], index);
			++activeFeatureCount;
		}
 	}
	list->gx_vertical_list_total_rows = activeFeatureCount;
}


#ifdef USIING_ORIGINAL_FEATURE_SCREEN

UINT FeatureSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	UINT myErr = -1;
	int feature;
	int numActive;
	FEATURESETTINGSSCREEN_CONTROL_BLOCK *FeatureWindowPtr = (FEATURESETTINGSSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		// Show RNet or NEXT FUNCITON and NEXT PROFILE depending on RNet setting.
		// Adjust the displayed information based upon the RNet setting.
		// .. If RNet is enabled, the NEXT FUNCTION feature becomes RNet TOGGLE
		// .. and NEXT PROFILE feature become RNet MENU.
		if (g_MainScreenFeatureInfo[RNET_ID].m_Enabled)
		{
			// Display as "RNet TOGGLE"
			myErr = gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_Prompt3, GX_STRING_ID_RNET_TOGGLE);
			// Display as "RNET USER MENU"
			myErr = gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_Prompt3, GX_STRING_ID_RNET_MENU);
		}
		else
		{
			// Display as NEXT FUNCTION
			myErr = gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_Prompt3, GX_STRING_ID_NEXT_FUNCTION);
			// Display as NEXT PROFILE
			myErr = gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_Prompt3, GX_STRING_ID_NEXT_PROFILE);
		}

		// Power status
		if (g_MainScreenFeatureInfo[0].m_Enabled)
		{
			gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_ToggleBtn1);
		}

		// Bluetooth
		if (g_MainScreenFeatureInfo[1].m_Enabled)
		{
			gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_ToggleBtn2);
		}

		// Next Function
		if (g_MainScreenFeatureInfo[2].m_Enabled)
		{
			gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_ToggleBtn3);
		}

		// Next Profile
		if (g_MainScreenFeatureInfo[3].m_Enabled)
		{
			gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_ToggleBtn4);
		}

		g_SettingsChanged = FALSE;

		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
		if (g_SettingsChanged)
		{
			numActive = 0;
			for (feature = 0; feature < 4; ++feature)
			{
				if (g_MainScreenFeatureInfo[feature].m_Enabled)
				{
					g_MainScreenFeatureInfo[feature].m_Location = numActive;
					++numActive;
				}
			}
		}
		break;

	//----------------------------------------------------------------------
	// Power Button
	case GX_SIGNAL(TOGGLE_BTN_1, GX_EVENT_TOGGLE_ON):
		g_MainScreenFeatureInfo[0].m_Enabled = TRUE;
		g_SettingsChanged = TRUE;
		break;
	case GX_SIGNAL(TOGGLE_BTN_1, GX_EVENT_TOGGLE_OFF):
		g_MainScreenFeatureInfo[0].m_Enabled = FALSE;
		g_SettingsChanged = TRUE;
		break;

	//----------------------------------------------------------------------
	// Bluetooth button
	case GX_SIGNAL(TOGGLE_BTN_2, GX_EVENT_TOGGLE_ON):
		g_MainScreenFeatureInfo[1].m_Enabled = TRUE;
		g_SettingsChanged = TRUE;
		break;
	case GX_SIGNAL(TOGGLE_BTN_2, GX_EVENT_TOGGLE_OFF):
		g_MainScreenFeatureInfo[1].m_Enabled = FALSE;
		g_SettingsChanged = TRUE;
		break;

	//----------------------------------------------------------------------
	// Next Function button
	case GX_SIGNAL(TOGGLE_BTN_3, GX_EVENT_TOGGLE_ON):
		g_MainScreenFeatureInfo[2].m_Enabled = TRUE;
		g_SettingsChanged = TRUE;
		break;
	case GX_SIGNAL(TOGGLE_BTN_3, GX_EVENT_TOGGLE_OFF):
		g_MainScreenFeatureInfo[2].m_Enabled = FALSE;
		g_SettingsChanged = TRUE;
		break;

	//----------------------------------------------------------------------
	// Next Profile Button
	case GX_SIGNAL(TOGGLE_BTN_4, GX_EVENT_TOGGLE_ON):
		g_MainScreenFeatureInfo[3].m_Enabled = TRUE;
		g_SettingsChanged = TRUE;
		break;
	case GX_SIGNAL(TOGGLE_BTN_4, GX_EVENT_TOGGLE_OFF):
		g_MainScreenFeatureInfo[3].m_Enabled = FALSE;
		g_SettingsChanged = TRUE;
		break;
	}

    myErr = gx_window_event_process(window, event_ptr);

	return 0;
}
#endif // USIING_ORIGINAL_FEATURE_SCREEN











