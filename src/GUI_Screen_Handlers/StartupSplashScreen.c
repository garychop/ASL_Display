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
                if (g_StartupDelayCounter > 15)  // Have we shown the startup screen long enough?
                {
                    screen_toggle((GX_WINDOW *)&MainUserScreen, window);
                    g_StartupDelayCounter = -1; // This prevents us from doing a "startup" delay should the Heart Beat stop.
                }
                else if (g_StartupDelayCounter == 10)    // We need to send a Version Request to the Head Array.
                {
                    SendGetVersionCommand ();
                    SendFeatureGetCommand();                // Send command to get the current users settings.
                }
            }
            break;
    } // end switch

    return GX_SUCCESS;
}


