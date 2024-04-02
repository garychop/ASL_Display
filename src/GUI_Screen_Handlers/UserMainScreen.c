//*****************************************************************************
// Filename: UserMainScreen.c
// Description: This file displays the User's Main Screen and handling
//	the front panel button pushes.
//
// Date: Aug 28, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "UserMainScreen.h"
#include "QueueDefinition.h"


//*************************************************************************************
// Forward Declarations
//*************************************************************************************

void DisplayPadStatus (PAD_STATUS_COLORS center_pad, PAD_STATUS_COLORS right_pad, PAD_STATUS_COLORS left_pad);
UINT DisplayMainScreenActiveFeatures ();

//*************************************************************************************
// This function initializes the information used to display the information on
// the main user screen.
//*************************************************************************************

VOID Initialize_MainScreenInfo()
{
    // Populate the screen stuff.
    // "DRIVE" information and description
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_HB_ID = DRIVE_MODE_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_Available = true;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_Location = 0;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_LargeDescriptionID = GX_STRING_ID_DRIVE_FEATURE;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_SmallDescriptionID = GX_STRING_ID_DRIVE_FEATURE;
    //g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_SmallIcon = GX_PIXELMAP_ID_DRIVEFEATUREICON_30X30;
    //g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_LargeIcon = GX_PIXELMAP_ID_DRIVEFEATUREICON_LARGE;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_SmallIcon = GX_PIXELMAP_ID_DRIVEFEATUREICON_30X30_GREEN;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_LargeIcon = GX_PIXELMAP_ID_DRIVEFEATUREICON_LARGE_GREEN;
    g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_FontColorID = GX_COLOR_ID_WHITE;

    // "Power ON/OFF" information and description
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_HB_ID = POWERONOFF_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = false;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Available = true;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Location = 0;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_LargeDescriptionID = GX_STRING_ID_POWER_ONOFF; //"POWER ON/OFF"
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_SmallDescriptionID = GX_STRING_ID_POWER_ONOFF;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_SmallIcon = GX_PIXELMAP_ID_POWERICON_30X30_ORANGE;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_LargeIcon = GX_PIXELMAP_ID_POWERICON_LARGE_ORANGE;

    // "Bluetooth" information and description
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_HB_ID = BLUETOOTH_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_Enabled = false;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_Available = true;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_Location = 1;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_LargeDescriptionID = GX_STRING_ID_BLUETOOTH;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_SmallDescriptionID = GX_STRING_ID_BLUETOOTH;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_SmallIcon = GX_PIXELMAP_ID_BLUETOOTH_30X30_ALLBLUE;
    g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_LargeIcon = GX_PIXELMAP_ID_BLUETOOTH_70X70_BLUE;

    // "Next Function" information and description
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_HB_ID = NEXT_FUNCTION_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = false;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Available = true;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Location = 2;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_FUNCTION; // "NEXT FUNCTION")
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_FUNCTION;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_30X30_YELLOW;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_70X70_YELLOWA;

    // "Next Profile" information and description
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_HB_ID = NEXT_PROFILE_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = false;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Available = true;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Location = 3;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_PROFILE; // "NEXT PROFILE"
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_PROFILE;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_PROFILE_30X30_K;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_PROFILE_70X70_K;

    // "RNet SEATING" information and description
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_HB_ID = RNET_MENU_SEATING_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Enabled = false;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = true;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Location = 4;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_SEATING;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_SEATING;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_SEATINGB_30X30;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_SEATINGB_70X70;

    // "SLEEP" information and description
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_HB_ID = RNET_SLEEP_FEATURE_HB_ID;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Enabled = false;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = true;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Location = 5;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_SLEEP;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_SLEEP;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_SLEEPB_30X30;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_SLEEPB_70X70;

    if (g_RNet_Active == false)
	{
		g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = false;
		g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Location = -1;
        g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = false;
        g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Location = -1;
	}

}

//*************************************************************************************

