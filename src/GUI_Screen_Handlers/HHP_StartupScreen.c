//*****************************************************************************
// Filename: HHP_StartupScreen.c
// Description: This file displays the Initial Programing Screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Function Name: HHP_Start_Screen_event_process
//
// Description: This handles the Startup Screen messages
//
//*************************************************************************************

UINT HHP_Start_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(PAD_SETTINGS_BTN_ID, GX_EVENT_CLICKED):      // When selected, goto Main Pad Setting scree.
        screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
        break;

    case GX_SIGNAL(SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
        break;

    case GX_SIGNAL(MORE_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&MoreSelectionScreen, window);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&MainUserScreen, window);
        SendSaveParameters();
        break;

    case GX_EVENT_SHOW:
        // We're entering the HHP feature, it's a good time to request "one-time" information.
        SendGetCalDataCommnd (LEFT_PAD);        // We send the commands to the Head Array to get the Calibration Data for all 3 pads.
        SendGetCalDataCommnd (RIGHT_PAD);
        SendGetCalDataCommnd (CENTER_PAD);
        SendNeutralDAC_GetCommand();            // We'll need the Neutral DAC "calibration" value.
        SendFeatureGetCommand();                // Send command to get the current users settings.
        SendDriveOffsetGet();                   // Send command to get the Drive Offset.
        SendGetVersionCommand ();
        break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

