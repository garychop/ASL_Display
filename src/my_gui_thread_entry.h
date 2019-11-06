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
#define using_keyboard
												 
//-------------------------------------------------------------------------
//define EVENT
//#define COMMUNCATION_ERROR_HAPPENED (GX_FIRST_APP_EVENT + 1)
#define UPDATE_DISPLAY_EVENT        (GX_FIRST_APP_EVENT + 2)
#define Display_Information_Screen  (GX_FIRST_APP_EVENT + 3)
#define KEY_PRESS_EVENT          		(GX_FIRST_APP_EVENT + 4)
#define ATT_DEFALT_EVENT          	(GX_FIRST_APP_EVENT + 5)
#define C505_DEFALT_EVENT          	(GX_FIRST_APP_EVENT + 6)
#define Return_InforScr_Event     	(GX_FIRST_APP_EVENT + 7)
#define Return_SettingScr_Event    	(GX_FIRST_APP_EVENT + 8)
#define Single_Page_Event    				(GX_FIRST_APP_EVENT + 9)

//define others
#define GX_KEY_RETURN_PREV_PAGE    0x1111

//-------------------------------------------------------------------------
#define DEC_TO_BCD(N) (uint8_t)( (N/10)*16 + N%10 )
#define BCD_TO_DEC(N) (uint8_t)( ((N&0xF0)/16)*10 + (N&0x0F) )
											
//-------------------------------------------------------------------------

#define i2c_cs         	IOPORT_PORT_05_PIN_01	//IOPORT_PORT_04_PIN_10
#define i2c_res        	IOPORT_PORT_05_PIN_02	//IOPORT_PORT_04_PIN_11
#define i2c_io         	IOPORT_PORT_05_PIN_03	//IOPORT_PORT_04_PIN_12
#define i2c_clk        	IOPORT_PORT_05_PIN_04	//IOPORT_PORT_04_PIN_13

#define beep_out  			IOPORT_PORT_03_PIN_08	//beep Control

#define BACKLIGHT_EN  	IOPORT_PORT_03_PIN_10

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

//#define FLASHRATE  20 //15

#define kbd_fwd         5
#define kbd_rev         6

#define sw_fwd          5
#define sw_rev          6
#define sw_fwd_rev      21

#define shut_down_timer1	800  	//20s

#define shut_down_timer2	2400	//60s


//-------------------------------------------------------------------------
typedef struct SETTING_DATA_STRUCT {
    GX_CHAR *name;
} SETTING_DATA;


uint8_t i2c_data[30],i2c_data_read;//, i2c_get_len;

uint8_t chk_status_timeout;
uint16_t Shut_down_display_timeout;
uint16_t Shut_down_display_timeout2; //use in setting mode

uint8_t LongHoldtmr;
 
unsigned int interrupt_save;

GX_CHAR text_temp[8][4];
GX_CHAR itos_string[14];
GX_CHAR time_string[14];

uint8_t page_information_screen_flag;	

uint8_t date_time_page_num;
uint8_t date_time_disp_num;
uint8_t date_time_row_num;
uint8_t rtcc_data[16];		//Normally, rtcc_data[0]:init addr; rtcc_data[1]~rtcc_data[7] -> Sec,Min,Hr,WD,Date,Ms,Yr
uint8_t chk_date_time_timeout;

ioport_level_t beep_level;

uint8_t prop_ver1, prop_ver2, prop_ver3;

//----------------------------
bool LCD_off_flag;
bool flonghold;
bool beep_on_flag;

#ifdef using_keyboard
bool populate_keyboard_flag;
#endif

bool change_flag;
bool do_not_save;

bool function_set_flag;

//-------------------------------------------------------------------------
void InitializeSys(void);
void BackLight(int ONOFF);
void LCD_ON(void);
void main_menu(void);
void Process_Touches (void);

uint8_t m250_pwr_on(uint8_t on_off);
uint8_t m250_setup_mode(uint8_t on_off);

uint8_t kbhit_sw(void);
uint8_t get_sw_code(void);
uint8_t get_sw(void);
void show_info_scrn(void);
void return_info_scrn(void);

void beep_set(uint8_t beepType, uint32_t beepPeriod);

UINT show_window(GX_WINDOW * p_new, GX_WIDGET * p_widget, bool detach_old);

//ASL-PROP
uint8_t get_PROP_version(void);

//-------------------------------------------------------------------------
#endif /* MY_GUI_THREAD_ENTRY_H_ */
