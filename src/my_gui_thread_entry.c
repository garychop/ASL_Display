  //-------------------------------------------------------------------------
#include "my_gui_thread.h"
#include "lcd.h"
#include <my_gui_thread_entry.h>
#include <stdio.h>
#include "gx_api.h"
#include "my_guix_resources.h"
#include "my_guix_specifications.h"

//-------------------------------------------------------------------------
GX_CHAR version_string[16]     = "Version: 0.0.1a";
GX_CHAR version_string2[20] = "Attendant: V0.0.1";
GX_CHAR version_string3[10]     		 	=  "V0.0.1";

//-------------------------------------------------------------------------
// Local variables
//-------------------------------------------------------------------------

// "MainMenuPrompts" is used to hold information about the 4 items displayed on the main user screen.
// This information includes the pixel maps, location and whether it's programmed for usage or not.
struct MainMenuPrompts
{
    int m_Location;
    int m_Active;       // FALSE is inactive, TRUE is active
    GX_PIXELMAP_PROMPT *m_SmallPrompt;
    GX_PIXELMAP_PROMPT *m_LargePrompt;
} g_ScreenPrompts[4];

GX_RECTANGLE g_HiddenRectangle = {0,0,0,0};

GX_RECTANGLE g_DiagnosticWidgetLocations[] = {
    {35, 32, 35+62, 32+90},
    {183, 32, 183+62, 32+90},
    {66, 140, 66+145, 140+42}};

// This holds the location of the prompts on the main screen for the purpose of showing and hiding the active features.
GX_RECTANGLE g_FeatureLocation[] = {
    {10, 16, 300, 86},
    {30, 94, 290, 130},
    {30, 130, 290, 162},
    {30, 166, 290, 198},
    {0,0,0,0}};


//-------------------------------------------------------------------------
extern GX_PROMPT * time_infor_pmpt_text;

//-------------------------------------------------------------------------
GX_WINDOW_ROOT * p_window_root;
GX_PROMPT * firmware_ver_text = &init_screen.init_screen_InitPrmpt2;
GX_PROMPT * first_pmpt_text = &init_screen.init_screen_InitPrmpt3;

//-------------------------------------------------------------------------
// Forward declarations.
//-------------------------------------------------------------------------
void my_gui_thread_entry(void);

static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload);

static void reset_check(void);

UINT DisplayMainScreenActiveFeatures ();

//-------------------------------------------------------------------------
void  g_timer0_callback(timer_callback_args_t * p_args) //25ms
{
  (void)p_args;


  if(Shut_down_display_timeout != 0) Shut_down_display_timeout--;

  if(Shut_down_display_timeout2 != 0) Shut_down_display_timeout2--;

  if(chk_status_timeout != 0) chk_status_timeout--;

  if(LongHoldtmr != 0) LongHoldtmr--;

  if(chk_date_time_timeout != 0) chk_date_time_timeout--;

  if(chk_screen_chg_timeout != 0) chk_screen_chg_timeout--;
  
}

//-------------------------------------------------------------------------
void  g_timer1_callback(timer_callback_args_t * p_args) //1612us
{
  (void)p_args;


  if(beep_level == IOPORT_LEVEL_LOW) beep_level = IOPORT_LEVEL_HIGH;
  else beep_level = IOPORT_LEVEL_LOW;

  g_ioport.p_api->pinWrite(beep_out, beep_level);
  
}

//-------------------------------------------------------------------------
void  g_timer2_callback(timer_callback_args_t * p_args) //819us
{
  (void)p_args;


  if(beep_level == IOPORT_LEVEL_LOW) beep_level = IOPORT_LEVEL_HIGH;
  else beep_level = IOPORT_LEVEL_LOW;

  g_ioport.p_api->pinWrite(beep_out, beep_level);
  
}

//-------------------------------------------------------------------------
void g_lcd_spi_callback(spi_callback_args_t * p_args)
{
  if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE) 
    tx_semaphore_ceiling_put(&g_my_gui_semaphore, 1);
    
}

