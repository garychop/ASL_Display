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

//*************************************************************************
// Typedefs
//*************************************************************************

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

typedef struct HHP_HA_MSG_S
{
    uint32_t m_MsgType;         // Use the above mentioned enum.
    union
    {
        struct
        {
            uint32_t m_HB_OK;   // Non0 if Heart Beat is OK, 0 = failed.
            uint32_t HB_Count;  // number that increments with each successful heart beat.
        } HeartBeatMsg;
        struct
        {
            uint32_t m_MsgArray[15];
        } WholeMsg;
    };
} HHP_HA_MSG_STRUCT;


#endif /* QUEUEDEFINITION_H_ */
