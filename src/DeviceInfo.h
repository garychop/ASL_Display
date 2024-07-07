//*****************************************************************************
// Filename: DeviceDefinition.h, Changed from PadInfo.h (Oct 15, 2023)
// 
// Description: This file declares function, typedefs and macros to support
//		the Deviceinformation
//
// Date: Aug 28, 2020.
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#ifndef DEVICE_INFORMATON_H
#define DEVICE_INFORMATON_H

//#include "ASL165_System.h"

// The devices are declared as follows:
//  Device #1 = Head Array, 3-quadrant
//  Device #2 = Driver Control, 4-quadrant
//  Device #3 = Sip-N-Puff
//  Device #4 = 2-Switch
//  Device #5 = ASL109-type SNP Head Array
typedef enum {HEAD_ARRY_DEVICE_IDX, DRIVER_4_QUAD_IDX, SIP_N_PUFF_DEVICE_IDX, TWO_SWITCH_DEVICE_IDX, SNP_HEAD_ARRAY_DEVICE_IDX, HUB_DEVICE_IDX, ENDOF_DEVICES_IDX} DEVICE_NUMBER_ENUM;
#define MAX_DEVICES (ENDOF_DEVICES_IDX)

#define MAX_NUM_OF_PADS (4)

typedef enum PHYSICAL_PAD {LEFT_PAD, RIGHT_PAD, CENTER_PAD, REVERSE_PAD, END_OF_PAD_ENUM} PHYSICAL_PAD_ENUM;
typedef enum PAD_DIRECTION {PAD_DIRECTION_OFF = 0, PAD_DIRECTION_LEFT, PAD_DIRECTION_FORWARD, PAD_DIRECTION_RIGHT, PAD_DIRECTION_REVERSE, PAD_DIRECTION_MODE, PAD_DIRECTION_INVALID} PAD_DIRECTION_ENUM;
typedef enum PAD_TYPE {PROPORTIONAL_PADTYPE, DIGITAL_PADTYPE, INVALID_PAD_TYPE} PAD_TYPE_ENUM;
// The positions in the following enum relate the 2 status bits for each pad where:
// ... D0 = Digital Sensor Active and D1 = Pressure Sensor is active therefore
// ... 00b = No active sensors,
// ... 01b = Digital Sensor active only,
// ... 10b = Pressure Sensor active.
// ... 11b = Digital and Pressure sensor active.
typedef enum {PAD_OFF, PAD_GREEN, PAD_WHITE, PAD_ORANGE} PAD_STATUS_COLORS_ENUM;
typedef enum {DRIVER_2_QUADRANT, DRIVER_3_QUADRANT, DRIVER_4_QUADRANT} DRIVER_QUADRANT_ENUM;
typedef enum {
    DRV_MODE_OFF = PAD_DIRECTION_OFF,
    DRV_MODE_SWITCH_LEFT = PAD_DIRECTION_LEFT,
    DRV_MODE_SWITCH_FORWARD = PAD_DIRECTION_FORWARD,
    DRV_MODE_SWITCH_RIGHT = PAD_DIRECTION_RIGHT,
    DRV_MODE_SWITCH_REVERSE = PAD_DIRECTION_REVERSE,
    DRV_MODE_SWITCH_PIN5 = PAD_DIRECTION_MODE,
    DRV_MODE_USER_PORT, DRV_MODE_SWITCH_SINGLE_TAP, DRV_MODE_SWITCH_DOUBLE_TAP, DRV_MODE_SWITCH_END} DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM;
//typedef enum {DRV_MODE_SWITCH_PIN5, DRV_MODE_SWITCH_REVERSE, DRV_MODE_SWITCH_FORWARD, DRV_MODE_SWITCH_LEFT, DRV_MODE_SWITCH_RIGHT, DRV_MODE_USER_PORT, DRV_MODE_SWITCH_SINGLE_TAP, DRV_MODE_SWITCH_DOUBLE_TAP, DRV_MODE_SWITCH_END} DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM;
typedef enum {HUB_MODE_SWITCH_PIN5, HUB_MODE_SWITCH_REVERSE, HUB_MODE_SWITCH_FORWARD, HUB_MODE_SWITCH_LEFT, HUB_MODE_SWITCH_RIGHT, HUB_MODE_USER_PORT, HUB_MODE_JUMP_TO_BT, HUB_MODE_SWITCH_END} HUB_PORT_SCHEMA_ENUM;
typedef enum {DISABLED = 0, ENABLED = 1} ENABLE_STATUS_ENUM;


typedef struct PAD_INFO_STRUCT_NAME
{
    enum PAD_TYPE m_PadType;
    enum PAD_DIRECTION m_PadDirection;
    unsigned short m_MinimumDriveValue;
    char m_MinimuDriveString[8];
    int m_PadMinimumCalibrationValue;
    int m_PadMaximumCalibrationValue;
    int8_t m_SNP_Threshold;
    GX_PIXELMAP_BUTTON *m_DirectionIcon;
} ION_PAD_INFO_STRUCT;

typedef struct
{
    DEVICE_NUMBER_ENUM m_DriverConfiguration;
    DRIVER_QUADRANT_ENUM m_DriverQuadrantSetting;
    GX_RESOURCE_ID m_DeviceNameStringID;
    ENABLE_STATUS_ENUM m_Enabled;
    ION_PAD_INFO_STRUCT m_PadInfo[MAX_NUM_OF_PADS];
    DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM m_Mode_Switch_Schema;
} DEVICE_INFO_STRUCT;

typedef enum {PAD_STATUS_OK, PAD_STATUS_ERROR} PAD_STATUS_ENUM;

typedef struct PadInfoStruct
{
    PAD_TYPE_ENUM m_PadType;
    PAD_DIRECTION_ENUM m_PadDirection;
    PAD_STATUS_ENUM m_PadStatus;
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
} PAD_INFO_STRUCT;


/*****************************************************************************
 * Global Variables
 */

extern DEVICE_INFO_STRUCT g_DeviceSettings[MAX_DEVICES];// This describes all driver controls.
extern DEVICE_INFO_STRUCT *gp_ProgrammingDevice;		// This points to the Device Structure of the device currently being setup/programmed.

extern DEVICE_NUMBER_ENUM g_ActiveDriverControlIdx;		// This contains the idx to the currently active Driver Control.
extern DEVICE_INFO_STRUCT* gp_ActiveDriverControl;		// This points to the currently Active Driver Control struture.

/*****************************************************************************
 * Function Prototypes
 */

VOID InitializeDriverControls(void);
void SetProgrammingDriverControl (DEVICE_INFO_STRUCT *device);
void Initialize_ION_HubPort_Button_Info (void);
void PopulateION_Attendant_ProgrammingInfo (void);
void AdvanceToNextDriverControl();


#endif // DEVICE_INFORMATON_H


