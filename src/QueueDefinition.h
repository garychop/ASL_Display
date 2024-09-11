//*****************************************************************************
//
// (c) COPYRIGHT, 2024 Adaptive Switch Technologies (ASL)
//
// All rights reserved. This file is the intellectual property of ASL and it may
// not be disclosed to others or used for any purposes without the written consent of ASL.
//
//*************************************************************************
// QueueDefinition.h
//
//  Created on: Nov 21, 2019
//      Author: Gary Chopcinski, Kg Solutions, LLC
//
//  This file contains the definitions for the message queues.
//
//*************************************************************************

#ifndef QUEUEDEFINITION_H_
#define QUEUEDEFINITION_H_

#include <stdio.h>
#include "tx_api.h"
#include "ASL165_System.h"
#include "BluetoothDeviceInfo.h"

//*************************************************************************
// Typedefs
//*************************************************************************

typedef enum SEND_DATA_BOOL {STOP_SENDING_DATA = 0, START_SENDING_DATA} SEND_DATA_ENUM;

typedef enum HHP_HA_MESSAGES_E
{
    ACK = 0x06,
    NAK = 0x15,
    HHP_HA_PAD_ASSIGMENT_GET = 0x30,
    HHP_HA_PAD_ASSIGMENT_SET = 0x31,
    HHP_HA_CALIBRATE_RANGE_GET = 0x32,
    HHP_HA_CALIBRATE_RANGE_SET = 0x33,
    HHP_HA_CALIBRATE_START_CMD = 0x34,
    HHP_HA_CALIBRATE_STOP_CMD = 0x35,
    HHP_HA_PAD_DATA_GET = 0x36,
    HHP_HA_VERSION_GET = 0x37,
    HHP_HA_FEATURE_GET = 0x38,
    HHP_HA_FEATURE_SET = 0x39,
    HHP_HA_ACTIVE_FEATURE_SET = 0x3a,
    HHP_HA_HEART_BEAT = 0x3b,
    HHP_HA_NEUTRAL_DAC_GET = 0x3c,
    HHP_HA_NEUTRAL_DAC_SET = 0x3d,
    HHP_HA_SAVE_PARAMETERS_CMD = 0x3e,
    HHP_HA_RESET_PARAMETERS_CMD = 0x3f,
    HHP_HA_DRIVE_OFFSET_GET = 0x40,
    HHP_HA_DRIVE_OFFSET_SET = 0x41,
    HHP_HA_ATTENDANT_SETTINGS_GET = 0x42,
    HHP_HA_ATTENDANT_SETTINGS_SET = 0x43,
    HHP_HA_ATTENDANT_CONTROLS_CMD = 0x44,
    HHP_HA_WHO_ARE_YOU_CMD = 0x46,
    HHP_HA_GET_DRIVER_CONTROL_ENABLE = 0x47,
    HHP_HA_SET_DRIVER_CONTROL_ENABLE = 0x48,
    HHP_HA_GET_DRIVER_CONTROL_INPUT_ASSIGNMENT = 0x49,
    HHP_HA_SET_DRIVER_CONTROL_INPUT_ASSIGNMENT = 0x4a,
    HHP_HA_BLUETOOTH_SETUP_GET_CMD = 0x4b,
    HHP_HA_BLUETOOTH_SETUP_SET_CMD = 0x4c,
    HHP_HA_DIAGNOSTIC_CMD = 0x4e,
    HHP_HA_SNP_CALIBRATION_CMD = 0x4f,
    HHP_HA_SNP_THRESHOLDS_GET = 0x50,
    HHP_HA_SNP_THRESHOLDS_SET = 0x51,
    HHP_HA_SNP_GET_RAWDATA_CMD = 0x52,
    HHP_HA_AUDITORY_SETTINGS_GET_CMD = 0x53,
    HHP_HA_AUDITORY_SETTINGS_SET_CMD = 0x54,
    HHP_HA_STANDBY_SELECT_GET_CMD,
    HHP_HA_STANDBY_SELECT_SET_CMD
} HHP_HA_MESSAGES_ENUM;

