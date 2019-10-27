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
void InitializeSys(void)
{
  LongHoldtmr = 0;

#ifdef using_keyboard  
  populate_keyboard_flag = 0;
#endif  
  page_information_screen_flag = 0;	//init_screen
  
  function_set_flag = 0;
  Shut_down_display_timeout2 = 0;
  
  beep_level = IOPORT_LEVEL_LOW;
  
}

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
    if (IOPORT_LEVEL_LOW == pin_state)
        sw_data = sw_data|0x02;
 
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

//-------------------------------------------------------------------------
//	main menu 
//-------------------------------------------------------------------------

void main_menu(void) 
{
  uint8_t 	sw_tmp;
  
	
  switch(get_sw()) {

    case kbd_fwd:    

        if(page_information_screen_flag == 16) {	//DateTime_screen
        	if(date_time_disp_num < 7) {
        		LongHoldtmr = 25;
            flonghold = 0;
            while(get_sw()==sw_fwd) {    // inc speed   
              if(LongHoldtmr == 0) {
              	if(date_time_disp_num == 0) {	//Hour
              		rtcc_data[10] = rtcc_data[3]&0xe0;
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[3]&0x1f);
              		
                	sw_tmp = rtcc_data[11]%5;
                	if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5 - sw_tmp);
                	else rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5);    // inc 5% every time
                	if(rtcc_data[11] > 12) rtcc_data[11] = 12;      // limit to 12
                	
                	rtcc_data[3] = DEC_TO_BCD(rtcc_data[11]) | rtcc_data[10];
                }
                else if(date_time_disp_num == 1) {	//Minute
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[2]&0x7f);
              		
                	sw_tmp = rtcc_data[11]%5;
                	if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5 - sw_tmp);
                	else rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5);    // inc 5% every time
                	if(rtcc_data[11] > 59) rtcc_data[11] = 59;      // limit to 59
                	
                	rtcc_data[2] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 2) {	//Second
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[1]&0x7f);
              		
                	sw_tmp = rtcc_data[11]%5;
                	if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5 - sw_tmp);
                	else rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5);    // inc 5% every time
                	if(rtcc_data[11] > 59) rtcc_data[11] = 59;      // limit to 59
                	
                	rtcc_data[1] = DEC_TO_BCD(rtcc_data[11]) | 0x80;
                }
                else if(date_time_disp_num == 3) {	//AM/PM
                	if( (rtcc_data[3]&0x20) == 0x20 ) {
                		rtcc_data[3] = rtcc_data[3]&0xdf;
                		rtcc_data[11] = 0;
                	}
              		else {
                		rtcc_data[3] = rtcc_data[3]|0x20;
                		rtcc_data[11] = 1;
              		}
                }
                else if(date_time_disp_num == 4) {	//Year
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[7]);
              		
                	sw_tmp = rtcc_data[11]%5;
                	if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5 - sw_tmp);
                	else rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5);    // inc 5% every time
                	if(rtcc_data[11] > 99) rtcc_data[11] = 99;      // limit to 99
                	
                	rtcc_data[7] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 5) {	//Month
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	sw_tmp = rtcc_data[11]%5;
                	if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5 - sw_tmp);
                	else rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5);    // inc 5% every time
                	if(rtcc_data[11] > 12) rtcc_data[11] = 12;      // limit to 12
                	
                	rtcc_data[6] = DEC_TO_BCD(rtcc_data[11]);
                	
                	//Check Maximum Date
                	rtcc_data[12] = BCD_TO_DEC(rtcc_data[5]&0x3f);
                	if(rtcc_data[12] > DateLimit[rtcc_data[11]]) {
                		rtcc_data[12] = DateLimit[rtcc_data[11]];  // limit
                		
                		rtcc_data[5] = DEC_TO_BCD(rtcc_data[12]);
                		
                		sprintf(text_temp[2], "%02d", rtcc_data[12]);
                		gx_prompt_text_set(DateTime_pmpt_pos[2], text_temp[2]);
                	}
                	 
                }
                else if(date_time_disp_num == 6) {	//Date
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[5]&0x3f);
              		rtcc_data[12] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	sw_tmp = rtcc_data[11]%5;
                	if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5 - sw_tmp);
                	else rtcc_data[11] = (uint8_t)(rtcc_data[11] + 5);    // inc 5% every time
                	if(rtcc_data[11] > DateLimit[rtcc_data[12]]) rtcc_data[11] = DateLimit[rtcc_data[12]];  // limit
                	
                	rtcc_data[5] = DEC_TO_BCD(rtcc_data[11]);
                }
                else rtcc_data[11] = 7; //Can't be here
                
                if(date_time_disp_num == 3) {
                	if(rtcc_data[11]) gx_prompt_text_set(DateTime_pmpt_pos[3], "PM");
        					else gx_prompt_text_set(DateTime_pmpt_pos[3], "AM");
                }
                else {
                	sprintf(text_temp[date_time_row_num], "%02d", rtcc_data[11]);
                	gx_prompt_text_set(DateTime_pmpt_pos[date_time_row_num], text_temp[date_time_row_num]);
                }
                
      					gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
                update_display();
                
                LongHoldtmr = 25;
                flonghold = 1;
               
               	//Refresh WDT
 	  						g_wdt.p_api->refresh(g_wdt.p_ctrl);
              }
              
            }
            
            if(get_sw() == 0 && flonghold == 0) {
            		if(date_time_disp_num == 0) {	//Hour
              		rtcc_data[10] = rtcc_data[3]&0xe0;
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[3]&0x1f);
              		
                	rtcc_data[11]++;
                	if(rtcc_data[11] > 12) rtcc_data[11] = 12;      // limit to 12
                	
                	rtcc_data[3] = DEC_TO_BCD(rtcc_data[11]) | rtcc_data[10];
                }
                else if(date_time_disp_num == 1) {	//Minute
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[2]&0x7f);
              		
                	rtcc_data[11]++;
                	if(rtcc_data[11] > 59) rtcc_data[11] = 59;      // limit to 59
                	
                	rtcc_data[2] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 2) {	//Second
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[1]&0x7f);
              		
                	rtcc_data[11]++;
                	if(rtcc_data[11] > 59) rtcc_data[11] = 59;      // limit to 59
                	
                	rtcc_data[1] = DEC_TO_BCD(rtcc_data[11]) | 0x80;
                }
                else if(date_time_disp_num == 3) {	//AM/PM
                	if( (rtcc_data[3]&0x20) == 0x20 ) {
                		rtcc_data[3] = rtcc_data[3]&0xdf;
                		rtcc_data[11] = 0;
                	}
              		else {
              			rtcc_data[3] = rtcc_data[3]|0x20;
                		rtcc_data[11] = 1;
              		}
                }
                else if(date_time_disp_num == 4) {	//Year
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[7]);
              		
                	rtcc_data[11]++;
                	if(rtcc_data[11] > 99) rtcc_data[11] = 99;      // limit to 99
                	
                	rtcc_data[7] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 5) {	//Month
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	rtcc_data[11]++;
                	if(rtcc_data[11] > 12) rtcc_data[11] = 12;      // limit to 12
                	
                	rtcc_data[6] = DEC_TO_BCD(rtcc_data[11]);
                	
                	//Check Maximum Date
                	rtcc_data[12] = BCD_TO_DEC(rtcc_data[5]&0x3f);
                	if(rtcc_data[12] > DateLimit[rtcc_data[11]]) {
                		rtcc_data[12] = DateLimit[rtcc_data[11]];  // limit
                		
                		rtcc_data[5] = DEC_TO_BCD(rtcc_data[12]);
                		
                		sprintf(text_temp[2], "%02d", rtcc_data[12]);
                		gx_prompt_text_set(DateTime_pmpt_pos[2], text_temp[2]);
                	}
                	
                }
                else if(date_time_disp_num == 6) {	//Date
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[5]&0x3f);
              		rtcc_data[12] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	rtcc_data[11]++;
                	if(rtcc_data[11] > DateLimit[rtcc_data[12]]) rtcc_data[11] = DateLimit[rtcc_data[12]];  // limit
                	
                	rtcc_data[5] = DEC_TO_BCD(rtcc_data[11]);
                }
                else rtcc_data[11] = 7; //Can't be here
                
            }
            
	          if(date_time_disp_num == 3) {
            	if(rtcc_data[11]) gx_prompt_text_set(DateTime_pmpt_pos[3], "PM");
        			else gx_prompt_text_set(DateTime_pmpt_pos[3], "AM");
            }
            else {
            	sprintf(text_temp[date_time_row_num], "%02d", rtcc_data[11]);
            	gx_prompt_text_set(DateTime_pmpt_pos[date_time_row_num], text_temp[date_time_row_num]);
            }
            
      			gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
            update_display();
            
            change_flag = 1;
            do_not_save = 0; 
          
          }
          else while( get_sw() != 0 );
             
        }

        break;
            
    case kbd_rev:      

        if(page_information_screen_flag == 16) {	//DateTime_screen
        	if(date_time_disp_num < 7) {
        		LongHoldtmr = 25;
            flonghold = 0;
            while(get_sw()==kbd_rev) {
              if(LongHoldtmr == 0) {
              	if(date_time_disp_num == 0) {	//Hour
              		rtcc_data[10] = rtcc_data[3]&0xe0;
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[3]&0x1f);
              		
              		if(rtcc_data[11] < 6) rtcc_data[11] = RtccMinLimit[date_time_disp_num];  //limit
                	else {
                	  sw_tmp = rtcc_data[11]%5;
                	  if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] - sw_tmp);
                	  else rtcc_data[11] = (uint8_t)(rtcc_data[11] - 5);  //dec 5% every time
                	}
              		
                	rtcc_data[3] = DEC_TO_BCD(rtcc_data[11]) | rtcc_data[10];
                }
                else if(date_time_disp_num == 1) {	//Minute
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[2]&0x7f);
              		
                	if(rtcc_data[11] < 6) rtcc_data[11] = RtccMinLimit[date_time_disp_num];  //limit
                	else {
                	  sw_tmp = rtcc_data[11]%5;
                	  if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] - sw_tmp);
                	  else rtcc_data[11] = (uint8_t)(rtcc_data[11] - 5);  //dec 5% every time
                	}
                	
                	rtcc_data[2] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 2) {	//Second
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[1]&0x7f);
              		
                	if(rtcc_data[11] < 6) rtcc_data[11] = RtccMinLimit[date_time_disp_num];  //limit
                	else {
                	  sw_tmp = rtcc_data[11]%5;
                	  if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] - sw_tmp);
                	  else rtcc_data[11] = (uint8_t)(rtcc_data[11] - 5);  //dec 5% every time
                	}
                	
                	rtcc_data[1] = DEC_TO_BCD(rtcc_data[11]) | 0x80;
                }
                else if(date_time_disp_num == 3) {	//AM/PM
                	if( (rtcc_data[3]&0x20) == 0x20 ) {
                		rtcc_data[3] = rtcc_data[3]&0xdf;
                		rtcc_data[11] = 0;
                	}
              		else {
              			rtcc_data[3] = rtcc_data[3]|0x20;
              			rtcc_data[11] = 1;
              		}
                }
                else if(date_time_disp_num == 4) {	//Year
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[7]);
              		
                	if(rtcc_data[11] < 6) rtcc_data[11] = RtccMinLimit[date_time_disp_num];  //limit
                	else {
                	  sw_tmp = rtcc_data[11]%5;
                	  if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] - sw_tmp);
                	  else rtcc_data[11] = (uint8_t)(rtcc_data[11] - 5);  //dec 5% every time
                	}
                	
                	rtcc_data[7] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 5) {	//Month
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	if(rtcc_data[11] < 6) rtcc_data[11] = RtccMinLimit[date_time_disp_num];  //limit
                	else {
                	  sw_tmp = rtcc_data[11]%5;
                	  if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] - sw_tmp);
                	  else rtcc_data[11] = (uint8_t)(rtcc_data[11] - 5);  //dec 5% every time
                	}
                	
                	rtcc_data[6] = DEC_TO_BCD(rtcc_data[11]);
                	
                	//Check Maximum Date
                	rtcc_data[12] = BCD_TO_DEC(rtcc_data[5]&0x3f);
                	if(rtcc_data[12] > DateLimit[rtcc_data[11]]) {
                		rtcc_data[12] = DateLimit[rtcc_data[11]];  // limit
                		
                		rtcc_data[5] = DEC_TO_BCD(rtcc_data[12]);
                		
                		sprintf(text_temp[2], "%02d", rtcc_data[12]);
                		gx_prompt_text_set(DateTime_pmpt_pos[2], text_temp[2]);
                	}
                	
                }
                else if(date_time_disp_num == 6) {	//Date
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[5]&0x3f);
              		rtcc_data[12] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	if(rtcc_data[11] < 6) rtcc_data[11] = RtccMinLimit[date_time_disp_num];  //limit
                	else {
                	  sw_tmp = rtcc_data[11]%5;
                	  if(sw_tmp) rtcc_data[11] = (uint8_t)(rtcc_data[11] - sw_tmp);
                	  else rtcc_data[11] = (uint8_t)(rtcc_data[11] - 5);  //dec 5% every time
                	}
                	
                	rtcc_data[5] = DEC_TO_BCD(rtcc_data[11]);
                }
                else rtcc_data[11] = 7; //Can't be here
                
                if(date_time_disp_num == 3) {
                	if(rtcc_data[11]) gx_prompt_text_set(DateTime_pmpt_pos[3], "PM");
        					else gx_prompt_text_set(DateTime_pmpt_pos[3], "AM");
                }
                else {
                	sprintf(text_temp[date_time_row_num], "%02d", rtcc_data[11]);
                	gx_prompt_text_set(DateTime_pmpt_pos[date_time_row_num], text_temp[date_time_row_num]);
                }
                
      					gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
                update_display();
                
                LongHoldtmr = 25;
                flonghold = 1;
               
               	//Refresh WDT
 	  						g_wdt.p_api->refresh(g_wdt.p_ctrl);
              }
              
            }
            
            if(get_sw() == 0 && flonghold == 0) {
            		if(date_time_disp_num == 0) {	//Hour
              		rtcc_data[10] = rtcc_data[3]&0xe0;
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[3]&0x1f);
              		
              		if(rtcc_data[11] > RtccMinLimit[date_time_disp_num]) rtcc_data[11]--;  
                	
                	rtcc_data[3] = DEC_TO_BCD(rtcc_data[11]) | rtcc_data[10];
                }
                else if(date_time_disp_num == 1) {	//Minute
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[2]&0x7f);
              		
                	if(rtcc_data[11] > RtccMinLimit[date_time_disp_num]) rtcc_data[11]--;  
                	
                	rtcc_data[2] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 2) {	//Second
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[1]&0x7f);
              		
                	if(rtcc_data[11] > RtccMinLimit[date_time_disp_num]) rtcc_data[11]--;  
                	
                	rtcc_data[1] = DEC_TO_BCD(rtcc_data[11]) | 0x80;
                }
                else if(date_time_disp_num == 3) {	//AM/PM
                	if( (rtcc_data[3]&0x20) == 0x20 ) {
                		rtcc_data[3] = rtcc_data[3]&0xdf;
                		rtcc_data[11] = 0;
                	}
              		else {
              			rtcc_data[3] = rtcc_data[3]|0x20;
                		rtcc_data[11] = 1;
              		}
                }
                else if(date_time_disp_num == 4) {	//Year
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[7]);
              		
                	if(rtcc_data[11] > RtccMinLimit[date_time_disp_num]) rtcc_data[11]--;  
                	
                	rtcc_data[7] = DEC_TO_BCD(rtcc_data[11]);
                }
                else if(date_time_disp_num == 5) {	//Month
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	if(rtcc_data[11] > RtccMinLimit[date_time_disp_num]) rtcc_data[11]--;  
                	
                	rtcc_data[6] = DEC_TO_BCD(rtcc_data[11]);
                	
                	//Check Maximum Date
                	rtcc_data[12] = BCD_TO_DEC(rtcc_data[5]&0x3f);
                	if(rtcc_data[12] > DateLimit[rtcc_data[11]]) {
                		rtcc_data[12] = DateLimit[rtcc_data[11]];  // limit
                		
                		rtcc_data[5] = DEC_TO_BCD(rtcc_data[12]);
                		
                		sprintf(text_temp[2], "%02d", rtcc_data[12]);
                		gx_prompt_text_set(DateTime_pmpt_pos[2], text_temp[2]);
                	}
                	
                }
                else if(date_time_disp_num == 6) {	//Date
              		rtcc_data[11] = BCD_TO_DEC(rtcc_data[5]&0x3f);
              		rtcc_data[12] = BCD_TO_DEC(rtcc_data[6]&0x1f);
              		
                	if(rtcc_data[11] > RtccMinLimit[date_time_disp_num]) rtcc_data[11]--;  
                	
                	rtcc_data[5] = DEC_TO_BCD(rtcc_data[11]);
                }
                else rtcc_data[11] = 7; //Can't be here
                
            }
            
	          if(date_time_disp_num == 3) {
            	if(rtcc_data[11]) gx_prompt_text_set(DateTime_pmpt_pos[3], "PM");
        			else gx_prompt_text_set(DateTime_pmpt_pos[3], "AM");
            }
            else {
            	sprintf(text_temp[date_time_row_num], "%02d", rtcc_data[11]);
            	gx_prompt_text_set(DateTime_pmpt_pos[date_time_row_num], text_temp[date_time_row_num]);
            }
            
      			gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
            update_display();
            
            change_flag = 1;
            do_not_save = 0; 
          
          }
          else while( get_sw() != 0 );
             
        }
        break;

    case sw_fwd_rev: 
    
    		if(page_information_screen_flag == 1) {
    			
  				R_BSP_SoftwareDelay(1500, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(1500);  
  				//reset the System
        	NVIC_SystemReset();
  				//can't come here  
        	while(get_sw() == sw_fwd_rev);
        }

        break;
                
    default:

        break;
  }
  
  update_display();
  
}

//-------------------------------------------------------------------------

        
