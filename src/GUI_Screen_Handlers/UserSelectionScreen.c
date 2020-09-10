//*****************************************************************************
// Filename: UserSelectionScreen.c
// Description: This file handles the User Selection screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Function Name: UserSelectionScreen_event_process
//
// Description: This handles the User Settings Screen messages
//
//*************************************************************************************

UINT UserSelectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
        break;

    case GX_SIGNAL(USER_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSettingsScreen, window);
        break;

    case GX_SIGNAL(FEATURE_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&FeatureSettingsScreen, window);
        break;

    case GX_SIGNAL(PERFORMANCE_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&PerformanceSelectionScreen, window);
        break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

