  //-------------------------------------------------------------------------
#include "my_gui_thread.h"
#include "lcd.h"
#include <my_gui_thread_entry.h>
#include <stdio.h>
#include "gx_api.h"
//#include "my_guix_resources.h"
#include "ASL_HHP_Display_GUIX_resources.h"
//#include "my_guix_specifications.h"
#include "ASL_HHP_Display_GUIX_specifications.h"
//-------------------------------------------------------------------------
GX_CHAR version_string[16]     = "Version: 0.0.1a";
GX_CHAR version_string2[20] = "Attendant: V0.0.1";
GX_CHAR version_string3[10]     		 	=  "V0.0.1";

//-------------------------------------------------------------------------
// Typdefs and defines
//-------------------------------------------------------------------------

enum ENUM_TIMER_IDS {ARROW_PUSHED_TIMER_ID = 1, CALIBRATION_TIMER_ID, PAD_ACTIVE_TIMER_ID};

//#define UP_ARROW_BTN_ID 2
//#define DOWN_ARROW_BTN_ID 0
#define LONG_PRESS_BUTTON_ID 1

#define min(a,b)   ((a < b) ? a : b)

#define GRAPH_CENTER_PT_XPOS 139    // From Left of screen
#define GRAPH_CENTER_PT_YPOS 130    // From Top of screen

//-------------------------------------------------------------------------
// Local variables
//-------------------------------------------------------------------------

// "MainMenuPrompts" is used to hold information about the 4 items displayed on the main user screen.
// This information includes the pixel maps, location and whether it's programmed for usage or not.
struct MainMenuPrompts
{
    int m_Location;
    int m_Active;       // FALSE is inactive, TRUE is active
    GX_PIXELMAP_PROMPT *m_SmallPrompt;
    GX_PIXELMAP_PROMPT *m_LargePrompt;
} g_ScreenPrompts[4];

GX_RECTANGLE g_HiddenRectangle = {0,0,0,0};

GX_RECTANGLE g_DiagnosticWidgetLocations[] = {
    {35, 32, 35+62, 32+90},
    {183, 32, 183+62, 32+90},
    {66, 140, 66+145, 140+42}};

// This holds the location of the prompts on the main screen for the purpose of showing and hiding the active features.
GX_RECTANGLE g_FeatureLocation[] = {
    {10, 16, 300, 86},
    {30, 94, 290, 130},
    {30, 130, 290, 162},
    {30, 166, 290, 198},
    {0,0,0,0}};

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

int g_ChangeScreen_WIP;
GX_WINDOW *g_GoBackScreen = GX_NULL;
GX_WINDOW *g_CalibrationScreen = GX_NULL;

// Timeout information
int g_TimeoutValue;
GX_PIXELMAP_BUTTON *g_TimeoutIcons[] = {
    &UserSettingsScreen.UserSettingsScreen_Timer_Off_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_10_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_15_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_20_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_25_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_30_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_40_Button,
    &UserSettingsScreen.UserSettingsScreen_Timer_50_Button,
    GX_NULL};
GX_RECTANGLE g_TimeoutValueLocation[] = {
    {140, 60, 140+88, 60+70},
    {0,0,0,0}};


// The following hold the Digital (non0) vs Proportional (0) setting for each pad.
//UINT g_LeftPadSetting = 0, g_RightPadSetting = 0, g_CenterPadSetting = 0;
enum PAD_DESIGNATION {LEFT_PAD, RIGHT_PAD, CENTER_PAD};
enum PAD_DIRECTION {OFF_DIRECTION = 0, LEFT_DIRECTION, FORWARD_DIRECTION, RIGHT_DIRECTION};
enum PAD_TYPE {PROPORTIONAL_PADTYPE, DIGITAL_PADTYPE};

int g_ClicksActive = FALSE;

struct PadInfoStruct
{
    enum PAD_TYPE m_PadType;
    enum PAD_DIRECTION m_PadDirection;
    int m_PadMinimumCalibrationValue;
    int m_PadMaximumCalibrationValue;
    int m_DiagnosticOff_ID;
    GX_PIXELMAP_BUTTON *m_DiagnosticOff_Widget;
    int m_DiagnosticDigital_ID;
    GX_PIXELMAP_BUTTON *m_DiagnosticDigital_Widget;
    int m_DiagnosticProportional_ID;
    GX_PIXELMAP_BUTTON *m_DiagnosticProportional_Widget;
    GX_RECTANGLE m_DiagnosticWidigetLocation;
    GX_PIXELMAP_BUTTON *m_DirectionIcons[4];
} g_PadSettings[3];

int g_SettingsChanged;
int g_CalibrationPadNumber;
int g_CalibrationStepNumber;
int g_PadValue;
int g_DeltaValue;

//-------------------------------------------------------------------------
extern GX_PROMPT * time_infor_pmpt_text;

//-------------------------------------------------------------------------
GX_WINDOW_ROOT * p_window_root;
GX_PROMPT * firmware_ver_text = &init_screen.init_screen_InitPrmpt2;
GX_PROMPT * first_pmpt_text = &init_screen.init_screen_InitPrmpt3;

//-------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------
void my_gui_thread_entry(void);

static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload);

#ifdef OK_TO_USE_RESET
static void reset_check(void);
#endif

UINT SettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT UserSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT FeatureSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT SetPadTypeScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT CalibrationScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);
UINT SetPadDirectionScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr);

UINT DisplayMainScreenActiveFeatures ();
UINT ShowHidePad (GX_EVENT *event_ptr);
void ShowPadTypes (void);
void ShowActiveFeatures (void);
void ShowUserSettingsItems (void);