//-------------------------------------------------------------------------
uint8_t eprm_read(uint16_t addr)	//need 163us?
{
  uint8_t i, data;
  uint16_t addr_temp;
  ioport_level_t pin_state;
  
  
  addr_temp = addr&0x00ff;
  if(addr&0x0100) addr_temp |= 0x0b00; 
  else addr_temp |= 0x0300;
  
  g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);
  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_LOW);
  R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
  
  //send address
  for(i = 0; i < 16; i++) {
    if( addr_temp&0x8000 ) {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_HIGH);//output_high(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);    
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //delay_us(1);
    }
    else {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_LOW);//output_low(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1); 
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    
    addr_temp = (uint16_t)(addr_temp<<1);
    
  }
	
	//read data
	data = 0;
  for(i = 0; i < 8; i++) {
    //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    g_ioport_on_ioport.pinRead(eprm_out, &pin_state); 
    if(pin_state == IOPORT_LEVEL_HIGH) data = (uint8_t)( (data<<1)|0x01 );
    else data = (uint8_t)( (data<<1)&0xfe );
    g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
    R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
    R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    
  }
  
  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);//output_high(eprm_sel);
	
  return data;
	
}
//-------------------------------------------------------------------------
uint8_t eprm_write(uint16_t addr, uint8_t value)	//need 2.72ms?
{
  uint8_t i, data, wait;
  uint16_t addr_temp;
  ioport_level_t pin_state;
  
  
//  output_low(PIN_B7);
  
  addr_temp = addr&0x00ff;
  if(addr&0x0100) addr_temp |= 0x0a00; 
  else addr_temp |= 0x0200;
  
  g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_LOW);//output_low(eprm_sel);
  R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);

  //send Write Enable
  data = 0x06;
  for(i = 0; i < 8; i++) {
    if( data&0x80 ) {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_HIGH);//output_high(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);    
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    else {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_LOW);//output_low(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1); 
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    
    data = (uint8_t)(data<<1);
    
  }

  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);//output_high(eprm_sel);
  R_BSP_SoftwareDelay(3,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(3);

  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_LOW);//output_low(eprm_sel);
  R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
  
  //send address
  for(i = 0; i < 16; i++) {
    if( addr_temp&0x8000 ) {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_HIGH);//output_high(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);    
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    else {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_LOW);//output_low(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1); 
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    
    addr_temp = (uint16_t)(addr_temp<<1);
    
  }
	
	//send value
	for(i = 0; i < 8; i++) {
    if( value&0x80 ) {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_HIGH);//output_high(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);    
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    else {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_LOW);//output_low(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1); 
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    
    value = (uint8_t)(value<<1);
    
  }

  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);//output_high(eprm_sel);
	
	//read Status Register
  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_LOW);//output_low(eprm_sel);
  R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
  
  //send instruction
  data = 0x05;
  for(i = 0; i < 8; i++) {
    if( data&0x80 ) {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_HIGH);//output_high(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);    
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    else {
      g_ioport.p_api->pinWrite(eprm_in, IOPORT_LEVEL_LOW);//output_low(eprm_in);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1); 
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
      R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
      g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
      //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
    }
    
    data = (uint8_t)(data<<1);
    
  }
	
  wait = 90; //waiting time //max: about 5ms 
  do {	
  	//read data
  	data = 0;
    for(i = 0; i < 8; i++) {
  	  //R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1); 
  	  g_ioport_on_ioport.pinRead(eprm_out, &pin_state); 
    	if(pin_state == IOPORT_LEVEL_HIGH) data = (uint8_t)( (data<<1)|0x01 );
    	else data = (uint8_t)( (data<<1)&0xfe );
    	
  	  g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_HIGH);//output_high(eprm_clk);
  	  R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
  	  g_ioport.p_api->pinWrite(eprm_clk, IOPORT_LEVEL_LOW);//output_low(eprm_clk);
  	  R_BSP_SoftwareDelay(1,BSP_DELAY_UNITS_MICROSECONDS);//delay_us(1);
  	  
  	}
  	
  	wait--;
    if(wait == 0) break; //time out
      
	}while(data&0x01);	//nomally 'wait' is change to 39


  g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);//output_high(eprm_sel);
	
	if(wait == 0) {
	//	eeprm_w_err++;
		return 1;						//err happenned
	}
	else return 0;
	
//	output_high(PIN_B7);
	
}

