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

VOID DiagnosticScreen_draw_event (GX_WINDOW *window)
{
    uint8_t pad;

    for (pad=0; pad<3; ++pad)
    {
        // Show the Raw and Demand values
        // Unfortunately, I have to use a "Global" or "Const" string with the gx_prompt_text_set function instead
        // of a local string variable in this function. It actually sends a pointer to the function and
        // not a copy of the string. That means that the last information is applied to all
        // gx_prompt_text_set calls.
        sprintf (g_PadSettings[pad].m_DriveDemandString, "%3d", g_PadSettings[pad].m_Proportional_DriveDemand);
        gx_prompt_text_set (g_PadSettings[pad].m_AdjustedValuePrompt, g_PadSettings[pad].m_DriveDemandString);
        sprintf (g_PadSettings[pad].m_RawValueString, "%3d", g_PadSettings[pad].m_Proportional_RawValue);
        gx_prompt_text_set (g_PadSettings[pad].m_RawValuePrompt, g_PadSettings[pad].m_RawValueString);
        if (g_PadSettings[pad].m_PadDirection == OFF_DIRECTION)
        {
            gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticOff_Widget , &g_PadSettings[pad].m_DiagnosticWidigetLocation);
            gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticProportional_Widget , &g_HiddenRectangle);
            gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticDigital_Widget , &g_HiddenRectangle);
        }
        else
        {
            if (g_PadSettings[pad].m_PadType == PROPORTIONAL_PADTYPE)
            {
                gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticProportional_Widget , &g_PadSettings[pad].m_DiagnosticWidigetLocation);
                gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticDigital_Widget , &g_HiddenRectangle);
            }
            else if (g_PadSettings[pad].m_PadType == DIGITAL_PADTYPE)
            {
                gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticDigital_Widget , &g_PadSettings[pad].m_DiagnosticWidigetLocation);
                gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticProportional_Widget , &g_HiddenRectangle);
            }
            else
            {
                gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticProportional_Widget , &g_HiddenRectangle);
                gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticDigital_Widget , &g_HiddenRectangle);
            }
        }
    }

    // Now show the status of the USER PORT
    if ((g_HeadArrayStatus1 & 0x40) == 0x40)
        gx_icon_button_pixelmap_set (&DiagnosticScreen.DiagnosticScreen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_ON);
    else
        gx_icon_button_pixelmap_set (&DiagnosticScreen.DiagnosticScreen_UserPort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);

    // Show the status of the MODE PORT
    if ((g_HeadArrayStatus1 & 0x80) == 0x80)
        gx_icon_button_pixelmap_set (&DiagnosticScreen.DiagnosticScreen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_ON);
    else
        gx_icon_button_pixelmap_set (&DiagnosticScreen.DiagnosticScreen_ModePort_IconButton, GX_PIXELMAP_ID_RADIOBUTTON_OFF);

    gx_window_draw(window);
}

//*************************************************************************************

UINT DiagnosticScreen_event_handler(GX_WINDOW *window, GX_EVENT *event_ptr)
{
    int pads;

    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:
            for (pads = 0; pads < 3; ++pads)
            {
                gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DiagnosticDigital_Widget, &g_HiddenRectangle);
                gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DiagnosticProportional_Widget, &g_HiddenRectangle);
                if (g_PadSettings[pads].m_PadDirection == OFF_DIRECTION)
                    gx_widget_resize ((GX_WIDGET*)g_PadSettings[pads].m_DiagnosticOff_Widget, &g_PadSettings[pads].m_DiagnosticWidigetLocation);
                else
                    gx_widget_resize ((GX_WIDGET*)g_PadSettings[pads].m_DiagnosticOff_Widget, &g_HiddenRectangle);
            }
            SendGetDataCommand (START_SENDING_DATA, END_OF_PAD_ENUM);
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
                screen_toggle((GX_WINDOW *)&MoreSelectionScreen, window);
            SendGetDataCommand (STOP_SENDING_DATA, END_OF_PAD_ENUM);
            break;

    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


