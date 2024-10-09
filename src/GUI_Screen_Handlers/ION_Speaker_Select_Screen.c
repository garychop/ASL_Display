//*************************************************************************
//
// (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
//
// All rights reserved. This file is the intellectual property of ASL and it may
// not be disclosed to others or used for any purposes without the written consent of ASL.
//
//*****************************************************************************
// Filename: ION_BT_SetupScreen.cpp
//
// Date: July 28, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"

//*************************************************************************************
// Macros
//*************************************************************************************

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

//*************************************************************************************
// Description: This handles the ION Bluetooth Device Setup Screen messages
//*************************************************************************************

UINT ION_Speaker_Select_Screen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
	//ION_SPEAKER_SELECT_SCREEN_CONTROL_BLOCK *SetupWindowPtr = (ION_SPEAKER_SELECT_SCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
		screen_toggle(PopPushedWindow(), window);	// This should go back to the ION Main Programming Window
		break;

	case GX_SIGNAL(EXT_SPKR_BTN_ID, GX_EVENT_CLICKED):
		g_SpeakerIndex = EXTERNAL_SPEAKER_E;
		PushWindow(window);
		screen_toggle((GX_WINDOW*)&ION_AuditorySettingsScreen, window);
		break;

	case GX_SIGNAL (INT_SPKR_BTN_ID, GX_EVENT_CLICKED):
		g_SpeakerIndex = INTERNAL_SPEAKER_E;
		PushWindow(window);
		screen_toggle((GX_WINDOW*)&ION_AuditorySettingsScreen, window);
		break;

	} // end switch

    gx_window_event_process(window, event_ptr);

	return 0;
}













