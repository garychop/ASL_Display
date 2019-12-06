//****************************************************************************
// Module       Queue Message Handling
// Filename     QueueMessage.c
// Project      ASL110 Display Unit
//----------------------------------------------------------------------------
// Controller   Renesas MCU ARM-Cortex 4
//
// Compiler     GNU GCC
//
// Description  This module is a support file to create and decipher message
//      exchanged in queues..
//
//----------------------------------------------------------------------------
// Date         November 22, 2019
//
//****************************************************************************
//
// This file is property of ASL, Inc. and can not be duplicated,
// copied or used without the expressed written permission by ASL, Inc.
//
//****************************************************************************
// Project Includes
//****************************************************************************

#include "my_gui_thread.h"
#include "QueueDefinition.h"
#include "HeadArray_CommunicationThread.h"
#include "tx_api.h"

//****************************************************************************
// Functions
//****************************************************************************

PHYSICAL_PAD_ENUM TranslatePad_CharToEnum (char pad)
{
    PHYSICAL_PAD_ENUM myPad = INVALID_PAD;

    switch (pad)
    {
        case 'L':
            myPad = LEFT_PAD;
            break;
        case 'R':
            myPad = RIGHT_PAD;
            break;
        case 'C':
            myPad = CENTER_PAD;
            break;
    }
    return myPad;
}

char TranslatePad_EnumToChar (PHYSICAL_PAD_ENUM pad)
{
    char myPad = '?';

    switch (pad)
    {
        case LEFT_PAD:
            myPad = 'L';
            break;
        case RIGHT_PAD:
            myPad = 'R';
            break;
        case CENTER_PAD:
            myPad = 'C';
            break;
        case INVALID_PAD:
        default:
            myPad = '?';
            break;

    }
    return myPad;
}

//****************************************************************************

PAD_DIRECTION_ENUM TranslatePadDirection_CharToEnum (char padDirection)
{
    PAD_DIRECTION_ENUM myDir = INVALID_DIRECTION;

    switch (padDirection)
    {
        case 'L':
            myDir = LEFT_DIRECTION;
            break;
        case 'R':
            myDir = RIGHT_DIRECTION;
            break;
        case 'F':
            myDir = FORWARD_DIRECTION;
            break;
//        case 'B':
//            myDir = REVERSE_DIRECTION;
//            break;
        case 'O':
            myDir = OFF_DIRECTION;
            break;
    }
    return myDir;
}

char TranslatePadDirection_EnumToChar (PAD_DIRECTION_ENUM padDirection)
{
    char myDir = '?';

    switch (padDirection)
    {
        case LEFT_DIRECTION:
            myDir = 'L';
            break;
        case RIGHT_DIRECTION:
            myDir = 'R';
            break;
        case FORWARD_DIRECTION:
            myDir = 'F';
            break;
//        case 'B':
//            myDir = REVERSE_DIRECTION;
//            break;
        case OFF_DIRECTION:
            myDir = 'O';
            break;
        case INVALID_DIRECTION:
        default:
            myDir = '?';
            break;
    }
    return myDir;
}

//****************************************************************************

PAD_TYPE_ENUM TranslatePadType_CharToEnum (char padType)
{
    if (padType == 'D')
        return DIGITAL_PADTYPE;
    else if (padType == 'P')
        return PROPORTIONAL_PADTYPE;

    return INVALID_PAD_TYPE;
}

char TranslatePadType_EnumToChar (PAD_TYPE_ENUM padType)
{
    if (padType == 'D')
        return DIGITAL_PADTYPE;
    else if (padType == 'P')
        return PROPORTIONAL_PADTYPE;

    return INVALID_PAD_TYPE;
}

//****************************************************************************
// Feature translation functions.
//****************************************************************************

char TranslateFeature_EnumToChar (FEATURE_ID_ENUM feature)
{
    //typedef enum FEATURE_ID {POWER_ONOFF_ID, BLUETOOTH_ID, NEXT_FUNCTION_ID, NEXT_PROFILE_ID, INVALID_FEATURE_ID} FEATURE_ID_ENUM;

    return ((char) (feature + 1));    // this should be one to one correlation, adjusted for fence-posting.
}

FEATURE_ID_ENUM TranslateFeature_CharToEnum (char feature)
{
    return ((FEATURE_ID_ENUM) (feature - 1));    // this should be one to one correlation, adjusted for fence-posting.
}

//****************************************************************************
// Funtion: SendGetVersionCommand
// Description: This creates a queue msg to ask for the Head Array Version.
//
//****************************************************************************

void SendGetVersionCommand (void)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_VERSION_GET;

    tx_queue_send(&g_GUI_to_COMM_queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

void SendVersionToGUI (uint8_t majorVersion, uint8_t minorVersion, uint8_t buildVersion)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_VERSION_GET;
    HHP_Msg.Version.m_Major = majorVersion;
    HHP_Msg.Version.m_Minor = minorVersion;
    HHP_Msg.Version.m_Build = buildVersion;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Funtion: Send_GetPadAssignmentMsg
// Description: This creates a queue msg. Translates the Pad ENUM to char and
//      sends the packet to the queue.
//****************************************************************************

void SendGetPadAssignmentMsg (PHYSICAL_PAD_ENUM pad)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_GET;
    msg.PadAssignmentRequestMsg.m_PhysicalPadNumber = pad;

    tx_queue_send(&g_GUI_to_COMM_queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
void SendSetPadAssignmentCommand (PHYSICAL_PAD_ENUM pad, PAD_DIRECTION_ENUM direction, PAD_TYPE_ENUM padType)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_SET;
    msg.PadAssignmentSetMsg.m_PhysicalPadNumber = pad;
    msg.PadAssignmentSetMsg.m_LogicalDirection = direction;
    msg.PadAssignmentSetMsg.m_PadType = padType;

    tx_queue_send(&g_GUI_to_COMM_queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

    SendGetPadAssignmentMsg (pad);      // This allows the system to retrieve the currently set directional settings from the Head Array.
}

//****************************************************************************
void SendPadGetResponse (PHYSICAL_PAD_ENUM physicalPad, PAD_DIRECTION_ENUM assignment, PAD_TYPE_ENUM padType)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_SET;
    HHP_Msg.PadAssignmentResponseMsg.m_PhysicalPadNumber = physicalPad;
    HHP_Msg.PadAssignmentResponseMsg.m_LogicalDirection = assignment;
    HHP_Msg.PadAssignmentResponseMsg.m_PadType = padType;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: SendModeChangeCommand
// Description: This function sends a message to the COMM Task to be sent to the Head Array
//  This message is to Change the Mode.
//****************************************************************************

void SendModeChangeCommand (FEATURE_ID_ENUM newMode)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_ACTIVE_FEATURE_SET;
    q_Msg.ModeChangeMsg.m_Mode = newMode;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

}

//****************************************************************************
// Function: SendCalibrationStartCommand and SendCalibrationStopCommand
// Description: This function sends a message to the COMM Task to be sent to the Head Array
//  This message is used to start calibration mode of the Head Array.
//  A second message is used to stop calibration mode of the Head Array.
//****************************************************************************

void SendCalibrationStartCommand (void)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_CALIBRATE_START_CMD;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

}

void SendCalibrationStopCommand (void)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_CALIBRATE_STOP_CMD;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

}
