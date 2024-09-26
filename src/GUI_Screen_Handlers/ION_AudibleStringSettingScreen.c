//*****************************************************************************
//
// (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
//
// All rights reserved. This file is the intellectual property of ASL and it may
// not be disclosed to others or used for any purposes without the written consent of ASL.
//
//*****************************************************************************
// Filename: ION_AudibleStringSettingScreen.cpp
//
// Date: Aug 20, 2024
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

AUDIO_SETTINGS_STRUCT g_AudioPhraseSettings[AUDIO_PHRASE_MAX_NUMBER] =
{
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_HELP_ME},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_HELLO},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_HI},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_EXCUSE_ME},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_OPEN_DOOR},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_AAC_DEVICE},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_THING_NOT_WORKING},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_GET_AIDE},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_GET_PACK},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_HOW_ARE_YOU},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_THANKS},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_MAKE_CALL},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_IM_THIRSTY},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_IM_HUNGRY},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_IM_IN_PAIN},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_CALL_FOR_HELP},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_BATHROOM},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_CHAIR_BROKEN},
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_LETS_PLAY },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_GET_TEACHER },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_IM_STUCK },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_CAN_I_TRY },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_MY_TURN },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_HELP_ME_PLAY },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_WHICH_WAY },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_DONT_PET_DOG },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_YES },
 { .m_Enabled = true, .m_Description = GX_STRING_ID_AUDIO_NO },
 { .m_Enabled = false, .m_Description = 0 },
 { .m_Enabled = false, .m_Description = 0 },
 { .m_Enabled = false, .m_Description = 0 },
 { .m_Enabled = false, .m_Description = 0 }
};

typedef struct
{
	int m_Location;     // This indicates the Main Screen location, 0=Top most, 3=bottom most
	int m_Enabled;	// This is true if this feature should be displayed for Enabling/Disabling. Typically based upon RNet setting.
	GX_RESOURCE_ID m_Description_ID;
	GX_WIDGET m_ItemWidget;
	GX_PROMPT m_PromptWidget;
	CUSTOM_CHECKBOX m_Checkbox;
} AV_SCREEN_INFO;
AV_SCREEN_INFO g_AudibleStringsScreenInfo[AUDIO_PHRASE_MAX_NUMBER];

/******************************************************************************
* Forward Declarations
*/
void SetupAudioPhraseSettings();
VOID InitializeAudioStringSettingsScreenInfo();
void CreateAudibleStringWidgets(GX_VERTICAL_LIST* list);
VOID AudibleStringList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index);
void UpdateAudibleStringFeatureSettings ();

/******************************************************************************
* Get Audio Phrase information from ION HUB
*/

void SetupAudioPhraseSettings()
{
	int idx;

	for (idx = 0; idx < AUDIO_PHRASE_MAX_NUMBER; ++idx)
	{
		g_AudioPhraseSettings[idx].m_Enabled = false;
	}
    for (idx = 0; idx < 8; ++idx)
    {
        if ((g_AP1 >> idx) & 0x01)
            g_AudioPhraseSettings[idx].m_Enabled = true;
    }
    for (idx = 0; idx < 8; ++idx)
    {
        if ((g_AP2 >> idx) & 0x01)
            g_AudioPhraseSettings[idx+8].m_Enabled = true;
    }
    for (idx = 0; idx < 8; ++idx)
    {
        if ((g_AP3 >> idx) & 0x01)
            g_AudioPhraseSettings[idx+16].m_Enabled = true;
    }
    for (idx = 0; idx < 8; ++idx)
    {
        if ((g_AP4 >> idx) & 0x01)
            g_AudioPhraseSettings[idx+24].m_Enabled = true;
    }
}

/******************************************************************************
* This function initializes the Screen Info based upon the String Settings
* Data Dictionary.
*/
VOID InitializeAudioStringSettingsScreenInfo()
{
	//typedef enum { AV_AAC_DEVICE, AV_HELP, AV_HELLO, AV_HOWAREYOU, AV_IM_THIRSTY, AV_SOMETHING_HURTS, AV_EXCUSE_ME, AV_OPEN_DOOR, AV_BROKEN, AV_BATHROOM, AV_LIST_END };
	int i;

	for (i = 0; i < AUDIO_PHRASE_MAX_NUMBER; ++i)
	{
		g_AudibleStringsScreenInfo[i].m_Location = i;
		g_AudibleStringsScreenInfo[i].m_Enabled = g_AudioPhraseSettings[i].m_Enabled;
		g_AudibleStringsScreenInfo[i].m_Description_ID = g_AudioPhraseSettings[i].m_Description;
	}
}

//*************************************************************************************
// This function populates the Feature List.
//*************************************************************************************

void CreateAudibleStringWidgets(GX_VERTICAL_LIST* list)
{
	int activeFeatureCount = 0;
	int index;

	activeFeatureCount = 0;
	for (index = 0; index < AUDIO_PHRASE_MAX_NUMBER; ++index)
	{
		if (g_AudibleStringsScreenInfo[index].m_Description_ID != (GX_RESOURCE_ID) 0)
		{
			AudibleStringList_callback(list, (GX_WIDGET*)&g_AudibleStringsScreenInfo[index], index);
			++activeFeatureCount;
		}
	}
	list->gx_vertical_list_total_rows = activeFeatureCount;
}

