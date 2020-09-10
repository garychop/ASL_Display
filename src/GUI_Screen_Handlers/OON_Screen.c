//*****************************************************************************
// Filename: OON_Screen.c
// Description: This file displays the screen that handles the Out of Neutral
//  screen and message handling.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"


//*************************************************************************************
// Out of Neutral Test screen processing.
//*************************************************************************************

VOID OON_Screen_draw_function (GX_WINDOW *window)
{
    g_ActiveScreen = (GX_WIDGET*) window;

    gx_window_draw(window);
}

//*************************************************************************************

UINT OON_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    gx_window_event_process(window, event_ptr);

    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL (OON_OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            //SendGetVersionCommand ();
            break;

        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_ChangeScreen_WIP = true;
            break;
    } // end switch

    return GX_SUCCESS;
}


