//*****************************************************************************
// Filename: DiagnosticScreen.c
// Description: This file handles the Diagnostics.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Function Name: DiagnosticScreen_event_handler
//
// Description: This handles the Diagnostic Screen messages
//
//*************************************************************************************

VOID ION_HeadArray_DiagnosticScreen_draw_event (GX_WINDOW *window)
{
    // Show the status of each of the 3 pads.
    if (g_PadSettings[CENTER_PAD].m_PadSensorStatus)
    {
        gx_widget_show (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_CenterPadDigital_Button);
        gx_widget_hide (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_CenterPadOff_Button);
    }
    else
    {
        gx_widget_hide (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_CenterPadDigital_Button);
        gx_widget_show (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_CenterPadOff_Button);
    }
    if (g_PadSettings[RIGHT_PAD].m_PadSensorStatus)
    {
        gx_widget_show (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_RightPadDigital_Button);
        gx_widget_hide (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_RightPadOff_Button);
    }
    else
    {
        gx_widget_hide (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_RightPadDigital_Button);
        gx_widget_show (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_RightPadOff_Button);
    }
    if (g_PadSettings[LEFT_PAD].m_PadSensorStatus)
    {
        gx_widget_show (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_LeftPadDigital_Button);
        gx_widget_hide (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_LeftPadOff_Button);
    }
    else
    {
        gx_widget_hide (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_LeftPadDigital_Button);
        gx_widget_show (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_LeftPadOff_Button);
    }

    // Now show the status of the USER PORT
    if ((g_HeadArrayStatus1 & 0x40) == 0x40)
        gx_icon_button_pixelmap_set (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_ON);
    else
        gx_icon_button_pixelmap_set (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);

    // Show the status of the MODE PORT
    if ((g_HeadArrayStatus1 & 0x80) == 0x80)
        gx_icon_button_pixelmap_set (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_ON);
    else
        gx_icon_button_pixelmap_set (&ION_HeadArray_DiagnosticScreen.ION_HeadArray_DiagnosticScreen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);

    gx_window_draw(window);
}

//*************************************************************************************

UINT ION_HeadArray_DiagnosticScreen_event_handler(GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:
            Send_DiagnosticCommand (1); // "1" disables commands to the Wheelchair
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            Send_DiagnosticCommand (0); // "0" disables commands to the Wheelchair
            screen_toggle(PopPushedWindow(), window);
            break;

    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


