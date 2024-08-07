//*****************************************************************************
// Filename: ResetScreen.c
// Description: This file handles the Reset screens.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Function Name: ResetScreen_event_process
//
// Description: This handles the Diagnostic Screen messages
//
//*************************************************************************************

UINT ResetScreen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{

    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
    if (g_WhoAmi == I_AM_FUSION)
    {
        screen_toggle((GX_WINDOW *)&MoreSelectionScreen, window);
    }
    else
    {
        screen_toggle(PopPushedWindow(), window);
    }
        break;

    case GX_SIGNAL(CONTINUE_BTN_ID, GX_EVENT_CLICKED):
        SendResetParameters();          // Send the RESET PARAMETERS command to the Head Array via the Msg Queue.
//        PushWindow (window);          // Don't do this so a POP window returns back to Main ION Programming menu.
        screen_toggle((GX_WINDOW *)&ResetFinishScreen, window);
        break;
    } // end switch
    return gx_window_event_process(window, event_ptr);
}

//*************************************************************************************

UINT ResetFinishScreen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        if (g_WhoAmi == I_AM_FUSION)
        {
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
        }
        else
        {
            screen_toggle(PopPushedWindow(), window);
        }
        break;

    } // end switch
    return gx_window_event_process(window, event_ptr);
}

