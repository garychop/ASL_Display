//*****************************************************************************
// Filename: PadScreens.c
// Description: This file handles the Pad Options and settings screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local Macros
//*************************************************************************************

static GX_RESOURCE_ID Pad_Direction_IDs [] = {GX_PIXELMAP_ID_PAD_OFF, GX_PIXELMAP_ID_LEFTWHITEARROW, GX_PIXELMAP_ID_UPWHITEARROW, GX_PIXELMAP_ID_RIGHTWHITEARROW, GX_PIXELMAP_ID_RIGHTWHITEARROW};


#define GRAPH_CENTER_PT_XPOS 139    // From Left of screen
#define GRAPH_CENTER_PT_YPOS 130    // From Top of screen

#define MINIMUM_CALIBRATION_WINDOW (15) // This is minimum value between the MIN and MAX calibration values.

//*************************************************************************************
// Local declarations.
//*************************************************************************************

RGB16_Struct g_Color;
GX_WINDOW *g_CalibrationScreen = GX_NULL;
bool g_TimerActive = false;             // I'm using to prevent re-arming the timer used to enter Pad Calibration Feature.

int g_CalibrationPadNumber;
int g_CalibrationStepNumber;

GX_RECTANGLE g_PadDirectionLocation[] = {
    {28, 55, 28+88, 55+70},
    {205, 55, 205+88, 55+70},
    {116, 150, 116+88, 150+70},
    {0,0,0,0}};

GX_RECTANGLE g_CalibrationPadLocations[] = {
    {36, 32, 36+62, 32+98},         // Left Pad location
    {184, 32, 184+62, 32+98},       // Right Pad location
    {67, 140, 67+145, 140+42},      // Center Pad Location
    {0,0,0,0}};

GX_RECTANGLE g_CalibrationPromptLocations[] = {
    {20, 4, 38+239, 4+33},          // Max and Min Prompt location
    {GRAPH_CENTER_PT_XPOS-25, GRAPH_CENTER_PT_YPOS-26-60, GRAPH_CENTER_PT_XPOS-25+50, GRAPH_CENTER_PT_YPOS-26-60+26},       // Pad Value prompt location
    {0,0,0,0}};

//*************************************************************************************
// Forward Function Declarations
//*************************************************************************************

void ConvertMinimumValue (uint8_t minSpeed, char *valueString);
void IncrementMinimumSpeed (uint8_t *minSpeed);
void ShowPadTypes (void);


//*************************************************************************************
// Function Name: PadOptionsSettingsScreen_event_process
//
// Description: This dispatches the Pad Option Settings Screen messages
//
//*************************************************************************************

