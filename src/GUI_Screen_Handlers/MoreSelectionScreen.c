//*****************************************************************************
// Filename: MoreSelectionScreen.c
// Description: This file handles the MORE screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local globals
//*************************************************************************************

GX_CHAR g_HeadArrayVersionString[20] = "ASL110: x.x.x";

//*************************************************************************************
// Function Name: MoreSelectionScreen_event_process
//
// Description: This functions handles the More Selection screen and dispatches
//      to the Diagnostic Screen or the Reset System screen.
//
//*************************************************************************************

UINT MoreSelectionScreen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        gx_prompt_text_set ((GX_PROMPT*)&MoreSelectionScreen.MoreSelectionScreen_VersionPrompt, ASL165_DispalyVersionString);
        sprintf (g_HeadArrayVersionString, "ASL110: %d.%d.%d", g_HA_Version_Major, g_HA_Version_Minor, g_HA_Version_Build);
        gx_prompt_text_set ((GX_PROMPT*)&MoreSelectionScreen.MoreSelectionScreen_HeadArray_VersionPrompt, g_HeadArrayVersionString);
        break;

    case GX_SIGNAL(GOTO_DIAGNOSTICS_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&DiagnosticScreen, window);
        break;

    case GX_SIGNAL(GOTO_RESET_SCREEN_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&ResetScreen, window);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
        break;

    } // end switch
    return gx_window_event_process(window, event_ptr);
}