//-------------------------------------------------------------------------
void init_rtcc(void)
{
	ssp_err_t err;
	
	
	rtcc_data[8] = 0;	//addr0 for reading ST
	g_i2c1.p_api->write(g_i2c1.p_ctrl, &rtcc_data[8], 1, true);

	err = g_i2c1.p_api->read(g_i2c1.p_ctrl, rtcc_data, 1, false);
  if(SSP_SUCCESS != err)
  {
      g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
  }
  
  if( (rtcc_data[0]&0x80) != 0x80 ) { //ST = 0
		rtcc_data[0] = 0; 		//addr0
		rtcc_data[1] = 0x81; 	//Data to TIMEKEEPING SECONDS VALUE REGISTER
		rtcc_data[2] = 0x01;
		rtcc_data[3] = 0x41; 	//set to 12 Hour Time Format
		rtcc_data[4] = 0x09;	//External Battery Backup Supply (VBAT) Enable
		rtcc_data[5] = 0x01;
		rtcc_data[6] = 0x01;
		rtcc_data[7] = 0x17;
		rtcc_data[8] = 0x80; 	//CONTROL REGISTER
		rtcc_data[9] = 80;		//OSCILLATOR DIGITAL TRIM REGISTER
		
		err = g_i2c1.p_api->write(g_i2c1.p_ctrl, rtcc_data, 10, false);
  	if(SSP_SUCCESS != err)
  	{
  	   g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
  	}
  	
  }  

}

//-------------------------------------------------------------------------
void read_rtcc(void)
{
	ssp_err_t err;
	
	
	rtcc_data[0] = 0;	//addr0
	g_i2c1.p_api->write(g_i2c1.p_ctrl, &rtcc_data[0], 1, true);

	err = g_i2c1.p_api->read(g_i2c1.p_ctrl, &rtcc_data[1], 7, false);
  if(SSP_SUCCESS != err)
  {
      g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
  }
  
}

//-------------------------------------------------------------------------
void write_rtcc(void)
{
	ssp_err_t err;
	
  
	rtcc_data[0] = 0; //addr0
	err = g_i2c1.p_api->write(g_i2c1.p_ctrl, rtcc_data, 8, false);
  if(SSP_SUCCESS != err)
  {
     g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
  }
  	
}

//-------------------------------------------------------------------------

/*********************************************************************************************************************
 * @brief  Sends a touch event to GUIX internal thread to call the GUIX event handler function
 *
 * @param[in] p_payload Touch panel message payload
***********************************************************************************************************************/
static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload)
{
    bool send_event = true;
    GX_EVENT gxe;
		GX_VALUE x_num;	//, y_num;
		

    switch (p_payload->event_type)
    {
    case SF_TOUCH_PANEL_EVENT_DOWN:
        gxe.gx_event_type = GX_EVENT_PEN_DOWN;
        break;
    case SF_TOUCH_PANEL_EVENT_UP:
        gxe.gx_event_type = GX_EVENT_PEN_UP;
        break;
    case SF_TOUCH_PANEL_EVENT_HOLD:
    case SF_TOUCH_PANEL_EVENT_MOVE:
        gxe.gx_event_type = GX_EVENT_PEN_DRAG;
        break;
    case SF_TOUCH_PANEL_EVENT_INVALID:
        send_event = false;
        break;
    default:
        break;
    }

    if (send_event)
    {
        /** Send event to GUI */
        gxe.gx_event_sender         = GX_ID_NONE;
        gxe.gx_event_target         = 0;  /* the event to be routed to the widget that has input focus */
        gxe.gx_event_display_handle = 0;

        gxe.gx_event_payload.gx_event_pointdata.gx_point_x = (GX_VALUE)(p_payload->y);//y_num;
        
        x_num = (GX_VALUE)(240 - p_payload->x);

        if(x_num < 140) {
           if(x_num < 5) x_num = 0;
           else if(x_num < 10) x_num = (GX_VALUE)(x_num - 5);
           else if(x_num < 20) x_num = (GX_VALUE)(x_num - 9);
           else if(x_num < 40) x_num = (GX_VALUE)(x_num - 19);
           else if(x_num < 50) x_num = (GX_VALUE)(x_num - 32);
           else if(x_num < 70) x_num = (GX_VALUE)(x_num - 30);
           else if(x_num < 90) x_num = (GX_VALUE)(x_num - 25);
           else if(x_num < 110) x_num = (GX_VALUE)(x_num -20);
           else if(x_num < 120) x_num = (GX_VALUE)(x_num -15);
           else x_num = (GX_VALUE)(x_num - 12);
        }
        if(x_num > 190) {
        	x_num = (GX_VALUE)(x_num + 10);
        	if(x_num > 240) x_num = 240;
        }

        gxe.gx_event_payload.gx_event_pointdata.gx_point_y = x_num;

        gx_system_event_send(&gxe);
    }
}
//-------------------------------------------------------------------------
void update_display(void)
{
    ssp_err_t err;

    sf_message_header_t * p_message = NULL;

    GX_EVENT gxe;


    gxe.gx_event_sender = GX_ID_NONE;

    gxe.gx_event_target = 0;

    gxe.gx_event_display_handle = 0;

    gxe.gx_event_type = UPDATE_DISPLAY_EVENT;

    gx_system_event_send(&gxe);
    
   	
    err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &my_gui_thread_message_queue, (sf_message_header_t **) &p_message, 10); //TX_WAIT_FOREVER); //
    if(!err) {
        switch (p_message->event_b.class_code)
        {
            case SF_MESSAGE_EVENT_CLASS_TOUCH:
                if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
                {
                    //sf_touch_panel_payload_t * p_touch_message = (sf_touch_panel_payload_t *) p_message;

                    /** Translate a touch event into a GUIX event */
                    guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
                }
                break;

            default:
                break;
        }

        /** Message is processed, so release buffer. */
        err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
        if (err)
        {
        		g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
        }

    }

}