void AdjustActiveFeaturePositions (FEATURE_ID_ENUM newMode)
{
    uint8_t featureCount, lineNumber, featureIdx;

    // Adjust available features based upon RNet setting.
    if (g_RNet_Active)
    {
        g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = true;
        g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = true;
    }
    else
    {
        g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = false;
        g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = false;
    }

    if (newMode >= NUM_FEATURES)    // Check for valid mode
        return;

    g_ActiveFeature = newMode;          // Store the active feature in global var.
    // Locate the Feature Information and set an Index to point to it.
    for (featureCount = 0; featureCount < NUM_FEATURES; ++featureCount)
    {
        if (g_MainScreenFeatureInfo[featureCount].m_HB_ID == newMode)
        {
            featureIdx = featureCount;
            break;
        }
    }

    lineNumber = 0;                     // Need to keep track of which line is next.

    for (featureCount = 0; featureCount < NUM_FEATURES; ++featureCount)
    {
        if ((g_MainScreenFeatureInfo[featureIdx].m_Enabled) && (g_MainScreenFeatureInfo[featureIdx].m_Available))
        {
            g_MainScreenFeatureInfo[featureIdx].m_Location = lineNumber;
            ++lineNumber;
        }
        else
        {
            g_MainScreenFeatureInfo[featureIdx].m_Location = -1;
        }
        ++featureIdx;               // Look at the next feature information.
        if (featureIdx >= NUM_FEATURES)         // Rollover
            featureIdx = 0;
    }
}

//*************************************************************************************
// Displays the Pad Status based upon the passed parameters
//*************************************************************************************
void DisplayPadStatus (PAD_STATUS_COLORS center_pad, PAD_STATUS_COLORS right_pad, PAD_STATUS_COLORS left_pad)
{
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_Green);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_Orange);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_White);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_Green);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_Orange);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_White);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_Green);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_Orange);
    gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_White);

    switch (center_pad)
    {
    case PAD_OFF:
        break;
    case PAD_GREEN:     // Only the Proximity Sensor is active.
        gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_Green);
        break;
    case PAD_ORANGE:    // Only paint orange if its set to Proportional and both
                        // .. the Proximity, Digital sensor and the Pressure
                        // .. sensor are active.
        if (g_PadSettings[CENTER_PAD].m_PadType == DIGITAL_PADTYPE)
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_Green);
        else
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_Orange);
        break;
    case PAD_WHITE: // If it's only the Pressure Sensor, then paint White only
                    // .. if the PAD TYPE is set to Proportional
        if (g_PadSettings[CENTER_PAD].m_PadType == PROPORTIONAL_PADTYPE)
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_CenterPad_White);
        break;
    }

    switch (left_pad)
    {
    case PAD_OFF:
        break;
    case PAD_GREEN:
        gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_Green);
        break;
    case PAD_ORANGE:    // Only paint orange if its set to Proportional
        if (g_PadSettings[LEFT_PAD].m_PadType == DIGITAL_PADTYPE)
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_Green);
        else
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_Orange);
        break;
    case PAD_WHITE:
        if (g_PadSettings[LEFT_PAD].m_PadType == PROPORTIONAL_PADTYPE)
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_LeftPad_White);
        break;
    }

    switch (right_pad)
    {
    case PAD_OFF:
        break;
    case PAD_GREEN:
        gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_Green);
        break;
    case PAD_ORANGE:    // Only paint orange if its set to Proportional
        if (g_PadSettings[RIGHT_PAD].m_PadType == DIGITAL_PADTYPE)
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_Green);
        else
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_Orange);
        break;
    case PAD_WHITE:
        if (g_PadSettings[RIGHT_PAD].m_PadType == PROPORTIONAL_PADTYPE)
            gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HAStatus_RightPad_White);
        break;
    }
}

//*************************************************************************************
// Function Name: DisplayMainScreenActiveFeatures
//
// Description: This displays the features that are active in the order specificed
//	in the Screen Prompts "objects".
//
//*************************************************************************************