UINT PadOptionsSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{

    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            break;

        case GX_SIGNAL(PAD_TYPE_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&SetPadTypeScreen, window);
            break;

        case GX_SIGNAL(PAD_DIRECTIONS_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&SetPadDirectionScreen, window);
            break;

        case GX_SIGNAL(MINIMUM_DRIVE_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&MinimumDriveScreen, window);
            break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: MinimumDriveScreen_event_process
//
// Description: This dispatches the Pad Option Settings Screen messages
//
//*************************************************************************************

void ConvertMinimumValue (uint8_t minSpeed, char *valueString)
{
    if (minSpeed == 0)
    {
        strcpy (valueString, "OFF");
    }
    else
    {
        sprintf (valueString, "%d%%", minSpeed);
    }
}

//*************************************************************************************
// This increments from 0 (off) to MAXIMUM_DRIVE SPEED starting at 15 and incrementing
// .. by 5.
//*************************************************************************************

void IncrementMinimumSpeed (uint8_t *minSpeed)
{
    // If the Head Array has newer firmware then the upper range is 40.
    if (g_HA_EEPROM_Version >= 5)
    {
        if (*minSpeed == 0)
            *minSpeed = 15;
        else if (*minSpeed >= MAXIMUM_DRIVE_SPEED_NEW_FIRMWARE)
            *minSpeed = 0;
        else
            *minSpeed += 5;
    }
    else    // It must be older Head Array firmware. The upper range is 30.
    {
        if (*minSpeed == 0)
            *minSpeed = 15;
        else if (*minSpeed >= MAXIMUM_DRIVE_SPEED_OLD_FIRMWARE)
            *minSpeed = 0;
        else
            *minSpeed += 5;
    }
}

//*************************************************************************************
// Function Name: MinimumDriveScreen_event_process
//
// Description: This dispatches the Pad Option Settings Screen messages
//
//*************************************************************************************
UINT MinimumDriveScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    bool needNewValue = false;

    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:   // The SHOW doesn't allow the screen to updated with the returned Head Array value.... DRAW does... see above.
            if (g_HA_EEPROM_Version >= 5)       // Firmware version 5 uses 3 drive offset values.
            {
                // Display the values in the 3 buttons
                ConvertMinimumValue (g_PadSettings[CENTER_PAD].m_MinimumDriveValue, g_PadSettings[CENTER_PAD].m_MinimuDriveString);
                gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button, g_PadSettings[CENTER_PAD].m_MinimuDriveString);
                ConvertMinimumValue (g_PadSettings[LEFT_PAD].m_MinimumDriveValue, g_PadSettings[LEFT_PAD].m_MinimuDriveString);
                gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_LeftPadPercentage_Button, g_PadSettings[LEFT_PAD].m_MinimuDriveString);
                ConvertMinimumValue (g_PadSettings[RIGHT_PAD].m_MinimumDriveValue, g_PadSettings[RIGHT_PAD].m_MinimuDriveString);
                gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_RightPadPercentage_Button, g_PadSettings[RIGHT_PAD].m_MinimuDriveString);
            }
            else    // Old school, Only one value is shown since the ASL110 firmware supports only one Minimum Drive value.
            {
                gx_widget_hide (&MinimumDriveScreen.MinimumDriveScreen_RightPadPercentage_Button);  // Hide the right pad button
                gx_widget_hide (&MinimumDriveScreen.MinimumDriveScreen_LeftPadPercentage_Button);   // Hide the Left pad button
                gx_widget_hide (&MinimumDriveScreen.MinimumDriveScreen_Prompt_ForEachPad);          // Hide the "FOR EACH PAD" prompt
                // Move the "SET MINIMUM DRIVE SPEED" prompt down for better appearance.
                MinimumDriveScreen.MinimumDriveScreen_Prompt_SetMinimumSpeed.gx_widget_size.gx_rectangle_top = 12;
                MinimumDriveScreen.MinimumDriveScreen_Prompt_SetMinimumSpeed.gx_widget_size.gx_rectangle_bottom = 44;   // Height + top: 32 + 12
                // Move the Center Pad button to the middle, upper part of the screen.
                MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button.gx_widget_size.gx_rectangle_left = 116;
                MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button.gx_widget_size.gx_rectangle_right = 116 + 80; // left + width
                MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button.gx_widget_size.gx_rectangle_top = 55;
                MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button.gx_widget_size.gx_rectangle_bottom = 55 + 64; // top + height
                // Show the value in the button.
                ConvertMinimumValue (g_PadSettings[CENTER_PAD].m_MinimumDriveValue, g_PadSettings[CENTER_PAD].m_MinimuDriveString);
                gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button, g_PadSettings[CENTER_PAD].m_MinimuDriveString);
            }
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
            break;

        case GX_SIGNAL (CENTER_PAD_PERCENTAGE_BTN_ID, GX_EVENT_CLICKED):
            IncrementMinimumSpeed (&g_PadSettings[CENTER_PAD].m_MinimumDriveValue);
            ConvertMinimumValue (g_PadSettings[CENTER_PAD].m_MinimumDriveValue, g_PadSettings[CENTER_PAD].m_MinimuDriveString);
            gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_CenterPadPercentage_Button, g_PadSettings[CENTER_PAD].m_MinimuDriveString);
            needNewValue = true;
            break;

        case GX_SIGNAL (LEFT_PAD_PERCENTAGE_BTN_ID, GX_EVENT_CLICKED):
            IncrementMinimumSpeed (&g_PadSettings[LEFT_PAD].m_MinimumDriveValue);
            ConvertMinimumValue (g_PadSettings[LEFT_PAD].m_MinimumDriveValue, g_PadSettings[LEFT_PAD].m_MinimuDriveString);
            gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_LeftPadPercentage_Button, g_PadSettings[LEFT_PAD].m_MinimuDriveString);
            needNewValue = true;
            break;

        case GX_SIGNAL (RIGHT_PAD_PERCENTAGE_BTN_ID, GX_EVENT_CLICKED):
            IncrementMinimumSpeed (&g_PadSettings[RIGHT_PAD].m_MinimumDriveValue);
            ConvertMinimumValue (g_PadSettings[RIGHT_PAD].m_MinimumDriveValue, g_PadSettings[RIGHT_PAD].m_MinimuDriveString);
            gx_text_button_text_set (&MinimumDriveScreen.MinimumDriveScreen_RightPadPercentage_Button, g_PadSettings[RIGHT_PAD].m_MinimuDriveString);
            needNewValue = true;
            break;

    }   // end switch

    // If the values changed, let's send the new value and request them (again) from the Head Array. This will set the
    // value if the Head Array validated the new value.
    if (needNewValue)
    {
        SendDriveOffsetSet (g_PadSettings[CENTER_PAD].m_MinimumDriveValue, g_PadSettings[LEFT_PAD].m_MinimumDriveValue, g_PadSettings[RIGHT_PAD].m_MinimumDriveValue);
        SendDriveOffsetGet ();
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: SetPadDirectionScreen_event_process
//
// Description: This functions process the event of the Set Pad Direction screen.
//
//*************************************************************************************

VOID SetPadDirectionScreen_draw_function (GX_WINDOW *window)
{
    UINT pads;
    GX_RESOURCE_ID myIcon;

    for (pads = 0; pads < 3; ++pads)
    {
        myIcon = Pad_Direction_IDs[g_PadSettings[pads].m_PadDirection];
        gx_pixelmap_button_pixelmap_set (g_PadSettings[pads].m_DirectionIcons, myIcon, myIcon, myIcon);
    }

    gx_window_draw(window);

#ifdef USE_ORIGINAL_CODE
    UINT pads, icons;

    for (pads = 0; pads < 3; ++pads)            // for each pad
    {
        for (icons = 0; icons < 5; ++icons)     // for each option, "hide" the icons.
        {
            gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DirectionIcons[icons], &g_HiddenRectangle);
        }
        // OK, now show the correct icon
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DirectionIcons[g_PadSettings[pads].m_PadDirection], &g_PadDirectionLocation[pads]);
    }

    gx_window_draw(window);
