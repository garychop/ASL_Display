//*****************************************************************************
// Filename: PerformanceSelectionScreen.c
// Description: This file handles the Performance Selection screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Function Name: PerformanceSelectionScreen_event_process
//
// Description: This dispatches the Pad Option Settings Screen messages
//
//*************************************************************************************

UINT PerformanceSelectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
            break;

        case GX_SIGNAL(GOTO_VEER_ADJUST_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&VeerAdjustScreen, window);
            break;

        case GX_SIGNAL(ATTENDANT_SETTING_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&AttendantSettingsScreen, window);
            break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