//-------------------------------------------------------------------------
/* Gui Test App Thread entry function */
void my_gui_thread_entry(void)
{
    ssp_err_t err;
    sf_message_header_t * p_message = NULL;
    UINT status = TX_SUCCESS;
    uint8_t i, test_num;
    
		//debug pins
    g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(TEST_PIN, IOPORT_LEVEL_LOW);
    
    g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);
  	g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);
		g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);

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
    // Power On/Off
    g_ScreenPrompts[0].m_Location = 0;
    g_ScreenPrompts[0].m_Active = TRUE;
    g_ScreenPrompts[0].m_LargePrompt = &Main_User_Screen.Main_User_Screen_PowerLargePrompt;
    g_ScreenPrompts[0].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_PowerSmallPrompt;
    // Bluetooth
    g_ScreenPrompts[1].m_Location = 1;
    g_ScreenPrompts[1].m_Active = TRUE;
    g_ScreenPrompts[1].m_LargePrompt = &Main_User_Screen.Main_User_Screen_BluetoothLargePrompt;
    g_ScreenPrompts[1].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_BluetoothSmallPrompt;
    // Next Function
    g_ScreenPrompts[2].m_Location = 2;
    g_ScreenPrompts[2].m_Active = TRUE;
    g_ScreenPrompts[2].m_LargePrompt = &Main_User_Screen.Main_User_Screen_FunctionNextLargePrompt;
    g_ScreenPrompts[2].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_FunctionNextSmallPrompt;
    // Next Profile
    g_ScreenPrompts[3].m_Location = 3;
    g_ScreenPrompts[3].m_Active = TRUE;
    g_ScreenPrompts[3].m_LargePrompt = &Main_User_Screen.Main_User_Screen_ProfileNextLargePrompt;
    g_ScreenPrompts[3].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_ProfileNextSmallPrompt;

    // Populate the default Pad settings.
    g_PadSettings[LEFT_PAD].m_PadDirection = LEFT_DIRECTION;
    g_PadSettings[LEFT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[OFF_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Off_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[RIGHT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_RightArrow_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[LEFT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_LeftArrow_Button;
    g_PadSettings[LEFT_PAD].m_DirectionIcons[FORWARD_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_ForwardArrow_Button;
    g_PadSettings[LEFT_PAD].m_PadMinimumCalibrationValue = 0;
    g_PadSettings[LEFT_PAD].m_PadMaximumCalibrationValue = 100;
    g_PadSettings[LEFT_PAD].m_DiagnosticWidigetLocation = g_DiagnosticWidgetLocations[LEFT_PAD];
    g_PadSettings[LEFT_PAD].m_DiagnosticOff_ID = LEFT_PAD_OFF_BTN_ID;
    g_PadSettings[LEFT_PAD].m_DiagnosticDigital_ID = LEFT_PAD_DIGITAL_BTN_ID;
    g_PadSettings[LEFT_PAD].m_DiagnosticProportional_ID = LEFT_PAD_PROP_BTN_ID;
    g_PadSettings[LEFT_PAD].m_DiagnosticOff_Widget = &DiagnosticScreen.DiagnosticScreen_LeftPadOff_Button;
    g_PadSettings[LEFT_PAD].m_DiagnosticProportional_Widget = &DiagnosticScreen.DiagnosticScreen_LeftPadProp_Button;
    g_PadSettings[LEFT_PAD].m_DiagnosticDigital_Widget = &DiagnosticScreen.DiagnosticScreen_LeftPadDigital_Button;

    g_PadSettings[RIGHT_PAD].m_PadDirection = RIGHT_DIRECTION;
    g_PadSettings[RIGHT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[OFF_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Off_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[RIGHT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_RightArrow_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[LEFT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_LeftArrow_Button;
    g_PadSettings[RIGHT_PAD].m_DirectionIcons[FORWARD_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_ForwardArrow_Button;
    g_PadSettings[RIGHT_PAD].m_PadMinimumCalibrationValue = 5;
    g_PadSettings[RIGHT_PAD].m_PadMaximumCalibrationValue = 95;
    g_PadSettings[RIGHT_PAD].m_DiagnosticWidigetLocation = g_DiagnosticWidgetLocations[RIGHT_PAD];
    g_PadSettings[RIGHT_PAD].m_DiagnosticOff_ID = RIGHT_PAD_OFF_BTN_ID;
    g_PadSettings[RIGHT_PAD].m_DiagnosticDigital_ID = RIGHT_PAD_DIGITAL_BTN_ID;
    g_PadSettings[RIGHT_PAD].m_DiagnosticProportional_ID = RIGHT_PAD_PROP_BTN_ID;
    g_PadSettings[RIGHT_PAD].m_DiagnosticOff_Widget = &DiagnosticScreen.DiagnosticScreen_RightPadOff_Button;
    g_PadSettings[RIGHT_PAD].m_DiagnosticProportional_Widget = &DiagnosticScreen.DiagnosticScreen_RightPadProp_Button;
    g_PadSettings[RIGHT_PAD].m_DiagnosticDigital_Widget = &DiagnosticScreen.DiagnosticScreen_RightPadDigital_Button;

    g_PadSettings[CENTER_PAD].m_PadDirection = FORWARD_DIRECTION;
    g_PadSettings[CENTER_PAD].m_PadType = PROPORTIONAL_PADTYPE;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[OFF_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Off_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[RIGHT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_RightArrow_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[LEFT_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_LeftArrow_Button;
    g_PadSettings[CENTER_PAD].m_DirectionIcons[FORWARD_DIRECTION] = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_ForwardArrow_Button;
    g_PadSettings[CENTER_PAD].m_PadMinimumCalibrationValue = 10;
    g_PadSettings[CENTER_PAD].m_PadMaximumCalibrationValue = 90;
    g_PadSettings[CENTER_PAD].m_DiagnosticWidigetLocation = g_DiagnosticWidgetLocations[CENTER_PAD];
    g_PadSettings[CENTER_PAD].m_DiagnosticOff_ID = CENTER_PAD_OFF_BTN_ID;
    g_PadSettings[CENTER_PAD].m_DiagnosticDigital_ID = CENTER_PAD_DIGITAL_BTN_ID;
    g_PadSettings[CENTER_PAD].m_DiagnosticProportional_ID = CENTER_PAD_PROP_BTN_ID;
    g_PadSettings[CENTER_PAD].m_DiagnosticOff_Widget = &DiagnosticScreen.DiagnosticScreen_CenterPadOff_Button;
    g_PadSettings[CENTER_PAD].m_DiagnosticProportional_Widget = &DiagnosticScreen.DiagnosticScreen_CenterPadProp_Button;
    g_PadSettings[CENTER_PAD].m_DiagnosticDigital_Widget = &DiagnosticScreen.DiagnosticScreen_CenterPadDigital_Button;

    /* Create the widgets we have defined with the GUIX data structures and resources. */
    GX_WIDGET * p_first_screen = NULL;
    
		
		/* Create the screens. */
    // gx_studio_named_widget_create("edit_screen", (GX_WIDGET *)p_window_root, &p_first_screen);
    // gx_studio_named_widget_create("DateTime_screen", (GX_WIDGET *)p_window_root, &p_first_screen);
    //gx_studio_named_widget_create("edit_screen", GX_NULL, GX_NULL);
    //gx_studio_named_widget_create("popup_screen2", GX_NULL, GX_NULL);
    //gx_studio_named_widget_create("DateTime_screen", GX_NULL, GX_NULL);
    //gx_studio_named_widget_create("popup_screen", GX_NULL, GX_NULL);

  #ifdef using_keyboard   
//    gx_studio_named_widget_create("keyboard_screen", GX_NULL, GX_NULL);
	#endif
		
//	gx_studio_named_widget_create("information_screen", GX_NULL, GX_NULL);
//    status = gx_studio_named_widget_create("HHP_Start_Screen", (GX_WIDGET *)p_window_root, &HHP_Start_Screen_Widget);

    gx_studio_named_widget_create("SetPadDirectionScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("SetPadTypeScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("PadCalibrationScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("FeatureSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("PadOptionsSettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("SettingsScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("DiagnosticScreen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("UserSettingsScreen", GX_NULL, GX_NULL);
	gx_studio_named_widget_create("HHP_Start_Screen", GX_NULL, GX_NULL);
    gx_studio_named_widget_create("Main_User_Screen", (GX_WIDGET *)p_window_root, &p_first_screen);    // Create and show first startup screen.

    /* Attach the first screen to the root so we can see it when the root is shown */
    gx_widget_attach(p_window_root, p_first_screen);

    /* Shows the root window to make it and patients screen visible. */
    gx_widget_show(p_window_root);

    /* Lets GUIX run. */
    gx_system_start();

//    gx_system_focus_claim(p_first_screen);

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


    /** Setup the ILI9341V **/
    ILI9341V_Init();
		
	InitializeSys();

  	LCD_ON();
		
	chk_status_timeout = 14; //(14+2)*25 = 400ms
	//debug_timer0_flag = 0;
	err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
	if (err != SSP_SUCCESS)
	{
		g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);	//Error
	}
		
	// version of firmware
	status = gx_prompt_text_set(firmware_ver_text, version_string);
	if (GX_SUCCESS != status)
    {
    	g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
    }
    
//    status = gx_prompt_text_set(first_pmpt_text, "Init...");
//	if (GX_SUCCESS != status)
//    {
//    	g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
//    }
    
	Process_Touches();

// Removed the RTCC.
//	init_rtcc();

    //delay 4.5s	
//    R_BSP_SoftwareDelay(4500, BSP_DELAY_UNITS_MILLISECONDS);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
 	
//    //Open WDT; 4.46s; PCLKB 30MHz
// 	g_wdt.p_api->open(g_wdt.p_ctrl, g_wdt.p_cfg);
//	//Start the WDT by refreshing it
// 	g_wdt.p_api->refresh(g_wdt.p_ctrl);
  
  	i = 0;
  	do {
  		test_num = get_PROP_version();
  		
    	//GC Replace the following with a tx_thread_sleep
  		//GC    R_BSP_SoftwareDelay(30, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(30);
  		tx_thread_sleep(10);
    	//Start the WDT by refreshing it
// 	  	g_wdt.p_api->refresh(g_wdt.p_ctrl);
    	if(i > 5)
    	    break;
    	i++;
  	} while(test_num != 0);

  	if (i > 5)
  	{
    	while(1)
    	{
    	    Process_Touches();
    		//reset_check();
    		tx_thread_sleep (2);
    	}
  	}
 	
  	//beep
//	beep_set(3, 300);
  	
  	Shut_down_display_timeout = shut_down_timer1;
  	
  	chk_status_timeout = 20;	//delay about 550ms
  	
    //Open WDT; 4.46s; PCLKB 30MHz
 	//  g_wdt.p_api->open(g_wdt.p_ctrl, g_wdt.p_cfg);
		//Start the WDT by refreshing it
 	//  g_wdt.p_api->refresh(g_wdt.p_ctrl);

  	
    while(1)
    {
        err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &my_gui_thread_message_queue, (sf_message_header_t **) &p_message, 10); //TX_WAIT_FOREVER); //
        if(!err)
        {
            switch (p_message->event_b.class_code)
            {
                case SF_MESSAGE_EVENT_CLASS_TOUCH:
                    if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
                    {
                        //sf_touch_panel_payload_t * p_touch_message = (sf_touch_panel_payload_t *) p_message;

                        // Translate a touch event into a GUIX event 
                        guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
                    }
                    break;

                default:
                    break;
            }

            //Message is processed, so release buffer.
            err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
            if (err)
                {
                    g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
                }

        }

        if( chk_status_timeout == 0 && LCD_off_flag == 0 )
        { //adding LCD_off_flag check is for clear 'sigma_dat1_get'
        
          get_PROP_version();//chk_sigma_status();
          chk_status_timeout = 20;	//delay about 550ms
          
          Process_Touches();
        }
        
        if(LCD_off_flag == 1)
            main_menu();
        
        if(Shut_down_display_timeout == 0 && Shut_down_display_timeout2 == 0 && LCD_off_flag == 0)
        {
          BackLight(OFF);
          LCD_off_flag = 1;
        }

        //Refresh WDT
 	  	g_wdt.p_api->refresh(g_wdt.p_ctrl);

    }
}

//*************************************************************************************
// Function Name: DisplayMainScreenActiveFeatures
//
// Description: This displays the features that are active in the order specificed
//  in the Screen Prompts "objects".
//
//*************************************************************************************

UINT DisplayMainScreenActiveFeatures ()
{
    int activeCount;
    int feature;

    // Count the number of active items so we can populate appropriately.
    // Hide the Non-Active features.
    activeCount = 0;
    for (feature = 0; feature < 4; ++feature)
    {
        if (g_ScreenPrompts[feature].m_Active == FALSE)
        {
            gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
            gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_HiddenRectangle);
            ++activeCount;
        }
    }

    // Locate the first feature to display
    for (feature = 0; feature < 4; ++feature)
    {
        if (g_ScreenPrompts[feature].m_Active)
        {
            switch (g_ScreenPrompts[feature].m_Location)
            {
            case 0: // Show the first line
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_FeatureLocation[0]);
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_HiddenRectangle);
                break;
            case 1: // Show second line item
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_FeatureLocation[1]);
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
                break;
            case 2: // Process third line item, move to the 2nd line
                // Hide Large Icon, show small icon
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_FeatureLocation[2]);
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
                break;
            case 3: // Process fourth line item, move to the 3rd line.
                // Hide Large Icon, show small icon
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_FeatureLocation[3]);
                gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
                break;
            }
        }
    }
    return GX_SUCCESS;
}



//*************************************************************************************
// Function Name: Main_User_Screen_event_process
//
// Description: This handles the User Screen messages.
//
//*************************************************************************************
VOID Main_User_Screen_draw_function(GX_WINDOW *window)
{
    gx_window_draw(window);

}

UINT Main_User_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    UINT feature;
    int activeCount;

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_TIMER:
        if (event_ptr->gx_event_payload.gx_event_timer_id == ARROW_PUSHED_TIMER_ID)
        {
            gx_widget_attach (p_window_root, (GX_WIDGET*) &HHP_Start_Screen);
            gx_widget_show ((GX_WIDGET*) &HHP_Start_Screen);
            g_ChangeScreen_WIP = TRUE;
        }
        break;
//    case GX_EVENT_PEN_DOWN: // We are going to determine if the Up or Down arrow buttons have been held for a
//                            // ... long time (2 seconds) and goto Programming if so.
//
//        if ((event_ptr->gx_event_target->gx_widget_id == DOWN_ARROW_ID))
//        if ((event_ptr->gx_event_target->gx_widget_name == "DownArrowButton") || (event_ptr->gx_event_target->gx_widget_name == "UpArrowButton"))
//        {
//            gx_system_timer_start(window, ARROW_PUSHED_TIMER_ID, 100, 0);
//            g_ChangeScreen_WIP = FALSE;
//        }
//        break;
//    case GX_EVENT_PEN_UP:
//            gx_system_timer_stop(window, ARROW_PUSHED_TIMER_ID);
//        break;
//
    case GX_EVENT_SHOW:
        g_GoBackScreen = window;        // Set the going back window.
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL (LONG_PRESS_BUTTON_ID, GX_EVENT_CLICKED):
        // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
        if (g_ChangeScreen_WIP)
        {
            g_ChangeScreen_WIP = FALSE;
            break;
        }
        // Count the number of active features to set a limit on location
        activeCount = 0;
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
                ++activeCount;
        }
        // Move Top Feature to Bottom and move Bottom upward.
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
            {
                if (g_ScreenPrompts[feature].m_Location == 0)
                    g_ScreenPrompts[feature].m_Location = activeCount-1;
                else if (g_ScreenPrompts[feature].m_Location == 1)
                    g_ScreenPrompts[feature].m_Location = 0;
                else if (g_ScreenPrompts[feature].m_Location == 2)
                    g_ScreenPrompts[feature].m_Location = min (1, activeCount-1);
                else if (g_ScreenPrompts[feature].m_Location == 3)
                    g_ScreenPrompts[feature].m_Location = min (2, activeCount-1);
            }
        }
        DisplayMainScreenActiveFeatures();
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
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
                ++activeCount;
        }

        // Move the features downward, limiting the movement by the number of Active Features.
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
            {
                if (g_ScreenPrompts[feature].m_Location == 0)
                    g_ScreenPrompts[feature].m_Location = min (1, activeCount);
                else if (g_ScreenPrompts[feature].m_Location == 1)
                    g_ScreenPrompts[feature].m_Location = min (2, activeCount);
                else if (g_ScreenPrompts[feature].m_Location == 2)
                    g_ScreenPrompts[feature].m_Location = min (3, activeCount);
                else if (g_ScreenPrompts[feature].m_Location == 3)
                    g_ScreenPrompts[feature].m_Location = 0;
                if (g_ScreenPrompts[feature].m_Location == activeCount)
                    g_ScreenPrompts[feature].m_Location = 0;
            }
        }
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL (BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &HHP_Start_Screen);
        gx_widget_show ((GX_WIDGET*) &HHP_Start_Screen);
        g_ChangeScreen_WIP = TRUE;
        break;

    }

    DisplayMainScreenActiveFeatures();  // Remove this when more of the previous code is "undefinded".

    gx_window_event_process(window, event_ptr);

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
    case GX_SIGNAL(DIAGNOSTIC_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &DiagnosticScreen);
        gx_widget_show ((GX_WIDGET*) &DiagnosticScreen);
        break;

    case GX_SIGNAL(SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &SettingsScreen);
        gx_widget_show ((GX_WIDGET*) &SettingsScreen);
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) g_GoBackScreen);
        gx_widget_show ((GX_WIDGET*) g_GoBackScreen);
        break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}