UINT DisplayMainScreenActiveFeatures ()
{
	int enabledCount;
	int feature;
	UINT myErr = GX_SUCCESS;

	AdjustActiveFeaturePositions (g_ActiveFeature);   // This function also store "g_ActiveFeature" if appropriate.

	// Adjust the displayed information based upon the RNet setting.
	// .. If RNet is enabled, the NEXT FUNCTION feature becomes RNet TOGGLE
	// .. and NEXT PROFILE feature become RNet MENU.
    if (g_RNet_Active)
	{
		// Display as "RNet TOGGLE"
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_TOGGLE;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_TOGGLE;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_TOGGLEB_30X30;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_TOGGLEA_70X70;

		// Display as "RNet MENU"
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_MENU;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_MENU;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_MENUB_30X30;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_MENUB_70X70;
	}
	else
	{
		// Display as NEXT FUNCTION
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_FUNCTION; // "NEXT FUNCTION")
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_FUNCTION;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_30X30_YELLOW;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_70X70_YELLOWA;

		// Display as NEXT PROFILE
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_PROFILE; // "NEXT PROFILE"
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_PROFILE;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_PROFILE_30X30_K;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_PROFILE_70X70_K;
	}

    // Count the number of active items so we can populate appropriately.
    // Hide the Non-Active features.
    enabledCount = 0;

    // Locate the first feature to display
    for (feature = 0; feature < NUM_FEATURES; ++feature)
    {
		if (g_MainScreenFeatureInfo[feature].m_Enabled && g_MainScreenFeatureInfo[feature].m_Available)
        {
            ++enabledCount;
            switch (g_MainScreenFeatureInfo[feature].m_Location)
            {
            case 0: // Show the first line
                myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FirstPrompt, g_MainScreenFeatureInfo[feature].m_LargeDescriptionID);
                myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FirstIcon, g_MainScreenFeatureInfo[feature].m_LargeIcon);
                break;
            case 1: // Show second line item
                myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_SecondPrompt, g_MainScreenFeatureInfo[feature].m_SmallDescriptionID);
                myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_SecondIcon, g_MainScreenFeatureInfo[feature].m_SmallIcon);
                break;
            case 2: // Show third line item
                myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_ThirdPrompt, g_MainScreenFeatureInfo[feature].m_SmallDescriptionID);
                myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_ThirdIcon, g_MainScreenFeatureInfo[feature].m_SmallIcon);
                break;
            case 3: // Show fourth line item
                myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FourthPrompt, g_MainScreenFeatureInfo[feature].m_SmallDescriptionID);
                myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FourthIcon, g_MainScreenFeatureInfo[feature].m_SmallIcon);
                break;
            case 4: // Show fifth line item
                myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FifthPrompt, g_MainScreenFeatureInfo[feature].m_SmallDescriptionID);
                myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FifthIcon, g_MainScreenFeatureInfo[feature].m_SmallIcon);
                break;
            default:
                break;
            }
        }
    }

    // If no features are enabled, simply draw the FUSION logo.
	if (enabledCount == 0)
	    gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_Fusion_Button);
    else
        gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_Fusion_Button);

	// Now blank any unused items.
    for ( ; enabledCount < NUM_FEATURES; ++enabledCount)   // Start with the number of items that are enabled.
    {
        switch (enabledCount)
        {
        case 0: // Hide the first line
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FirstPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FirstIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 1: // Hide second line item
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_SecondPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_SecondIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 2: // Hide the 3rd line
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_ThirdPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_ThirdIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 3: // Hide the 4th line
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FourthPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FourthIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 4: // Hide the fifth line item
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FifthPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FifthIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        default:
            break;
        } // end of switch
    } // end of for

    // Show or Hide Attendant Icon
    if (g_AttendantSettings & 0x01)
    {
        gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_Attendant_Button);
    }
    else
    {
        gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_Attendant_Button);
    }

    if (g_ShowPadsOnMainScreen == true) // FEATURE_ENABLED)
    {
        // Show the wire-representation of the Head Array.
        gx_widget_show ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HA_Status);

        // Display the Pad Status based upon info from the ASL110 Heartbeat message
        DisplayPadStatus (g_PadSettings[CENTER_PAD].m_PadSensorStatus, g_PadSettings[RIGHT_PAD].m_PadSensorStatus, g_PadSettings[LEFT_PAD].m_PadSensorStatus);
    }
    else
    {
        gx_widget_hide ((GX_WIDGET*) &MainUserScreen.MainUserScreen_HA_Status);
    }

    return myErr;
}

//*************************************************************************************
// Function Name: MainUserScreen_event_process
//
// Description: This handles the User Screen messages.
//
//*************************************************************************************

VOID MainUserScreen_draw_function(GX_WINDOW *window)
{
    DisplayMainScreenActiveFeatures();  // Redraw the items.

    gx_window_draw(window);
}

