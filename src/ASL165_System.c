/******************************************************************************
 * ASL165_System.c
 *
 *  Created on: Mar 13, 2024
 *      Author: G. Chopcinski, Kg Solutions, LLC
 *
 *  Description: Essentially, this file contains all global data.
 *
******************************************************************************/
#include <stdio.h>
#include <math.h>
#include "ASL165_System.h"

//-------------------------------------------------------------------------
// Global Variables.
//-------------------------------------------------------------------------

uint8_t g_HA_Version_Major, g_HA_Version_Minor, g_HA_Version_Build, g_HA_EEPROM_Version;
MAIN_SCREEN_FEATURE g_MainScreenFeatureInfo[NUM_FEATURES];
PAD_INFO g_PadSettings[3];

bool g_ClicksActive = false;
bool g_PowerUpInIdle = false;
bool g_RNet_Active = false;
MODE_SWITCH_SCHEMA_ENUM g_Mode_Switch_Schema = MODE_SWITCH_PIN5;

int g_SettingsChanged;
int8_t g_StartupDelayCounter = 0;
int g_ChangeScreen_WIP;

// These are received from the Head Array in the Heart Beat Message
FEATURE_ID_ENUM g_ActiveFeature = POWER_ONOFF_ID;     // this indicates the active feature.
uint8_t g_HeadArrayStatus1 = 0x00;
uint8_t g_HeadArrayStatus2 = 0x00;

GX_WIDGET *g_ActiveScreen = GX_NULL;
int16_t g_NeutralDAC_Constant = 2048;
int16_t g_NeutralDAC_Setting = 2048;
int16_t g_NeutralDAC_Range = 400;
bool g_WaitingForVeerResponse = false;

// Added in Version 1.9.x
uint8_t g_AttendantSettings;    // D0 = 1 = Attendant Active, D1 = 0 = Proportional, D2 = 0 = Override
uint8_t g_AttendantTimeout;     // 0=127 seconds, 0 = No Timeout

uint8_t g_WhoAmi = 0x0;         // Default to Unknown

/* end of file */