//*************************************************************************************
// Function Name: ShowHidePad
//
// Description: This functions displays an green-illuminated pad simulating an active pad.
//
//*************************************************************************************

UINT ShowHidePad (GX_EVENT *event_ptr)
{
    int pad;
//  GX_WIDGET *myWidget;
    GX_VALUE xPos, yPos;

    // We can only look at the PEN UP and PEN DOWN events and it only provides
    if (event_ptr->gx_event_target->gx_widget_first_child == NULL)
        return GX_FAILURE;
    // We are going to identify the "press" pad by its location.
    xPos = event_ptr->gx_event_payload.gx_event_pointdata.gx_point_x;
    yPos = event_ptr->gx_event_payload.gx_event_pointdata.gx_point_y;

    // Now traverse the entire set of widgets for this window looking for the the "Pressed" icon.
    for (pad = 0; pad <= 2; ++pad)
    {
        if ((yPos >= g_PadSettings[pad].m_DiagnosticWidigetLocation.gx_rectangle_top)
            && (yPos <= g_PadSettings[pad].m_DiagnosticWidigetLocation.gx_rectangle_bottom)
            && (xPos >= g_PadSettings[pad].m_DiagnosticWidigetLocation.gx_rectangle_left)
            && (xPos <= g_PadSettings[pad].m_DiagnosticWidigetLocation.gx_rectangle_right))
        // If we got the right pad, then do something.
        if (g_PadSettings[pad].m_PadDirection != OFF_DIRECTION)
        {
            // Determine if we are going to show it or hide it.
            if (event_ptr->gx_event_type == GX_EVENT_PEN_DOWN)
            {
                // Determine if we show the Proportional (Orange) or Digital (Green)
                if (g_PadSettings[pad].m_PadType == PROPORTIONAL_PADTYPE)
                    gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticProportional_Widget , &g_PadSettings[pad].m_DiagnosticWidigetLocation);
                else
                    gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticDigital_Widget , &g_PadSettings[pad].m_DiagnosticWidigetLocation);
            }
            else // PEN UP
            {
                // Now we are going to hide it.
                if (g_PadSettings[pad].m_PadType == PROPORTIONAL_PADTYPE)
                    gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticProportional_Widget , &g_HiddenRectangle);
                else
                    gx_widget_resize ((GX_WIDGET*)g_PadSettings[pad].m_DiagnosticDigital_Widget , &g_HiddenRectangle);
            }
        } // endif !OFF
    } // end while
    return (GX_SUCCESS);
}

