/**************************************************************************
Project: ASL (for ASL Controller)
---------------------------------------------------------------------
V0.0.1 Jul22, 2019

#- for ASL
---------------------------------------------------------------------
***************************************************************************/
#ifndef MY_GUI_THREAD_ENTRY_H_
#define MY_GUI_THREAD_ENTRY_H_


//-------------------------------------------------------------------------
#define DEC_TO_BCD(N) (uint8_t)( (N/10)*16 + N%10 )
#define BCD_TO_DEC(N) (uint8_t)( ((N&0xF0)/16)*10 + (N&0x0F) )
											
//-------------------------------------------------------------------------

#define beep_out  			IOPORT_PORT_03_PIN_08	//beep Control

#define BACKLIGHT_CONTROL_PIN  	IOPORT_PORT_03_PIN_10

#define GRNLED_PORT        	IOPORT_PORT_07_PIN_10	//IOPORT_PORT_06_PIN_00

#define TEST_PIN        IOPORT_PORT_07_PIN_12

#define eprm_sel	  		IOPORT_PORT_03_PIN_07
#define eprm_out	  		IOPORT_PORT_03_PIN_04
#define eprm_in	  			IOPORT_PORT_03_PIN_05
#define eprm_clk	  		IOPORT_PORT_03_PIN_06

//-------------------------------------------------------------------------

#define ON   1
#define OFF  0
#define TRUE 1
#define FALSE 0

#define kbd_fwd         5
#define kbd_rev         6

#define sw_fwd          5
#define sw_rev          6
#define sw_fwd_rev      21

#define shut_down_timer1	800  	//20s
#define shut_down_timer2	2400	//60s

uint8_t i2c_data[30],i2c_data_read;//, i2c_get_len;

//*****************************************************************************************************
// This is need to preserve the status of the Interrups when macro TX_DISABLE and TX_RESTORE are used.
unsigned int interrupt_save;

//*****************************************************************************************************
// This holds the Major, Minor and Build #'s of the ASL110 Head Array
uint8_t g_HA_MajorVersion, g_HA_MinorVersion, g_HA_BuildVersion;

//*****************************************************************************************************
// Beeping information
bool g_BeepControlFlag;     // NonZero means we are beeping locally; 0 = No beeping.
ioport_level_t g_BeepPortStatus;

//*****************************************************************************************************
void Process_Touches (void);

UINT show_window(GX_WINDOW * p_new, GX_WIDGET * p_widget, bool detach_old);

//ASL-PROP
uint8_t get_PROP_version(void);

//-------------------------------------------------------------------------
#endif /* MY_GUI_THREAD_ENTRY_H_ */