//-------------------------------------------------------------------------
void show_info_scrn(void)
{
    ssp_err_t err;

    sf_message_header_t * p_message = NULL;

    GX_EVENT gxe;


    gxe.gx_event_sender = GX_ID_NONE;

    gxe.gx_event_target = 0;

    gxe.gx_event_display_handle = 0;

    gxe.gx_event_type = Display_Information_Screen;

    gx_system_event_send(&gxe);
   
    err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &my_gui_thread_message_queue, (sf_message_header_t **) &p_message, 10); //TX_WAIT_FOREVER);
    if(!err) {
        switch (p_message->event_b.class_code)
        {
            case SF_MESSAGE_EVENT_CLASS_TOUCH:
                if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
                {
                    //sf_touch_panel_payload_t * p_touch_message = (sf_touch_panel_payload_t *) p_message;

                    // Translate a touch event into a GUIX event 
                    guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
                }
                break;

            default:
                break;
        }

        // Message is processed, so release buffer.
        err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
        if (err)
        {
        		g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
        }
				
    }

}
//-------------------------------------------------------------------------
void return_info_scrn(void)
{
    ssp_err_t err;

    sf_message_header_t * p_message = NULL;

    GX_EVENT gxe;


    gxe.gx_event_sender = GX_ID_NONE;

    gxe.gx_event_target = 0;

    gxe.gx_event_display_handle = 0;

    gxe.gx_event_type = Return_InforScr_Event;

    gx_system_event_send(&gxe);
   
    err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &my_gui_thread_message_queue, (sf_message_header_t **) &p_message, 10); //TX_WAIT_FOREVER);
    if(!err) {
        switch (p_message->event_b.class_code)
        {
            case SF_MESSAGE_EVENT_CLASS_TOUCH:
                if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
                {
                    //sf_touch_panel_payload_t * p_touch_message = (sf_touch_panel_payload_t *) p_message;

                    // Translate a touch event into a GUIX event 
                    guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
                }
                break;

            default:
                break;
        }

        // Message is processed, so release buffer.
        err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
        if (err)
        {
        		g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
        }
				
    }

}
//-------------------------------------------------------------------------
static void reset_check(void) 
{
	UINT status = TX_SUCCESS;
	
   
  switch(get_sw()) {
    case sw_fwd_rev: 
    		status = gx_prompt_text_set(first_pmpt_text, "Reset Device");
				if (GX_SUCCESS != status)
    		{
    			g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    		}
    		
    		update_display();
           
  			
//GC  			R_BSP_SoftwareDelay(1500, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(1500);
  			
  			//reset the System
        NVIC_SystemReset();
  			//can't come here  
        while(get_sw() == sw_fwd_rev);
        break;

    default:
//GC    		R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(100);
       	break;
  }

}

