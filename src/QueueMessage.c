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

PAD_DESIGNATION_ENUM TranslatePad (char pad)
{
    PAD_DESIGNATION_ENUM myPad = INVALID_PAD;

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

//****************************************************************************

PAD_DIRECTION_ENUM TranslatePadDirection (char padDirection)
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

//****************************************************************************

void SendPadAssignmentRequestMsg (char pad)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_GET;
    msg.PadAssignmentRequestMsg.m_PhysicalPadNumber = pad;

    tx_queue_send(&g_GUI_to_COMM_queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
void SendPadAssignmentSetCommand (char pad, char direction)
{
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_SET;
    msg.PadAssignmentSetMsg.m_PhysicalPadNumber = pad;
    msg.PadAssignmentSetMsg.m_LogicalDirection = direction;

    tx_queue_send(&q_COMM_to_GUI_Queue, &msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

    SendPadAssignmentRequestMsg (pad);      // This allows the system to retrieve the currently set directional settings from the Head Array.
}

//****************************************************************************
void SendPadAssignmentResponse (char physicalPad, char assignment)
{
    HHP_HA_MSG_STRUCT HHP_Msg;

    HHP_Msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_GET_RESPONSE;
    HHP_Msg.PadAssignmentResponseMsg.m_PhysicalPadNumber = physicalPad;
    HHP_Msg.PadAssignmentResponseMsg.m_LogicalDirection = assignment;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HHP_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".
}

//****************************************************************************
// Function: SendModeChangeCommand
// Description: This function sends a message to the COMM Task to be sent to the Head Array
//  This message is to Change the Mode.
//****************************************************************************

void SendModeChangeCommand (uint8_t newMode)
{
    GUI_MSG_STRUCT q_Msg;
    uint8_t myMode = 0x00;

    q_Msg.m_MsgType = HHP_HA_MODE_CHANGE_SET;
    switch (newMode)
    {   // This translate the array position into the command data.
        case 0: myMode = 0x01; break;   // Power On/Off
        case 1: myMode = 0x02; break;   // Bluetooth
        case 2: myMode = 0x03; break;   // Next Function
        case 3: myMode = 0x04; break;   // Next Profile
    }
    q_Msg.ModeChangeMsg.m_Mode = myMode;

    tx_queue_send(&g_GUI_to_COMM_queue, &q_Msg, 10); // TX_NO_WAIT. Without a wait the process seems to be too fast for the processing of the "send".

}
