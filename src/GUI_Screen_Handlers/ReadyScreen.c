//*****************************************************************************
// Filename: ReadyScreen.c
// Description: This file displays the screen that indicates Power Off
//  and handles the messages thereafter.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Function Name: Ready_Screen_event_process
//
// Description: This handles the Ready Screen messages, this screen is the one
//      that indicates that the "System is Powered off, please hit switch".
//
//*************************************************************************************
VOID Ready_Screen_draw_function(GX_WINDOW *window)
{
    g_ActiveScreen = (GX_WIDGET*) window;

    if (g_WhoAmi == I_AM_FUSION)
    {
        gx_prompt_text_id_set (&ReadyScreen.ReadyScreen_DevicePrompt, GX_STRING_ID_FUSION_OFF);
    }
    else
    {
        gx_prompt_text_id_set (&ReadyScreen.ReadyScreen_DevicePrompt, GX_STRING_ID_ION_OFF);
    }
    gx_window_draw(window);
}

//*************************************************************************************

UINT Ready_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL(POWER_ON_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            break;

        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_ChangeScreen_WIP = true;
            break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

