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
    g_SpeakerSettings[INTERNAL_SPEAKER_E].m_CuesActive = (data[0] & 0x02) ? true : false;
    g_SpeakerSettings[INTERNAL_SPEAKER_E].m_TonesActive = (data[0] & 0x04) ? true : false;
    g_SpeakerSettings[INTERNAL_SPEAKER_E].m_Voice = (data[0] >> 4);
    g_SpeakerSettings[INTERNAL_SPEAKER_E].m_AuditoryVolumeLevel = data[1];
    g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_CuesActive = (data[2] & 0x02) ? true : false;
    g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_TonesActive = (data[2] & 0x04) ? true : false;
    g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_Voice = (data[2] >> 4);
    g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_AuditoryVolumeLevel = data[3];
    g_AP1 = data[4];
    g_AP2 = data[5];
    g_AP3 = data[6];
    g_AP4 = data[7];
    SetupAudioPhraseSettings();
}

/*************************************************************************************
 * This function retrieves the values to send to the HUB.
 */
void SendAuditorySettings ()
{
    uint8_t settings[8];

    if (g_ION_ClicksActive)
        settings[0] = 0x01;
    // Do internal speaker
    if (g_SpeakerSettings[INTERNAL_SPEAKER_E].m_CuesActive)
        settings[0] = 0x02;
    if (g_SpeakerSettings[INTERNAL_SPEAKER_E].m_TonesActive)
        settings[0] = 0x04;
    settings[0] |= (uint8_t) (g_SpeakerSettings[INTERNAL_SPEAKER_E].m_Voice << 4); // put into upper nibble
    settings[1]= g_SpeakerSettings[INTERNAL_SPEAKER_E].m_AuditoryVolumeLevel;
    // Do external speaker
    if (g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_CuesActive)
        settings[2] = 0x02;
    if (g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_TonesActive)
        settings[2] = 0x04;
    settings[2] |= (uint8_t) (g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_Voice << 4); // put into upper nibble
    settings[3]= g_SpeakerSettings[EXTERNAL_SPEAKER_E].m_AuditoryVolumeLevel;
    settings[4] = g_AP1;
    settings[5] = g_AP2;
    settings[6] = g_AP3;
    settings[7] = g_AP4;
    SendAuditorySettingSetCommand_toHub (settings);
}


/******************************************************************************
 *
 */
VOID ION_AuditorySettingsScreen_draw_function(GX_WINDOW* window)
{
	if (g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel == 0)
		strcpy(g_VolumeLevelStr, "OFF");
	else
	{
		sprintf(g_VolumeLevelStr, "%2d%%", g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel);
	}
	gx_prompt_text_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Auditory_Volume_Prompt, g_VolumeLevelStr);

    // PAD Clicks
	if (g_SpeakerSettings[g_SpeakerIndex].m_ClicksActive)
        gx_button_select ((GX_BUTTON*) &ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_PadClicksToggleBtn);
    else
        gx_button_deselect((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_PadClicksToggleBtn, true);

    // Set Voice
    switch (g_SpeakerSettings[g_SpeakerIndex].m_Voice)
    {
    case MALE_VOICE_AUDIBLE:
        gx_multi_line_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_AUDIO_MALE_VOICE);
        break;
    case FEMALE_VOICE_AUDIBLE:
        gx_multi_line_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_AUDIO_FEMALE_VOICE);
        break;
    case CHILDS_VOICE_AUDIBLE:
        gx_multi_line_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_AUDIO_CHILDS_VOICE);
        break;
    case AUDIBLE_TYPE_END:
    case SILENCE_AUDIBLE:
    default:
        gx_multi_line_text_button_text_id_set(&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_Audible_Selection_Button, GX_STRING_ID_AUDIO_VOICE_OFF);
        break;
    } // end switch

    if (g_SpeakerSettings[g_SpeakerIndex].m_CuesActive)
        gx_button_select((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_CuesToggleBtn);
    else
        gx_button_deselect((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_CuesToggleBtn, true);

    // Set Tones button
    if (g_SpeakerSettings[g_SpeakerIndex].m_TonesActive)
        gx_button_select((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_TonesToggleBtn);
    else
        gx_button_deselect((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_TonesToggleBtn, true);

    gx_window_draw(window);
}

/*************************************************************************************
* Description: This handles the Feature Settings Screen messages
*/

UINT ION_AuditorySettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	switch (event_ptr->gx_event_type)
	{
//	    case GX_EVENT_SHOW:
//		    break;

		// Adjust Volume
		case GX_SIGNAL(AUDITORY_VOLUMEUP_BTN, GX_EVENT_CLICKED):
			if (g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel <= 95)
			    g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel += 5; // nomatterwhatitry. This warning always barks.
			else
			    g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel = 100;
		    gx_system_dirty_mark ((GX_WIDGET*) window);
			break;
		case GX_SIGNAL(AUDITORY_VOLUMEDOWN_BTN, GX_EVENT_CLICKED):
			if (g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel > 5)
			    g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel -= 5;
			else
			    g_SpeakerSettings[g_SpeakerIndex].m_AuditoryVolumeLevel = 0;
		    gx_system_dirty_mark ((GX_WIDGET*) window);
			break;

		// Pad Clicks toggle button processing
		case GX_SIGNAL(PAD_CLICKS_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
            g_SpeakerSettings[g_SpeakerIndex].m_ClicksActive = true;
			break;
		case GX_SIGNAL(PAD_CLICKS_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
            g_SpeakerSettings[g_SpeakerIndex].m_ClicksActive = false;
			break;

        // CUES toggle button processing
        case GX_SIGNAL(CUES_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
            g_SpeakerSettings[g_SpeakerIndex].m_CuesActive = true;
            break;
        case GX_SIGNAL(CUES_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
            g_SpeakerSettings[g_SpeakerIndex].m_CuesActive = false;
            break;

        // TONES toggle button processing
        case GX_SIGNAL(TONES_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
            g_SpeakerSettings[g_SpeakerIndex].m_TonesActive = true;
            break;
        case GX_SIGNAL(TONES_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
            g_SpeakerSettings[g_SpeakerIndex].m_TonesActive = false;
            break;

		// Voice button processing
		case GX_SIGNAL(AUDIBLE_SELECTION_BTN, GX_EVENT_CLICKED):
            ++g_SpeakerSettings[g_SpeakerIndex].m_Voice;
            if (AUDIBLE_TYPE_END == g_SpeakerSettings[g_SpeakerIndex].m_Voice)
                g_SpeakerSettings[g_SpeakerIndex].m_Voice = (AUDIBLE_TYPE_ENUM)0;
		    gx_system_dirty_mark ((GX_WIDGET*) window);
			break;

		case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            SendAuditorySettings();
            screen_toggle(PopPushedWindow(), window);
			break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}


