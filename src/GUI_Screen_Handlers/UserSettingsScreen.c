//*****************************************************************************
// Filename: UserSettingsScreen.c
// Description: This file handles the User Settings screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local Variables
//*************************************************************************************

char g_TimeoutValueString[8] = "OFF";

//*************************************************************************************

UINT UserSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    uint8_t myActiveFeatures, activeFeature2;
    char tmpChar[8];

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        if (g_ClicksActive)
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_ClicksToggleBtn);

        // Power Up in Idle
        if (g_PowerUpInIdle)    // If powering up in idle state is enable
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_PowerUpToggleBtn);

        // RNet Enabled setting
        if (g_RNet_Active)
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_RNET_ToggleBtn);

        // Mode switch schema. Either PIN5 operation or Toggle F/R.
        if (g_Mode_Switch_Schema == MODE_SWITCH_REVERSE)
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_ModeReverse_ToggleBtn);

        // Populate the Timeout button with the current setting or "OFF".
        if (g_TimeoutValue == 0)
            strcpy (g_TimeoutValueString, "OFF");
        else
        {
            // sprintf (g_TimeoutValueString, "%1.1g", (float) (g_TimeoutValue / 10.0f));
            // Floating point doesn't work for some odd reason.
            // I'm doing a hack to display the value in a X.X format.
            sprintf (g_TimeoutValueString, "%d.", g_TimeoutValue / 10);
            sprintf (tmpChar, "%d", g_TimeoutValue % 10);
            strcat (g_TimeoutValueString, tmpChar);
        }
        gx_text_button_text_set (&UserSettingsScreen.UserSettingsScreen_Timeout_Button, g_TimeoutValueString);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
        CreateEnabledFeatureStatus(&myActiveFeatures, &activeFeature2);
        SendFeatureSetting (myActiveFeatures, g_TimeoutValue, activeFeature2);
        SendFeatureGetCommand();                // Send command to get the current users settings.
        break;

    //----------------------------------------------------------
    // CLICK toggle button processing
    case GX_SIGNAL(CLICKS_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
        g_ClicksActive = true;
        break;
    case GX_SIGNAL(CLICKS_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
        g_ClicksActive = false;
        break;

    //----------------------------------------------------------
    // Power Up in IDLE toggle button
    case GX_SIGNAL(POWER_UP_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
        g_PowerUpInIdle = true;
        break;
    case GX_SIGNAL(POWER_UP_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
        g_PowerUpInIdle = false;
        break;

    //----------------------------------------------------------
    // RNet Enable toggle button
    case GX_SIGNAL(RNET_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
        g_RNet_Active = true;
        break;
    case GX_SIGNAL(RNET_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
        g_RNet_Active = false;
        break;

    case GX_SIGNAL(TIMEOUT_BTN_ID, GX_EVENT_CLICKED):
        switch (g_TimeoutValue)
        {
            case 0:
                g_TimeoutValue = 10;
                break;
            case 10:
            case 15:
            case 20:
            case 25:
                g_TimeoutValue =  (uint8_t)(g_TimeoutValue + 5);
                break;
            case 30:
            case 40:
                g_TimeoutValue = (uint8_t)(g_TimeoutValue + 10);
                break;
            case 50:
                g_TimeoutValue = 0;
                break;
        } // end switch
        if (g_TimeoutValue == 0)
            strcpy (g_TimeoutValueString, "OFF");
        else
        {
            sprintf (g_TimeoutValueString, "%d.", g_TimeoutValue / 10);
            sprintf (tmpChar, "%d", g_TimeoutValue % 10);
            strcat (g_TimeoutValueString, tmpChar);
        }
        gx_text_button_text_set (&UserSettingsScreen.UserSettingsScreen_Timeout_Button, g_TimeoutValueString);
        break;

    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