//-------------------------------------------------------------------------
/* Gui Test App Thread entry function */
void my_gui_thread_entry(void)
{
    ssp_err_t err;
    sf_message_header_t * p_message = NULL;
    UINT status = TX_SUCCESS;
    uint8_t i, test_num;
    int check_temp_num;
    
    
		//debug pins
    g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(TEST_PIN, IOPORT_LEVEL_LOW);
    
    g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);
  	g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);
		g_ioport.p_api->pinWrite(beep_out, IOPORT_LEVEL_LOW);

    /* Initializes GUIX. */
    status = gx_system_initialize();
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    /* Initializes GUIX drivers. */
    err = g_sf_el_gx0.p_api->open (g_sf_el_gx0.p_ctrl, g_sf_el_gx0.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    gx_studio_display_configure ( MAIN_DISPLAY,
                                  g_sf_el_gx0.p_api->setup,
                                  LANGUAGE_ENGLISH,
                                  MAIN_DISPLAY_THEME_1,
                                  &p_window_root );

    err = g_sf_el_gx0.p_api->canvasInit(g_sf_el_gx0.p_ctrl, p_window_root);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    // Populate the screen stuff.
    // Power On/Off
    g_ScreenPrompts[0].m_Location = 0;
    g_ScreenPrompts[0].m_Active = TRUE;
    g_ScreenPrompts[0].m_LargePrompt = &Main_User_Screen.Main_User_Screen_PowerLargePrompt;
    g_ScreenPrompts[0].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_PowerSmallPrompt;
    // Bluetooth
    g_ScreenPrompts[1].m_Location = 1;
    g_ScreenPrompts[1].m_Active = TRUE;
    g_ScreenPrompts[1].m_LargePrompt = &Main_User_Screen.Main_User_Screen_BluetoothLargePrompt;
    g_ScreenPrompts[1].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_BluetoothSmallPrompt;
    // Next Function
    g_ScreenPrompts[2].m_Location = 2;
    g_ScreenPrompts[2].m_Active = TRUE;
    g_ScreenPrompts[2].m_LargePrompt = &Main_User_Screen.Main_User_Screen_FunctionNextLargePrompt;
    g_ScreenPrompts[2].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_FunctionNextSmallPrompt;
    // Next Profile
    g_ScreenPrompts[3].m_Location = 3;
    g_ScreenPrompts[3].m_Active = TRUE;
    g_ScreenPrompts[3].m_LargePrompt = &Main_User_Screen.Main_User_Screen_ProfileNextLargePrompt;
    g_ScreenPrompts[3].m_SmallPrompt = &Main_User_Screen.Main_User_Screen_ProfileNextSmallPrompt;


    /* Create the widgets we have defined with the GUIX data structures and resources. */
    GX_WIDGET * p_first_screen = NULL;
    
		
		/* Create the screens. */
    gx_studio_named_widget_create("edit_screen", (GX_WIDGET *)p_window_root, &p_first_screen);

    gx_studio_named_widget_create("popup_screen2", (GX_WIDGET *)p_window_root, &p_first_screen);

    gx_studio_named_widget_create("DateTime_screen", (GX_WIDGET *)p_window_root, &p_first_screen);

    gx_studio_named_widget_create("popup_screen", (GX_WIDGET *)p_window_root, &p_first_screen);

  #ifdef using_keyboard   
    gx_studio_named_widget_create("keyboard_screen", (GX_WIDGET *)p_window_root, &p_first_screen);
	#endif
		
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }
		
		status = gx_studio_named_widget_create("information_screen",
                                           (GX_WIDGET *)p_window_root, &p_first_screen);
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }
    
    status = gx_studio_named_widget_create("init_screen",
                                           (GX_WIDGET *)p_window_root, &p_first_screen);
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    // Create and show first startup screen.
    status = gx_studio_named_widget_create("Main_User_Screen",
                                           (GX_WIDGET *)p_window_root, &p_first_screen);
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    /* Attach the first screen to the root so we can see it when the root is shown */
   gx_widget_attach(p_window_root, p_first_screen);

    /* Shows the root window to make it and patients screen visible. */
    status = gx_widget_show(p_window_root);
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    /* Lets GUIX run. */
    status = gx_system_start();
    if(TX_SUCCESS != status)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    gx_system_focus_claim(p_first_screen);

    /** Open the SPI driver to initialize the LCD **/
    err = g_rspi_lcdc.p_api->open(g_rspi_lcdc.p_ctrl, g_rspi_lcdc.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    // Open the I2C driver for CLOCK 
    err = g_i2c1.p_api->open(g_i2c1.p_ctrl, g_i2c1.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }

    // Open the ADC driver for Thermistor 
    err = g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    if(SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }
		
		g_ioport.p_api->pinWrite(eprm_sel, IOPORT_LEVEL_HIGH);


    /** Setup the ILI9341V **/
    ILI9341V_Init();
		
	InitializeSys();

  	LCD_ON();
		
	chk_status_timeout = 14; //(14+2)*25 = 400ms
	//debug_timer0_flag = 0;
	err = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg);
	if (err != SSP_SUCCESS)
	{
		g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);	//Error
	}
		
	// version of firmware
	status = gx_prompt_text_set(firmware_ver_text, version_string);
	if (GX_SUCCESS != status)
    {
    	g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
    }
    
//    status = gx_prompt_text_set(first_pmpt_text, "Init...");
//	if (GX_SUCCESS != status)
//    {
//    	g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
//    }
    
    update_display();

// Removed the RTCC.
//	init_rtcc();

    //delay 4.5s	