//*************************************************************************************
// Function Name: DiagnosticScreen_event_handler
//
// Description: This handles the Diagnostic Screen messages
//
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
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &HHP_Start_Screen);
        gx_widget_show ((GX_WIDGET*) &HHP_Start_Screen);
        break;

    case GX_EVENT_PEN_DOWN:
    case GX_EVENT_PEN_UP:
        ShowHidePad (event_ptr);
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

UINT SettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &HHP_Start_Screen);
        gx_widget_show ((GX_WIDGET*) &HHP_Start_Screen);
        break;

    case GX_SIGNAL(GOTO_PAD_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &PadOptionsSettingsScreen);
        gx_widget_show ((GX_WIDGET*) &PadOptionsSettingsScreen);
        break;

    case GX_SIGNAL(GOTO_USER_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &UserSettingsScreen);
        gx_widget_show ((GX_WIDGET*) &UserSettingsScreen);
        break;

    case GX_SIGNAL(FEATURES_SETTINGS_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &FeatureSettingsScreen);
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen);
        break;
    }

    gx_window_event_process(window, event_ptr);

    return (GX_SUCCESS);
}

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
        gx_widget_attach (p_window_root, (GX_WIDGET*) &SettingsScreen);
        gx_widget_show ((GX_WIDGET*) &SettingsScreen);
        break;

    case GX_SIGNAL(GOTO_PAD_TYPE_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &SetPadTypeScreen);
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen);
        break;

    case GX_SIGNAL(GOTO_PAD_DIRECTIONS_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &SetPadDirectionScreen);
        gx_widget_show ((GX_WIDGET*) &SetPadDirectionScreen);
        break;
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
            for (icons = 0; icons < 4; ++icons)
            {
                gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DirectionIcons[icons], &g_HiddenRectangle);
            }
            gx_widget_resize ((GX_WIDGET*) g_PadSettings[pads].m_DirectionIcons[g_PadSettings[pads].m_PadDirection], &g_PadDirectionLocation[pads]);
        }
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &PadOptionsSettingsScreen);
        gx_widget_show ((GX_WIDGET*) &PadOptionsSettingsScreen);
        break;
    // Process LEFT button pushes
    case GX_SIGNAL(LEFT_PAD_OFF_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[OFF_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[LEFT_DIRECTION], &g_PadDirectionLocation[LEFT_PAD]);
        g_PadSettings[LEFT_PAD].m_PadDirection = LEFT_DIRECTION;
        break;
    case GX_SIGNAL(LEFT_PAD_LEFT_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[LEFT_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[FORWARD_DIRECTION], &g_PadDirectionLocation[LEFT_PAD]);
        g_PadSettings[LEFT_PAD].m_PadDirection = FORWARD_DIRECTION;
        break;
    case GX_SIGNAL(LEFT_PAD_FORWARD_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[FORWARD_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[RIGHT_DIRECTION], &g_PadDirectionLocation[LEFT_PAD]);
        g_PadSettings[LEFT_PAD].m_PadDirection = RIGHT_DIRECTION;
        break;
    case GX_SIGNAL(LEFT_PAD_RIGHT_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[RIGHT_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[LEFT_PAD].m_DirectionIcons[OFF_DIRECTION], &g_PadDirectionLocation[LEFT_PAD]);
        g_PadSettings[LEFT_PAD].m_PadDirection = OFF_DIRECTION;
        break;
    // Process RIGHT button pushes
    case GX_SIGNAL(RIGHT_PAD_OFF_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[OFF_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[LEFT_DIRECTION], &g_PadDirectionLocation[RIGHT_PAD]);
        g_PadSettings[RIGHT_PAD].m_PadDirection = LEFT_DIRECTION;
        break;
    case GX_SIGNAL(RIGHT_PAD_LEFT_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[LEFT_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[FORWARD_DIRECTION], &g_PadDirectionLocation[RIGHT_PAD]);
        g_PadSettings[RIGHT_PAD].m_PadDirection = FORWARD_DIRECTION;
        break;
    case GX_SIGNAL(RIGHT_PAD_FORWARD_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[FORWARD_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[RIGHT_DIRECTION], &g_PadDirectionLocation[RIGHT_PAD]);
        g_PadSettings[RIGHT_PAD].m_PadDirection = RIGHT_DIRECTION;
        break;
    case GX_SIGNAL(RIGHT_PAD_RIGHT_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[RIGHT_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[RIGHT_PAD].m_DirectionIcons[OFF_DIRECTION], &g_PadDirectionLocation[RIGHT_PAD]);
        g_PadSettings[RIGHT_PAD].m_PadDirection = OFF_DIRECTION;
        break;
    // Process CENTER PAD button pushes
    case GX_SIGNAL(CENTER_PAD_OFF_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[OFF_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[LEFT_DIRECTION], &g_PadDirectionLocation[CENTER_PAD]);
        g_PadSettings[CENTER_PAD].m_PadDirection = LEFT_DIRECTION;
        break;
    case GX_SIGNAL(CENTER_PAD_LEFT_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[LEFT_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[FORWARD_DIRECTION], &g_PadDirectionLocation[CENTER_PAD]);
        g_PadSettings[CENTER_PAD].m_PadDirection = FORWARD_DIRECTION;
        break;
    case GX_SIGNAL(CENTER_PAD_FORWARD_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[FORWARD_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[RIGHT_DIRECTION], &g_PadDirectionLocation[CENTER_PAD]);
        g_PadSettings[CENTER_PAD].m_PadDirection = RIGHT_DIRECTION;
        break;
    case GX_SIGNAL(CENTER_PAD_RIGHT_ARROW_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[RIGHT_DIRECTION], &g_HiddenRectangle);
        gx_widget_resize ((GX_WIDGET*) g_PadSettings[CENTER_PAD].m_DirectionIcons[OFF_DIRECTION], &g_PadDirectionLocation[CENTER_PAD]);
        g_PadSettings[CENTER_PAD].m_PadDirection = OFF_DIRECTION;
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

void ShowUserSettingsItems (void)
{
    int feature;

    if (g_ClicksActive)
    {
        gx_widget_show ((GX_WIDGET*) &UserSettingsScreen.UserSettingsScreen_Clicks_ActiveIcon);
        gx_widget_hide ((GX_WIDGET*) &UserSettingsScreen.UserSettingsScreen_Clicks_InactiveIcon);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &UserSettingsScreen.UserSettingsScreen_Clicks_InactiveIcon);
        gx_widget_hide ((GX_WIDGET*) &UserSettingsScreen.UserSettingsScreen_Clicks_ActiveIcon);
    }
    // Show the Timeout Value
    for (feature = 0; g_TimeoutIcons[feature] != GX_NULL; ++feature)
    {
        gx_widget_resize ((GX_WIDGET*) g_TimeoutIcons[feature], &g_TimeoutValueLocation[1]);
    }
    gx_widget_resize ((GX_WIDGET*) g_TimeoutIcons[g_TimeoutValue], &g_TimeoutValueLocation[0]);
}

UINT UserSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &SettingsScreen);
        gx_widget_show ((GX_WIDGET*) &SettingsScreen);
        break;

        // Click (Audio) Feature handling
    case GX_SIGNAL(CLICKS_INACTIVE_ICON, GX_EVENT_CLICKED):
    case GX_SIGNAL(CLICKS_ACTIVE_ICON, GX_EVENT_CLICKED):
        g_ClicksActive = (g_ClicksActive == TRUE ? FALSE : TRUE);
        break;

    case GX_SIGNAL(TIMER_OFF_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_10_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_15_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_20_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_25_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_30_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_40_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(TIMER_50_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_resize ((GX_WIDGET*) g_TimeoutIcons[g_TimeoutValue], &g_HiddenRectangle);
        ++g_TimeoutValue;
        if (g_TimeoutIcons[g_TimeoutValue] == GX_NULL)
            g_TimeoutValue = 0;
        gx_widget_resize ((GX_WIDGET*) g_TimeoutIcons[g_TimeoutValue], &g_TimeoutValueLocation[0]);
        break;
    }

    ShowUserSettingsItems();

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//*************************************************************************************
// Function Name: ShowActiveFeatures
//
// Description: This displays the Active/Inactive status of the features.
//
//*************************************************************************************

void ShowActiveFeatures (void)
{
    // Power status
    if (g_ScreenPrompts[0].m_Active)
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Power_ActiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Power_InactiveIcon);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Power_InactiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Power_ActiveIcon);
    }
    // Bluetooth
    if (g_ScreenPrompts[1].m_Active)
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Bluetooth_ActiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Bluetooth_InactiveIcon);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Bluetooth_InactiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_Bluetooth_ActiveIcon);
    }
    // Next Function
    if (g_ScreenPrompts[2].m_Active)
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextFunction_ActiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextFunction_InactiveIcon);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextFunction_InactiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextFunction_ActiveIcon);
    }
    // Next Profile
    if (g_ScreenPrompts[3].m_Active)
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextProfile_ActiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextProfile_InactiveIcon);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextProfile_InactiveIcon);
        gx_widget_hide ((GX_WIDGET*) &FeatureSettingsScreen.FeatureSettingsScreen_NextProfile_ActiveIcon);
    }
}


