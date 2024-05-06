//*****************************************************************************
// Filename: ASL165_System.h
// Description: This file declares function, typedefs and macros to the 
//		system wide usage.
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#ifndef ASL165_SYSTEM_H
#define ASL165_SYSTEM_H

#include <stdio.h>
#define min(a,b)   ((a < b) ? a : b)

#include "ASL_HHP_Display_GUIX_resources.h"
#include "ASL_HHP_Display_GUIX_specifications.h"
#include "custom_checkbox.h"

#define ASL165_DispalyVersionString "ASL165: 2.0.1"


//#define MAXIMUM_DRIVE_SPEED (40)
#define MAXIMUM_DRIVE_SPEED_OLD_FIRMWARE (30)
#define MAXIMUM_DRIVE_SPEED_NEW_FIRMWARE (40)

#define FEATURE_TOGGLE_BUTTON_ID 1000	// this is used as the dynamically created buttons in the feature list.

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

typedef enum {DRIVE_FEATURE_ID, POWER_ONOFF_ID, SWITCH_DRV_CTRL_FEATURE_ID, BLUETOOTH_FEATURE_ID, NEXT_FUNCTION_OR_TOGGLE_ID, NEXT_PROFILE_OR_USER_MENU_ID, RNET_SEATING_ID, RNET_SLEEP_FEATURE_ID, PAD_SENSOR_DISPLAY_FEATURE_ID, NUM_FEATURES} FEATURE_ID_ENUM; // "invalid" must be last enum
typedef enum {INVALID_FEATURE_HB_ID,
    POWERONOFF_FEATURE_HB_ID = 1,
    BLUETOOTH_FEATURE_HB_ID = 2,
    NEXT_FUNCTION_FEATURE_HB_ID = 3,
    NEXT_PROFILE_FEATURE_HB_ID = 4,
    RNET_MENU_SEATING_FEATURE_HB_ID = 5,
    RNET_SLEEP_FEATURE_HB_ID = 6,
    DRIVE_MODE_FEATURE_HB_ID = 7,
    SWITCH_DRIVER_FEATURE_HB_ID = 8,
    STANDBY_SELECT_STANDBY_FEATURE_HB_ID = 9,
    STANDBY_SELECT_MODE_SELECT_FEATURE_HB_ID = 10
} HEARTBEAT_FEATURE_ID_ENUM;
typedef enum {HEAD_ARRY_DEVICE_IDX, DRIVER_4_QUAD_IDX, SIP_N_PUFF_DEVICE_IDX, TWO_SWITCH_DEVICE_IDX, SNP_HEAD_ARRAY_DEVICE_IDX, ENDOF_DEVICES_IDX} DEVICE_NUMBER_ENUM;
typedef enum ENUM_TIMER_IDS {ARROW_PUSHED_TIMER_ID = 1, CALIBRATION_TIMER_ID, PAD_ACTIVE_TIMER_ID} ENUM_TIMER_IDS_ENUM;
typedef enum ENUM_MODE_SWITCH_SCHEMA {MODE_SWITCH_PIN5, MODE_SWITCH_REVERSE} MODE_SWITCH_SCHEMA_ENUM;
typedef enum PHYSICAL_PAD {LEFT_PAD, RIGHT_PAD, CENTER_PAD, REVERSE_PAD, END_OF_PAD_ENUM} PHYSICAL_PAD_ENUM;
typedef enum PAD_DIRECTION {OFF_DIRECTION = 0, LEFT_DIRECTION, FORWARD_DIRECTION, RIGHT_DIRECTION, REVERSE_DIRECTION, INVALID_DIRECTION} PAD_DIRECTION_ENUM;
typedef enum PAD_TYPE {PROPORTIONAL_PADTYPE, DIGITAL_PADTYPE, INVALID_PAD_TYPE} PAD_TYPE_ENUM;
typedef enum PAD_STATUS {PAD_STATUS_OK = 0, PAD_STATUS_ERROR = 1} PAD_STATUS_ENUM;
typedef enum ENABLE_STATUS {FEATURE_DISABLED = 0, FEATURE_ENABLED = 1} ENABLE_STATUS_ENUM;

// Bit masks pertaining to the REQUEST FEATURE SETTNG CMD HA<->HHP comms command.
#define FUNC_FEATURE_POWER_ON_OFF_BIT_MASK              (0x01)
#define FUNC_FEATURE_OUT_CTRL_TO_BT_MODULE_BIT_MASK     (0x02)
#define FUNC_FEATURE_NEXT_FUNCTION_BIT_MASK             (0x04)
#define FUNC_FEATURE_NEXT_PROFILE_BIT_MASK              (0x08)
#define FUNC_FEATURE_SOUND_ENABLED_BIT_MASK             (0x10)
#define FUNC_FEATURE_POWER_UP_IN_IDLE_BIT_MASK          (0x20)
#define FUNC_FEATURE_RNET_SEATING_MASK                  (0x80)

// Bit bask for Feature set 2
#define FUNC_FEATURE2_RNET_SLEEP_BIT_MASK               (0x01)
#define FUNC_FEATURE2_MODE_REVERSE_BIT_MASK             (0x02)
#define FUNC_FEATURE2_SHOW_PADS_BIT_MASK                (0x04)
#define FUNC_FEATURE2_DRIVING_MODE_BIT_MASK             (0x08)
#define FUNC_FEATURE2_SWITCH_DRIVER_MODE_BIT_MASK       (0x10)


