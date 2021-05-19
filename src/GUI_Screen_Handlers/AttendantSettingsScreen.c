//*****************************************************************************
// Filename: AttendantSettingsScreen.cpp
//
// Date: May 16, 2021
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

//*************************************************************************************
// Forward Declarations
//*************************************************************************************


//*************************************************************************************
// Function Name: FeatureSettingsScreen_event_process
//
// Description: This handles the Feature Settings Screen messages
//
//*************************************************************************************

UINT AttendantSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{

    switch (event_ptr->gx_event_type)
	{
    case GX_EVENT_SHOW:
        // D0 = 1 = Active
        if (g_AttendantSettings & 0x01)
        {
            gx_button_select ((GX_BUTTON*) &AttendantSettingsScreen.AttendantSettingsScreen_ActiveToggleBtn);
        }
        else
        {
            gx_button_deselect ((GX_BUTTON*) &AttendantSettingsScreen.AttendantSettingsScreen_ActiveToggleBtn, true);
        }
        // D1 = 0 = Proportional
        if (g_AttendantSettings & 0x02)
        {
            gx_button_select ((GX_BUTTON*) &AttendantSettingsScreen.AttendantSettingsScreen_ProportionalToggleBtn);
        }
        else
        {
            gx_button_deselect ((GX_BUTTON*) &AttendantSettingsScreen.AttendantSettingsScreen_ProportionalToggleBtn, false);
        }

        // Override
        if (g_AttendantSettings & 0x04)
        {
            gx_button_deselect ((GX_BUTTON*) &AttendantSettingsScreen.AttendantSettingsScreen_OverrideToggleBtn, false);
        }
        else
        {
            gx_button_select ((GX_BUTTON*) &AttendantSettingsScreen.AttendantSettingsScreen_OverrideToggleBtn);
        }

        break;

    // Active toggle button processing
    case GX_SIGNAL(ACTIVE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
        g_AttendantSettings |= 0x01;        // D0 = 1 = active
        break;
    case GX_SIGNAL(ACTIVE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
        g_AttendantSettings &= 0xfe;        // D0 = 1 = active, 0 = inactive
        break;

    // Proportional toggle button processing
    case GX_SIGNAL(PROPORTIONAL_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
        g_AttendantSettings |= 2;           // D1 = 1 = Proporiotnal, 0 = Digital
        break;
    case GX_SIGNAL(PROPORTIONAL_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
        g_AttendantSettings &= 0xfd;    // D1 = 0 = Digital
        break;

    // Override toggle button processing
    case GX_SIGNAL(OVERRIDE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_ON):
        g_AttendantSettings &= 0xfb;        // D2 = 0 = ASSIST
        break;
    case GX_SIGNAL(OVERRIDE_TOGGLE_BTN_ID, GX_EVENT_TOGGLE_OFF):
        g_AttendantSettings |= 0x04;        // D2 = 1 = OVERRIDE
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        SendAttendantSettingsSet_toHeadArray (g_AttendantSettings, g_AttendantTimeout);
        screen_toggle((GX_WINDOW *)&PerformanceSelectionScreen, window);
        break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}











