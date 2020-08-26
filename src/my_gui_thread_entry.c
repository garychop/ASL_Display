//*************************************************************************
// my_gui_thread_entry.
//
//  Created on: Nov 21, 2019
//      Author: Gary Chopcinski, Kg Solutions, LLC
//
//  This file contains the execution for the GUI message processing, screen
//  handling and message handling from the Communication Task.
//
//*************************************************************************

#include "my_gui_thread.h"
#include "lcd.h"
#include <my_gui_thread_entry.h>
#include <stdio.h>
#include <math.h>
#include "gx_api.h"
#include "ASL_HHP_Display_GUIX_resources.h"
#include "ASL_HHP_Display_GUIX_specifications.h"
#include "QueueDefinition.h"
#include "HeadArray_CommunicationThread.h"

//-------------------------------------------------------------------------
// Typedefs and defines
//-------------------------------------------------------------------------

enum ENUM_TIMER_IDS {ARROW_PUSHED_TIMER_ID = 1, CALIBRATION_TIMER_ID, PAD_ACTIVE_TIMER_ID};

#define min(a,b)   ((a < b) ? a : b)

#define GRAPH_CENTER_PT_XPOS 139    // From Left of screen
#define GRAPH_CENTER_PT_YPOS 130    // From Top of screen

#define MAXIMUM_DRIVE_SPEED_OLD_FIRMWARE (30)
#define MAXIMUM_DRIVE_SPEED_NEW_FIRMWARE (40)

//-------------------------------------------------------------------------
// Local variables
//-------------------------------------------------------------------------

GX_RECTANGLE g_HiddenRectangle = {0,0,0,0};

GX_RECTANGLE g_DiagnosticWidgetLocations[] = {
    {35, 32, 35+62, 32+90},
    {183, 32, 183+62, 32+90},
    {66, 140, 66+145, 140+42}};

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

// Timeout information
uint8_t g_TimeoutValue;
typedef struct TIMEOUT_STRUCT
{
    GX_PIXELMAP_BUTTON *m_TimeoutIcon;
    uint8_t m_TimeoutValue;
} TimeoutInfo_S;

// This is used for setting the colors in the Calibration Pies, but can be used generically.
typedef struct myColorS
{
    union
    {
        GX_COLOR gx_color;
        struct
        {
            GX_COLOR blue : 5;
            GX_COLOR green : 6;
            GX_COLOR red : 5;
            GX_COLOR spare : 16;
        } rgb;
    };
} RGB16_Struct;
RGB16_Struct g_Color;

struct MainScreenFeatureInfo_struct
{
    int m_Location;     // This indicates the Main Screen location, 0=Top most, 3=bottom most
    int m_Enabled;      // Indicates if this feature is active.
    GX_RESOURCE_ID m_SmallIcon;
    GX_RESOURCE_ID m_LargeIcon;
    GX_RESOURCE_ID m_SmallDescriptionID;
    GX_RESOURCE_ID m_LargeDescriptionID;
} g_MainScreenFeatureInfo[NUM_FEATURES];

struct PadInfoStruct
{
    enum PAD_TYPE m_PadType;
    enum PAD_DIRECTION m_PadDirection;
    uint8_t m_MinimumDriveValue;
    char m_MinimuDriveString[8];
    int16_t m_PadMinimumCalibrationValue;
    int16_t m_PadMaximumCalibrationValue;
    uint16_t m_Minimum_ADC_Threshold;
    uint16_t m_Maximum_ADC_Threshold;
    GX_PIXELMAP_BUTTON *m_DiagnosticOff_Widget;
    GX_PIXELMAP_BUTTON *m_DiagnosticDigital_Widget;
    GX_PIXELMAP_BUTTON *m_DiagnosticProportional_Widget;
    GX_RECTANGLE m_DiagnosticWidigetLocation;
    GX_PIXELMAP_BUTTON *m_DirectionIcons[5];
    GX_PROMPT *m_RawValuePrompt;
    GX_PROMPT *m_AdjustedValuePrompt;
    uint16_t m_Proportional_RawValue;
    uint16_t m_Proportional_DriveDemand;
    GX_CHAR m_DriveDemandString[8];         // Unfortunately, I have to use a "Global" or "Const" string with the gx_prompt_text_set function instead
    GX_CHAR m_RawValueString[8];            // of a local string variable in this function. It actually sends a pointer to the function and
                                            // not a copy of the string. That means that the last information is applied to all
                                            // gx_prompt_text_set calls.
} g_PadSettings[3];

//-------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------

GX_CHAR ASL165_DispalyVersionString[32] = "See in code below";
GX_CHAR g_HeadArrayVersionString[20] = "";
uint8_t g_HA_Version_Major, g_HA_Version_Minor, g_HA_Version_Build, g_HA_EEPROM_Version;

int g_SettingsChanged;
int g_CalibrationPadNumber;
int g_CalibrationStepNumber;
int g_ClicksActive = FALSE;
int g_PowerUpInIdle = false;
int16_t g_StartupDelayCounter = 0;
int g_ChangeScreen_WIP;
FEATURE_ID_ENUM g_ActiveFeature = POWER_ONOFF_ID;     // this indicates the active feature.
GX_WINDOW *g_GoBackScreen = GX_NULL;
GX_WINDOW *g_CalibrationScreen = GX_NULL;
GX_WIDGET *g_ActiveScreen = GX_NULL;
GX_WINDOW_ROOT * p_window_root;
//bool g_UseNewPrompt = false;
char g_SliderValue[20] = "gc";
int16_t g_NeutralDAC_Constant = 2048;
int16_t g_NeutralDAC_Setting = 2048;
int16_t g_NeutralDAC_Range = 400;
bool g_WaitingForVeerResponse = false;
char g_MinimuDriveString[8] = "20%";
char g_TimeoutValueString[8] = "OFF";
UINT g_TimerActive = false;             // I'm using to prevent re-arming the timer used to enter Pad Calibration Feature.

extern const sf_touch_panel_i2c_cfg_t g_sf_touch_panel_i2c0_cfg_extend;
extern const sf_touch_panel_i2c_chip_t g_sf_touch_panel_i2c_chip_sx8651;

//-------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------
void my_gui_thread_entry(void);

static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload);
VOID screen_toggle(GX_WINDOW *new_win, GX_WINDOW *old_win);

#ifdef OK_TO_USE_RESET
static void reset_check(void);
#endif

UINT SettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT UserSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT FeatureSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT SetPadTypeScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT CalibrationScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT SetPadDirectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
void ConvertMinimumValue (uint8_t minSpeed, char *valueString);
void IncrementMinimumSpeed (uint8_t *minSpeed);

UINT DisplayMainScreenActiveFeatures ();
void AdjustActiveFeature (FEATURE_ID_ENUM newMode);
void CreateEnabledFeatureStatus(uint8_t *myActiveFeatures);
void ShowPadTypes (void);
void ProcessCommunicationMsgs ();

//-------------------------------------------------------------------------
/* Gui Test App Thread entry function */
void my_gui_thread_entry(void)
{
    ssp_err_t err;
    UINT status = TX_SUCCESS;

		//debug pins
    g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(TEST_PIN, IOPORT_LEVEL_LOW);
    
//    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);
  	g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);
	g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);