//*************************************************************************************
// Function Name: FeatureList_callback
//
// Description: This function handles drawing and processing the List Items.
//
//*************************************************************************************

VOID AudibleStringList_callback(GX_VERTICAL_LIST *list, GX_WIDGET *widget, INT index)
{
    GX_RECTANGLE childsize;
	AV_SCREEN_INFO*feature = (AV_SCREEN_INFO*)widget;
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
		custom_checkbox_create(&feature->m_Checkbox, &feature->m_ItemWidget, &DefaultCheckboxInfo, &childsize, feature->m_Enabled);

        gx_utility_rectangle_define(&childsize, 0, 0, (GX_VALUE) (feature->m_ItemWidget.gx_widget_size.gx_rectangle_right - 58 - 14), 52);
		gx_prompt_create(&feature->m_PromptWidget, NULL, &feature->m_ItemWidget, 0, GX_STYLE_TRANSPARENT | GX_STYLE_BORDER_NONE | GX_STYLE_ENABLED, 0, &childsize);
        //gx_widget_fill_color_set(&feature->m_PromptWidget, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY, GX_COLOR_ID_DARK_GRAY);
        gx_prompt_text_color_set(&feature->m_PromptWidget, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
		gx_prompt_text_id_set(&feature->m_PromptWidget, feature->m_Description_ID);
	}

}


//*************************************************************************************
// This function gets the settings from the Feature List and populate the Feature
// settings.
//
//*************************************************************************************

void UpdateAudibleStringFeatureSettings ()
{
	int idx;
	
	for (idx = 0; idx < AV_LIST_END; ++idx)
	{
		if (g_AudibleStringsScreenInfo[idx].m_Checkbox.gx_widget_style & GX_STYLE_BUTTON_PUSHED)
		{
			g_AudioPhraseSettings[idx].m_Enabled = true;;
		}
		else
		{
			g_AudioPhraseSettings[idx].m_Enabled = false;
		}
	}
}

//*************************************************************************************
// Function Name: FeatureSettingsScreen_event_process
//
// Description: This handles the Feature Settings Screen messages
//
//*************************************************************************************

UINT ION_Audible_Setting_Screen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
	int index;
	GX_BOOL result;
	ION_AUDIBLESTRING_SETTINGS_SCREEN_CONTROL_BLOCK *FeatureWindowPtr = (ION_AUDIBLESTRING_SETTINGS_SCREEN_CONTROL_BLOCK*) window;

	//GX_EVENT myEvent;

	switch (event_ptr->gx_event_type)
	{
		case GX_EVENT_SHOW:
			InitializeAudioStringSettingsScreenInfo();
			CreateAudibleStringWidgets(&FeatureWindowPtr->ION_AudibleString_Settings_Screen_AudibleString_FeatureListBox);
			if (FeatureWindowPtr->ION_AudibleString_Settings_Screen_AudibleString_FeatureListBox.gx_vertical_list_total_rows < 5)
				gx_widget_hide((GX_WIDGET*)&FeatureWindowPtr->ION_AudibleString_Settings_Screen_FeatureList_vertical_scroll);
			// "Showing" the scrool bar doesn't work here. I put it in the processing of the OK button.
			//else
			//	gx_widget_show ((GX_WIDGET*) &FeatureWindowPtr->FeatureSettingsScreen_FeatureList_vertical_scroll);
			break;

		case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
			// Clean up wdigets so next time this screen is shown it behaves correctly.
			for (index = 0; index < AUDIO_PHRASE_MAX_NUMBER; ++index)
			{
				gx_widget_created_test(&g_AudibleStringsScreenInfo[index].m_Checkbox, &result);
				if (result)
					gx_widget_delete((GX_WIDGET*)&g_AudibleStringsScreenInfo[index].m_Checkbox);
				gx_widget_created_test(&g_AudibleStringsScreenInfo[index].m_PromptWidget, &result);
				if (result)
					gx_widget_delete((GX_WIDGET*)&g_AudibleStringsScreenInfo[index].m_PromptWidget);
				gx_widget_created_test(&g_AudibleStringsScreenInfo[index].m_ItemWidget, &result);
				if (result)
					gx_widget_delete((GX_WIDGET*)&g_AudibleStringsScreenInfo[index].m_ItemWidget);
			}
			gx_widget_show ((GX_WIDGET*) &FeatureWindowPtr->ION_AudibleString_Settings_Screen_FeatureList_vertical_scroll);
	        screen_toggle(PopPushedWindow(), window);
			UpdateAudibleStringFeatureSettings();
			FeatureWindowPtr->ION_AudibleString_Settings_Screen_AudibleString_FeatureListBox.gx_vertical_list_child_count = 0;
			break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}













