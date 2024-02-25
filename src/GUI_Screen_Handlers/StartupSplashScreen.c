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
    gx_window_event_process(window, event_ptr);

    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
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
                if (g_StartupDelayCounter > 16)  // Have we shown the startup screen long enough?
                {
                    screen_toggle((GX_WINDOW *)&MainUserScreen, window);
                    g_StartupDelayCounter = -1; // This prevents us from doing a "startup" delay should the Heart Beat stop.
                }
                else if (g_StartupDelayCounter == 4)
                {
                    SendWhoAmICommand();            // We want to know who we are connected with... either Fusion or ION.
                }
                else if (g_StartupDelayCounter == 8)    // We need to send a Version Request to the Head Array.
                {
                    SendGetVersionCommand ();
                    // Check to see if the connected device reponded. Fusion's will not respond
                    // ... ION's will respond.
                    if (DEVICE_ID_ION == g_WhoAmI)  // Did we get a response from ION.
                    {
                        gx_pixelmap_button_pixelmap_set (&StartupSplashScreen.StartupSplashScreen_StartupPrompt,
                                                         GX_PIXELMAP_ID_ION_LOGO_REDWHITE, GX_PIXELMAP_ID_ION_LOGO_REDWHITE, GX_PIXELMAP_ID_ION_LOGO_REDWHITE);
                    }
                    else
                    {
                        gx_pixelmap_button_pixelmap_set (&StartupSplashScreen.StartupSplashScreen_StartupPrompt,
                                                         GX_PIXELMAP_ID_FUSION_LOGO_REDWHITE, GX_PIXELMAP_ID_FUSION_LOGO_REDWHITE, GX_PIXELMAP_ID_FUSION_LOGO_REDWHITE);
                    }
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
            }
            break;
    } // end switch

    return GX_SUCCESS;
}