#endif // #ifdef USE_ORIGINAL_CODE
}

//*************************************************************************************

UINT SetPadDirectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    UINT pads;
    GX_RESOURCE_ID myIcon;

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        for (pads = 0; pads < 3; ++pads)
        {
            myIcon = Pad_Direction_IDs[0]; // This is the OFF icon
            gx_pixelmap_button_pixelmap_set (g_PadSettings[pads].m_DirectionIcons, myIcon, myIcon, myIcon);
        }
        SendGetPadAssignmentMsg (LEFT_PAD);
        SendGetPadAssignmentMsg (RIGHT_PAD);
        SendGetPadAssignmentMsg (CENTER_PAD);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
        break;

    // Process LEFT button pushes
    case GX_SIGNAL(LEFT_PAD_BTN_ID, GX_EVENT_CLICKED):
        ++g_PadSettings[LEFT_PAD].m_PadDirection;
        if (g_PadSettings[LEFT_PAD].m_PadDirection >= INVALID_DIRECTION)
            g_PadSettings[LEFT_PAD].m_PadDirection = OFF_DIRECTION;
        SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
        SendGetPadAssignmentMsg (LEFT_PAD);
        break;

    // Process RIGHT button pushes
    case GX_SIGNAL(RIGHT_PAD_BTN_ID, GX_EVENT_CLICKED):
        ++g_PadSettings[RIGHT_PAD].m_PadDirection;
        if (g_PadSettings[RIGHT_PAD].m_PadDirection >= INVALID_DIRECTION)
            g_PadSettings[RIGHT_PAD].m_PadDirection = OFF_DIRECTION;
        SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
        SendGetPadAssignmentMsg (RIGHT_PAD);
        break;

    // Process CENTER PAD button pushes
    case GX_SIGNAL(CENTER_PAD_BTN_ID, GX_EVENT_CLICKED):
        ++g_PadSettings[CENTER_PAD].m_PadDirection;
        if (g_PadSettings[CENTER_PAD].m_PadDirection >= INVALID_DIRECTION)
            g_PadSettings[CENTER_PAD].m_PadDirection = OFF_DIRECTION;
        SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
        SendGetPadAssignmentMsg (CENTER_PAD);
        break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: SetPadTypeScreen_event_process
