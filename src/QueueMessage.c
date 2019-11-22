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

#include "QueueDefinition.h"
#include "tx_api.h"

//****************************************************************************
// Functions
//****************************************************************************

void SendPadAssignmentRequestMsg (char pad, TX_QUEUE *queue)
{
    uint32_t qStatus;
    GUI_MSG_STRUCT msg;

    msg.m_MsgType = HHP_HA_PAD_ASSIGMENT_GET;
    msg.PadAssignmentRequestMsg.m_PhysicalPadNumber = pad;
    if (queue != NULL)
    {
        qStatus = tx_queue_send(queue, &msg, TX_NO_WAIT);
    }
}