// This structure is used to send messages from the GUI to the Head Array Communication Task
typedef struct GUI_MSG_S
{
    HHP_HA_MESSAGES_ENUM m_MsgType;         // Use the above mentioned enum.
    union
    {
        struct  // Used for HHP_HA_PAD_ASSIGMENT_GET msg send by GUI to COMM Task to retrieve Physical Directional setup.
        {
            PHYSICAL_PAD_ENUM m_PhysicalPadNumber;
        } PadAssignmentRequestMsg;
        struct  // Used for HHP_HA_MODE_CHANGE_SET send by GUI to COMM task to set new User Mode.
        {
            FEATURE_ID_ENUM m_Mode;     // 0x01 = Power On/Off, 0x02=Bluetooth, 0x04 = Next Function, 0x05 = Next Profile
        } ModeChangeMsg;
        struct          // Supports the HHP_HA_PAD_ASSIGMENT_SET message from the GUI to the Head Array
        {
            PHYSICAL_PAD_ENUM m_PhysicalPadNumber;
            PAD_DIRECTION_ENUM m_LogicalDirection;
            PAD_TYPE_ENUM m_PadType;     // Digital vs Proportional, "D" or "P"
        } PadAssignmentSetMsg;
        struct          // Use this struct to start and stop data from being read.
        {
            SEND_DATA_ENUM m_Start;    // non0 = Start, 0 = Stop
            PHYSICAL_PAD_ENUM m_PadID;
        } GetDataMsg;
        struct
        {
            PHYSICAL_PAD_ENUM m_PadID;
        } GetCalibrationData;
        struct
        {
            PHYSICAL_PAD_ENUM m_PadID;
            uint16_t m_MinThreshold;
            uint16_t m_MaxThreshold;
        } SendCalibrationData;
        struct
        {
            uint8_t m_FeatureActiveList;
            uint8_t m_Timeout;
            uint8_t m_FeatureListByte2;
        } SendFeatureActiveList;
        struct
        {   // Used with HHP_HA_NEUTRAL_DAC_SET
            int16_t m_DAC_Setting;
        } SendNeutralDAC_Setting;
        struct
        {   // Used with HHP_HA_DRIVE_OFFSET_SET to send Drive Offset Head Array
            uint8_t m_CenterPad_DriveOffset;
            uint8_t m_LeftPad_DriveOffset;
            uint8_t m_RightPad_DriveOffset;
        } SendDriveOffset;
        struct
        { // used with HHP_HA_ATTENDANT_CONTROLS_CMD from GUI to COMM
            uint8_t m_AttendantActive;  // 0 = Inactive, 1 = active
            int8_t m_SpeedDemand;
            int8_t m_DirectionDemand;
        } SendAttendantControl;
        struct
        {   // used with HHP_HA_ATTENDANT_SETTINGS_SET
            uint8_t m_AttendantSettings;
            uint8_t m_AttendantTimeout;
        } SendAttendantSettings;
        struct
        {
            DEVICE_NUMBER_ENUM m_DeviceID;
            ENABLE_STATUS_ENUM m_Enabled; // only used when "setting" enable.
        } DriverControlEnable;
        struct
        {   // used with HHP_HA_BLUETOOTH_SETUP_GET_CMD and SET CMD
            uint8_t m_SlotNumber;   // Used in GET and SETcommand
            BT_DEVICE_TYPE m_DeviceIdenfication; // Used in SET command only
            BT_COLOR m_Color; // Used in SET command only
            BT_STATUS_ENUM m_Status; // Used in SET command only
        } Send_BT_DeviceDefinition;
        struct
        {   // Used with HHP_HA_GET_DRIVER_CONTROL_INPUT_ASSIGNMENT
            DEVICE_NUMBER_ENUM m_DeviceID;
        } ION_GetPadAssignment;
        struct
        {   // Used with HHP_HA_SET_DRIVER_CONTROL_INPUT_ASSIGNMENT
            DEVICE_NUMBER_ENUM m_DeviceID;
            PAD_DIRECTION_ENUM m_ForwardPadAssignment;
            PAD_DIRECTION_ENUM m_LeftPadAssignemnt;
            PAD_DIRECTION_ENUM m_RightPadAssignment;
            PAD_DIRECTION_ENUM m_ReversePadAssignment;
            DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM m_ModeSwitchSchema;
        } ION_SetPadAssignment;
        struct
        {   // Used with HHP_HA_SNP_THRESHOLDS_GET and SET
            DEVICE_NUMBER_ENUM m_DeviceID;
            int8_t m_SoftSip;
            int8_t m_HardSip;
            int8_t m_SoftPuff;
            int8_t m_HardPuff;
        } ION_SNP_Calibration_CMD_s;
        struct
        {   // Used with HHP_HA_AUDITORY_SETTINGS_GET_CMD
            uint8_t m_AuditorySetting;
            uint8_t m_Volume;
            uint8_t m_AP1;
            uint8_t m_AP2;
            uint8_t m_AP3;
            uint8_t m_AP4;
        } ION_Auditory_Struct;
        struct
        {
            uint32_t m_MsgArray[15];
        } WholeMsg;
    };
} GUI_MSG_STRUCT;

