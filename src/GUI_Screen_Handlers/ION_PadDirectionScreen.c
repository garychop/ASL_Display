//*****************************************************************************
// Filename: ION_PadDirectionScreen.cpp
//
// Date: Sept 13, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
// This file created based upon Fusion Pad Direction code.
//*****************************************************************************

#include "ASL165_System.h"
#include "DeviceInfo.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local Macros
//*************************************************************************************

#define X_OFFSET (0) // (20)	// "20" Needed for GUIX Demo, but not real display
#define Y_OFFSET (0) // (20)	// ditto

#define BTN_HEIGHT (70)
#define BTN_WIDTH (88)

// Left, Top, Right, Bottom
static GX_RECTANGLE g_ION_3Quadrant_ButtonLocation[] = {
	{27+X_OFFSET, 55+Y_OFFSET, 27+BTN_WIDTH+X_OFFSET, 55+BTN_HEIGHT+Y_OFFSET},		// Left Pad
	{205+X_OFFSET, 55+Y_OFFSET, 205+BTN_WIDTH+X_OFFSET, 55+BTN_HEIGHT+Y_OFFSET},	// Right Pad
	{116+X_OFFSET, 150+Y_OFFSET, 116+BTN_WIDTH+X_OFFSET, 150+BTN_HEIGHT+Y_OFFSET},	// Forward/Cetner Pad
	{0,0,0,0},	// Reverse Pad
	{0,0,0,0}};

// Left, Top, Right, Bottom
static GX_RECTANGLE g_ION_4Quadrant_ButtonLocation[] = {
	{27+X_OFFSET, 88+Y_OFFSET, 27+BTN_WIDTH+X_OFFSET, 88+BTN_HEIGHT+Y_OFFSET},		// Left Pad
	{205+X_OFFSET, 88+Y_OFFSET, 205+BTN_WIDTH+X_OFFSET, 88+BTN_HEIGHT+Y_OFFSET},	// Right Pad
	{116+X_OFFSET, 26+Y_OFFSET, 116+BTN_WIDTH+X_OFFSET, 26+BTN_HEIGHT+Y_OFFSET},	// Forward/Cetner Pad
	{116+X_OFFSET, 150+Y_OFFSET, 116+BTN_WIDTH+X_OFFSET, 150+BTN_HEIGHT+Y_OFFSET},	// Reverse Pad
	{0,0,0,0}};

//*************************************************************************************
// External References
//*************************************************************************************

//*************************************************************************************
// Local/Global variables
//*************************************************************************************
static const GX_RESOURCE_ID Pad_Direction_IDs [] = {
    GX_PIXELMAP_ID_PAD_OFF,
    GX_PIXELMAP_ID_LEFTWHITEARROW,
    GX_PIXELMAP_ID_UPWHITEARROW,
    GX_PIXELMAP_ID_RIGHTWHITEARROW,
    GX_PIXELMAP_ID_ARROWDOWN_88X70,
    GX_PIXELMAP_ID_MODE_88X70
};

static bool g_DidChange = false;
/*
 * Forward Declarations
 */
static void SelectNextDirection (DRIVER_QUADRANT_ENUM drvConfig, PAD_DIRECTION_ENUM *direction);
static void ShowPadDirectionalIcons ();

