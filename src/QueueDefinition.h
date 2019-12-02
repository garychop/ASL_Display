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

//*************************************************************************
// Typedefs
//*************************************************************************

typedef enum PAD_DESIGNATION {LEFT_PAD, RIGHT_PAD, CENTER_PAD, INVALID_PAD} PAD_DESIGNATION_ENUM;
typedef enum PAD_DIRECTION {OFF_DIRECTION = 0, LEFT_DIRECTION, FORWARD_DIRECTION, RIGHT_DIRECTION, INVALID_DIRECTION} PAD_DIRECTION_ENUM;
typedef enum PAD_TYPE {PROPORTIONAL_PADTYPE, DIGITAL_PADTYPE, INVALID_PAD_TYPE} PAD_TYPE_ENUM;
typedef enum FEATURE_ID {POWER_ONOFF_ID, BLUETOOTH_ID, NEXT_FUNCTION_ID, NEXT_PROFILE_ID, INVALID_FEATURE_ID} FEATURE_ID_ENUM;

typedef enum HHP_HA_MESSAGES_E
{
    HHP_HA_PAD_ASSIGMENT_GET = 0x30,
    HHP_HA_PAD_ASSIGMENT_GET_RESPONSE = 0x31,
    HHP_HA_PAD_ASSIGMENT_SET = 0x32,
    HHP_HA_PAD_ASSIGMENT_SET_RESPONSE = 0x33,
    HHP_HA_CALIBRATE_RANGE_GET = 0x34,
    HHP_HA_CALIBRATE_RANGE_GET_RESPONSE = 0x35,
    HHP_HA_CALIBRATE_RANGE_SET = 0x36,
    HHP_HA_CALIBRATE_RANGE_SET_RESPONSE = 0x37,
    HHP_HA_CALIBRATE_START_CMD = 0x38,
    HHP_HA_CALIBRATE_STOP_CMD = 0x39,
    HHP_HA_PAD_DATA_GET = 0x3a,
    HHP_HA_PAD_DATA_GET_RESPONSE = 0x3b,
    HHP_HA_VERSION_GET = 0x3c,
    HHP_HA_VERSION_GET_RESPONSE = 0x3d,
    HHP_HA_FEATURE_SETTING_GET = 0x3e,
    HHP_HA_FEATURE_SETTING_GET_RESPONSE = 0x3f,
    HHP_HA_MODE_CHANGE_SET = 0x42,
    HHP_HA_MODE_CHANGE_SET_RESPONSE = 0x43,
    HHP_HA_HEART_BEAT = 0x44,
    HHP_HA_HEART_BEAT_RESPONSE = 0x45
} HHP_HA_MESSAGES_ENUM;

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
            uint8_t m_ActiveMode;   // 1=Power On/Off, 2=Bluetooh, 3=Next Function 4=Next Profile
            uint8_t m_HA_Status;    // bit0, Head Array Ready Status, 0 = idle, 1 = Ready
                                    // bit1, Left Pad, 0 = Disconnected, 1 = connected.
                                    // bit2, right pad, 0 = Disconnected, 1 = connected.
                                    // bit3, center pad, 0 = Disconnected, 1 = connected.
                                    // bit4, Out of neutral, 0 = OK, 1 = Out of Neutral
        } HeartBeatMsg;
        struct          // Supports the HHP_HA_PAD_ASSIGMENT_GET_RESPONSE message from the Head Array
        {
            char m_PhysicalPadNumber;
            char m_LogicalDirection;
        } PadAssignmentResponseMsg;
        struct          // This supports the
        {
            uint8_t m_Mode;     // 0x01 = Power On/Off, 0x02=Bluetooth, 0x04 = Next Function, 0x05 = Next Profile
        } ModeChangeMsg;
        struct
        {
            uint32_t m_MsgArray[15];
        } WholeMsg;
    };
} HHP_HA_MSG_STRUCT;

typedef struct GUI_MSG_S
{
    HHP_HA_MESSAGES_ENUM m_MsgType;         // Use the above mentioned enum.
    union
    {
        struct  // Used for HHP_HA_PAD_ASSIGMENT_GET msg send by GUI to COMM Task to retrieve Physical Directional setup.
        {
            char m_PhysicalPadNumber;
        } PadAssignmentRequestMsg;
        struct  // Used for HHP_HA_MODE_CHANGE_SET send by GUI to COMM task to set new User Mode.
        {
            uint8_t m_Mode;     // 0x01 = Power On/Off, 0x02=Bluetooth, 0x04 = Next Function, 0x05 = Next Profile
        } ModeChangeMsg;
        struct          // Supports the HHP_HA_PAD_ASSIGMENT_SET message from the GUI to the Head Array
        {
            char m_PhysicalPadNumber;
            char m_LogicalDirection;
        } PadAssignmentSetMsg;
        struct
        {
            uint32_t m_MsgArray[15];
        } WholeMsg;
    };
} GUI_MSG_STRUCT;

// Support functions that format and send the command to the Head Array.
extern void SendPadAssignmentRequestMsg (char pad);
extern void SendPadAssignmentResponse (char physicalPad, char assignment);
extern void SendModeChangeCommand (uint8_t newMode);
extern void SendPadAssignmentSetCommand (char pad, char direction);

// Helper functions
extern PAD_DESIGNATION_ENUM TranslatePad (char pad);
extern PAD_DIRECTION_ENUM TranslatePadDirection (char padDirection);

#endif /* QUEUEDEFINITION_H_ */
