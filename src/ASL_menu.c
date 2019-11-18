//-------------------------------------------------------------------------
#include <stdio.h>
#include "my_gui_thread.h"
#include <my_gui_thread_entry.h>

//-------------------------------------------------------------------------

extern GX_PROMPT * DateTime_pmpt_pos[];
extern GX_PROMPT * DateTime_pmpt_text;

//-------------------------------------------------------------------------

const uint8_t DateLimit[13] = {
  30,     
  31, //Jan    
  28,     
  31,     
  30,     
  31,     
  30,     
  31,     
  31,     
  30,     
  31,     
  30,   
  31, //Dec    
};

const uint8_t RtccMinLimit[8] = {
  1,     
  0,     
  0,     
  0,  //AM/PM   
  0,     
  1,     
  1, 	//Date    
  1,     
};

//-------------------------------------------------------------------------
void BackLight(int ONOFF) 
{
  if (ONOFF == ON) g_ioport.p_api->pinWrite(BACKLIGHT_EN, IOPORT_LEVEL_HIGH);	//output_high(BACKLIGHT_EN);
  else  g_ioport.p_api->pinWrite(BACKLIGHT_EN, IOPORT_LEVEL_LOW);	//output_low(BACKLIGHT_EN); 

}

//-------------------------------------------------------------------------
void LCD_ON(void)
{
	
  BackLight(ON);
  Shut_down_display_timeout = shut_down_timer1;
  if(function_set_flag) Shut_down_display_timeout2 = shut_down_timer2;
  LCD_off_flag = 0;    //Havn't turned off LCD
  
}

//-------------------------------------------------------------------------
// Function Name: khbit_sw
//
//-------------------------------------------------------------------------

uint8_t kbhit_sw(void) 
{ 
	uint8_t sw_data;
	ioport_level_t pin_state;

	sw_data = 0;

    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_05, &pin_state);	//SW1
    if (IOPORT_LEVEL_LOW == pin_state)
        sw_data = sw_data|0x01;

    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_06, &pin_state);	//SW2
    if (IOPORT_LEVEL_LOW == pin_state) sw_data = sw_data|0x02;
 
	return sw_data;
}

//-------------------------------------------------------------------------

uint8_t get_sw_code(void) 
{
  uint8_t rtn1, rtn2, rtn3;
  

//  restart_wdt();
	//Refresh WDT
 	g_wdt.p_api->refresh(g_wdt.p_ctrl);
  if(kbhit_sw()==0) return 0;	// no sw pressed
  else {
    rtn1 = kbhit_sw();
    R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(10);
    rtn2 = kbhit_sw();

    if(rtn1 == rtn2) return rtn1;	// has sw
    else {
      R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(10);
      rtn3 = kbhit_sw();
 
      if( (rtn1==rtn3) || (rtn2==rtn3) ) return rtn3;	// has sw
      else return 0;	// no sw
    }      
  }
  
}

//-------------------------------------------------------------------------
uint8_t get_sw(void) 
{
  uint8_t get_value;
  

  get_value = get_sw_code();   
  
  if(LCD_off_flag == 1 && get_value != 0) LCD_ON();
  if(get_value != 0 && function_set_flag) Shut_down_display_timeout2 = shut_down_timer2;
  
  switch(get_value) {
    case 0:     // no sw
      return 0;
      break;         
    
    case 0x01:  // S5
      return 5;						//sw_fwd
      break;
       
    case 0x02:  // S4
      return 6;						//sw_rev
      break;
       
    case 0x03:  // S5+S4
      return 21;					//sw_fwd_rev
      break;
       
    default:    // error;
      return 0;
      break;
        
  }
  
}

//-------------------------------------------------------------------------
// beepType 2,3;  beepPeriod: ms
void beep_set(uint8_t beepType, uint32_t beepPeriod)
{
	if(beep_on_flag == 0) return;
	
  switch(beepType) {
    case 2:
      g_timer1.p_api->open(g_timer1.p_ctrl, g_timer1.p_cfg);	//1.6128ms
      R_BSP_SoftwareDelay(beepPeriod, BSP_DELAY_UNITS_MILLISECONDS);
      g_timer1.p_api->close(g_timer1.p_ctrl);
      g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);
      break;
       
    case 3:         
      g_timer2.p_api->open(g_timer2.p_ctrl, g_timer2.p_cfg);	//819.2us
      R_BSP_SoftwareDelay(beepPeriod, BSP_DELAY_UNITS_MILLISECONDS);
      g_timer2.p_api->close(g_timer2.p_ctrl);
      g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);
      break;
       
  }
  
  g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);

}


        
