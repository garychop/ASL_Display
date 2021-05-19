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

UINT DisplayMainScreenActiveFeatures ();

//*************************************************************************************
// This function initializes the information used to display the information on
// the main user screen.
//*************************************************************************************

VOID Initialize_MainScreenInfo()
{
    // Populate the screen stuff.
    // "Power ON/OFF" information and description
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Available = true;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Location = 0;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_LargeDescriptionID = GX_STRING_ID_POWER_ONOFF; //"POWER ON/OFF"
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_SmallDescriptionID = GX_STRING_ID_POWER_ONOFF;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_SmallIcon = GX_PIXELMAP_ID_POWERICON_30X30;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_LargeIcon = GX_PIXELMAP_ID_POWERICON_LARGE;

    // "Bluetooth" information and description
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Available = true;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Location = 1;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_LargeDescriptionID = GX_STRING_ID_BLUETOOTH;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_SmallDescriptionID = GX_STRING_ID_BLUETOOTH;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_SmallIcon = GX_PIXELMAP_ID_BLUETOOTH_30X30;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_LargeIcon = GX_PIXELMAP_ID_BLUETOOTH_70X70;

    // "Next Function" information and description
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Available = true;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Location = 2;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_FUNCTION; // "NEXT FUNCTION")
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_FUNCTION;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_30X30;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_70X70;

    // "Next Profile" information and description
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Available = true;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Location = 3;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_PROFILE; // "NEXT PROFILE"
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_PROFILE;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_PROFILENEXT_30X30;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_PROFILENEXT_70X70;

    // "RNet SEATING" information and description
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = true;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Location = 4;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_SEATING;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_SEATING;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_SEATING_30X30;
    g_MainScreenFeatureInfo[RNET_SEATING_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_LOGO_70X70;

    // "SLEEP" information and description
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Enabled = true;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = true;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Location = 5;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_SLEEP;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_SLEEP;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_SLEEP_30X30;
    g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_LOGO_70X70;

    if (g_RNet_Active == false)
	{
		g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = false;
		g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Location = 0;
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

	// Adjust the displayed information based upon the RNet setting.
	// .. If RNet is enabled, the NEXT FUNCTION feature becomes RNet TOGGLE
	// .. and NEXT PROFILE feature become RNet MENU.
    if (g_RNet_Active)
	{
		// Display as "RNet TOGGLE"
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_TOGGLE;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_TOGGLE;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_TOGGLEFR_30X30;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_LOGO_70X70;

		// Display as "RNet MENU"
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_MENU;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_MENU;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_MENU_30X30;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_LOGO_70X70;
	}
	else
	{
		// Display as NEXT FUNCTION
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_FUNCTION; // "NEXT FUNCTION")
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_FUNCTION;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_30X30;
		g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_70X70;

		// Display as NEXT PROFILE
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_PROFILE; // "NEXT PROFILE"
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_PROFILE;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_PROFILENEXT_30X30;
		g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_PROFILENEXT_70X70;
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

	// Show the Fusion Logo if no features are enabled.... We are just driving.
	MainUserScreen.MainUserScreen_Fusion_Button.gx_widget_size.gx_rectangle_top = 60;
	MainUserScreen.MainUserScreen_Fusion_Button.gx_widget_size.gx_rectangle_right = 310;
	MainUserScreen.MainUserScreen_Fusion_Button.gx_widget_size.gx_rectangle_bottom = 160;
	if (enabledCount == 0)
		MainUserScreen.MainUserScreen_Fusion_Button.gx_widget_size.gx_rectangle_left = 10;
	else
		MainUserScreen.MainUserScreen_Fusion_Button.gx_widget_size.gx_rectangle_left = 400;

	// Now blank any unused items.
    for ( ; enabledCount < NUM_FEATURES; ++enabledCount)   // Start with the number of items that are enabled.
    {
        switch (enabledCount)
        {
        case 0: // Show the first line
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FirstPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FirstIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 1: // Show second line item
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_SecondPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_SecondIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 2: // Process third line item, move to the 2nd line
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_ThirdPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_ThirdIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 3: // Process fourth line item, move to the 3rd line.
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FourthPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FourthIcon, GX_PIXELMAP_ID_BLANK_30X30);
            break;
        case 4: // Show fifth line item
            myErr = gx_prompt_text_id_set (&MainUserScreen.MainUserScreen_FifthPrompt, GX_STRING_ID_BLANK);
            myErr = gx_icon_button_pixelmap_set (&MainUserScreen.MainUserScreen_FifthIcon, GX_PIXELMAP_ID_BLANK_30X30);
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
    g_ActiveScreen = (GX_WIDGET*) window;

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
	        screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
			g_ChangeScreen_WIP = true;
		}
		break;
//	case GX_EVENT_PEN_DOWN:	// We are going to determine if the Up or Down arrow buttons have been held for a
//							// ... long time (2 seconds) and goto Programming if so.
//
//		if ((event_ptr->gx_event_target->gx_widget_name == "DownArrowButton") || (event_ptr->gx_event_target->gx_widget_name == "UpArrowButton"))
//		{
//			gx_system_timer_start(window, ARROW_PUSHED_TIMER_ID, 100, 0);
//                g_ChangeScreen_WIP = false;
//		}
//		break;
//	case GX_EVENT_PEN_UP:
//		gx_system_timer_stop(window, ARROW_PUSHED_TIMER_ID);
//		break;
//
	case GX_EVENT_SHOW:
		DisplayMainScreenActiveFeatures();
		break;

	case GX_SIGNAL (LONG_PRESS_BUTTON_ID, GX_EVENT_CLICKED):
		// TODO: Add code to process long presses.
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

    case GX_SIGNAL (ATTENDANT_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&AttendantScreen, window);
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
	} // end switch

    myErr = gx_window_event_process(window, event_ptr);

	return myErr;
}

