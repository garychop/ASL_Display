//*****************************************************************************
// Filename: ION_Diag_4QuadScreen.cpp
//
// Date: Oct 24, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "DeviceInfo.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local Macros
//*************************************************************************************

//*************************************************************************************
// External References
//*************************************************************************************

extern int g_SNP_Nozzle_Value;

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

void DisplayDirectionCapabilities(void);

//*************************************************************************************
// This function shows the directions icons
//*************************************************************************************
void DisplayDirectionCapabilities()
{
	if (gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_2_QUADRANT)
	{
		gx_widget_hide((GX_WIDGET*)&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_FWD);
		gx_widget_hide((GX_WIDGET*)&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_REV);
	}
	else // else we are going to display 4 quadrant information.
	{
		gx_widget_show((GX_WIDGET*)&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_FWD);
		gx_widget_show((GX_WIDGET*)&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_REV);
	}
}

//*************************************************************************************
// THis function displays the status of the Directions
//*************************************************************************************

void ION_Diag_4Quad_Screen_draw_event(GX_WINDOW *window)
{
    // If we doing a 4-Quadrant device (SNP or 4-Switch), then show the FORWARD and REVERSE pad status
    if (gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_4_QUADRANT)
	{
		if (g_PadSettings[CENTER_PAD].m_PadSensorStatus)
			gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_FWD, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALON, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALON);
		else
			gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_FWD, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALOFF, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALOFF);

		if (g_PadSettings[REVERSE_PAD].m_PadSensorStatus)
			gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_REV, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALON, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALON);
		else
			gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_REV, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALOFF, GX_PIXELMAP_ID_DIAG4QUAD_VERTICALOFF);
	}

	if (g_PadSettings[RIGHT_PAD].m_PadSensorStatus)
		gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_RIGHT, GX_PIXELMAP_ID_DIAG4QUAD_HORIZON, GX_PIXELMAP_ID_DIAG4QUAD_HORIZON);
	else
		gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_RIGHT, GX_PIXELMAP_ID_DIAG4QUAD_HORIZOFF, GX_PIXELMAP_ID_DIAG4QUAD_HORIZOFF);

	if (g_PadSettings[LEFT_PAD].m_PadSensorStatus)
		gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_LEFT, GX_PIXELMAP_ID_DIAG4QUAD_HORIZON, GX_PIXELMAP_ID_DIAG4QUAD_HORIZON);
	else
		gx_icon_pixelmap_set(&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_LEFT, GX_PIXELMAP_ID_DIAG4QUAD_HORIZOFF, GX_PIXELMAP_ID_DIAG4QUAD_HORIZOFF);

    // Now show the status of the USER PORT
    if ((g_HeadArrayStatus1 & 0x40) == 0x40)
        gx_icon_button_pixelmap_set (&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_ON);
    else
        gx_icon_button_pixelmap_set (&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);

    // Show the status of the MODE PORT
    if ((g_HeadArrayStatus1 & 0x80) == 0x80)
        gx_icon_button_pixelmap_set (&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_ON);
    else
        gx_icon_button_pixelmap_set (&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);

    gx_window_draw(window);

}

//*************************************************************************************
// Function Name: DiagnosticScreen_event_handler
//
// Description: This handles the Diagnostic Screen messages
//
//*************************************************************************************

UINT ION_Diag_4Quad_Screen_event_handler(GX_WINDOW *window, GX_EVENT *event_ptr)
{
	//ION_DIAG_4QUAD_SCREEN_CONTROL_BLOCK *windowPtr = (ION_DIAG_4QUAD_SCREEN_CONTROL_BLOCK*) window;

    switch (event_ptr->gx_event_type)
    {
	case GX_EVENT_SHOW:
		DisplayDirectionCapabilities();
        Send_DiagnosticCommand (1); // "1" disables commands to the Wheelchair
		//ION_Diag_4Quad_Screen_draw_event (window);
		//gx_icon_button_pixelmap_set (&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);
		//gx_icon_button_pixelmap_set (&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
		// The following is a 'hack' that allows the FWD and REV icons to be displayed after the 2-Switch 
		// .. screen is used. For some odd reason, the FWD and REV icons could not be seen in the
		// .. 4 Quadrant Diagnostics otherwise.
		gx_widget_show((GX_WIDGET*)&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_FWD);
		gx_widget_show((GX_WIDGET*)&ION_Diag_4Quad_Screen.ION_Diag_4Quad_Screen_Diag_REV);
        Send_DiagnosticCommand (0); // "0" disables commands to the Wheelchair
		screen_toggle(PopPushedWindow(), window);
		break;
    } // end switch

    return gx_window_event_process(window, event_ptr);
}

