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
#include "ASL165_System.h"
#include "QueueDefinition.h"
#include "HeadArray_CommunicationThread.h"
#include "tx_api.h"

//****************************************************************************
// External References
//****************************************************************************


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
    if (padType == DIGITAL_PADTYPE)
        return 'D';
    else if (padType == PROPORTIONAL_PADTYPE)
        return 'P';
    else // TODO: May want something more done here at some point.
        return 'P';
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
// Function: SendGetVersionCommand
// Description: This creates a queue msg to ask for the Head Array Version.
//
//****************************************************************************

void SendGetVersionCommand (void)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_VERSION_GET;

    tx_queue_send(&g_GUI_to_COMM_queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

void SendVersionToGUI (uint8_t majorVersion, uint8_t minorVersion, uint8_t buildVersion, uint8_t eeprom)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_VERSION_GET;
    HHP_Msg.Version.m_Major = majorVersion;
    HHP_Msg.Version.m_Minor = minorVersion;
    HHP_Msg.Version.m_Build = buildVersion;
    HHP_Msg.Version.m_EEPROM_Version = eeprom;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: SendGetDataCommand
// Description: This function creates a msg to tell the COMM task to
//      start or stop sending "Get Pad Data" msgs to the Head Array.
//****************************************************************************
void SendGetDataCommand (uint8_t start, PHYSICAL_PAD_ENUM pad)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_PAD_DATA_GET;
    msg.GetDataMsg.m_Start = start;   // non0 = Start getting data, 0 = Stop getting data.
    msg.GetDataMsg.m_PadID = pad;             // specific pad or INVALID_PAD for "all".

    tx_queue_send(&g_GUI_to_COMM_queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: Send_GetPadAssignmentMsg
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

//****************************************************************************
// Function: Calibrate Functions.
//****************************************************************************

void SendGetCalDataCommnd (PHYSICAL_PAD_ENUM pad)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_CALIBRATE_RANGE_GET;
    q_Msg.GetCalibrationData.m_PadID = pad;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

void SendCalDataResponse (PHYSICAL_PAD_ENUM physicalPad, uint16_t minADC, uint16_t maxADC, uint16_t minThreshold, uint16_t maxThreshold)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_CALIBRATE_RANGE_GET;

    HHP_Msg.CalibrationDataResponse.m_PadID = physicalPad;
    HHP_Msg.CalibrationDataResponse.m_MinADC = minADC;
    HHP_Msg.CalibrationDataResponse.m_MaxADC = maxADC;
    HHP_Msg.CalibrationDataResponse.m_MinThreshold = minThreshold;
    HHP_Msg.CalibrationDataResponse.m_MaxThreshold = maxThreshold;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

void SendCalibrationData (PHYSICAL_PAD_ENUM pad, uint16_t min, uint16_t max)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_CALIBRATE_RANGE_SET;
    q_Msg.SendCalibrationData.m_PadID = pad;
    q_Msg.SendCalibrationData.m_MinThreshold = min;
    q_Msg.SendCalibrationData.m_MaxThreshold = max;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: Feature Set and Get command handling.
//****************************************************************************

void SendFeatureGetCommand (void)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_FEATURE_GET;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

// THis processes the response sending the info from the COMM -> GUI.
void SendFeatureGet (uint8_t featureSet, uint8_t timeout, uint8_t featureSet2)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_FEATURE_GET;
    HHP_Msg.GetFeatureResponse.m_FeatureSet = featureSet;
    HHP_Msg.GetFeatureResponse.m_Timeout = timeout;
    HHP_Msg.GetFeatureResponse.m_FeatureSet2 = featureSet2;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

}

// This sends the Feature Active List and the Timeout value to the Head Array via the COMM task.
void SendFeatureSetting (uint8_t activeFeatures, uint8_t timeoutValue, uint8_t activeFeatures2)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_FEATURE_SET;
    q_Msg.SendFeatureActiveList.m_FeatureActiveList = activeFeatures;
    q_Msg.SendFeatureActiveList.m_Timeout = timeoutValue;
    q_Msg.SendFeatureActiveList.m_FeatureListByte2 = activeFeatures2;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// This sends the Neutral DAC Setting Get command
//****************************************************************************

//****************************************************************************
// Function called by GUI to send data to COMM task via Queue
void SendNeutralDAC_GetCommand ()
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_NEUTRAL_DAC_GET;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function called by COMM task to send data to GUI handler via Queue
void SendNeutralDAC_GetResponse (int16_t DAC_Constant, int16_t DAC_Value, int16_t RangeValue)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_NEUTRAL_DAC_GET;
    HHP_Msg.NeutralDAC_Get_Response.m_DAC_Constant = DAC_Constant;
    HHP_Msg.NeutralDAC_Get_Response.m_NeutralDAC_Value = DAC_Value;
    HHP_Msg.NeutralDAC_Get_Response.m_Range = RangeValue;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

}

//****************************************************************************
// Function called by GUI to send data to COMM task via Queue
void SendNeutralDAC_Set (int16_t newDAC_Setting)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_NEUTRAL_DAC_SET;
    q_Msg.SendNeutralDAC_Setting.m_DAC_Setting = newDAC_Setting;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function called by GUI to send to COMM task via Queue to tell the
// head array to save all parameters.
//****************************************************************************

void SendSaveParameters (void)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_SAVE_PARAMETERS_CMD;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function called by GUI to send to COMM task via Queue to tell the
// head array to Reset all parameters.
//****************************************************************************

void SendResetParameters (void)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_RESET_PARAMETERS_CMD;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: SendDriveOffsetGet
// Description: Function called by GUI task to retrieve the Drive Offset
//          value from the Head Array via the Communication Task.
//****************************************************************************

void SendDriveOffsetGet (void)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_DRIVE_OFFSET_GET;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: SendDriveOffsetGet
// Description: Function called by COMM task to send Drive Offset value to GUI via Queue
//****************************************************************************

void SendDriveOffsetSet (uint8_t CenterPad_driveOffset, uint8_t LeftPad_driveOffset, uint8_t RightPad_driveOffset)
{
    GUI_MSG_STRUCT q_Msg;

    q_Msg.m_MsgType = HHP_HA_DRIVE_OFFSET_SET;
    q_Msg.SendDriveOffset.m_CenterPad_DriveOffset = CenterPad_driveOffset;
    q_Msg.SendDriveOffset.m_LeftPad_DriveOffset = LeftPad_driveOffset;
    q_Msg.SendDriveOffset.m_RightPad_DriveOffset = RightPad_driveOffset;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: SendDriveOffsetGet
// Description: Function called by COMM task to send Drive Offset value to GUI via Queue
//****************************************************************************

void SendDriveOffsetGetResponse (uint8_t CenterPad, uint8_t LeftPad, uint8_t RightPad)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_DRIVE_OFFSET_GET;
    HHP_Msg.DriveOffset_Get_Response.m_CenterPad_DriveOffset = CenterPad;
    HHP_Msg.DriveOffset_Get_Response.m_LeftPad_DriveOffset = LeftPad;
    HHP_Msg.DriveOffset_Get_Response.m_RightPad_DriveOffset = RightPad;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}