//    R_BSP_SoftwareDelay(4500, BSP_DELAY_UNITS_MILLISECONDS);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
 	
//    //Open WDT; 4.46s; PCLKB 30MHz
// 	g_wdt.p_api->open(g_wdt.p_ctrl, g_wdt.p_cfg);
//	//Start the WDT by refreshing it
// 	g_wdt.p_api->refresh(g_wdt.p_ctrl);
  
  	i = 0;
  	do {
  		test_num = get_PROP_version();
  		
    	//GC Replace the following with a tx_thread_sleep
  		//GC    R_BSP_SoftwareDelay(30, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(30);
  		tx_thread_sleep(10);
    	//Start the WDT by refreshing it
// 	  	g_wdt.p_api->refresh(g_wdt.p_ctrl);
    	if(i > 5)
    	    break;
    	i++;
  	} while(test_num != 0);

  	if (i > 5)
  	{
  	    beep_set(3, 300);
//  	    status = gx_prompt_text_set(first_pmpt_text, "Communication Fail");
//		if (GX_SUCCESS != status)
//    	{
//    		g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
//    	}
		update_display();
    
    	while(1)
    	{
    		reset_check();
    		tx_thread_sleep (2);
    	}
  	}
 	
	show_info_scrn();

  	//beep
	beep_set(3, 300);
  	
  	Shut_down_display_timeout = shut_down_timer1;
  	
  	chk_status_timeout = 20;	//delay about 550ms
  	
 	check_temp_num = 0;
    
    //Open WDT; 4.46s; PCLKB 30MHz
 	//  g_wdt.p_api->open(g_wdt.p_ctrl, g_wdt.p_cfg);
		//Start the WDT by refreshing it
 	//  g_wdt.p_api->refresh(g_wdt.p_ctrl);

  	
  	chk_screen_chg_timeout = 16; //for Static Test
  						
    while(1)
    {
        err = g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &my_gui_thread_message_queue, (sf_message_header_t **) &p_message, 10); //TX_WAIT_FOREVER); //
        if(!err)
        {
            switch (p_message->event_b.class_code)
            {
                case SF_MESSAGE_EVENT_CLASS_TOUCH:
                    if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
                    {
                        //sf_touch_panel_payload_t * p_touch_message = (sf_touch_panel_payload_t *) p_message;

                        // Translate a touch event into a GUIX event 
                        guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
                    }
                    break;

                default:
                    break;
            }

            //Message is processed, so release buffer.
            err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
            if (err)
                {
                    g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);
                }

        }

        if( chk_status_timeout == 0 && LCD_off_flag == 0 ) { //adding LCD_off_flag check is for clear 'sigma_dat1_get' 
        
          get_PROP_version();//chk_sigma_status();
          chk_status_timeout = 20;	//delay about 550ms
          
          update_display();
        }
        
        if(LCD_off_flag == 1)
            main_menu();
        
        if(Shut_down_display_timeout == 0 && Shut_down_display_timeout2 == 0 && LCD_off_flag == 0) {
          BackLight(OFF);
          LCD_off_flag = 1;
        }
				
				//Check Date_Time
    		if(chk_date_time_timeout == 0) {
    			
    			if(page_information_screen_flag == 1) {
    				read_rtcc();
    				
						if( (rtcc_data[3]&0x20) == 0x20 ) sprintf(time_string, "%02x:%02x", rtcc_data[3]&0x1f, rtcc_data[2]);
        		else sprintf(time_string, "%02x:%02x", rtcc_data[3]&0x1f, rtcc_data[2]);
        	
    				gx_prompt_text_set(time_infor_pmpt_text, time_string);
    				
    				chk_date_time_timeout = 40;  //check every 1s
    				
    				update_display();
        	}   
  			
  				check_temp_num++;
  				if(check_temp_num > 10) {	//11s
  					check_temp_num = 0;
  				}
    		}
        
        //for Static Test
        if(page_information_screen_flag == 1 && chk_screen_chg_timeout == 0 && LCD_off_flag == 1) {	
        	chk_screen_chg_timeout = 64; //for Static Test
        	show_window((GX_WINDOW*)&information_screen, (GX_WIDGET*)&information_screen, false);
          chk_screen_chg_timeout = 32; //for Static Test
        }
        
			 	//Refresh WDT
 	  		g_wdt.p_api->refresh(g_wdt.p_ctrl);

    }
}

//-------------------------------------------------------------------------

//*************************************************************************************
// Function Name: DisplayMainScreenActiveFeatures
//
// Description: This displays the features that are active in the order specificed
//  in the Screen Prompts "objects".
//
//*************************************************************************************

