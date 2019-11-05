//****************************************************************************
// Module
// Filename
// Project
//----------------------------------------------------------------------------
// Controller
//
// Compiler
//
// Description
//
//----------------------------------------------------------------------------
// Date
//
//****************************************************************************
//
// This file is property of ASL, Inc. and can not be duplicated,
// copied or used without the expressed written permission by ASL, Inc.
//
//****************************************************************************
// Project Includes
//****************************************************************************

#include "DigitalControl.h"
//#include "r_ioport_api.h"

//****************************************************************************
// Macros
//****************************************************************************

//****************************************************************************
// Defines
//****************************************************************************

#define BEEP_OUT        IOPORT_PORT_03_PIN_08   //beep Control
#define GRNLED_PORT     IOPORT_PORT_07_PIN_10   //IOPORT_PORT_06_PIN_00
#define UP_ARROW_BTN_PORT   IOPORT_PORT_00_PIN_05
#define DOWN_ARROW_BTN_PORT IOPORT_PORT_00_PIN_06

#define UP_ARROW_BTN    0x01
#define DOWN_ARROW_BTN  0x02

//****************************************************************************
// Typedefs
//****************************************************************************

typedef struct st_ButtonInfo
{
    ioport_port_pin_t m_PortID;
    ioport_level_t m_KeyState;
    uint16_t m_Counter;
    uint16_t m_Mask;
} st_ButtonInfo_t;

//****************************************************************************
// Imported Global Variables
//****************************************************************************

//****************************************************************************
// Global Variables
//****************************************************************************

st_ButtonInfo_t g_ButtonInfo[] = {{UP_ARROW_BTN_PORT, IOPORT_LEVEL_LOW,0, UP_ARROW_BTN},
                                  {DOWN_ARROW_BTN_PORT, IOPORT_LEVEL_LOW,0, DOWN_ARROW_BTN}};

uint16_t g_ArrowState = 0;          // Where 0x01 = Up, 0x02, Down
uint16_t g_OldArrowState = 0;

//****************************************************************************
// External Prototypes
//****************************************************************************

//****************************************************************************
// Prototypes Of Local Functions
//****************************************************************************


/* Digital Control entry function */
void DigitalControl_entry(void)
{
//    bool myFlag = 0;
    ioport_level_t pin_state;

    tx_thread_sleep (50);      // 10 milliseconds increments.

    while (1)
    {
        // Do keyboard input processing including debouncing. Arrow Buttons are active LOW.
        for (uint16_t i=0; i<2; ++i)
        {
            g_ioport.p_api->pinRead(g_ButtonInfo[i].m_PortID, &pin_state);     // Get current status of switch input
            if (pin_state == g_ButtonInfo[i].m_KeyState)                       // Did the state change?
            {                                                                   // No, the status remains the same.
                if (++g_ButtonInfo[i].m_Counter > 8)                           // Are we same state long enough, 8*10 milliseconds
                {
                    g_ButtonInfo[i].m_Counter = 8;                             // Yep, make sure we don't exceed the uint.
                    if (pin_state == IOPORT_LEVEL_HIGH)                         // Is it High or low.
                    {
                        g_ArrowState &= (0xff ^ g_ButtonInfo[i].m_Mask);        // Couldn't use "~" without getting compile warnings. A true hack
                    }
                    else
                    {
                        g_ArrowState |= g_ButtonInfo[i].m_Mask;                // It's high, set the state high.
                    }
                }
            }
            else    // Yes, the state of switch DID change.
            {
                g_ButtonInfo[i].m_Counter = 0;                                 // Reset the counter
                g_ButtonInfo[i].m_KeyState = pin_state;                        // retain the current state of the switch.
            }
        }

        // Process a change in switch state and turn on LED when active as diagnostics.
        if (g_OldArrowState!=g_ArrowState)
        {
            g_OldArrowState = g_ArrowState;
            if ((g_ArrowState &= (UP_ARROW_BTN | DOWN_ARROW_BTN)) == 0x00)
                g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_HIGH);       // Turn off LED
            else
                g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);        // Turn on LED
        }

//        if (myFlag == 0)
//        {
//            g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
//            myFlag = 1;
//        }
        // The following make the buzzer sound but poorly
#ifdef OK_TO_USE_BAD_BUZZER
        g_ioport.p_api->pinWrite(BEEP_OUT, IOPORT_LEVEL_LOW);
        tx_thread_sleep (1);
        g_ioport.p_api->pinWrite(BEEP_OUT, IOPORT_LEVEL_HIGH);
        tx_thread_sleep (1);
#endif //OK_TO_USE_BAD_BUZZER
        tx_thread_sleep (1);
    }
}