//*************************************************************************************
// Set Pad Directions in buttons
//*************************************************************************************
static void ShowPadDirectionalIcons ()
{
	if ((gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_3_QUADRANT) || (gp_ProgrammingDevice->m_DriverQuadrantSetting == DRIVER_2_QUADRANT))
	{
		// Locate and set LEFT pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_LeftPad_Button, &g_ION_3Quadrant_ButtonLocation[0]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_LeftPad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection]);
		// Locate and set RIGHT pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_RightPad_Button, &g_ION_3Quadrant_ButtonLocation[1]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_RightPad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection]);
		// Locate and set CENTER pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_CenterPad_Button, &g_ION_3Quadrant_ButtonLocation[2]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_CenterPad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection]);
		// Locate and set REVERSE pad, Hide this pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_ReversePad_Button, &g_ION_3Quadrant_ButtonLocation[3]);
	}
	else // Must be a 4 quadrant device.
	{
		// Locate and set LEFT pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_LeftPad_Button, &g_ION_4Quadrant_ButtonLocation[0]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_LeftPad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection]);
		// Locate and set RIGHT pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_RightPad_Button, &g_ION_4Quadrant_ButtonLocation[1]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_RightPad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection]);
		// Locate and set FORWARD/CENTER pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_CenterPad_Button, &g_ION_4Quadrant_ButtonLocation[2]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_CenterPad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection]);
		// Locate and set REVERSE pad
		gx_widget_resize ((GX_WIDGET*) &ION_PadDirectionScreen.ION_PadDirectionScreen_ReversePad_Button, &g_ION_4Quadrant_ButtonLocation[3]);
		gx_pixelmap_button_pixelmap_set (&ION_PadDirectionScreen.ION_PadDirectionScreen_ReversePad_Button, Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_PadDirection], Pad_Direction_IDs[gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_PadDirection]);
	}

}

//*************************************************************************************
// This function advances to the next direction based upon the 3/4 Configuration
//*************************************************************************************

static void SelectNextDirection (DRIVER_QUADRANT_ENUM drvConfig, PAD_DIRECTION_ENUM *direction)
{
	++*direction;
	if (drvConfig == DRIVER_3_QUADRANT)
	{
		if (*direction >= PAD_DIRECTION_INVALID)
			*direction = (PAD_DIRECTION_ENUM) 0;
		else if (*direction == PAD_DIRECTION_REVERSE)
			*direction = PAD_DIRECTION_MODE;
	}
	else // Must be 4 quadrant
	{
		if (*direction >= PAD_DIRECTION_INVALID)
			*direction = (PAD_DIRECTION_ENUM) 0;
	}
	g_DidChange = true;
}

//*************************************************************************************
// Function Name: SetPadDirectionScreen_event_process
//
// Description: This functions process the event of the Set Pad Direction screen.
//
//*************************************************************************************

UINT ION_PadDirectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	UINT myErr = GX_SUCCESS;
	//ION_PADDIRECTIONSCREEN_CONTROL_BLOCK *windowPtr = (ION_PADDIRECTIONSCREEN_CONTROL_BLOCK*) window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		// Show the correct number of pads and their settings according to Driver Configuration Setting (3 or 4 quadrant)
	    g_DidChange = false;
		ShowPadDirectionalIcons ();
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
	    if (g_DidChange)
	    {
	        SendDriverControlPadAssigments (gp_ProgrammingDevice->m_DriverConfiguration,
	                                        gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection,
                                            gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection,
                                            gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection,
                                            gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_PadDirection,
                                            gp_ProgrammingDevice->m_Mode_Switch_Schema);
	    }
	    screen_toggle(PopPushedWindow(), window);
		break;

	// Process LEFT button pushes
	case GX_SIGNAL(LEFT_PAD_BTN_ID, GX_EVENT_CLICKED):
		SelectNextDirection (gp_ProgrammingDevice->m_DriverQuadrantSetting, &gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_PadDirection);
		ShowPadDirectionalIcons ();
		break;
	
	// Process RIGHT button pushes
	case GX_SIGNAL(RIGHT_PAD_BTN_ID, GX_EVENT_CLICKED):
		SelectNextDirection (gp_ProgrammingDevice->m_DriverQuadrantSetting, &gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_PadDirection);
		ShowPadDirectionalIcons ();
		break;
	
	// Process CENTER PAD button pushes
	case GX_SIGNAL(CENTER_PAD_BTN_ID, GX_EVENT_CLICKED):
		SelectNextDirection (gp_ProgrammingDevice->m_DriverQuadrantSetting, &gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_PadDirection);
		ShowPadDirectionalIcons ();
		break;

	// Process CENTER PAD button pushes
	case GX_SIGNAL(REVERSE_PAD_BTN_ID, GX_EVENT_CLICKED):
		SelectNextDirection (gp_ProgrammingDevice->m_DriverQuadrantSetting, &gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_PadDirection);
		ShowPadDirectionalIcons ();
		break;

	}

	myErr = gx_window_event_process(window, event_ptr);

	return myErr;
}

