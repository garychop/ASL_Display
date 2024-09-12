/******************************************************************************
 *
 * (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
 *
 * All rights reserved. This file is the intellectual property of ASL and it may
 * not be disclosed to others or used for any purposes without the written consent of ASL.
 *
*******************************************************************************
 * Filename: ION_AuditorSettingsScreen.cpp
 *
 * Date: Feb 11, 2024
 *
 * Author: G. Chopcinski, Kg Solutions, LLC
 *
*****************************************************************************/

#include "ASL165_System.h"
#include "QueueDefinition.h"

/*************************************************************************************
* Local/Global variables
*/

uint8_t g_OriginalVolumeLevel = 0;
uint8_t g_OriginalAuditoryVoiceActive = 0;
bool g_OriginalION_ClicksActive = false;

char g_VolumeLevelStr[8];

/*************************************************************************************
* Forward and extern Declarations
*/
void StoreAuditorySettings (uint8_t *data);
void SendAuditorySettings ();
extern void SetupAudioPhraseSettings();

/*************************************************************************************
 * This function stores the values (presumably from the HUB) for use by the Screen.
 * @param setting
 * @param volume
 */
void StoreAuditorySettings (uint8_t *data)
{
    g_ION_ClicksActive = (data[0] & 0x01) ? true : false;
    g_Audible_Setting = (data[0] >> 4);
    g_AuditoryVolumeLevel = data[1];
    g_AP1 = data[2];
    g_AP2 = data[3];
    g_AP3 = data[4];
    g_AP4 = data[5];
    SetupAudioPhraseSettings();
}

/*************************************************************************************
 * This function retrieves the values to send to the HUB.
 * @param setting
 * @param volume
 */
void SendAuditorySettings ()
{
    uint8_t settings[6];

    settings[0] = 0x0;
    if (g_ION_ClicksActive)
        settings[0] = 0x01;
    settings[0] |= (uint8_t) (g_Audible_Setting << 4); // put into upper nibble
    settings[1]= g_AuditoryVolumeLevel;
    settings[2] = g_AP1;
    settings[3] = g_AP2;
    settings[4] = g_AP3;
    settings[5] = g_AP4;
    SendAuditorySettingSetCommand_toHub (settings);
}


/******************************************************************************
 *
 */
VOID ION_AuditorySettingsScreen_draw_function(GX_WINDOW* window)
{
	if (g_AuditoryVolumeLevel == 0)
		strcpy(g_VolumeLevelStr, "OFF");
	else
	{
		sprintf(g_VolumeLevelStr, "%2d%%", g_AuditoryVolumeLevel);
	}
	gx_prompt_text_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Auditory_Volume_Prompt, g_VolumeLevelStr);

    // PAD Clicks
    if (g_ION_ClicksActive)
        gx_button_select ((GX_BUTTON*) &ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_PadClicksToggleBtn);
    else
        gx_button_deselect((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_PadClicksToggleBtn, true);

    // Set "Tone" Clicks
    switch (g_Audible_Setting)
    {
    case TONES_AUDIBLE:
        gx_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_TONES);
        break;
    case MALE_VOICE_AUDIBLE:
        gx_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_MALE_VOICE);
        break;
    case FEMALE_VOICE_AUDIBLE:
        gx_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_FEMALE_VOICE);
        break;
    case CHILDS_VOICE_AUDIBLE:
        gx_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_CHILDS_VOICE);
        break;
    case AUDIBLE_TYPE_END:
    case SILENCE_AUDIBLE:
    default:
        gx_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_NONE);
        break;
    } // end switch


    gx_window_draw(window);
}

/*************************************************************************************
* Description: This handles the Feature Settings Screen messages
*/

UINT ION_AuditorySettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	switch (event_ptr->gx_event_type)
	{
	    case GX_EVENT_SHOW:
		    g_OriginalVolumeLevel = g_AuditoryVolumeLevel;
		    g_OriginalAuditoryVoiceActive = g_Audible_Setting;
		    g_OriginalION_ClicksActive = g_ION_ClicksActive;
		    break;

		// Adjust Volume
		case GX_SIGNAL(AUDITORY_VOLUMEUP_BTN, GX_EVENT_CLICKED):
			if (g_AuditoryVolumeLevel <= 95)
				g_AuditoryVolumeLevel += 5;
			else
				g_AuditoryVolumeLevel = 100;
		    gx_system_dirty_mark ((GX_WIDGET*) window);
			break;
		case GX_SIGNAL(AUDITORY_VOLUMEDOWN_BTN, GX_EVENT_CLICKED):
			if (g_AuditoryVolumeLevel > 5)
				g_AuditoryVolumeLevel -= 5;
			else
				g_AuditoryVolumeLevel = 0;
		    gx_system_dirty_mark ((GX_WIDGET*) window);
			break;

		// Clicks toggle button processing
		case GX_SIGNAL(PAD_CLICKS_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
            g_ION_ClicksActive = true;
			break;
		case GX_SIGNAL(PAD_CLICKS_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
            g_ION_ClicksActive = false;
			break;

		// Enhanced toggle button processing
		case GX_SIGNAL(AUDIBLE_SELECTION_BTN, GX_EVENT_CLICKED):
            ++g_Audible_Setting;
            if (AUDIBLE_TYPE_END == g_Audible_Setting)
                g_Audible_Setting = (AUDIBLE_TYPE_ENUM)0;
		    gx_system_dirty_mark ((GX_WIDGET*) window);
			break;

		case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            if ((g_OriginalVolumeLevel != g_AuditoryVolumeLevel)
             || (g_OriginalAuditoryVoiceActive != g_Audible_Setting)
             || (g_OriginalION_ClicksActive != g_ION_ClicksActive))
             {
                SendAuditorySettings();
             }

            screen_toggle(PopPushedWindow(), window);
			break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}