UINT DisplayMainScreenActiveFeatures ()
{
    int activeCount;
    int feature;
    UINT myErr = GX_SUCCESS;

    // Count the number of active items so we can populate appropriately.
    // Hide the Non-Active features.
    activeCount = 0;
    for (feature = 0; feature < 4; ++feature)
    {
        if (g_ScreenPrompts[feature].m_Active == FALSE)
        {
            myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
            myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_HiddenRectangle);
            ++activeCount;
        }
    }

    // Locate the first feature to display
    for (feature = 0; feature < 4; ++feature)
    {
        if (g_ScreenPrompts[feature].m_Active)
        {
            switch (g_ScreenPrompts[feature].m_Location)
            {
            case 0: // Show the first line
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_FeatureLocation[0]);
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_HiddenRectangle);
                break;
            case 1: // Show second line item
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_FeatureLocation[1]);
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
                break;
            case 2: // Process third line item, move to the 2nd line
                // Hide Large Icon, show small icon
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_FeatureLocation[2]);
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
                break;
            case 3: // Process fourth line item, move to the 3rd line.
                // Hide Large Icon, show small icon
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_SmallPrompt, &g_FeatureLocation[3]);
                myErr = gx_widget_resize ((GX_WIDGET*) g_ScreenPrompts[feature].m_LargePrompt, &g_HiddenRectangle);
                break;
            }
        }
    }
    return myErr;
}


//*************************************************************************************
// Function Name: Template_event_function
//
// Description: This handles any messages sent to the template window
//
//*************************************************************************************

UINT Template_event_function (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    UINT myErr = 0;

#ifdef OLD_CODE_THAT_MAY_BE_USEFUL_SOMEDAY

    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (window->gx_widget_name == "PadCalibrationScreen")
        {
            if (g_CalibrationStepNumber == 0)       // We are doing minimum
            {
                if (g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue > 4)
                    g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue -= 5;
                myErr = gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);
            }
            else if (g_CalibrationStepNumber == 1)  // Doing maximum
            {
                if (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue > 4)
                    g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue -= 5;
                myErr = gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
            }
            gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        }
        break;
    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
        if (window->gx_widget_name == "PadCalibrationScreen")
        {
            if (g_CalibrationStepNumber == 0)       // We are doing minimum
            {
                if (g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue < 100)
                    g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue += 5;
                myErr = gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMinimumCalibrationValue);
            }
            else if (g_CalibrationStepNumber == 1)  // Doing maximum
            {
                if (g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue < 100)
                    g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue += 5;
                myErr = gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_Value_Prompt, g_PadSettings[g_CalibrationPadNumber].m_PadMaximumCalibrationValue);
            }
            gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        }
        break;
    case GX_EVENT_PEN_DOWN: // We are going to determine if the PAD button is pressed and start a timer to increment the
                            // ... long time (2 seconds) and goto Programming if so.
        if (event_ptr->gx_event_target->gx_widget_name == "PadActiveButton")
        {
            g_DeltaValue = +5;
            gx_system_timer_start(window, PAD_ACTIVE_TIMER_ID, 8, 8);
            //myErr = gx_slider_value_set((GX_SLIDER*)&PadCalibrationScreen.PadCalibrationScreen_PadValue_Slider, &PadCalibrationScreen.PadCalibrationScreen_PadValue_Slider.gx_slider_info, g_PadValue);
        }
        break;
    case GX_EVENT_TIMER:
        if (event_ptr->gx_event_payload.gx_event_timer_id == PAD_ACTIVE_TIMER_ID)
        {
            g_PadValue += g_DeltaValue;
            if (g_PadValue > 100)
                g_PadValue = 100;
            if (g_PadValue <= 0)
                gx_system_timer_stop(window, PAD_ACTIVE_TIMER_ID);
            if (g_PadValue > 0)
            {
                myErr = gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_PadValue_Prompt, &g_CalibrationPromptLocations[1]);
                myErr = gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_PadValue_Prompt, g_PadValue);
            }
            else
            {
                myErr = gx_widget_resize ((GX_WIDGET*) &PadCalibrationScreen.PadCalibrationScreen_PadValue_Prompt, &g_HiddenRectangle);
                myErr = gx_numeric_prompt_value_set (&PadCalibrationScreen.PadCalibrationScreen_PadValue_Prompt, g_PadValue);
            }
            gx_system_dirty_mark(g_CalibrationScreen);      // This forces the gauge to be updated and redrawn
        }
        break;
    case GX_EVENT_PEN_UP:
        {
            g_DeltaValue = -5;
        }
        break;
    } // end switch