//
// Description: This handles the Set Pad Screen messages
//
//*************************************************************************************
void ShowPadTypes (void)
{
    if (g_PadSettings[LEFT_PAD].m_PadType == DIGITAL_PADTYPE)  // Digital?
    {
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
    }
    if (g_PadSettings[RIGHT_PAD].m_PadType == DIGITAL_PADTYPE) // Digital?
    {
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
    }
    if (g_PadSettings[CENTER_PAD].m_PadType == DIGITAL_PADTYPE)    // Digital?
    {
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadProportional_Button);
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadDigital_Button);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadProportional_Button);
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadDigital_Button);
    }
}

VOID SetPadTypeScreen_Draw_Function (GX_WINDOW *window)
{
    ShowPadTypes();

    gx_window_draw(window);
}

UINT SetPadTypeScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:
            g_ChangeScreen_WIP = false;
            // Hide all buttons/icons. THe "Get Pad Assignment" response will issue a "redraw" this screen after it gets the current settings from the Head Array.
            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadProportional_Button);
            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadDigital_Button);
            SendGetPadAssignmentMsg (LEFT_PAD);
            SendGetPadAssignmentMsg (RIGHT_PAD);
            SendGetPadAssignmentMsg (CENTER_PAD);
            g_TimerActive = false;
            break;
        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
            break;
        case GX_SIGNAL(RIGHT_PAD_DIGITAL_BTN_ID, GX_EVENT_CLICKED):
            if (!g_ChangeScreen_WIP)
            {
                g_PadSettings[RIGHT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
                SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
                SendGetPadAssignmentMsg (RIGHT_PAD);
                g_TimerActive = false;
            }
            break;
        case GX_SIGNAL(RIGHT_PAD_PROPORTIONAL_BTN_ID, GX_EVENT_CLICKED):
            if (!g_ChangeScreen_WIP)
            {
                g_PadSettings[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
                SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
                SendGetPadAssignmentMsg (RIGHT_PAD);
                g_TimerActive = false;
            }
            break;
        case GX_SIGNAL(LEFT_PAD_DIGITAL_BTN_ID, GX_EVENT_CLICKED):
            if (!g_ChangeScreen_WIP)
            {
                g_PadSettings[LEFT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
                SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
                SendGetPadAssignmentMsg (LEFT_PAD);
                g_TimerActive = false;
            }
            break;
        case GX_SIGNAL(LEFT_PAD_PROPORTIONAL_BTN_ID, GX_EVENT_CLICKED):
            if (!g_ChangeScreen_WIP)
            {
                g_PadSettings[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
                SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
                SendGetPadAssignmentMsg (LEFT_PAD);
                g_TimerActive = false;
            }
            break;
        case GX_SIGNAL(CENTER_PAD_DIGITAL_BTN_ID, GX_EVENT_CLICKED):
            if (!g_ChangeScreen_WIP)
            {
                g_PadSettings[CENTER_PAD].m_PadType = PROPORTIONAL_PADTYPE;
                SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
                SendGetPadAssignmentMsg (CENTER_PAD);
                g_TimerActive = false;
            }
            break;
        case GX_SIGNAL(CENTER_PAD_PROPORTIONAL_BTN_ID, GX_EVENT_CLICKED):
            if (!g_ChangeScreen_WIP)
            {
                g_PadSettings[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
                SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
                SendGetPadAssignmentMsg (CENTER_PAD);
                g_TimerActive = false;
            }
            break;

        case GX_EVENT_TIMER:
            if (event_ptr->gx_event_payload.gx_event_timer_id == CALIBRATION_TIMER_ID)
            {
                gx_system_timer_stop(window, CALIBRATION_TIMER_ID);
                screen_toggle((GX_WINDOW *)&PadCalibrationScreen, window);
                g_ChangeScreen_WIP = true;
                g_TimerActive = false;
            }
            break;
        case GX_EVENT_PEN_DOWN: // We are going to determine if the Up or Down arrow buttons have been held for a
                                // ... long time (2 seconds) and goto calibration if so.

            if (g_TimerActive == false)
            {
                if (event_ptr->gx_event_target->gx_widget_id == CENTER_PAD_PROPORTIONAL_BTN_ID)
                {
                    g_CalibrationPadNumber = CENTER_PAD;
                    gx_system_timer_start(window, CALIBRATION_TIMER_ID, 100, 0);
                    g_TimerActive = true;
                }
                else if (event_ptr->gx_event_target->gx_widget_id == LEFT_PAD_PROPORTIONAL_BTN_ID)
                {
                    g_CalibrationPadNumber = LEFT_PAD;
                    gx_system_timer_start(window, CALIBRATION_TIMER_ID, 100, 0);
                    g_TimerActive = true;
                }
                else if (event_ptr->gx_event_target->gx_widget_id == RIGHT_PAD_PROPORTIONAL_BTN_ID)
                {
                    g_CalibrationPadNumber = RIGHT_PAD;
                    gx_system_timer_start(window, CALIBRATION_TIMER_ID, 100, 0);
                    g_TimerActive = true;
                }
            }
            break;
        case GX_EVENT_PEN_UP:
            gx_system_timer_stop(window, CALIBRATION_TIMER_ID);
            g_TimerActive = false;
            break;

    } // end swtich

//    ShowPadTypes();

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


//*************************************************************************************
// Function Name: CalibrationScreen_draw
//
// Description: This callback function is called when Drawing is required by GUIX.
//      This function draws the Guage.
//
//*************************************************************************************
char myValStr[16];

VOID CalibrationScreen_draw (GX_WINDOW *window)
{
    GX_BRUSH *brush;
    GX_BRUSH originalBrush;
    INT raw100, pieSide;
    uint16_t padValue;
    float f1, f2, f3, f4;

    gx_window_draw(window);

    gx_context_brush_get(&brush);
    originalBrush = *brush;

    // Draw the background
    brush->gx_brush_line_color = 0xffffff;  // GX_COLOR_LIGHTGRAY;
    brush->gx_brush_width = 1;
    g_Color.rgb.red = 0x10;      // Trying to make gray
    g_Color.rgb.blue = 0x10;
    g_Color.rgb.green = 0x20;
    brush->gx_brush_fill_color = g_Color.gx_color;
            // BLUE 0xc001010ff; // 0x808080;  // GX_COLOR_DARKGRAY;

    gx_canvas_pie_draw (GRAPH_CENTER_PT_XPOS, GRAPH_CENTER_PT_YPOS, 55, -5, 185);   // This draws the outside pad a little bigger to show the minimum better.
    //gx_canvas_pie_draw (GRAPH_CENTER_PT_XPOS, GRAPH_CENTER_PT_YPOS, 55, 0, 180);

    // Draw the Pad pie
    if (g_PadSettings[g_CalibrationPadNumber].m_Proportional_RawValue > 0)             // Anything less than 175-180 is too small of a pie to see; if it's 180 it draws a full circle.
    {
        // Use the pad max ADC and adjust to 100%.
        f1 = (float) g_PadSettings[g_CalibrationPadNumber].m_Maximum_ADC_Threshold;
        f2 = 100.00f / f1;      // Create percentage of ADC threshold.
        f3 = (float) g_PadSettings[g_CalibrationPadNumber].m_Proportional_RawValue;
        f4 = f3 * f2;
        padValue = (uint16_t) f4;

        // John Mattes sanity check.
        if (padValue < 1)
            padValue = 1;
        else if (padValue > 100)
            padValue = 100;

        raw100 = 100 - padValue;
        raw100 *= 100;                  // Integer math, yuch!
        raw100 *= 18;                   // This converts the percentage to degrees which is a factor of 1.8
        pieSide = raw100 / 1000;        // This is includes the decimal shift.
        brush->gx_brush_width = 2;
        g_Color.rgb.red = 0;
        g_Color.rgb.blue = 0;
        g_Color.rgb.green = 0x20;
        brush->gx_brush_fill_color = g_Color.gx_color; //  GX_COLOR_GREEN;
        brush->gx_brush_line_color = g_Color.gx_color;
        gx_context_brush_set(brush);        // Not really required. It seems to change the color to yellow without this call.
        gx_canvas_pie_draw (GRAPH_CENTER_PT_XPOS, GRAPH_CENTER_PT_YPOS, 54, pieSide, 180);
    }

    // Draw the minimum pie
    // Calculate the position of the upper side of the pie.
    // The arc is drawn as follows:
    //       0 degrees = 3:00 clock time
    //      90 degrees = 12:00 noon clock time
    //      180 degrees = 9:00 clock time
    //      270 degrees = 6:00 clock time.
    raw100 = 100 - g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue;
    raw100 *= 100;                  // Integer math, yuch!
    raw100 *= 18;                   // This converts the percentage to degrees which is a factor of 1.8
    pieSide = raw100 / 1000;        // This is includes the decimal shift.
    if (pieSide > 175)              // Anything less than 175-180 is too small of a pie to see.
        pieSide = 175;
    brush->gx_brush_fill_color = GX_COLOR_YELLOW;   // Draw in yellow.
    brush->gx_brush_line_color = GX_COLOR_YELLOW;
    brush->gx_brush_width = 1;
    gx_context_brush_set(brush);        // Not really required. It seems to change the color to yellow without this call.
    gx_canvas_pie_draw (GRAPH_CENTER_PT_XPOS, GRAPH_CENTER_PT_YPOS, 40, pieSide, 180);

    // Draw the Maximum Pie
    raw100 = 100 - g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue;      // Integer math, yuch!
    raw100 *= 100;
    raw100 *= 18;                   // This converts the percentage to degrees which is a factor of 1.8
    pieSide = raw100 / 1000;        // This is includes the decimal shift.
    if (pieSide < 5)                        // Anything less than 0-5 is too small of a sliver to see.
        pieSide = 5;
    g_Color.rgb.red = 0b11111;
    g_Color.rgb.blue = 0;
    g_Color.rgb.green = 0b01011;
    brush->gx_brush_fill_color = g_Color.gx_color; //  GX_COLOR_GREEN;
    brush->gx_brush_line_color = g_Color.gx_color;
    brush->gx_brush_width = 1;
    gx_context_brush_set(brush);        // Not really required. It seems to change the color to yellow without this call.
    gx_canvas_pie_draw (GRAPH_CENTER_PT_XPOS, GRAPH_CENTER_PT_YPOS, 40, 0, pieSide);

    *brush = originalBrush;
}

//*************************************************************************************
// Function Name: CalibrationScreen_event_process
//
// Description: This handles the Set Pad Screen messages
//
//*************************************************************************************

UINT CalibrationScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_MaximumInstructionsText, &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_MinimumInstructionsText, &g_CalibrationPromptLocations[0]);

        g_CalibrationStepNumber = 0;    // "0" = Setting Minimum, "1" = Setting Maximum value.

        gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);

        // show the PAD under calibration and hide the other PADs.
        gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_CenterPadON_Button, &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_LeftPadON_Button, &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_RightPadON_Button, &g_HiddenRectangle);
        switch (g_CalibrationPadNumber)
        {
        case LEFT_PAD:
            gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_LeftPadON_Button, &g_CalibrationPadLocations[0]);
            break;
        case RIGHT_PAD:
            gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_RightPadON_Button, &g_CalibrationPadLocations[1]);
            break;
        case CENTER_PAD:
            gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_CenterPadON_Button, &g_CalibrationPadLocations[2]);
            break;
        } // end switch (g_CalibrationPadNumber)

        SendCalibrationStartCommand();      // The Head Array to enter the "Calibrate state"; this allows the pad to be activated without driving the wheelchair.

        SendGetDataCommand (START_SENDING_DATA, g_CalibrationPadNumber);    // Start asking the Head Array for data.

        g_CalibrationScreen = window;       // Store for use by screen update process.
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        if (g_CalibrationStepNumber == 0)           // Let's do maximum calibration
        {
            gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_MaximumInstructionsText, &g_CalibrationPromptLocations[0]);
            gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_MinimumInstructionsText, &g_HiddenRectangle);
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
            ++g_CalibrationStepNumber;
        }
        else if (g_CalibrationStepNumber == 1)
        {
            screen_toggle((GX_WINDOW *)&SetPadTypeScreen, window);

            SendGetDataCommand (STOP_SENDING_DATA, g_CalibrationPadNumber);     // We will stop asking for data from the head array.

            SendCalibrationStopCommand();           // This tells the Head Array to EXIT Calibration Mode.

            // Tell the head array what the new cal values are.
            SendCalibrationData (g_CalibrationPadNumber, (uint16_t) g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue,
                    (uint16_t) g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
        }
        break;
    case GX_SIGNAL(DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (g_CalibrationStepNumber == 0)       // We are doing minimum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue > 2)
                --g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);
        }
        else if (g_CalibrationStepNumber == 1)  // Doing maximum
        {
            if ((g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue > 2) && (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue - g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue > MINIMUM_CALIBRATION_WINDOW))
                --g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
        }
        gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        break;
    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (g_CalibrationStepNumber == 0)       // We are doing minimum
        {
            if ((g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue < 100) && (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue - g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue > MINIMUM_CALIBRATION_WINDOW))
                ++g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);
        }
        else if (g_CalibrationStepNumber == 1)  // Doing maximum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue < 100)
                ++g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
        }
        gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        break;

    }
    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