//    gx_system_focus_claim(p_first_screen);
    R_BSP_SoftwareDelay(250, BSP_DELAY_UNITS_MILLISECONDS);

    /** Open the SPI driver to initialize the LCD **/
    err = g_rspi_lcdc.p_api->open(g_rspi_lcdc.p_ctrl, g_rspi_lcdc.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

    // Open the I2C driver for CLOCK
    err = g_i2c1.p_api->open(g_i2c1.p_ctrl, g_i2c1.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

    // Open the ADC driver for Thermistor
    err = g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

        g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);


    // Setup the ILI9341V LCD Driver and Touchscreen.
    ILI9341V_Init();

    if (g_sf_touch_panel_i2c0_cfg_extend.p_chip == &g_sf_touch_panel_i2c_chip_sx8651)
    {
        strcpy (ASL165_DispalyVersionString, "ASL165: 1.8.0 [B]");
    }
    else
    {
        strcpy (ASL165_DispalyVersionString, "ASL165: 1.8.0 [A]");
    }

    /* Initializes GUIX. */
    status = gx_system_initialize();
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

    /* Initializes GUIX drivers. */
    err = g_sf_el_gx0.p_api->open (g_sf_el_gx0.p_ctrl, g_sf_el_gx0.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

    gx_studio_display_configure ( MAIN_DISPLAY,
                                  g_sf_el_gx0.p_api->setup,
                                  LANGUAGE_ENGLISH,
                                  MAIN_DISPLAY_THEME_1,
                                  &p_window_root );

    err = g_sf_el_gx0.p_api->canvasInit(g_sf_el_gx0.p_ctrl, p_window_root);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

    // Populate the screen stuff.
    // "Power ON/OFF" information and description
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = TRUE;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Location = 0;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_LargeDescriptionID = GX_STRING_ID_POWER_ONOFF; //"POWER ON/OFF"
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_SmallDescriptionID = GX_STRING_ID_POWER_ONOFF;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_SmallIcon = GX_PIXELMAP_ID_POWERICON_30X30;
    g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_LargeIcon = GX_PIXELMAP_ID_POWERICON_LARGE;

    // "Bluetooth" information and description
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Enabled = TRUE;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Location = 1;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_LargeDescriptionID = GX_STRING_ID_BLUETOOTH;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_SmallDescriptionID = GX_STRING_ID_BLUETOOTH;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_SmallIcon = GX_PIXELMAP_ID_BLUETOOTH_30X30;
    g_MainScreenFeatureInfo[BLUETOOTH_ID].m_LargeIcon = GX_PIXELMAP_ID_BLUETOOTH_70X70;

    // "Next Function" information and description
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = TRUE;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Location = 2;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_FUNCTION; // "NEXT FUNCTION")
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_FUNCTION;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_SmallIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_30X30;
    g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_LargeIcon = GX_PIXELMAP_ID_FUNCTIONNEXT_70X70;

    // "Next Profile" information and description
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = TRUE;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Location = 3;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeDescriptionID = GX_STRING_ID_NEXT_PROFILE; // "NEXT PROFILE"
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallDescriptionID = GX_STRING_ID_NEXT_PROFILE;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_SmallIcon = GX_PIXELMAP_ID_PROFILENEXT_30X30;
    g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_LargeIcon = GX_PIXELMAP_ID_PROFILENEXT_70X70;

    // "RNet Seating" information and description
    g_MainScreenFeatureInfo[RNET_ID].m_Enabled = FALSE;
    g_MainScreenFeatureInfo[RNET_ID].m_Location = 3;
    g_MainScreenFeatureInfo[RNET_ID].m_LargeDescriptionID = GX_STRING_ID_RNET_SEATING;
    g_MainScreenFeatureInfo[RNET_ID].m_SmallDescriptionID = GX_STRING_ID_RNET_SEATING;
    g_MainScreenFeatureInfo[RNET_ID].m_SmallIcon = GX_PIXELMAP_ID_RNET_SEATING_30X30;
    g_MainScreenFeatureInfo[RNET_ID].m_LargeIcon = GX_PIXELMAP_ID_RNET_LOGO_70X70;

    // Populate the default Pad settings.
    g_PadSettings[LEFT_PAD].m_PadDirection = INVALID_DIRECTION;
    g_PadSettings[LEFT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[LEFT_PAD].m_MinimumDriveValue = 20;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[OFF_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Off_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[RIGHT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_RightArrow_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[LEFT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_LeftArrow_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[FORWARD_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_ForwardArrow_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[INVALID_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Question_Button;
    g_PadSettings[LEFT_PAD].m_PadMinimumCalibrationValue = 0;
    g_PadSettings[LEFT_PAD].m_PadMaximumCalibrationValue = 100;
    g_PadSettings[LEFT_PAD].m_Minimum_ADC_Threshold = 50;
    g_PadSettings[LEFT_PAD].m_Maximum_ADC_Threshold = 220;
    g_PadSettings[LEFT_PAD].m_DiagnosticWidigetLocation = g_DiagnosticWidgetLocations[LEFT_PAD];
    g_PadSettings[LEFT_PAD].m_DiagnosticOff_Widget = &DiagnosticScreen.DiagnosticScreen_LeftPadOff_Button;
    g_PadSettings[LEFT_PAD].m_DiagnosticProportional_Widget = &DiagnosticScreen.DiagnosticScreen_LeftPadProp_Button;
    g_PadSettings[LEFT_PAD].m_DiagnosticDigital_Widget = &DiagnosticScreen.DiagnosticScreen_LeftPadDigital_Button;
    g_PadSettings[LEFT_PAD].m_RawValuePrompt = &DiagnosticScreen.DiagnosticScreen_LeftPad_RawValue_Prompt;
    g_PadSettings[LEFT_PAD].m_AdjustedValuePrompt = &DiagnosticScreen.DiagnosticScreen_LeftPad_Adjusted_Prompt;
    g_PadSettings[LEFT_PAD].m_Proportional_RawValue = 0;
    g_PadSettings[LEFT_PAD].m_Proportional_DriveDemand = 0;
    strcpy (g_PadSettings[LEFT_PAD].m_RawValueString, "");
    strcpy (g_PadSettings[LEFT_PAD].m_DriveDemandString, "");


    g_PadSettings[RIGHT_PAD].m_PadDirection = INVALID_DIRECTION;
    g_PadSettings[RIGHT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[RIGHT_PAD].m_MinimumDriveValue = 20;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[OFF_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Off_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[RIGHT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_RightArrow_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[LEFT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_LeftArrow_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[FORWARD_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_ForwardArrow_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[INVALID_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Question_Button;
    g_PadSettings[RIGHT_PAD].m_PadMinimumCalibrationValue = 0;
    g_PadSettings[RIGHT_PAD].m_PadMaximumCalibrationValue = 100;
    g_PadSettings[RIGHT_PAD].m_Minimum_ADC_Threshold = 25;
    g_PadSettings[RIGHT_PAD].m_Maximum_ADC_Threshold = 220;
    g_PadSettings[RIGHT_PAD].m_DiagnosticWidigetLocation = g_DiagnosticWidgetLocations[RIGHT_PAD];
    g_PadSettings[RIGHT_PAD].m_DiagnosticOff_Widget = &DiagnosticScreen.DiagnosticScreen_RightPadOff_Button;
    g_PadSettings[RIGHT_PAD].m_DiagnosticProportional_Widget = &DiagnosticScreen.DiagnosticScreen_RightPadProp_Button;
    g_PadSettings[RIGHT_PAD].m_DiagnosticDigital_Widget = &DiagnosticScreen.DiagnosticScreen_RightPadDigital_Button;
    g_PadSettings[RIGHT_PAD].m_RawValuePrompt = &DiagnosticScreen.DiagnosticScreen_RightPad_RawValue_Prompt;
    g_PadSettings[RIGHT_PAD].m_AdjustedValuePrompt = &DiagnosticScreen.DiagnosticScreen_RightPad_Adjusted_Prompt;
    g_PadSettings[RIGHT_PAD].m_Proportional_RawValue = 0;
    g_PadSettings[RIGHT_PAD].m_Proportional_DriveDemand = 0;
    strcpy (g_PadSettings[RIGHT_PAD].m_RawValueString, "");
    strcpy (g_PadSettings[RIGHT_PAD].m_DriveDemandString, "");


    g_PadSettings[CENTER_PAD].m_PadDirection = INVALID_DIRECTION;
    g_PadSettings[CENTER_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[CENTER_PAD].m_MinimumDriveValue = 20;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[OFF_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Off_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[RIGHT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_RightArrow_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[LEFT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_LeftArrow_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[FORWARD_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_ForwardArrow_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[INVALID_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Question_Button;
    g_PadSettings[CENTER_PAD].m_PadMinimumCalibrationValue = 0;
    g_PadSettings[CENTER_PAD].m_PadMaximumCalibrationValue = 100;
    g_PadSettings[CENTER_PAD].m_Minimum_ADC_Threshold = 20;
    g_PadSettings[CENTER_PAD].m_Maximum_ADC_Threshold = 220;
    g_PadSettings[CENTER_PAD].m_DiagnosticWidigetLocation = g_DiagnosticWidgetLocations[CENTER_PAD];
    g_PadSettings[CENTER_PAD].m_DiagnosticOff_Widget = &DiagnosticScreen.DiagnosticScreen_CenterPadOff_Button;
    g_PadSettings[CENTER_PAD].m_DiagnosticProportional_Widget = &DiagnosticScreen.DiagnosticScreen_CenterPadProp_Button;
    g_PadSettings[CENTER_PAD].m_DiagnosticDigital_Widget = &DiagnosticScreen.DiagnosticScreen_CenterPadDigital_Button;
    g_PadSettings[CENTER_PAD].m_RawValuePrompt = &DiagnosticScreen.DiagnosticScreen_CenterPad_RawValue_Prompt;
    g_PadSettings[CENTER_PAD].m_AdjustedValuePrompt = &DiagnosticScreen.DiagnosticScreen_CenterPad_Adjusted_Prompt;
    g_PadSettings[CENTER_PAD].m_Proportional_RawValue = 0;
    g_PadSettings[CENTER_PAD].m_Proportional_DriveDemand = 0;
    strcpy (g_PadSettings[CENTER_PAD].m_RawValueString, "");
    strcpy (g_PadSettings[CENTER_PAD].m_DriveDemandString, "");

    /* Create the widgets we have defined with the GUIX data structures and resources. */
    GX_WIDGET * p_first_screen = NULL;
    
    gx_studio_named_widget_create("DiagnosticScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("FeatureSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("HHP_Start_Screen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("MainUserScreen", GX_NULL, GX_NULL);    // Create and show first startup screen.
    gx_studio_named_widget_create("MinimumDriveScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("MoreSelectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("PerformanceSelectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("OON_Screen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("PadCalibrationScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("PadOptionsSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ReadyScreen", GX_NULL, GX_NULL);    // Create and show first startup screen.
    gx_studio_named_widget_create("ResetScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ResetFinishScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("SetPadDirectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("SetPadTypeScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("UserSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("UserSelectionScreen", GX_NULL, GX_NULL);
    //gx_studio_named_widget_create("SettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("VeerAdjustScreen", GX_NULL, GX_NULL);

    gx_studio_named_widget_create("StartupSplashScreen", (GX_WIDGET *)p_window_root, &p_first_screen);    // Create and show first startup screen.

    /* Attach the first screen to the root so we can see it when the root is shown */
    gx_widget_attach(p_window_root, p_first_screen);

    /* Shows the root window to make it and patients screen visible. */
    gx_widget_show(p_window_root);

    g_ActiveScreen = (GX_WIDGET*)&StartupSplashScreen;     // Save we can determine who's got control of the screen.

    /* Lets GUIX run. */
    gx_system_start();

	g_ioport.p_api->pinWrite(BACKLIGHT_CONTROL_PIN, IOPORT_LEVEL_HIGH);      // Turn off the backlight

	err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
	if (err != SSP_SUCCESS)
	{
		g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);	//Error
	}
		
//	Process_Touches();      // This accepts the Touch screen information and sends it to the GUIX process.

    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
 	
//    //Open WDT; 4.46s; PCLKB 30MHz
// 	g_wdt.p_api->open(g_wdt.p_ctrl, g_wdt.p_cfg);
//	//Start the WDT by refreshing it
// 	g_wdt.p_api->refresh(g_wdt.p_ctrl);
  
//  	i = 0;
//  	do {
//  		test_num = get_PROP_version();
//
//    	//GC Replace the following with a tx_thread_sleep
//  		//GC    R_BSP_SoftwareDelay(30, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(30);
//  		tx_thread_sleep(10);
//    	//Start the WDT by refreshing it
//// 	  	g_wdt.p_api->refresh(g_wdt.p_ctrl);
//    	if(i > 5)
//    	    break;
//    	i++;
//  	} while(test_num != 0);
//

    while(1)
    {
        Process_Touches();          // Process the GUI touches and the Front Panel Arrows Pushes.

        ProcessCommunicationMsgs ();    // Process any messages from the Head Array Comm process.

        tx_thread_sleep (1);
    }

    //Open WDT; 4.46s; PCLKB 30MHz
 	//  g_wdt.p_api->open(g_wdt.p_ctrl, g_wdt.p_cfg);
 	//  g_wdt.p_api->refresh(g_wdt.p_ctrl);     //Start the WDT by refreshing it
}

//*************************************************************************************
// Function: Process_Communication_Msgs
//
//*************************************************************************************

void ProcessCommunicationMsgs ()
{
    GX_EVENT gxe;
    uint32_t qStatus;
    HHP_HA_MSG_STRUCT HeadArrayMsg;
    ULONG numMsgs;
    PHYSICAL_PAD_ENUM myPad;

    // Is there anything to process, i.e. Is there anything from the Head Array Comm Process?
    tx_queue_info_get (&q_COMM_to_GUI_Queue, NULL, &numMsgs, NULL, NULL, NULL, NULL);
    if (numMsgs == 0)
    {
        return;
    }

    // Get message or return if error.
    qStatus = tx_queue_receive (&q_COMM_to_GUI_Queue, &HeadArrayMsg, TX_NO_WAIT);
    if (qStatus != TX_SUCCESS)
        return;

    switch (HeadArrayMsg.m_MsgType)
    {
        case HHP_HA_HEART_BEAT:
            if (HeadArrayMsg.HeartBeatMsg.m_HB_OK)
            {
                if (g_ActiveScreen->gx_widget_id == STARTUP_SPLASH_SCREEN_ID)
                {
                    gxe.gx_event_type = GX_SIGNAL (HB_OK_ID, GX_EVENT_CLICKED);
                    gxe.gx_event_sender = GX_ID_NONE;
                    gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                    gxe.gx_event_display_handle = 0;
                    gx_system_event_send(&gxe);
                }
                else if (g_ActiveScreen->gx_widget_id == MAIN_USER_SCREEN_ID)
                {
                    if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x01)   // Bit0 = 1 if Head Array in "Ready", Power On mode.
                    {
                        if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x20) == 0x20)// Out of Neutral?
                        {
                            gxe.gx_event_type = GX_SIGNAL (HB_OON_ID, GX_EVENT_CLICKED);
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                        // This triggers redrawing the main screen if the mode changes.
                        else if (g_ActiveFeature != HeadArrayMsg.HeartBeatMsg.m_ActiveMode)
                        {
                            AdjustActiveFeature ((FEATURE_ID_ENUM)(HeadArrayMsg.HeartBeatMsg.m_ActiveMode));   // This function also store "g_ActiveFeature" if appropriate.
                            gxe.gx_event_type = GX_EVENT_REDRAW;
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                    }
                    else if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x00)// Bit 0 = 0; means Head Array in Power Off, Idle Mode, we are recommending to change screens.
                    {
                        gxe.gx_event_type = GX_SIGNAL (POWER_OFF_ID, GX_EVENT_CLICKED);
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == READY_SCREEN_ID)
                {
                    if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x01)   // Bit0 = 1 if Head Array in "Ready", Power On mode.
                    {
                        gxe.gx_event_type = GX_SIGNAL (POWER_ON_ID, GX_EVENT_CLICKED);
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == OON_SCREEN_ID)
                {
                    if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x20) == 0x00)   // We are OK to go... Neutral Test passed.
                    {
                        gxe.gx_event_type = GX_SIGNAL (OON_OK_BTN_ID, GX_EVENT_CLICKED);
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
            }
            else    // Failed Heart Beat
            {
                if (g_ActiveScreen->gx_widget_id == MAIN_USER_SCREEN_ID)
                {
                    gxe.gx_event_type = GX_SIGNAL (HB_TIMEOUT_ID, GX_EVENT_CLICKED);
                    gxe.gx_event_sender = GX_ID_NONE;
                    gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                    gxe.gx_event_display_handle = 0;
                    gx_system_event_send(&gxe);
                }
            }
    //            if (HeadArrayMsg.HeartBeatMsg.HB_Count == 25)
    //            {
    //                gxe.gx_event_type = GX_EVENT_REDRAW;
    //                gxe.gx_event_sender = GX_ID_NONE;
    //                gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
    //                gxe.gx_event_display_handle = 0;
    //                gx_system_event_send(&gxe);
    //                g_UseNewPrompt = true;
    //            }

            break;

        case HHP_HA_PAD_ASSIGMENT_SET:  // Yes, the COMM task is responding with a "set" command.
            myPad = HeadArrayMsg.PadAssignmentResponseMsg.m_PhysicalPadNumber;
            if (myPad != INVALID_PAD)
            {
                g_PadSettings[myPad].m_PadDirection = HeadArrayMsg.PadAssignmentResponseMsg.m_LogicalDirection;
                g_PadSettings[myPad].m_PadType = HeadArrayMsg.PadAssignmentResponseMsg.m_PadType;
            }
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_VERSION_GET:
            sprintf (g_HeadArrayVersionString, "ASL110: %d.%d.%d", HeadArrayMsg.Version.m_Major, HeadArrayMsg.Version.m_Minor, HeadArrayMsg.Version.m_Build);
            g_HA_Version_Major = HeadArrayMsg.Version.m_Major;
            g_HA_Version_Minor = HeadArrayMsg.Version.m_Minor;
            g_HA_Version_Build = HeadArrayMsg.Version.m_Build;
            g_HA_EEPROM_Version = HeadArrayMsg.Version.m_EEPROM_Version;

            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_PAD_DATA_GET:
            myPad = HeadArrayMsg.GetDataMsg.m_PadID;        // Get Physical Pad ID
            if (myPad < INVALID_PAD)
            {
                g_PadSettings[myPad].m_Proportional_RawValue = HeadArrayMsg.GetDataMsg.m_RawData;
                g_PadSettings[myPad].m_Proportional_DriveDemand = HeadArrayMsg.GetDataMsg.m_DriveDemand;
            }
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_CALIBRATE_RANGE_GET:
            myPad = HeadArrayMsg.GetDataMsg.m_PadID;        // Get Physical Pad ID
            if (myPad < INVALID_PAD)
            {
                g_PadSettings[myPad].m_Minimum_ADC_Threshold = HeadArrayMsg.CalibrationDataResponse.m_MinADC;
                g_PadSettings[myPad].m_Maximum_ADC_Threshold = HeadArrayMsg.CalibrationDataResponse.m_MaxADC;
                g_PadSettings[myPad].m_PadMinimumCalibrationValue = (int16_t) (HeadArrayMsg.CalibrationDataResponse.m_MinThreshold);
                g_PadSettings[myPad].m_PadMaximumCalibrationValue = (int16_t) (HeadArrayMsg.CalibrationDataResponse.m_MaxThreshold);
            }
            break;

        case HHP_HA_FEATURE_GET:
            g_TimeoutValue = HeadArrayMsg.GetFeatureResponse.m_Timeout;
            g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x01 ? true : false); // Power On/Off
            g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x02 ? true : false); // Bluetooth
            g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x04 ? true : false); // Next Function
            g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x08 ? true : false); // Next Profile
            g_ClicksActive = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x10 ? true : false);              // Clicks on/off
            g_PowerUpInIdle = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x20 ? true : false);              // Clicks on/off
            g_MainScreenFeatureInfo[RNET_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & 0x80 ? true : false); // Process RNet
            AdjustActiveFeature (g_ActiveFeature);   // This function also store "g_ActiveFeature" if appropriate.
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_NEUTRAL_DAC_GET:
            g_NeutralDAC_Constant = HeadArrayMsg.NeutralDAC_Get_Response.m_DAC_Constant;
            g_NeutralDAC_Setting = HeadArrayMsg.NeutralDAC_Get_Response.m_NeutralDAC_Value;
            g_NeutralDAC_Range= HeadArrayMsg.NeutralDAC_Get_Response.m_Range;
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            g_WaitingForVeerResponse = false;
            break;

        case HHP_HA_DRIVE_OFFSET_GET:
            g_PadSettings[CENTER_PAD].m_MinimumDriveValue = HeadArrayMsg.DriveOffset_Get_Response.m_CenterPad_DriveOffset;
            g_PadSettings[LEFT_PAD].m_MinimumDriveValue = HeadArrayMsg.DriveOffset_Get_Response.m_LeftPad_DriveOffset;
            g_PadSettings[RIGHT_PAD].m_MinimumDriveValue = HeadArrayMsg.DriveOffset_Get_Response.m_RightPad_DriveOffset;
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        default:
            break;
    } // end switch
}

//******************************************************************************************
// Detach one window and attach another window to root. This function checks if the
// parent window is already attached and attaches itself to it. If is already attached
// it simply displays the widget.
//
// Used it to "Change Screens".
//
//******************************************************************************************

VOID screen_toggle(GX_WINDOW *new_win, GX_WINDOW *old_win)
{
    if (!new_win->gx_widget_parent)
    {
        gx_widget_attach(p_window_root, (GX_WIDGET *)new_win);
    }
    else
    {
        gx_widget_show((GX_WIDGET *)new_win);
    }
    gx_widget_detach((GX_WIDGET *)old_win);
}

//*************************************************************************************
// Startup Screen
//*************************************************************************************

VOID StartupSplashScreen_draw_function (GX_WINDOW *window)
{

    g_ActiveScreen = (GX_WIDGET*) window;

    gx_window_draw(window);
}

//*************************************************************************************

UINT StartupSplashScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    gx_window_event_process(window, event_ptr);

    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_GoBackScreen = window;        // Set the going back window.
            g_ChangeScreen_WIP = TRUE;
            break;

        case GX_SIGNAL (HB_OK_ID, GX_EVENT_CLICKED):
            if (g_StartupDelayCounter < 0)      // If we've been here before but are recovering from a Heart Beat timeout, don't wait, just goto the Main User Screen.
            {
                screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            }
            else // OK, we are starting up.
            {
                ++g_StartupDelayCounter;
                if (g_StartupDelayCounter > 15)  // Have we shown the startup screen long enough?
                {
                    screen_toggle((GX_WINDOW *)&MainUserScreen, window);
                    g_StartupDelayCounter = -1; // This prevents us from doing a "startup" delay should the Heart Beat stop.
                }
                else if (g_StartupDelayCounter == 10)    // We need to send a Version Request to the Head Array.
                {
                    SendFeatureGetCommand();                // Send command to get the current users settings.
                    SendGetVersionCommand ();
                }
            }
            break;
    } // end switch

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: DisplayMainScreenActiveFeatures
//
// Description: This displays the features that are active in the order specified
//  in the Screen Prompts "objects".
//
//*************************************************************************************

void AdjustActiveFeature (FEATURE_ID_ENUM newMode)
{
    uint8_t featureCount, myMode, lineNumber;

    if (newMode >= NUM_FEATURES)    // Check for valid mode
        return;

    g_ActiveFeature = newMode;          // Store the active feature in global var.
    myMode = (uint8_t) newMode;
    lineNumber = 0;                     // Need to keep track of which line is next.

    for (featureCount = 0; featureCount < NUM_FEATURES; ++featureCount)
    {
        if (g_MainScreenFeatureInfo[myMode].m_Enabled)
        {
            g_MainScreenFeatureInfo[myMode].m_Location = lineNumber;
            ++lineNumber;
        }
        else
        {
            g_MainScreenFeatureInfo[myMode].m_Location = 5;
        }
        ++myMode;               // Look at the next feature information.
        if (myMode >= NUM_FEATURES)         // Rollover
            myMode = 0;
    }
}

//*************************************************************************************

UINT DisplayMainScreenActiveFeatures ()
{
    int enabledCount;
    int feature;
    UINT myErr = GX_SUCCESS;

    // Adjust the displayed information based upon the RNet setting.
    // .. If RNet is enabled, the NEXT FUNCTION feature becomes RNet TOGGLE
    // .. and NEXT PROFILE feature become RNet MENU.
    if (g_MainScreenFeatureInfo[RNET_ID].m_Enabled)
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
        if (g_MainScreenFeatureInfo[feature].m_Enabled)
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
            }
        }
    }
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

UINT MainUserScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    uint8_t feature;
    int activeCount;

    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_TIMER:
            if (event_ptr->gx_event_payload.gx_event_timer_id == ARROW_PUSHED_TIMER_ID)
            {
                screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
                g_ChangeScreen_WIP = TRUE;
            }
            break;

        case GX_EVENT_SHOW:
            g_GoBackScreen = window;        // Set the going back window.
            DisplayMainScreenActiveFeatures();
            break;

        case GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
            // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
            if (g_ChangeScreen_WIP)
            {
                g_ChangeScreen_WIP = FALSE;
                break;
            }
            // Count the number of active features to set a limit on location
            activeCount = 0;
            for (feature = 0; feature < NUM_FEATURES; ++feature)
            {
                if (g_MainScreenFeatureInfo[feature].m_Enabled)
                    ++activeCount;
            }
            // Move Top Feature to Bottom and move Bottom upward.
            for (feature = 0; feature < NUM_FEATURES; ++feature)
            {
                if (g_MainScreenFeatureInfo[feature].m_Enabled)
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
                }
            }
            //DisplayMainScreenActiveFeatures();
            break;

        case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
            // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
            if (g_ChangeScreen_WIP)
            {
                g_ChangeScreen_WIP = FALSE;
                break;
            }
            // Count the number of active features to set a limit on location
            activeCount = 0;
            for (feature = 0; feature < NUM_FEATURES; ++feature)
            {
                if (g_MainScreenFeatureInfo[feature].m_Enabled)
                    ++activeCount;
            }
            --activeCount;  // Translate the Number of items to Based Zero line number.

            // Move the features downward, limiting the movement by the number of Active Features.
            for (feature = 0; feature < NUM_FEATURES; ++feature)
            {
                if (g_MainScreenFeatureInfo[feature].m_Enabled)
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
                }
            }
            //DisplayMainScreenActiveFeatures();
            break;

        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_ChangeScreen_WIP = TRUE;
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
    } // end swtich

   // DisplayMainScreenActiveFeatures();  // Remove this when more of the previous code is "undefinded".

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: Ready_Screen_event_process
//
// Description: This handles the Ready Screen messages, this screen is the one
//      that indicates that the "System is Powered off, please hit switch".
//
//*************************************************************************************
VOID Ready_Screen_draw_function(GX_WINDOW *window)
{
    g_ActiveScreen = (GX_WIDGET*) window;

    gx_window_draw(window);
}

//*************************************************************************************

UINT Ready_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL(POWER_ON_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            break;

        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_ChangeScreen_WIP = TRUE;
            break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Out of Neutral Test screen processing.
//*************************************************************************************

VOID OON_Screen_draw_function (GX_WINDOW *window)
{
    g_ActiveScreen = (GX_WIDGET*) window;

    gx_window_draw(window);
}

UINT OON_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    gx_window_event_process(window, event_ptr);

    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL (OON_OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&MainUserScreen, window);
            //SendGetVersionCommand ();
            break;

        case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            g_ChangeScreen_WIP = TRUE;
            break;
    } // end switch

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: HHP_Start_Screen_event_process
//
// Description: This handles the Startup Screen messages
//
//*************************************************************************************

UINT HHP_Start_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(PAD_SETTINGS_BTN_ID, GX_EVENT_CLICKED):      // When selected, goto Main Pad Setting scree.
        screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
        break;

    case GX_SIGNAL(SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
        break;

    case GX_SIGNAL(MORE_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&MoreSelectionScreen, window);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)g_GoBackScreen, window);
        SendSaveParameters();
        break;

    case GX_EVENT_SHOW:
        // We're entering the HHP feature, it's a good time to request "one-time" information.
        SendGetCalDataCommnd (LEFT_PAD);        // We send the commands to the Head Array to get the Calibration Data for all 3 pads.
        SendGetCalDataCommnd (RIGHT_PAD);
        SendGetCalDataCommnd (CENTER_PAD);
        SendNeutralDAC_GetCommand();            // We'll need the Neutral DAC "calibration" value.
        SendFeatureGetCommand();                // Send command to get the current users settings.
        SendDriveOffsetGet();                   // Send command to get the Drive Offset.
        SendGetVersionCommand ();
        break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

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
            SendGetDataCommand (START_SENDING_DATA, INVALID_PAD);
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
                screen_toggle((GX_WINDOW *)&MoreSelectionScreen, window);
            SendGetDataCommand (STOP_SENDING_DATA, INVALID_PAD);
            break;

    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: SettingsScreen_event_process
//
// Description: This handles the Settings Screen messages
//
//*************************************************************************************

#ifdef OLD_SCHOOL
UINT SettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
            break;

        case GX_SIGNAL(GOTO_PAD_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
            break;

        case GX_SIGNAL(GOTO_USER_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&UserSettingsScreen, window);
            //SendFeatureGetCommand();        // Send command to get the current users settings.
            break;

        case GX_SIGNAL(FEATURES_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&FeatureSettingsScreen, window);
            //SendFeatureGetCommand();        // Send command to get the current users settings.
            break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return (GX_SUCCESS);
}
#endif

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
// Function Name: PerformanceSelectionScreen_event_process
//
// Description: This dispatches the Pad Option Settings Screen messages
//
//*************************************************************************************

UINT PerformanceSelectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
            break;

        case GX_SIGNAL(GOTO_VEER_ADJUST_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&VeerAdjustScreen, window);
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
}

//*************************************************************************************

UINT SetPadDirectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    UINT pads, icons;
    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:
            // Show correct settings for LEFT pad. Off, Right, Forward or Left.
            // First let's hide all choices for all pads.
            for (pads = 0; pads < 3; ++pads)
            {
                for (icons = 0; icons < 5; ++icons)
                {
                    gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DirectionIcons[icons], &g_HiddenRectangle);
                }
                gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DirectionIcons[INVALID_DIRECTION], &g_PadDirectionLocation[pads]);
            }
            SendGetPadAssignmentMsg (LEFT_PAD);
            SendGetPadAssignmentMsg (RIGHT_PAD);
            SendGetPadAssignmentMsg (CENTER_PAD);
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&PadOptionsSettingsScreen, window);
            break;
        // Process LEFT button pushes
        case GX_SIGNAL(LEFT_PAD_OFF_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[LEFT_PAD].m_PadDirection = LEFT_DIRECTION;
            SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
            SendGetPadAssignmentMsg (LEFT_PAD);
            break;
        case GX_SIGNAL(LEFT_PAD_LEFT_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[LEFT_PAD].m_PadDirection = FORWARD_DIRECTION;
            SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
            SendGetPadAssignmentMsg (LEFT_PAD);
            break;
        case GX_SIGNAL(LEFT_PAD_FORWARD_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[LEFT_PAD].m_PadDirection = RIGHT_DIRECTION;
            SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
            SendGetPadAssignmentMsg (LEFT_PAD);
            break;
        case GX_SIGNAL(LEFT_PAD_RIGHT_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[LEFT_PAD].m_PadDirection = OFF_DIRECTION;
            SendSetPadAssignmentCommand (LEFT_PAD, g_PadSettings[LEFT_PAD].m_PadDirection, g_PadSettings[LEFT_PAD].m_PadType);
            SendGetPadAssignmentMsg (LEFT_PAD);
            break;
        // Process RIGHT button pushes
        case GX_SIGNAL(RIGHT_PAD_OFF_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[RIGHT_PAD].m_PadDirection = LEFT_DIRECTION;
            SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
            SendGetPadAssignmentMsg (RIGHT_PAD);
            break;
        case GX_SIGNAL(RIGHT_PAD_LEFT_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[RIGHT_PAD].m_PadDirection = FORWARD_DIRECTION;
            SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
            SendGetPadAssignmentMsg (RIGHT_PAD);
            break;
        case GX_SIGNAL(RIGHT_PAD_FORWARD_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[RIGHT_PAD].m_PadDirection = RIGHT_DIRECTION;
            SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
            SendGetPadAssignmentMsg (RIGHT_PAD);
            break;
        case GX_SIGNAL(RIGHT_PAD_RIGHT_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[RIGHT_PAD].m_PadDirection = OFF_DIRECTION;
            SendSetPadAssignmentCommand (RIGHT_PAD, g_PadSettings[RIGHT_PAD].m_PadDirection, g_PadSettings[RIGHT_PAD].m_PadType);
            SendGetPadAssignmentMsg (RIGHT_PAD);
            break;
        // Process CENTER PAD button pushes
        case GX_SIGNAL(CENTER_PAD_OFF_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[CENTER_PAD].m_PadDirection = LEFT_DIRECTION;
            SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
            SendGetPadAssignmentMsg (CENTER_PAD);
            break;
        case GX_SIGNAL(CENTER_PAD_LEFT_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[CENTER_PAD].m_PadDirection = FORWARD_DIRECTION;
            SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
            SendGetPadAssignmentMsg (CENTER_PAD);
            break;
        case GX_SIGNAL(CENTER_PAD_FORWARD_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[CENTER_PAD].m_PadDirection = RIGHT_DIRECTION;
            SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
            SendGetPadAssignmentMsg (CENTER_PAD);
            break;
        case GX_SIGNAL(CENTER_PAD_RIGHT_ARROW_BTN_ID, GX_EVENT_CLICKED):
            g_PadSettings[CENTER_PAD].m_PadDirection = OFF_DIRECTION;
            SendSetPadAssignmentCommand (CENTER_PAD, g_PadSettings[CENTER_PAD].m_PadDirection, g_PadSettings[CENTER_PAD].m_PadType);
            SendGetPadAssignmentMsg (CENTER_PAD);
            break;
    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: VeerAdjust_Screen_event_handler and Slider_Draw_function
//
// Description: These handle the Veer Adjust Screen.
//
//*************************************************************************************

//*************************************************************************************

VOID VeerAdjust_Screen_draw_function (GX_WINDOW *window)
{
    gx_window_draw(window);
}

//*************************************************************************************

UINT VeerSlider_event_function(GX_PIXELMAP_SLIDER *widget, GX_EVENT *event_ptr)
{
    int16_t newVal;
    int16_t totalSteps;
    int16_t increment;
    int16_t sliderAsFound;
    int16_t minimumDAC;

    gx_pixelmap_slider_event_process(widget, event_ptr);    // This must be before the processing
                                                            // to allow the values to be stored in the widget.

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        break;
    case GX_EVENT_PEN_DOWN:
    case GX_EVENT_PEN_DRAG:
    case GX_EVENT_PEN_MOVE:
        sliderAsFound = (int16_t) widget->gx_slider_info.gx_slider_info_current_val;
        totalSteps = (int16_t)(widget->gx_slider_info.gx_slider_info_max_val - widget->gx_slider_info.gx_slider_info_min_val);
        increment = 5;      // This means that each slider step is worth 5 DAC counts.
        minimumDAC = (int16_t)(g_NeutralDAC_Constant - (increment * (totalSteps/2)));       // lowest value.
        newVal = (int16_t) (minimumDAC + (increment * sliderAsFound));
        // Send the new data to the Head Array, but only when the process of Pen Down, Drag and Up are done. No sense sending it multiple times.
        if (!g_WaitingForVeerResponse)
        {
            SendNeutralDAC_Set(newVal);
            SendNeutralDAC_GetCommand();
            g_WaitingForVeerResponse = true;
        }
        break;
    default:
        break;
    } // end switch

    return GX_SUCCESS;
}

//*************************************************************************************

VOID Slider_Draw_Function (GX_PIXELMAP_SLIDER *slider)
{
    int myVal, totalSteps; // offset, increment,
    int DACCountsPerStep, MinimumDAC_Counts, MaximumDAC_Counts;

    // We need to convert the target DAC Counts Setting to the number of Slider Steps.
    // The slider is 0-20 representing the extreme left to the extreme right of the slider.

    // Get the number of steps in the Screen's Slider widget.
    totalSteps = (int16_t)(slider->gx_slider_info.gx_slider_info_max_val - slider->gx_slider_info.gx_slider_info_min_val);
    // Get the number of DAC Counts between the new setting and the midpoint.
    DACCountsPerStep = 5;
    MinimumDAC_Counts = g_NeutralDAC_Constant - (DACCountsPerStep * (totalSteps/2));
    MaximumDAC_Counts = g_NeutralDAC_Constant + (DACCountsPerStep * (totalSteps/2));
    // Check for DAC Setting outside of displayable range and set to min or max slider position.
    if (g_NeutralDAC_Setting < MinimumDAC_Counts)
        myVal = slider->gx_slider_info.gx_slider_info_min_val;
    else if (g_NeutralDAC_Setting > MaximumDAC_Counts)
        myVal = slider->gx_slider_info.gx_slider_info_max_val;
    else
        myVal = (g_NeutralDAC_Setting - MinimumDAC_Counts) / DACCountsPerStep;


    slider->gx_slider_info.gx_slider_info_current_val = myVal;
    gx_pixelmap_slider_draw (slider);
    myVal = slider->gx_slider_info.gx_slider_info_current_val - (slider->gx_slider_info.gx_slider_info_max_val/2);
    sprintf (g_SliderValue, "%2d", myVal);

    gx_text_button_text_set (&VeerAdjustScreen.VeerAdjustScreen_SliderValue_Button, g_SliderValue);
}

//*************************************************************************************

UINT VeerAdjust_Screen_event_handler (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&PerformanceSelectionScreen, window);
        break;

    case GX_EVENT_SHOW:
        g_WaitingForVeerResponse = false;
        SendNeutralDAC_GetCommand();
        break;

    case GX_EVENT_PEN_UP:
        break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


//*************************************************************************************
// Function Name: UserSelectionScreen_event_process
//
// Description: This handles the User Settings Screen messages
//
//*************************************************************************************

UINT UserSelectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
        break;

    case GX_SIGNAL(USER_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSettingsScreen, window);
        break;

    case GX_SIGNAL(FEATURE_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&FeatureSettingsScreen, window);
        break;

    case GX_SIGNAL(PERFORMANCE_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&PerformanceSelectionScreen, window);
        break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: UserSettingsScreen_event_process
//
// Description: This handles the User Settings Screen messages
//
//*************************************************************************************
void CreateEnabledFeatureStatus(uint8_t *myActiveFeatures)
{
    uint8_t myMask;
    uint8_t feature;

    *myActiveFeatures = 0x0;
    myMask = 0x01;
    for (feature = 0; feature < 4 /*NUM_FEATURES*/; ++feature)  // Because the RNet feature is in 0x40.
    {
        if (g_MainScreenFeatureInfo[feature].m_Enabled)
        {
            // Create the byte to send to the COMM Task to tell Head Array what features are active.
            *myActiveFeatures |= myMask;     // Set bit.
        }
        myMask = (uint8_t)(myMask << 1);    // Rotate the bit.
    }

    // Add clicks in D4 of byte.
    if (g_ClicksActive)
        *myActiveFeatures |= 0x10;
    // Add power up in D5 of byte;
    if (g_PowerUpInIdle)
        *myActiveFeatures |= 0x20;
    // Add RNet Enable in D6 of byte.
    if (g_MainScreenFeatureInfo[RNET_ID].m_Enabled)
        *myActiveFeatures |= 0x80;

}

//*************************************************************************************

UINT UserSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    uint8_t myActiveFeatures;
    char tmpChar[8];

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        if (g_ClicksActive)
        {
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_ClicksToggleBtn);
        }

        // Power Up in Idle
        if (g_PowerUpInIdle)    // If powering up in idle state is enable
        {
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_PowerUpToggleBtn);
        }

        // RNet Enabled setting
        if (g_MainScreenFeatureInfo[RNET_ID].m_Enabled)
        {
            gx_button_select ((GX_BUTTON*) &UserSettingsScreen.UserSettingsScreen_RNET_ToggleBtn);
        }

        // Populate the Timeout button with the current setting or "OFF".
        if (g_TimeoutValue == 0)
            strcpy (g_TimeoutValueString, "OFF");
        else
        {
            // sprintf (g_TimeoutValueString, "%1.1g", (float) (g_TimeoutValue / 10.0f));
            // Floating point doesn't work for some odd reason.
            // I'm doing a hack to display the value in a X.X format.
            sprintf (g_TimeoutValueString, "%d.", g_TimeoutValue / 10);
            sprintf (tmpChar, "%d", g_TimeoutValue % 10);
            strcat (g_TimeoutValueString, tmpChar);
        }
        gx_text_button_text_set (&UserSettingsScreen.UserSettingsScreen_Timeout_Button, g_TimeoutValueString);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
        CreateEnabledFeatureStatus(&myActiveFeatures);
        SendFeatureSetting (myActiveFeatures, g_TimeoutValue);
        SendFeatureGetCommand();                // Send command to get the current users settings.
        break;

    //----------------------------------------------------------
    // CLICK toggle button processing
    case GX_SIGNAL(CLICKS_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
        g_ClicksActive = TRUE;
        break;
    case GX_SIGNAL(CLICKS_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
        g_ClicksActive = FALSE;
        break;

    //----------------------------------------------------------
    // Power Up in IDLE toggle button
    case GX_SIGNAL(POWER_UP_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
        g_PowerUpInIdle = TRUE;
        break;
    case GX_SIGNAL(POWER_UP_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
        g_PowerUpInIdle = FALSE;
        break;

    //----------------------------------------------------------
    // RNet Enable toggle button
    case GX_SIGNAL(RNET_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
        g_MainScreenFeatureInfo[RNET_ID].m_Enabled = TRUE;
        break;
    case GX_SIGNAL(RNET_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
        g_MainScreenFeatureInfo[RNET_ID].m_Enabled = FALSE;
        break;

    case GX_SIGNAL(TIMEOUT_BTN_ID, GX_EVENT_CLICKED):
        switch (g_TimeoutValue)
        {
            case 0:
                g_TimeoutValue = 10;
                break;
            case 10:
            case 15:
            case 20:
            case 25:
                g_TimeoutValue =  (uint8_t)(g_TimeoutValue + 5);
                break;
            case 30:
            case 40:
                g_TimeoutValue = (uint8_t)(g_TimeoutValue + 10);
                break;
            case 50:
                g_TimeoutValue = 0;
                break;
        } // end switch
        if (g_TimeoutValue == 0)
            strcpy (g_TimeoutValueString, "OFF");
        else
        {
            sprintf (g_TimeoutValueString, "%d.", g_TimeoutValue / 10);
            sprintf (tmpChar, "%d", g_TimeoutValue % 10);
            strcat (g_TimeoutValueString, tmpChar);
        }
        gx_text_button_text_set (&UserSettingsScreen.UserSettingsScreen_Timeout_Button, g_TimeoutValueString);
        break;

    } // end switch

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: FeatureSettingsScreen_event_process
//
// Description: This handles the Feature Settings Screen messages
//
//*************************************************************************************

UINT FeatureSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    uint8_t myActiveFeatures;

    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:
            // Show RNet or NEXT FUNCITON and NEXT PROFILE depending on RNet setting.
            // Adjust the displayed information based upon the RNet setting.
            // .. If RNet is enabled, the NEXT FUNCTION feature becomes RNet TOGGLE
            // .. and NEXT PROFILE feature become RNet MENU.
            if (g_MainScreenFeatureInfo[RNET_ID].m_Enabled)
            {
                // Display as "RNet TOGGLE"
                gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_NextFunctionPrompt, GX_STRING_ID_RNET_TOGGLE);
                // Display as "RNET USER MENU"
                gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_NextProfilePrompt, GX_STRING_ID_RNET_MENU);
            }
            else
            {
                // Display as NEXT FUNCTION
                gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_NextFunctionPrompt, GX_STRING_ID_NEXT_FUNCTION);
                // Display as NEXT PROFILE
                gx_prompt_text_id_set (&FeatureSettingsScreen.FeatureSettingsScreen_NextProfilePrompt, GX_STRING_ID_NEXT_PROFILE);
            }

            // Power status
            if (g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled)
            {
                gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_PowerToggleBtn);
            }

            // Bluetooth
            if (g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Enabled)
            {
                gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_BluetoothToggleBtn);
            }

            // Next Function
            if (g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled)
            {
                gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_NextFunctionToggleBtn);
            }

            // Next Profile
            if (g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled)
            {
                gx_button_select ((GX_BUTTON*) &FeatureSettingsScreen.FeatureSettingsScreen_NextProfileToggleBtn);
            }

            g_SettingsChanged = FALSE;
            break;

        case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
            screen_toggle((GX_WINDOW *)&UserSelectionScreen, window);
            CreateEnabledFeatureStatus(&myActiveFeatures);
            SendFeatureSetting (myActiveFeatures, g_TimeoutValue);
            SendFeatureGetCommand();                // Send command to get the current users settings.
            break;

        //----------------------------------------------------------------------
        // Power Button
        case GX_SIGNAL(POWER_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
            g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = TRUE;
            g_SettingsChanged = TRUE;
            break;
        case GX_SIGNAL(POWER_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
            g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = FALSE;
            g_SettingsChanged = TRUE;
            break;

        //----------------------------------------------------------------------
        // Bluetooth button
        case GX_SIGNAL(BLUETOOTH_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
            g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Enabled = TRUE;
            g_SettingsChanged = TRUE;
            break;
        case GX_SIGNAL(BLUETOOTH_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
            g_MainScreenFeatureInfo[BLUETOOTH_ID].m_Enabled = FALSE;
            g_SettingsChanged = TRUE;
            break;

        //----------------------------------------------------------------------
        // Next Function button
        case GX_SIGNAL(NEXT_FUNCTION_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
            g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = TRUE;
            g_SettingsChanged = TRUE;
            break;
        case GX_SIGNAL(NEXT_FUNCTION_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
            g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = FALSE;
            g_SettingsChanged = TRUE;
            break;

        //----------------------------------------------------------------------
        // Next Profile Button
        case GX_SIGNAL(NEXT_PROFILE_TOGGLE_BTN, GX_EVENT_TOGGLE_ON):
            g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = TRUE;
            g_SettingsChanged = TRUE;
            break;
        case GX_SIGNAL(NEXT_PROFILE_TOGGLE_BTN, GX_EVENT_TOGGLE_OFF):
            g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = FALSE;
            g_SettingsChanged = TRUE;
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
            g_ChangeScreen_WIP = FALSE;
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
                g_ChangeScreen_WIP = TRUE;
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

    padValue = g_PadSettings[g_CalibrationPadNumber].m_Proportional_RawValue;    // Get the Pad value.

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
        g_CalibrationStepNumber = 0;

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
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue > 2)
                --g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
        }
        gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        break;
    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (g_CalibrationStepNumber == 0)       // We are doing minimum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue < 100)
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

//*************************************************************************************
// Function Name: ResetScreen_event_process
//
// Description: This handles the Diagnostic Screen messages
//
//*************************************************************************************

UINT ResetScreen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{

    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&MoreSelectionScreen, window);
        break;

    case GX_SIGNAL(CONTINUE_BTN_ID, GX_EVENT_CLICKED):
        SendResetParameters();          // Send the RESET PARAMETERS command to the Head Array via the Msg Queue.
        screen_toggle((GX_WINDOW *)&ResetFinishScreen, window);
        break;
    } // end switch
    return gx_window_event_process(window, event_ptr);
}

//*************************************************************************************

UINT ResetFinishScreen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&HHP_Start_Screen, window);
        break;

    } // end switch
    return gx_window_event_process(window, event_ptr);
}

//-------------------------------------------------------------------------
void  g_timer0_callback(timer_callback_args_t * p_args) //25ms
{
    (void)p_args;

}

//-------------------------------------------------------------------------
void  g_timer1_callback(timer_callback_args_t * p_args) //1612us
{
  (void)p_args;


  if(g_BeepPortStatus == IOPORT_LEVEL_LOW) g_BeepPortStatus = IOPORT_LEVEL_HIGH;
  else g_BeepPortStatus = IOPORT_LEVEL_LOW;

  g_ioport.p_api->pinWrite(beep_out, g_BeepPortStatus);

}

//-------------------------------------------------------------------------
void  g_timer2_callback(timer_callback_args_t * p_args) //819us
{
  (void)p_args;


  if(g_BeepPortStatus == IOPORT_LEVEL_LOW) g_BeepPortStatus = IOPORT_LEVEL_HIGH;
  else g_BeepPortStatus = IOPORT_LEVEL_LOW;

  g_ioport.p_api->pinWrite(beep_out, g_BeepPortStatus);

}

//-------------------------------------------------------------------------
void g_lcd_spi_callback(spi_callback_args_t * p_args)
{
  if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    tx_semaphore_ceiling_put(&g_my_gui_semaphore, 1);

}

//-------------------------------------------------------------------------
sf_touch_panel_event_t g_Events[128] = {SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID,
                                      SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID};
int g_EventCounter = 0;

/*********************************************************************************************************************
 * @brief  Sends a touch event to GUIX internal thread to call the GUIX event handler function
 *
 * @param[in] p_payload Touch panel message payload
***********************************************************************************************************************/
static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload)
{
    bool send_event = true;
    GX_EVENT gxe;
    GX_VALUE x_num;

    g_Events[g_EventCounter] = p_payload->event_type;
    ++g_EventCounter;
    g_EventCounter &= 0xff;
    if (g_EventCounter > 0xff)
        while (1) ;

    switch (p_payload->event_type)
    {
    case SF_TOUCH_PANEL_EVENT_DOWN:
        gxe.gx_event_type = GX_EVENT_PEN_DOWN;
        break;
    case SF_TOUCH_PANEL_EVENT_UP:
        gxe.gx_event_type = GX_EVENT_PEN_UP;
        break;
    case SF_TOUCH_PANEL_EVENT_HOLD:
    case SF_TOUCH_PANEL_EVENT_MOVE:
        gxe.gx_event_type = GX_EVENT_PEN_MOVE;
        break;
    case SF_TOUCH_PANEL_EVENT_INVALID:
        send_event = false;
        break;
    default:
        break;
    }

    if (send_event)
    {
        /** Send event to GUI */
        gxe.gx_event_sender         = GX_ID_NONE;
        gxe.gx_event_target         = 0;  /* the event to be routed to the widget that has input focus */
        gxe.gx_event_display_handle = 0;

        gxe.gx_event_payload.gx_event_pointdata.gx_point_x = (GX_VALUE)(p_payload->y);  //y_num;
        gxe.gx_event_payload.gx_event_pointdata.gx_point_y = (GX_VALUE)(p_payload->x);

        x_num = (GX_VALUE)(240 - p_payload->x);
//#define USE_X_NUM_ADJUSTEMENT
#ifdef USE_X_NUM_ADJUSTEMENT
        if(x_num < 140) {
           if(x_num < 5) x_num = 0;
           else if(x_num < 10) x_num = (GX_VALUE)(x_num - 5);
           else if(x_num < 20) x_num = (GX_VALUE)(x_num - 9);
           else if(x_num < 40) x_num = (GX_VALUE)(x_num - 19);
           else if(x_num < 50) x_num = (GX_VALUE)(x_num - 32);
           else if(x_num < 70) x_num = (GX_VALUE)(x_num - 30);
           else if(x_num < 90) x_num = (GX_VALUE)(x_num - 25);
           else if(x_num < 110) x_num = (GX_VALUE)(x_num -20);
           else if(x_num < 120) x_num = (GX_VALUE)(x_num -15);
           else x_num = (GX_VALUE)(x_num - 12);
        }
        if(x_num > 190) {
            x_num = (GX_VALUE)(x_num + 10);
            if(x_num > 240) x_num = 240;
        }
#endif
        gxe.gx_event_payload.gx_event_pointdata.gx_point_y = x_num;

        gx_system_event_send(&gxe);
    }
}
//-------------------------------------------------------------------------
void Process_Touches (void)
{
    ssp_err_t err;

    sf_message_header_t * p_message = NULL;

    err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &my_gui_thread_message_queue, (sf_message_header_t **) &p_message, 1); //TX_WAIT_FOREVER); //
    if(!err)
    {
        switch (p_message->event_b.class_code)
        {
            case SF_MESSAGE_EVENT_CLASS_TOUCH:
                if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
                {
                    // Translate a touch event into a GUIX event
                    guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
                }
                break;

            default:
                break;
        }

        // Message is processed, so release buffer.
        err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
    }

}