//*************************************************************************************
// Function Name: Main_User_Screen_event_process
//
// Description: This handles the User Screen messages.
//
//*************************************************************************************
UINT MainUserScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	UINT myErr = GX_SUCCESS;
	uint8_t feature;
	int activeCount;

	switch (event_ptr->gx_event_type)
	{
	    case GX_EVENT_TIMER:
        if (event_ptr->gx_event_payload.gx_event_timer_id == ARROW_PUSHED_TIMER_ID)
		{
            if (g_WhoAmi == I_AM_FUSION)
                screen_toggle((GX_WINDOW*)&HHP_Start_Screen, window);
            else
                screen_toggle((GX_WINDOW*)&ION_BT_DeviceSelectionScreen, window);
			g_ChangeScreen_WIP = true;
		}
		break;

    case GX_EVENT_SHOW:
        g_ActiveScreen = (GX_WIDGET*) window;
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL (ATTENDANT_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&AttendantScreen, window);
        break;

	case GX_SIGNAL (LONG_PRESS_BUTTON_ID, GX_EVENT_CLICKED):
		DisplayMainScreenActiveFeatures();
        //Use the following to show Out Of Neutral screen
		//screen_toggle((GX_WINDOW *)&OON_Screen, window);
		break;

    case GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
        // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
        if (g_ChangeScreen_WIP)
        {
            g_ChangeScreen_WIP = false;
            break;
        }
        // Count the number of active features to set a limit on location
        activeCount = 0;
        for (feature = 0; feature < NUM_FEATURES; ++feature)
        {
            if ((g_MainScreenFeatureInfo[feature].m_Enabled) && (g_MainScreenFeatureInfo[feature].m_Available))
                ++activeCount;
        }
        // Move Top Feature to Bottom and move Bottom upward.
        for (feature = 0; feature < NUM_FEATURES; ++feature)
        {
            if ((g_MainScreenFeatureInfo[feature].m_Enabled) && (g_MainScreenFeatureInfo[feature].m_Available))
            {
                if (g_MainScreenFeatureInfo[feature].m_Location == 0)
                    g_MainScreenFeatureInfo[feature].m_Location = activeCount-1;
                else if (g_MainScreenFeatureInfo[feature].m_Location == 1)
                {
                    g_MainScreenFeatureInfo[feature].m_Location = 0;
                    SendModeChangeCommand (feature);        // Send this to the Head Array
                }
                else if (g_MainScreenFeatureInfo[feature].m_Location == 2)
                    g_MainScreenFeatureInfo[feature].m_Location = min (1, activeCount-1);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 3)
                    g_MainScreenFeatureInfo[feature].m_Location = min (2, activeCount-1);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 4)
                    g_MainScreenFeatureInfo[feature].m_Location = min (3, activeCount-1);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 5)
                    g_MainScreenFeatureInfo[feature].m_Location = min (4, activeCount-1);
            }
        }
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
        // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
        if (g_ChangeScreen_WIP)
        {
            g_ChangeScreen_WIP = false;
            break;
        }
        // Count the number of active features to set a limit on location
        activeCount = 0;
        for (feature = 0; feature < NUM_FEATURES; ++feature)
        {
            if ((g_MainScreenFeatureInfo[feature].m_Enabled) && (g_MainScreenFeatureInfo[feature].m_Available))
                ++activeCount;
        }
        --activeCount;  // Translate the Number of items to Based Zero line number.

        // Move the features downward, limiting the movement by the number of Active Features.
        for (feature = 0; feature < NUM_FEATURES; ++feature)
        {
            if ((g_MainScreenFeatureInfo[feature].m_Enabled) && (g_MainScreenFeatureInfo[feature].m_Available))
            {
                if (g_MainScreenFeatureInfo[feature].m_Location == activeCount)
                {
                    g_MainScreenFeatureInfo[feature].m_Location = 0;
                    SendModeChangeCommand (feature);        // Send this to the Head Array
                }
                else if (g_MainScreenFeatureInfo[feature].m_Location == 0)
                    g_MainScreenFeatureInfo[feature].m_Location = min (1, activeCount);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 1)
                    g_MainScreenFeatureInfo[feature].m_Location = min (2, activeCount);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 2)
                    g_MainScreenFeatureInfo[feature].m_Location = min (3, activeCount);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 3)
                    g_MainScreenFeatureInfo[feature].m_Location = min (4, activeCount);
                else if (g_MainScreenFeatureInfo[feature].m_Location == 4)
                    g_MainScreenFeatureInfo[feature].m_Location = min (5, activeCount);
            }
        }
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
        g_ChangeScreen_WIP = true;
        break;

    case GX_SIGNAL (HB_TIMEOUT_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&StartupSplashScreen, window);
        break;

    case GX_SIGNAL (POWER_OFF_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&ReadyScreen, window);
        break;

    case GX_SIGNAL (HB_OON_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&OON_Screen, window);
        break;

    case GX_SIGNAL (PAD_ERROR_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&Error_Screen, window);
        break;

    case GX_SIGNAL (GOTO_BT_SUBMENU_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&ION_BT_UserSelectionScreen, window);
        break;
	} // end switch

    myErr = gx_window_event_process(window, event_ptr);

    return myErr;
}