// The positions in the following enum relate the 2 status bits for each pad where:
// ... D0 = Digital Sensor Active and D1 = Pressure Sensor is active therefore
// ... 00b = No active sensors,
// ... 01b = Digital Sensor active only,
// ... 10b = Pressure Sensor active.
// ... 11b = Digital and Pressure sensor active.
typedef enum PAD_STATUS_COLORS_ENUM {PAD_OFF, PAD_GREEN, PAD_WHITE, PAD_ORANGE} PAD_STATUS_COLORS;


typedef struct MAIN_SCREEN_FEATURE_STRUCT
{
    FEATURE_ID_ENUM m_HB_ID;    // This establishes the connection between the ION Hub / Fusion and this feature.
    int m_Location;     // This indicates the Main Screen location, 0=Top most, 3=bottom most
	int /*bool*/ m_Available;	// This is true if this feature should be displayed for Enabling/Disabling. Typically based upon RNet setting.
    int /*bool*/ m_Enabled;      // Indicates if this feature is active.
    GX_RESOURCE_ID m_SmallIcon;
    GX_RESOURCE_ID m_LargeIcon;
    GX_RESOURCE_ID m_SmallDescriptionID;
    GX_RESOURCE_ID m_LargeDescriptionID;
    GX_RESOURCE_ID m_FontColorID;
	GX_WIDGET m_ItemWidget;
	GX_PROMPT m_PromptWidget;
	GX_CHECKBOX m_ButtonWidget;
    CUSTOM_CHECKBOX m_Checkbox;
} MAIN_SCREEN_FEATURE;

// This structure is used by each screen.
typedef struct
{
    int m_Enabled;
    GX_RESOURCE_ID m_LargeDescriptionID;
    GX_WIDGET m_ItemWidget;
    GX_PROMPT m_PromptWidget;
    GX_TEXT_BUTTON m_ButtonWidget;
    GX_MULTI_LINE_TEXT_BUTTON m_MultiLineButtonWidget;
    CUSTOM_CHECKBOX m_Checkbox;
} PROGRAMMING_SCREEN_INFO;

extern void CleanupInfoStruct(PROGRAMMING_SCREEN_INFO* info, GX_VERTICAL_LIST* list, int depth);

typedef struct PadInfoStruct
{
    enum PAD_TYPE m_PadType;
    enum PAD_DIRECTION m_PadDirection;
    enum PAD_STATUS m_PadStatus;
    uint8_t m_PadSensorStatus;
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
    //GX_PIXELMAP_BUTTON *m_DirectionIcons[5];
    GX_PIXELMAP_BUTTON *m_DirectionIcons;
    GX_PROMPT *m_RawValuePrompt;
    GX_PROMPT *m_AdjustedValuePrompt;
    uint16_t m_Proportional_RawValue;
    uint16_t m_Proportional_DriveDemand;
    GX_CHAR m_DriveDemandString[8];         // Unfortunately, I have to use a "Global" or "Const" string with the gx_prompt_text_set function instead
    GX_CHAR m_RawValueString[8];            // of a local string variable in this function. It actually sends a pointer to the function and
                                            // not a copy of the string. That means that the last information is applied to all
                                            // gx_prompt_text_set calls.
} PAD_INFO;

//*****************************************************************************
// GLOBAL VARIABLES
//*****************************************************************************

extern int8_t g_StartupDelayCounter;
extern int g_ChangeScreen_WIP;
extern unsigned char g_HA_Version_Major, g_HA_Version_Minor, g_HA_Version_Build, g_HA_EEPROM_Version;
extern bool g_ClicksActive;
extern bool g_PowerUpInIdle;
extern uint8_t g_TimeoutValue;
extern bool g_RNet_Active;
extern int8_t g_BluetoothSubIndex;
extern bool g_ShowPadsOnMainScreen;
extern MODE_SWITCH_SCHEMA_ENUM g_Mode_Switch_Schema;
extern HEARTBEAT_FEATURE_ID_ENUM g_ActiveFeature;     // this indicates the active feature.
extern GX_WIDGET *g_ActiveScreen;
extern DEVICE_NUMBER_ENUM g_ActiveDriverControl;

extern MAIN_SCREEN_FEATURE g_MainScreenFeatureInfo[];
extern PAD_INFO g_PadSettings[];

// These are used to store for display the values used during Pad Calibration
extern int g_CalibrationPadNumber;
extern int16_t g_NeutralDAC_Constant;
extern int16_t g_NeutralDAC_Setting;
extern int16_t g_NeutralDAC_Range;
extern bool g_WaitingForVeerResponse;
extern uint8_t g_HeadArrayStatus1, g_HeadArrayStatus2;
extern GX_RECTANGLE g_HiddenRectangle;

// Added in Version 1.9.x
extern uint8_t g_AttendantSettings;    // D0 = 1 = Attendant Active, D1 = 0 = Proportional, D2 = 0 = Override
extern uint8_t g_AttendantTimeout;     // 0=127 seconds, 0 = No Timeout

typedef enum {I_AM_FUSION, I_AM_ION} WHOAMI_ENUM;
extern WHOAMI_ENUM g_WhoAmi;         // Default to Unknown

//*****************************************************************************
// EXTERNAL, GLOBALLY available functions
//*****************************************************************************

VOID screen_toggle(GX_WINDOW *new_win, GX_WINDOW *old_win);
VOID screen_switch(GX_WIDGET *parent, GX_WIDGET *new_screen);
void PushWindow (GX_WINDOW* window);
GX_WINDOW *PopPushedWindow();

void AdjustActiveFeaturePositions (FEATURE_ID_ENUM newMode);
void SaveSystemStatus (uint8_t Status1, uint8_t Status2);
void ProcessCommunicationMsgs ();
void CreateEnabledFeatureStatus(uint8_t *myActiveFeatures, uint8_t *features2);



#endif // ASL165_SYSTEM_H
