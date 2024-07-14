//*****************************************************************************
// Filename: ION_AuditorSettingsScreen.cpp
//
// Date: Feb 11, 2024
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

/*************************************************************************************
* Local/Global variables
*/

bool g_AuditoryTonesActive = false;
bool g_AuditoryVoiceActive = false;
uint8_t g_AuditoryVolumeLevel = 20;
bool g_ION_ClicksActive = false;


uint8_t g_OriginalVolumeLeve;
bool g_OriginalAuditoryTonesActive = false;
bool g_OriginalAuditoryVoiceActive = false;
bool g_OriginalION_ClicksActive = false;

char g_VolumeLevelStr[8];

/*************************************************************************************
* Forward Declarations
*/
void StoreAuditorySettings (uint8_t setting, uint8_t volume);
void SendAuditorySettings ();

/*************************************************************************************
 * This function stores the values (presumably from the HUB) for use by the Screen.
 * @param setting
 * @param volume
 */
void StoreAuditorySettings (uint8_t setting, uint8_t volume)
{
    g_ION_ClicksActive = (setting & 0x01) ? true : false;
    g_AuditoryTonesActive = (setting & 0x02) ? true : false;
    g_AuditoryVoiceActive = (setting & 0x04) ? true : false;

    g_AuditoryVolumeLevel = volume;

}

/*************************************************************************************
 * This function retrieves the values to send to the HUB.
 * @param setting
 * @param volume
 */
void SendAuditorySettings ()
{
    uint8_t setting, volume;

    setting = 0x00;
    if (g_ION_ClicksActive)
        setting = 0x01;
    if (g_AuditoryTonesActive)
        setting |= 0x02;
    if (g_AuditoryVoiceActive)
        setting |= 0x04;

    volume = g_AuditoryVolumeLevel;

    SendAuditorySettingSetCommand_toHub (setting, volume);
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
    if (g_AuditoryTonesActive)
        gx_button_select ((GX_BUTTON*) &ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_ToneToggleBtn);
    else
        gx_button_deselect((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_ToneToggleBtn, true);

    // Set "Voice" toggle button
    if (g_AuditoryVoiceActive)
        gx_button_select((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_VoiceToggleBtn);
    else
        gx_button_deselect((GX_BUTTON*)&ION_AuditorySettingsScreen.ION_AuditorySettingsScreen_VoiceToggleBtn, true);

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
		    g_OriginalVolumeLeve = g_AuditoryVolumeLevel;
		    g_OriginalAuditoryTonesActive = g_AuditoryTonesActive;
		    g_OriginalAuditoryVoiceActive = g_AuditoryVoiceActive;
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

		// Tone toggle button processing
		case GX_SIGNAL(TONE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
		    gx_system_dirty_mark ((GX_WIDGET*) window);
		    g_AuditoryTonesActive = true;
		    g_AuditoryVoiceActive = false;     // Turn off voice setting.
			break;
		case GX_SIGNAL(TONE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
            g_AuditoryTonesActive = false;
			break;

		// Enhanced toggle button processing
		case GX_SIGNAL(VOICE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
		    gx_system_dirty_mark ((GX_WIDGET*) window);
		    g_AuditoryVoiceActive = true;
			g_AuditoryTonesActive = false; // Turn off TONE setting
			break;
		case GX_SIGNAL(VOICE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
            g_AuditoryVoiceActive = false;
			break;

		case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            if ((g_OriginalVolumeLeve != g_AuditoryVolumeLevel)
             || (g_OriginalAuditoryTonesActive != g_AuditoryTonesActive)
             || (g_OriginalAuditoryVoiceActive != g_AuditoryVoiceActive)
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


