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
#include "ASL165_System.h"
#include "QueueDefinition.h"
#include "HeadArray_CommunicationThread.h"
#include "UserMainScreen.h"
#include "BluetoothDeviceInfo.h"

//-------------------------------------------------------------------------
// Typedefs and defines
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Local variables
//-------------------------------------------------------------------------

GX_RECTANGLE g_HiddenRectangle = {0,0,0,0};

GX_RECTANGLE g_DiagnosticWidgetLocations[] = {
    {35, 32, 35+62, 32+90},
    {183, 32, 183+62, 32+90},
    {66, 140, 66+145, 140+42}};

// Timeout information
uint8_t g_TimeoutValue;
typedef struct TIMEOUT_STRUCT
{
    GX_PIXELMAP_BUTTON *m_TimeoutIcon;
    uint8_t m_TimeoutValue;
} TimeoutInfo_S;

//-------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------

GX_WINDOW_ROOT * p_window_root;

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
UINT CalibrationScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);

//-------------------------------------------------------------------------
// External function declarations
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
/* Gui Test App Thread entry function */
void my_gui_thread_entry(void)
{
    ssp_err_t err;
    UINT status = TX_SUCCESS;

		//debug pins
    g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(TEST_PIN, IOPORT_LEVEL_LOW);

//    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);   // Head Array Communication Line. This is not needed.

  	g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);
	g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);

    status = gx_system_initialize();
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }

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

    Initialize_MainScreenInfo();
    InitializeBluetoothDeviceInformation();

    /* Create the widgets we have defined with the GUIX data structures and resources. */
    GX_WIDGET * p_first_screen = NULL;

    gx_studio_named_widget_create("AttendantScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("AttendantSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("DiagnosticScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("Error_Screen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("FeatureSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("HHP_Start_Screen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_BT_ActiveScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_BT_DeviceSelectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_BT_SetupScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_BT_UserSelectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_DriverSelectScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_DriverControlProgrammingScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_MainProgrammingScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_PadDirectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("ION_SIPnPuffProgrammingScreen", GX_NULL, GX_NULL);
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
    gx_studio_named_widget_create("SNP_CalibrationScreen", GX_NULL, GX_NULL);
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

    gx_system_focus_claim(p_first_screen);

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

    // Populate the default Left Pad settings.
    g_PadSettings[LEFT_PAD].m_PadDirection = PAD_DIRECTION_LEFT;
    g_PadSettings[LEFT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[LEFT_PAD].m_PadStatus = PAD_STATUS_OK;
    g_PadSettings[LEFT_PAD].m_PadSensorStatus = PAD_OFF;
    g_PadSettings[LEFT_PAD].m_MinimumDriveValue = 20;
    g_PadSettings[LEFT_PAD].m_DirectionIcons = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Button;
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

    // Populate the Right Pad default settings.
    g_PadSettings[RIGHT_PAD].m_PadDirection = PAD_DIRECTION_RIGHT;
    g_PadSettings[RIGHT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[RIGHT_PAD].m_PadStatus = PAD_STATUS_OK;
    g_PadSettings[RIGHT_PAD].m_PadSensorStatus = PAD_OFF;
    g_PadSettings[RIGHT_PAD].m_MinimumDriveValue = 20;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Button;
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

    // Populate the Center Pad default settings.
    g_PadSettings[CENTER_PAD].m_PadDirection = PAD_DIRECTION_FORWARD;
    g_PadSettings[CENTER_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[CENTER_PAD].m_PadStatus = PAD_STATUS_OK;
    g_PadSettings[CENTER_PAD].m_PadSensorStatus = PAD_OFF;
    g_PadSettings[CENTER_PAD].m_MinimumDriveValue = 20;
    g_PadSettings[CENTER_PAD].m_DirectionIcons = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
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

    g_ioport.p_api->pinWrite (BACKLIGHT_CONTROL_PIN, IOPORT_LEVEL_HIGH);      // Turn off the backlight

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

void SaveSystemStatus (uint8_t status1, uint8_t status2)
{
    g_HeadArrayStatus1 = status1;
    g_HeadArrayStatus2 = status2;
}

//*************************************************************************************
// Function Name: UserSettingsScreen_event_process
//
// Description: This handles the User Settings Screen messages
//
//*************************************************************************************
void CreateEnabledFeatureStatus(uint8_t *myActiveFeatures, uint8_t *ActiveFeatures_Byte2)
{
    uint8_t myMask;
    uint8_t feature;

    // Adjust available features based upon RNet setting.
    if (g_RNet_Active)
    {
        g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = TRUE;
        g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = TRUE;
    }
    else
    {
        g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available = FALSE;
        g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Available = FALSE;
    }

    *myActiveFeatures = 0x0;
    myMask = 0x01;
    for (feature = 0; feature < 4 /*NUM_FEATURES*/; ++feature)  // Because the RNet feature is in 0x40.
    {
        if ((g_MainScreenFeatureInfo[feature].m_Enabled) && (g_MainScreenFeatureInfo[feature].m_Available))
        {
            // Create the byte to send to the COMM Task to tell Head Array what features are active.
            *myActiveFeatures |= myMask;     // Set bit.
        }
        myMask = (uint8_t)(myMask << 1);    // Rotate the bit.
    }

    // Add clicks in D4 of byte.
    if (g_ClicksActive)
        *myActiveFeatures |= FUNC_FEATURE_SOUND_ENABLED_BIT_MASK;
    // Add power up in D5 of byte;
    if (g_PowerUpInIdle)
        *myActiveFeatures |= FUNC_FEATURE_POWER_UP_IN_IDLE_BIT_MASK;
    // Add RNet Enable in D6 of byte.
    if (g_RNet_Active)
        *myActiveFeatures |= FUNC_FEATURE_RNET_SEATING_MASK;

    // Now assemble the second byte of features.
    *ActiveFeatures_Byte2 = 0x0;
//    if (g_RNet_Sleep_Feature)           // Add RNet Sleep feature setting.
    if ((g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Enabled) && (g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Available))
        *ActiveFeatures_Byte2 |= FUNC_FEATURE2_RNET_SLEEP_BIT_MASK;
    if (g_Mode_Switch_Schema == HUB_MODE_SWITCH_REVERSE)    // Add Mode Switch schema setting
        *ActiveFeatures_Byte2 |= FUNC_FEATURE2_MODE_REVERSE_BIT_MASK;
    if (g_ShowPadsOnMainScreen)
        *ActiveFeatures_Byte2 |= FUNC_FEATURE2_SHOW_PADS_BIT_MASK;
}

//-------------------------------------------------------------------------
void g_timer0_callback(timer_callback_args_t * p_args) //25ms
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
//#define CAPTURE_TOUCHES
#ifdef CAPTURE_TOUCHES
sf_touch_panel_event_t g_Events[128] = {SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID,
                                      SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID, SF_TOUCH_PANEL_EVENT_INVALID};
int g_EventCounter = 0;
#endif

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

#ifdef CAPTURE_TOUCHES
    g_Events[g_EventCounter] = p_payload->event_type;
    ++g_EventCounter;
    g_EventCounter &= 0xff;
    if (g_EventCounter > 0xff)
        while (1) ;
#endif // CAPTURE_TOUCHES

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
        gxe.gx_event_type = GX_EVENT_PEN_DRAG; // GX_EVENT_PEN_MOVE;
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