#endif

    myErr = gx_window_event_process(window, event_ptr);

    return myErr;
}


//*************************************************************************************
// Function Name: Main_User_Screen_event_process
//
// Description: This handles the User Screen messages.
//
//*************************************************************************************
VOID Main_User_Screen_draw_function(GX_WINDOW *window)
{
    gx_window_draw(window);

}

UINT Main_User_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    UINT myErr = 0;

    #ifdef SOMEDAY_THIS_WILL_BECOME_USEFUL
    UINT feature;
    int activeCount;

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_TIMER:
        if (event_ptr->gx_event_payload.gx_event_timer_id == ARROW_PUSHED_TIMER_ID)
        {
            myErr = gx_widget_attach (root, (GX_WIDGET*) &HHP_Start_Screen);
            myErr = gx_widget_show ((GX_WIDGET*) &HHP_Start_Screen);
            g_ChangeScreen_WIP = TRUE;
        }
        break;
    case GX_EVENT_PEN_DOWN: // We are going to determine if the Up or Down arrow buttons have been held for a
                            // ... long time (2 seconds) and goto Programming if so.

        if ((event_ptr->gx_event_target->gx_widget_name == "DownArrowButton") || (event_ptr->gx_event_target->gx_widget_name == "UpArrowButton"))
        {
            gx_system_timer_start(window, ARROW_PUSHED_TIMER_ID, 100, 0);
            g_ChangeScreen_WIP = FALSE;
        }
        break;
    case GX_EVENT_PEN_UP:
            gx_system_timer_stop(window, ARROW_PUSHED_TIMER_ID);
        break;

    case GX_EVENT_SHOW:
        g_GoBackScreen = window;        // Set the going back window.
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL (DOWN_ARROW_BTN_ID, GX_EVENT_CLICKED):
    case GX_SIGNAL (LONG_PRESS_BUTTON_ID, GX_EVENT_CLICKED):
        // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
        if (g_ChangeScreen_WIP)
        {
            g_ChangeScreen_WIP = FALSE;
            break;
        }
        // Count the number of active features to set a limit on location
        activeCount = 0;
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
                ++activeCount;
        }
        // Move Top Feature to Bottom and move Bottom upward.
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
            {
                if (g_ScreenPrompts[feature].m_Location == 0)
                    g_ScreenPrompts[feature].m_Location = activeCount-1;
                else if (g_ScreenPrompts[feature].m_Location == 1)
                    g_ScreenPrompts[feature].m_Location = 0;
                else if (g_ScreenPrompts[feature].m_Location == 2)
                    g_ScreenPrompts[feature].m_Location = min (1, activeCount-1);
                else if (g_ScreenPrompts[feature].m_Location == 3)
                    g_ScreenPrompts[feature].m_Location = min (2, activeCount-1);
            }
        }
        DisplayMainScreenActiveFeatures();
        break;

    case GX_SIGNAL(UP_ARROW_BTN_ID, GX_EVENT_CLICKED):
        // This is necessary to prevent the subsequent "Clicked" message from advancing the feature when we are changing to the Programming screen.
        if (g_ChangeScreen_WIP)
        {
            g_ChangeScreen_WIP = FALSE;
            break;
        }
        // Count the number of active features to set a limit on location
        activeCount = 0;
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
                ++activeCount;
        }

        // Move the features downward, limiting the movement by the number of Active Features.
        for (feature = 0; feature < 4; ++feature)
        {
            if (g_ScreenPrompts[feature].m_Active)
            {
                if (g_ScreenPrompts[feature].m_Location == 0)
                    g_ScreenPrompts[feature].m_Location = min (1, activeCount);
                else if (g_ScreenPrompts[feature].m_Location == 1)
                    g_ScreenPrompts[feature].m_Location = min (2, activeCount);
                else if (g_ScreenPrompts[feature].m_Location == 2)
                    g_ScreenPrompts[feature].m_Location = min (3, activeCount);
                else if (g_ScreenPrompts[feature].m_Location == 3)
                    g_ScreenPrompts[feature].m_Location = 0;
                if (g_ScreenPrompts[feature].m_Location == activeCount)
                    g_ScreenPrompts[feature].m_Location = 0;
            }
        }
        DisplayMainScreenActiveFeatures();
        break;
    }
#endif

    DisplayMainScreenActiveFeatures();  // Remove this when more of the previous code is "undefinded".

    myErr = gx_window_event_process(window, event_ptr);

    return myErr;
}

