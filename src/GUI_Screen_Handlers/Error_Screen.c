//*****************************************************************************
// Filename: Error_Screen.c
// Description: This file displays the screen that handles the Pad Errors
//
// Date: Nov 8, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Global Variables
//*************************************************************************************

const GX_RECTANGLE g_ErrorPadLocations[] = {
    {55, 36, 55+62, 36+98},         // Left Pad location
    {204, 36, 204+62, 36+98},       // Right Pad location
    {89, 146, 89+145, 146+42},      // Center Pad Location
    {0,0,0,0}};

//*************************************************************************************
// Out of Neutral Test screen processing.
//*************************************************************************************

VOID Error_Screen_draw_function (GX_WINDOW *window)
{
    g_ActiveScreen = (GX_WIDGET*) window;

    if (g_PadSettings[LEFT_PAD].m_PadStatus == PAD_STATUS_ERROR)
        gx_widget_resize ((GX_WIDGET*) &Error_Screen.Error_Screen_LeftPadError_Icon , &g_ErrorPadLocations[0]);

    if (g_PadSettings[RIGHT_PAD].m_PadStatus == PAD_STATUS_ERROR)
        gx_widget_resize ((GX_WIDGET*) &Error_Screen.Error_Screen_RightPadError_Icon , &g_ErrorPadLocations[1]);

    if (g_PadSettings[CENTER_PAD].m_PadStatus == PAD_STATUS_ERROR)
        gx_widget_resize ((GX_WIDGET*) &Error_Screen.Error_Screen_CenterPadError_Icon , &g_ErrorPadLocations[2]);

    gx_window_draw(window);
}