//*************************************************************************************
// Function Name: FeatureSettingsScreen_event_process
//
// Description: This handles the Feature Settings Screen messages
//
//*************************************************************************************

UINT FeatureSettingsScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    int feature;
    int numActive;

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        g_SettingsChanged = FALSE;
        break;

    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &SettingsScreen);
        gx_widget_show ((GX_WIDGET*) &SettingsScreen);
        if (g_SettingsChanged)
        {
            numActive = 0;
            for (feature = 0; feature < 4; ++feature)
            {
                if (g_ScreenPrompts[feature].m_Active)
                {
                    g_ScreenPrompts[feature].m_Location = numActive;
                    ++numActive;
                }
            }
        }
        break;

        // Power Feature handling
//    case GX_SIGNAL(POWER_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL(POWER_INACTIVE_ICON, GX_EVENT_CLICKED):
    case GX_SIGNAL(POWER_ACTIVE_ICON, GX_EVENT_CLICKED):
        g_ScreenPrompts[0].m_Active = (g_ScreenPrompts[0].m_Active==TRUE ? FALSE : TRUE);
        g_SettingsChanged = TRUE;
        break;

        // Bluetooth Feature handling
    case GX_SIGNAL(BLUETOOTH_INACTIVE_ICON, GX_EVENT_CLICKED):
    case GX_SIGNAL(BLUETOOTH_ACTIVE_ICON, GX_EVENT_CLICKED):
        g_ScreenPrompts[1].m_Active = (g_ScreenPrompts[1].m_Active==TRUE ? FALSE : TRUE);
        g_SettingsChanged = TRUE;
        break;

        // Next Function Feature handling
    case GX_SIGNAL(NEXT_FUNCTION_INACTIVE_ICON, GX_EVENT_CLICKED):
    case GX_SIGNAL(NEXT_FUNCTION_ACTIVE_ICON, GX_EVENT_CLICKED):
        g_ScreenPrompts[2].m_Active = (g_ScreenPrompts[2].m_Active==TRUE ? FALSE : TRUE);
        g_SettingsChanged = TRUE;
        break;

        // Next Profile Feature handling
    case GX_SIGNAL(NEXT_PROFILE_INACTIVE_ICON, GX_EVENT_CLICKED):
    case GX_SIGNAL(NEXT_PROFILE_ACTIVE_ICON, GX_EVENT_CLICKED):
        g_ScreenPrompts[3].m_Active = (g_ScreenPrompts[3].m_Active==TRUE ? FALSE : TRUE);
        g_SettingsChanged = TRUE;
        break;
    }

    ShowActiveFeatures ();

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
    if (g_PadSettings[LEFT_PAD].m_PadType)  // Digital?
    {
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
    }
    if (g_PadSettings[RIGHT_PAD].m_PadType) // Digital?
    {
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
    }
    else
    {
        gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
        gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
    }
    if (g_PadSettings[CENTER_PAD].m_PadType)    // Digital?
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
UINT SetPadTypeScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        g_ChangeScreen_WIP = FALSE;
        break;
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        gx_widget_attach (p_window_root, (GX_WIDGET*) &PadOptionsSettingsScreen);
        gx_widget_show ((GX_WIDGET*) &PadOptionsSettingsScreen);
        break;
    case GX_SIGNAL(RIGHT_PAD_DIGITAL_BTN_ID, GX_EVENT_CLICKED):
        if (!g_ChangeScreen_WIP)
        {
//            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
//            gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
            g_PadSettings[RIGHT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
        }
        break;
    case GX_SIGNAL(RIGHT_PAD_PROPORTIONAL_BTN_ID, GX_EVENT_CLICKED):
        if (!g_ChangeScreen_WIP)
        {
//            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadProportional_Button);
//            gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_RightPadDigital_Button);
            g_PadSettings[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
        }
        break;
    case GX_SIGNAL(LEFT_PAD_DIGITAL_BTN_ID, GX_EVENT_CLICKED):
        if (!g_ChangeScreen_WIP)
        {
//            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
//            gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
            g_PadSettings[LEFT_PAD].m_PadType = PROPORTIONAL_PADTYPE;
        }
        break;
    case GX_SIGNAL(LEFT_PAD_PROPORTIONAL_BTN_ID, GX_EVENT_CLICKED):
        if (!g_ChangeScreen_WIP)
        {
//            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadProportional_Button);
//            gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_LeftPadDigital_Button);
            g_PadSettings[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
        }
        break;
    case GX_SIGNAL(CENTER_PAD_DIGITAL_BTN_ID, GX_EVENT_CLICKED):
        if (!g_ChangeScreen_WIP)
        {
 //           gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadDigital_Button);
 //           gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadProportional_Button);
            g_PadSettings[CENTER_PAD].m_PadType = PROPORTIONAL_PADTYPE;
        }
        break;
    case GX_SIGNAL(CENTER_PAD_PROPORTIONAL_BTN_ID, GX_EVENT_CLICKED):
        if (!g_ChangeScreen_WIP)
        {
//            gx_widget_hide ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadProportional_Button);
//            gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen.SetPadTypeScreen_CenterPadDigital_Button);
            g_PadSettings[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
        }
        break;

    case GX_EVENT_TIMER:
        if (event_ptr->gx_event_payload.gx_event_timer_id == CALIBRATION_TIMER_ID)
        {
            gx_system_timer_stop(window, CALIBRATION_TIMER_ID);
            gx_widget_attach (p_window_root, (GX_WIDGET*) &PadCalibrationScreen);
            gx_widget_show ((GX_WIDGET*) &PadCalibrationScreen);
            g_ChangeScreen_WIP = TRUE;
        }
        break;
    case GX_EVENT_PEN_DOWN: // We are going to determine if the Up or Down arrow buttons have been held for a
                            // ... long time (2 seconds) and goto calibration if so.

        if (event_ptr->gx_event_target->gx_widget_id == CENTER_PAD_PROPORTIONAL_BTN_ID)
        {
            g_CalibrationPadNumber = CENTER_PAD;
            gx_system_timer_start(window, CALIBRATION_TIMER_ID, 100, 0);
        }
        else if (event_ptr->gx_event_target->gx_widget_id == LEFT_PAD_PROPORTIONAL_BTN_ID)
        {
            g_CalibrationPadNumber = LEFT_PAD;
            gx_system_timer_start(window, CALIBRATION_TIMER_ID, 100, 0);
        }
        else if (event_ptr->gx_event_target->gx_widget_id == RIGHT_PAD_PROPORTIONAL_BTN_ID)
        {
            g_CalibrationPadNumber = RIGHT_PAD;
            gx_system_timer_start(window, CALIBRATION_TIMER_ID, 100, 0);
        }
        break;
    case GX_EVENT_PEN_UP:
            gx_system_timer_stop(window, CALIBRATION_TIMER_ID);
        break;

    }

    ShowPadTypes();

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

    gx_window_draw(window);

    gx_context_brush_get(&brush);
    originalBrush = *brush;

    // Draw the background
    brush->gx_brush_line_color = GX_COLOR_LIGHTGRAY;
    brush->gx_brush_width = 3;
    brush->gx_brush_fill_color = GX_COLOR_DARKGRAY;
    gx_canvas_pie_draw (GRAPH_CENTER_PT_XPOS, GRAPH_CENTER_PT_YPOS, 55, -5, 185);

    // Draw the Pad pie
    if (g_PadValue > 0)             // Anything less than 175-180 is too small of a pie to see; if it's 180 it draws a full circle.
    {
        raw100 = 100 - g_PadValue;
        raw100 *= 100;                  // Integer math, yuch!
        raw100 *= 18;                   // This converts the percentage to degrees which is a factor of 1.8
        pieSide = raw100 / 1000;        // This is includes the decimal shift.
        brush->gx_brush_width = 2;
        brush->gx_brush_fill_color = GX_COLOR_GREEN;
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
    //brush->gx_brush_line_color = GX_COLOR_BLACK;
    //brush->gx_brush_style = GX_BRUSH_OUTLINE;
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
    brush->gx_brush_fill_color = 0xff6a00;  // Orange
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
            gx_widget_attach (p_window_root, (GX_WIDGET*) &SetPadTypeScreen);
            gx_widget_show ((GX_WIDGET*) &SetPadTypeScreen);
        }
        break;
    case GX_SIGNAL(DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (g_CalibrationStepNumber == 0)       // We are doing minimum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue > 4)
                g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue -= 5;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);
        }
        else if (g_CalibrationStepNumber == 1)  // Doing maximum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue > 4)
                g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue -= 5;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
        }
        gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        break;
    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (g_CalibrationStepNumber == 0)       // We are doing minimum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue < 100)
                g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue += 5;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);
        }
        else if (g_CalibrationStepNumber == 1)  // Doing maximum
        {
            if (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue < 100)
                g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue += 5;
            gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
        }
        gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        break;

    }
    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

