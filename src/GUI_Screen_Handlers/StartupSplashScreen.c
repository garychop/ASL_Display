//*****************************************************************************
// Filename: StartupSplashScreen.c
// Description: This file displays the Startup Splash screen and handles
//  the messages thereafter.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Startup Screen
//*************************************************************************************

VOID StartupSplashScreen_draw_function (GX_WINDOW *window)
{
    g_ActiveScreen = (GX_WIDGET*) window;

    gx_window_draw(window);
}

//*************************************************************************************

UINT StartupSplashScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    uint8_t counter;

    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            if (I_AM_ION == g_WhoAmi)
                screen_toggle((GX_WINDOW *)&ION_BT_DeviceSelectionScreen, window);
            else
                screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_ChangeScreen_WIP = true;
            break;

        case GX_SIGNAL (HB_OK_ID, GX_EVENT_CLICKED):
            if (g_StartupDelayCounter < 0)      // If we've been here before but are recovering from a Heart Beat timeout, don't wait, just goto the Main User Screen.
            {
                screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            }
            else // OK, we are starting up.
            {
                ++g_StartupDelayCounter;
                if (g_StartupDelayCounter > 18)  // Have we shown the startup screen long enough?
                {
                    screen_toggle((GX_WINDOW *)&MainUserScreen, window);
                    g_StartupDelayCounter = -1; // This prevents us from doing a "startup" delay should the Heart Beat stop.
                }
                else if (g_StartupDelayCounter == 6)    // We need to send a Version Request to the Head Array.
                {
                    SendWhoAmiCommand ();
                }
                else if (g_StartupDelayCounter == 8)    // We need to send a Version Request to the Head Array.
                {
                    if (g_WhoAmi)   // This is the response from the ION if connected.
                    {
                        gx_pixelmap_button_pixelmap_set (&StartupSplashScreen.StartupSplashScreen_pixelmap_button, GX_PIXELMAP_ID_ION_LOGO_REDWHITE, GX_PIXELMAP_ID_ION_LOGO_REDWHITE, GX_PIXELMAP_ID_ION_LOGO_REDWHITE);
                    }
                    else
                    {
                        gx_pixelmap_button_pixelmap_set (&StartupSplashScreen.StartupSplashScreen_pixelmap_button, GX_PIXELMAP_ID_FUSION_LOGO_REDWHITE, GX_PIXELMAP_ID_FUSION_LOGO_REDWHITE, GX_PIXELMAP_ID_FUSION_LOGO_REDWHITE);
                    }
                    SendGetVersionCommand ();
                }
                else if (g_StartupDelayCounter == 10)    // We need to send a Version Request to the Head Array.
                {
                    SendFeatureGetCommand();                // Send command to get the current users settings.
                }
                else if (g_StartupDelayCounter == 12)    // We need to send a Version Request to the Head Array.
                {
                    SendAttendantSettingsGet_toHeadArray(); // Request Attendant Settings
                }
                else if (g_StartupDelayCounter == 14)
                {
                    SendGetPadAssignmentMsg (LEFT_PAD);
                    SendGetPadAssignmentMsg (RIGHT_PAD);
                    SendGetPadAssignmentMsg (CENTER_PAD);
                }
                else if (g_StartupDelayCounter == 16)
                {
                    for (counter = 0; counter < MAX_BLUETOOTH_DEVICES; ++counter)
                    {
                        Send_Get_BT_DeviceDefinitions (counter);
                    }
                }
            }
            break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