// Functions that send the command from the GUI task to the Head Array Communication task.
extern void SendGetPadAssignmentMsg (PHYSICAL_PAD_ENUM pad);
extern void SendSetPadAssignmentCommand (PHYSICAL_PAD_ENUM pad, PAD_DIRECTION_ENUM direction, PAD_TYPE_ENUM padType);
extern void SendModeChangeCommand (uint8_t newMode);
extern void SendCalibrationStartCommand (void);
extern void SendCalibrationStopCommand (void);
extern void SendGetVersionCommand (void);
extern void SendGetDataCommand (SEND_DATA_ENUM start, PHYSICAL_PAD_ENUM pad);   // or INVALID_PAD to get all data, for Diagnostic Screen.
extern void SendGetCalDataCommnd (PHYSICAL_PAD_ENUM pad);
extern void SendCalibrationData (PHYSICAL_PAD_ENUM pad, uint16_t min, uint16_t max);
extern void SendFeatureGetCommand (void);
extern void SendFeatureSetting (uint8_t myActiveFeatures, uint8_t TimeoutValue, uint8_t Features2);
extern void SendNeutralDAC_GetCommand (void);
extern void SendNeutralDAC_Set (int16_t newDAC_Setting);
extern void SendSaveParameters (void);
extern void SendResetParameters (void);
extern void SendDriveOffsetGet (void);
extern void SendDriveOffsetSet (uint8_t CenterPad_driveOffset, uint8_t LeftPad_driveOffset, uint8_t RightPad_driveOffset);
extern void SendAttendantControl_toHeadArray (uint8_t active, int8_t speed, int8_t direction);
extern void SendAttendantSettingsGet_toHeadArray (void);
extern void SendAttendantSettingsSet_toHeadArray (uint8_t attendantSettings, uint8_t attendantTimeout);
extern void SendWhoAmiCommand (void);
extern void RequestDriverEnableStatus (DEVICE_NUMBER_ENUM deviceIdx);
extern void SendDriverEnable (DEVICE_NUMBER_ENUM, ENABLE_STATUS_ENUM);
extern void SendDriverControlPadAssignmentRequest (DEVICE_NUMBER_ENUM);
extern void SendDriverControlPadAssigments (DEVICE_NUMBER_ENUM, PAD_DIRECTION_ENUM, PAD_DIRECTION_ENUM, PAD_DIRECTION_ENUM, PAD_DIRECTION_ENUM, DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM);
extern void Send_Get_BT_DeviceDefinitions (uint8_t slotNumber);
extern void Send_Set_BT_DeviceDefinitions (uint8_t slotNumber, BT_DEVICE_TYPE devID, BT_COLOR color, BT_STATUS_ENUM bt_status);
extern void Send_DiagnosticCommand (uint8_t enable);
extern void SendSNPThresholdGet (DEVICE_NUMBER_ENUM device);
extern void SendSNPThresholdSet (DEVICE_NUMBER_ENUM device, int8_t soft_sip, int8_t hard_sip, int8_t soft_puff, int8_t hard_puff);
extern void SendAuditorySettingGetCommand_toHub (void);
extern void SendAuditorySettingSetCommand_toHub (uint8_t*);