//-------------------------------------------------------------------------
void  g_timer0_callback(timer_callback_args_t * p_args) //25ms
{
  (void)p_args;


  if(Shut_down_display_timeout != 0) Shut_down_display_timeout--;

  if(Shut_down_display_timeout2 != 0) Shut_down_display_timeout2--;

  if(chk_status_timeout != 0) chk_status_timeout--;

  if(LongHoldtmr != 0) LongHoldtmr--;

  if(chk_date_time_timeout != 0) chk_date_time_timeout--;

}

//-------------------------------------------------------------------------
void  g_timer1_callback(timer_callback_args_t * p_args) //1612us
{
  (void)p_args;


  if(beep_level == IOPORT_LEVEL_LOW) beep_level = IOPORT_LEVEL_HIGH;
  else beep_level = IOPORT_LEVEL_LOW;

  g_ioport.p_api->pinWrite(beep_out, beep_level);

}

//-------------------------------------------------------------------------
void  g_timer2_callback(timer_callback_args_t * p_args) //819us
{
  (void)p_args;


  if(beep_level == IOPORT_LEVEL_LOW) beep_level = IOPORT_LEVEL_HIGH;
  else beep_level = IOPORT_LEVEL_LOW;

  g_ioport.p_api->pinWrite(beep_out, beep_level);

}

