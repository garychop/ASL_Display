//*****************************************************************************
//
// (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
//
// All rights reserved. This file is the intellectual property of ASL and it may
// not be disclosed to others or used for any purposes without the written consent of ASL.
//
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
#include "DeviceInfo.h"

#define ASL165_DispalyVersionString "ASL165: 2.0.1"


//#define MAXIMUM_DRIVE_SPEED (40)
#define MAXIMUM_DRIVE_SPEED_OLD_FIRMWARE (30)
#define MAXIMUM_DRIVE_SPEED_NEW_FIRMWARE (40)
#define MAX_NUM_OF_PADS (4)

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

typedef enum {DRIVE_FEATURE_ID, AUDIBLE_PHRASE_ID, POWER_ONOFF_ID, SWITCH_DRV_CTRL_FEATURE_ID, BLUETOOTH_FEATURE_ID, NEXT_FUNCTION_OR_TOGGLE_ID, NEXT_PROFILE_OR_USER_MENU_ID, RNET_SEATING_ID, RNET_SLEEP_FEATURE_ID, PAD_SENSOR_DISPLAY_FEATURE_ID, NUM_FEATURES} FEATURE_ID_ENUM; // "invalid" must be last enum
typedef enum {INVALID_FEATURE_HB_ID,
    POWERONOFF_FEATURE_HB_ID = 1,
    BLUETOOTH_FEATURE_HB_ID = 2,
    NEXT_FUNCTION_FEATURE_HB_ID = 3,
    NEXT_PROFILE_FEATURE_HB_ID = 4,
    RNET_MENU_SEATING_FEATURE_HB_ID = 5,
    RNET_SLEEP_FEATURE_HB_ID = 6,
    DRIVE_MODE_FEATURE_HB_ID = 7,
    SWITCH_DRIVER_FEATURE_HB_ID = 8,
    AUDIO_PHRASES_HB_ID = 9
    //STANDBY_SELECT_STANDBY_FEATURE_HB_ID = 10,
    //STANDBY_SELECT_MODE_SELECT_FEATURE_HB_ID = 11
} HEARTBEAT_FEATURE_ID_ENUM;

typedef enum ENUM_TIMER_IDS {ARROW_PUSHED_TIMER_ID = 1, CALIBRATION_TIMER_ID, PAD_ACTIVE_TIMER_ID} ENUM_TIMER_IDS_ENUM;
typedef enum {SILENCE_AUDIBLE = 0, MALE_VOICE_AUDIBLE, FEMALE_VOICE_AUDIBLE, CHILDS_VOICE_AUDIBLE, AUDIBLE_TYPE_END} AUDIBLE_TYPE_ENUM;

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
#define FUNC_FEATURE2_SAY_SOMETHING_BIT_MASK            (0x20)


typedef struct MAIN_SCREEN_FEATURE_STRUCT
{
    HEARTBEAT_FEATURE_ID_ENUM m_HB_ID;    // This establishes the connection between the ION Hub / Fusion and this feature.
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

typedef enum {AV_AAC_DEVICE, AV_HELP, AV_HELLO, AV_HOWAREYOU, AV_IM_THIRSTY, AV_SOMETHING_HURTS, AV_EXCUSE_ME, AV_OPEN_DOOR, AV_BROKEN, AV_BATHROOM, AV_LIST_END} AUDIO_STRING_ID_ENUM;

// This holds the Audio Phrase information.
#define AUDIO_PHRASE_MAX_NUMBER (32)
typedef struct
{
    unsigned char m_Enabled;
    GX_RESOURCE_ID m_Description;   // Contains the GUIX String Description
} AUDIO_SETTINGS_STRUCT;
extern AUDIO_SETTINGS_STRUCT g_AudioPhraseSettings[AUDIO_PHRASE_MAX_NUMBER];


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
extern HUB_PORT_SCHEMA_ENUM g_Mode_Switch_Schema;
extern HEARTBEAT_FEATURE_ID_ENUM g_ActiveFeature;     // this indicates the active feature.
extern GX_WIDGET *g_ActiveScreen;

extern AUDIBLE_TYPE_ENUM g_Audible_Setting;
extern uint8_t g_AuditoryVolumeLevel;
extern bool g_TonesActive;
extern bool g_CuesActive;
extern bool g_ION_ClicksActive;
extern uint8_t g_AP1, g_AP2, g_AP3, g_AP4;      // 32 Audio Phrases for SAY SOMETHING feature
extern int8_t g_SaySomething_HHP_Index;

extern MAIN_SCREEN_FEATURE g_MainScreenFeatureInfo[];
extern PAD_INFO_STRUCT g_PadSettings[];

// These are used to store for display the values used during Pad Calibration
extern int g_CalibrationPadNumber;
extern int16_t g_NeutralDAC_Constant;
extern int16_t g_NeutralDAC_Setting;
extern int16_t g_NeutralDAC_Range;
extern bool g_WaitingForVeerResponse;
extern uint8_t g_HeadArrayStatus1;
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
void ProcessIncomingMessages ();
void CreateEnabledFeatureStatus(uint8_t *myActiveFeatures, uint8_t *features2);

//extern void CleanupInfoStruct(PROGRAMMING_SCREEN_INFO* info, GX_VERTICAL_LIST* list, int depth);


#endif // ASL165_SYSTEM_H