// This structure is used to send information from the Head Array Communication Task to the GUI task.
typedef struct HHP_HA_MSG_S
{
    HHP_HA_MESSAGES_ENUM m_MsgType;         // Use the above mentioned enum.
    union
    {
        struct hb_msg   // Supports the HHP_HA_HEART_BEAT_RESPONSE message from the Head Array
        {
            uint32_t m_HB_OK;   // Non0 if Heart Beat is OK, 0 = failed.
            uint32_t m_HB_Count;  // number that increments with each successful heart beat.
            HEARTBEAT_FEATURE_ID_ENUM m_ActiveMode;   // 1=Power On/Off, 2=Bluetooh, 3=Next Function 4=Next Profile
            uint8_t m_HA_Status;    // bit0, Head Array Ready Status, 0 = idle, 1 = Ready
                                    // bit1, Left Pad, 0 = Disconnected, 1 = connected.
                                    // bit2, right pad, 0 = Disconnected, 1 = connected.
                                    // bit3, center pad, 0 = Disconnected, 1 = connected.
                                    // bit4, Out of neutral, 0 = OK, 1 = Out of Neutral
            uint8_t m_HA_SensorStatus;  // Bit0 = Center Pad Proximity Sensor Active
                                        // Bit1 = Center Pad Pressure Sensor Active
                                        // Bit2 = Right Pad Proximity Sensor Active
                                        // Bit3 = Right Pad Pressure Sensor Active
                                        // Bit4 = Left Pad Proximity Sensor Active
                                        // Bit5 = Left Pad Pressure Sensor Active
            uint8_t m_SubIndex;     // Used to clarify / Expand Active Mode.
            DEVICE_NUMBER_ENUM m_ActiveDriverControl;
        } HeartBeatMsg;
        struct          // Supports the HHP_HA_PAD_ASSIGMENT_GET_RESPONSE message from the Head Array
        {
            PHYSICAL_PAD_ENUM m_PhysicalPadNumber;
            PAD_DIRECTION_ENUM m_LogicalDirection;
            PAD_TYPE_ENUM m_PadType;     // Digital vs Proportional, "D" or "P"
        } PadAssignmentResponseMsg;
        struct          // This supports the
        {
            FEATURE_ID_ENUM m_Mode;     // 1=Power On/Off, 2=Bluetooh, 3=Next Function 4=Next Profile
        } ModeChangeMsg;
        struct
        {
            uint8_t m_Major;
            uint8_t m_Minor;
            uint8_t m_Build;
            uint8_t m_EEPROM_Version;
        } Version;
        struct          // Use this struct to start and stop data from being read.
        {
            PHYSICAL_PAD_ENUM m_PadID;
            uint16_t m_RawData;
            uint16_t m_DriveDemand;
        } GetDataMsg;
        struct
        {
            PHYSICAL_PAD_ENUM m_PadID;
            uint16_t m_MinADC;
            uint16_t m_MaxADC;
            uint16_t m_MinThreshold;
            uint16_t m_MaxThreshold;
        } CalibrationDataResponse;
        struct
        {
            uint8_t m_FeatureSet;
            uint8_t m_Timeout;
            uint8_t m_FeatureSet2;  // Added in Rev T of HHP Protocol.
        } GetFeatureResponse;
        struct
        {
            int16_t m_DAC_Constant;
            int16_t m_NeutralDAC_Value;
            int16_t m_Range;
        } NeutralDAC_Get_Response;
        struct
        {   // Used with HHP_HA_DRIVE_OFFSET_SET to return Drive Offset value from Head Array to HHP
            uint8_t m_CenterPad_DriveOffset;
            uint8_t m_LeftPad_DriveOffset;
            uint8_t m_RightPad_DriveOffset;
        } DriveOffset_Get_Response;
        struct
        {   // Used with HHP_HA_ATTENDANT_SETTINGS_GET
            uint8_t m_AttendantSettings;
            uint8_t m_AttendantTimeout;
        } AttendantSettings_Get_Response;
        struct
        {   // Used with HHP_HA_GET_DRIVER_CONTROL_ENABLE
            DEVICE_NUMBER_ENUM m_DeviceID;
            ENABLE_STATUS_ENUM m_Enabled;
        } DriverControlEnable;
        struct
        {   // Used with HHP_HA_WHO_ARE_YOU_CMD command
            uint8_t m_WhoAmi;
        } WhoAmI_Response;
        struct
        {   // Used with HHP_HA_GET_DRIVER_CONTROL_INPUT_ASSIGNMENT
            DEVICE_NUMBER_ENUM m_DeviceID;
            PAD_DIRECTION_ENUM m_FowardPad;
            PAD_DIRECTION_ENUM m_LeftPad;
            PAD_DIRECTION_ENUM m_RightPad;
            PAD_DIRECTION_ENUM m_ReversePad;
            DRIVER_CONTROL_MODE_SWITCH_SCHEMA_ENUM m_ModeSwitchSchema;
        } DriverControlPadAssignemt;
        struct
        {   // used with HHP_HA_BLUETOOTH_SETUP_GET_CMD and SET CMD
            uint8_t m_SlotNumber;   // Used in GET and SETcommand
            BT_DEVICE_TYPE m_DeviceIdenfication; // Used in SET command only
            BT_COLOR m_Color; // Used in SET command only
            BT_STATUS_ENUM m_Status; // Used in SET command only
        } BT_DeviceDefinition;
        struct
        {   // Used with HHP_HA_AUDITORY_SETTINGS_SET_CMD
            uint8_t m_AuditorySetting;
            uint8_t m_Volume;
            uint8_t m_AP1;
            uint8_t m_AP2;
            uint8_t m_AP3;
            uint8_t m_AP4;
        } ION_Auditory_Struct;
        struct
        {
            uint32_t m_MsgArray[15];
        } WholeMsg;
    };
} HHP_HA_MSG_STRUCT;