//-------------------------------------------------------------------------
void g_lcd_spi_callback(spi_callback_args_t * p_args)
{
  if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    tx_semaphore_ceiling_put(&g_my_gui_semaphore, 1);

}

//-------------------------------------------------------------------------

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
        gxe.gx_event_type = GX_EVENT_PEN_DRAG;
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
#ifdef USE
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

#ifdef USE_OLD_CODE
    GX_EVENT gxe;

    gxe.gx_event_sender = GX_ID_NONE;
    gxe.gx_event_target = 0;
    gxe.gx_event_display_handle = 0;
    gxe.gx_event_type = UPDATE_DISPLAY_EVENT;
    gx_system_event_send(&gxe);
#endif

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

//-------------------------------------------------------------------------
#ifdef OK_TO_USE_RESET

static void reset_check(void)
{
    UINT status = TX_SUCCESS;


  switch(get_sw()) {
    case sw_fwd_rev:
            status = gx_prompt_text_set(first_pmpt_text, "Reset Device");
                if (GX_SUCCESS != status)
            {
                g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
            }

                Process_Touches();


//GC            R_BSP_SoftwareDelay(1500, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(1500);

            //reset the System
        NVIC_SystemReset();
            //can't come here
        while(get_sw() == sw_fwd_rev);
        break;

    default:
//GC            R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(100);
        break;
  }

}
#endif
