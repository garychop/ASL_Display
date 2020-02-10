//****************************************************************************
// Module       Digital Control
// Filename     DigitalControl_entry.c
// Project      ASL110 Display Unit
//----------------------------------------------------------------------------
// Controller   Renesas MCU ARM-Cortex 4
//
// Compiler     GNU GCC
//
// Description  This module is a ThreadX task used to read the Digital Control Input
//              and Output.
//
//      These signals include the following:
//          - UP and DOWN arrow button inputs.
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
#include "ASL_HHP_Display_GUIX_specifications.h"

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
typedef enum E_BUTTON_STATES {NO_BUTTONS_ACTIVE, SOME_BUTTON_ACTIVE, BOTH_BUTTONS_ACTIVE, WAIT_FOR_NO_ACTIVE} BUTTON_STATES_ENUM;

typedef struct st_ButtonInfo
{
    ioport_port_pin_t m_PortID;
    uint8_t m_Mask;
    ULONG m_GUI_ButtonID;
} st_ButtonInfo_t;

//****************************************************************************
// Imported Global Variables
//****************************************************************************

//****************************************************************************
// Global Variables
//****************************************************************************

st_ButtonInfo_t g_ButtonInfo[] = {{UP_ARROW_BTN_PORT, UP_ARROW_BTN, GX_SIGNAL (UP_ARROW_BTN_ID, GX_EVENT_CLICKED)},
                                  {DOWN_ARROW_BTN_PORT, DOWN_ARROW_BTN, GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED)}};

BUTTON_STATES_ENUM g_ButtonFunction = NO_BUTTONS_ACTIVE;
uint8_t g_OldButtonState;
uint8_t g_Counter = 0;

//****************************************************************************
// External Prototypes
//****************************************************************************

//****************************************************************************
// Prototypes Of Local Functions
//****************************************************************************

void Read_Arrow_Buttons(void);

//****************************************************************************
/* Digital Control entry function */
//****************************************************************************

void DigitalControl_entry(void)
{

    tx_thread_sleep (50);      // 10 milliseconds increments.

    while (1)
    {
        // Do keyboard input processing including debouncing. Arrow Buttons are active LOW.
        Read_Arrow_Buttons();

#ifdef OK_TO_USE_BAD_BUZZER
        // The following make the buzzer sound but poorly
        g_ioport.p_api->pinWrite(BEEP_OUT, IOPORT_LEVEL_LOW);
        tx_thread_sleep (1);
        g_ioport.p_api->pinWrite(BEEP_OUT, IOPORT_LEVEL_HIGH);
        tx_thread_sleep (1);
#endif //OK_TO_USE_BAD_BUZZER
        tx_thread_sleep (1);
    }
}

//****************************************************************************
// Function: Read_Arrow_Buttons(void)
//
// Description: This function reads the Up and Down arrow buttons on the
//          front panel and sets global variables.
//
// PDL start
//      Set "OldState" = NoActiveButtons
//      Get UP arrow state
//      Get DOWN arrow state
//      combine them into ButtonState
//      case NoneActive:
//          if (ButtonState != 0)
//              Set OldState = ButtonState
//              Set to "SomethingActive"
//          endcase
//      case SomethingActive:
//          if (ButtonState = 0)
//              clear counter
//              Set OldState = ButtonState
//              set to "NoneActive"
//          else if (OldState != ButtonState)
//              clear counter
//              set OldState = ButtonState
//          else
//              ++counter;
//              if (counter > 8)
//                  // Process Up, Down or BOTH buttons pushed.
//                  if (both) send BOTH msg
//                  else if (down) send Down Msg
//                  else if (up) send Up Msg
//                  set to WaitForNoneActive
//              endif
//          endcase
//      case "WatiForNoneActive:
//          if (ButtonState == 0)
//              clear counter
//              set OldState = ButtonState
//              set to NoneActive
//          endcase
//
//****************************************************************************

void Read_Arrow_Buttons(void)
{
    ioport_level_t pin_state;
    GX_EVENT gxe;
    uint8_t buttonState;
    uint8_t i;

    // Get the current state of all (both) front panel switches.
    buttonState = 0;
    for (i=0; i<2; ++i)
    {
        g_ioport.p_api->pinRead(g_ButtonInfo[i].m_PortID, &pin_state);     // Get current status of switch input
        if (pin_state == IOPORT_LEVEL_LOW)                         // (active) Low? Yes, the button is pushed.
        {
            buttonState |= g_ButtonInfo[i].m_Mask;                // It's high, set the state high.
        }
    }

    // Let's process the data as a state engine.
    switch (g_ButtonFunction)
    {
        case NO_BUTTONS_ACTIVE:
            g_Counter = 0;          // Doesn't hurt to clear the counter of every cycle.
            if (buttonState != 0)   // Buttons are active high.
            {
                g_OldButtonState = buttonState;
                g_ButtonFunction = SOME_BUTTON_ACTIVE;
            }
            break;
        case SOME_BUTTON_ACTIVE:
            if (buttonState == 0)                // Are no switches active?
                g_ButtonFunction = NO_BUTTONS_ACTIVE;
            else if (g_OldButtonState == (UP_ARROW_BTN | DOWN_ARROW_BTN)) // Are both switches active?
            {
                g_Counter = 0;
                g_ButtonFunction = BOTH_BUTTONS_ACTIVE;
            }
            else if (buttonState != g_OldButtonState) // did it flip/flop? Not likely but maybe.
            {
                g_OldButtonState = buttonState;
                g_Counter = 0;          // Rats! something changed, we need to wait even longer.
            }
            else // it hasn't changed and it's one button pushed.
            {
                if (++g_Counter > 12)      // have we debounced enough? times 10 milliseconds.
                {
                    for (i=0; i<2; ++i) // locate the pushed button and send it's respective message to the GUI task.
                    {
                        if (buttonState == g_ButtonInfo[i].m_Mask)
                        {
                            gxe.gx_event_type = g_ButtonInfo[i].m_GUI_ButtonID;
                            gxe.gx_event_sender         = GX_ID_NONE;
                            gxe.gx_event_target         = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                            g_ButtonFunction = WAIT_FOR_NO_ACTIVE;
                        }
                    }
                }
            }
            break;
        case BOTH_BUTTONS_ACTIVE:
            if (buttonState == (UP_ARROW_BTN | DOWN_ARROW_BTN))
            {
                if (++g_Counter > 200)
                {
                    gxe.gx_event_type = GX_SIGNAL(BOTH_ARROW_BTN_ID, GX_EVENT_CLICKED);
                    gxe.gx_event_sender = GX_ID_NONE;
                    gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                    gxe.gx_event_display_handle = 0;
                    gx_system_event_send(&gxe);
                    g_ButtonFunction = WAIT_FOR_NO_ACTIVE;
                }
            }
            else
            {
                g_ButtonFunction = WAIT_FOR_NO_ACTIVE;
            }
            break;
        case WAIT_FOR_NO_ACTIVE:
            g_Counter = 0;
            if (buttonState == 0)
            {
                g_OldButtonState = buttonState;
                g_ButtonFunction = NO_BUTTONS_ACTIVE;
            }
            break;
    } // end switch

}