// Functions that send the command from the Head Array Communication Task to the GUI task.
extern void SendPadGetResponse (PHYSICAL_PAD_ENUM physicalPad, PAD_DIRECTION_ENUM direction, PAD_TYPE_ENUM padType);
extern void SendVersionToGUI (uint8_t majorVersion, uint8_t minorVersion, uint8_t buildVersion, uint8_t eeprom);
extern void SendCalDataResponse (PHYSICAL_PAD_ENUM physicalPad, uint16_t minADC, uint16_t maxADC, uint16_t minThreshold, uint16_t maxThreshold);
extern void SendFeatureGet (uint8_t featureSet, uint8_t timeout, uint8_t featureSet2);
extern void SendDriveOffsetGetResponse (uint8_t, uint8_t, uint8_t);
extern void SendAttendantSettingsGet (uint8_t attendantSettings, uint8_t attendantTimeout);
extern void SendDriverEnableToGUI (DEVICE_NUMBER_ENUM, ENABLE_STATUS_ENUM);
extern void Send_Get_BT_DeviceDefinitions_Response (uint8_t slotNumber, BT_DEVICE_TYPE devID, BT_COLOR color, BT_STATUS_ENUM bt_status);
extern void ProcessDriveControlPadAssignemnt_Response (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
extern void Send_Auditory_Setting_ToGUI (uint8_t*);

// Helper functions
extern PHYSICAL_PAD_ENUM TranslatePad_CharToEnum (char pad);
extern char TranslatePad_EnumToChar (PHYSICAL_PAD_ENUM pad);

extern PAD_DIRECTION_ENUM TranslatePadDirection_CharToEnum (char padDirection);
extern char TranslatePadDirection_EnumToChar (PAD_DIRECTION_ENUM padDirection);

extern PAD_TYPE_ENUM TranslatePadType_CharToEnum (char padType);
extern char TranslatePadType_EnumToChar (PAD_TYPE_ENUM padType);

extern FEATURE_ID_ENUM TranslateFeature_CharToEnum (char feature);
extern char TranslateFeature_EnumToChar (FEATURE_ID_ENUM feature);

extern void SendNeutralDAC_GetResponse (int16_t constant, int16_t DAC_Value, int16_t Range);


#endif /* QUEUEDEFINITION_H_ */
