//-------------------------------------------------------------------------
#include <stdio.h>
#include "tx_api.h"
#include "gx_api.h"
#include "my_guix_resources.h"
#include "my_guix_specifications.h"
#include "hal_data.h"
#include "my_gui_thread.h"
#include <my_gui_thread_entry.h>

//-------------------------------------------------------------------------
extern GX_WINDOW_ROOT * p_window_root;

#ifdef using_keyboard
extern GX_CONST GX_THEME *main_display_theme_table[];
#endif

extern GX_CHAR version_string3[];

//-------------------------------------------------------------------------
GX_PROMPT * time_infor_pmpt_text = &information_screen.information_screen_TimePrmpt;
GX_PROMPT * prop_ver_pmpt_text = &information_screen.information_screen_PropVersionPrompt;
GX_PROMPT * hhp_ver_pmpt_text = &information_screen.information_screen_HhpVersionPrompt;

#ifdef using_keyboard
GX_PROMPT * second_pmpt_text = &keyboard_screen.keyboard_screen_KeyboardPrompt;
GX_SINGLE_LINE_TEXT_INPUT * my_text_input = &keyboard_screen.keyboard_screen_keyboard_input_field;
#endif

GX_PROMPT * DateTime_pmpt_text = &DateTime_screen.DateTime_screen_DateTimePrompt;
GX_PROMPT * edit_pmpt_text = &edit_screen.edit_screen_EditPrompt;

//#ifdef has_cold_mode
//GX_PROMPT * cold_mode_pmpt = &version_screen.version_screen_ColdMdPrmpt;
//GX_TEXT_BUTTON * cold_mode_btn = &version_screen.version_screen_btn_cold_md;
//#endif


GX_PROMPT * DateTime_list_pos[4] = {
		&DateTime_screen.DateTime_screen_DateTime_list_pos0,
		&DateTime_screen.DateTime_screen_DateTime_list_pos1,
		&DateTime_screen.DateTime_screen_DateTime_list_pos2,
		&DateTime_screen.DateTime_screen_DateTime_list_pos3,
};

GX_PROMPT * DateTime_pmpt_pos[4] = {
		&DateTime_screen.DateTime_screen_DateTime_prmpt_pos0,
		&DateTime_screen.DateTime_screen_DateTime_prmpt_pos1,
		&DateTime_screen.DateTime_screen_DateTime_prmpt_pos2,
		&DateTime_screen.DateTime_screen_DateTime_prmpt_pos3,
};

#ifdef using_keyboard
// declare a keyboard_key control block, derived from GX_PIXELMAP_BUTTON
typedef struct KEY_WIDGET_STRUCT {
    GX_PIXELMAP_BUTTON_MEMBERS_DECLARE
    GX_RESOURCE_ID icon;
    char *text;
    USHORT key_val;
} KEY_WIDGET;

// declare a structure to define one keyboard key 
typedef struct KEY_LAYOUT_ENTRY_STRUCT {
    GX_VALUE   xoffset;
    GX_VALUE   yoffset;
    GX_RESOURCE_ID normal_background;
    GX_RESOURCE_ID selected_background;
    GX_RESOURCE_ID icon;
    char      *text;
    USHORT     key_val;
} KEY_LAYOUT_ENTRY;

#define NUM_DEFAULT_KEYS  12//6
KEY_WIDGET key_control_blocks[NUM_DEFAULT_KEYS];

KEY_LAYOUT_ENTRY key_layout_1[NUM_DEFAULT_KEYS + 1] = {
    {  0,    80,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "1",     	'1'},
    { 80,    80,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "2",     	'2'}, 
    {160,    80,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "3",     	'3'}, 
    {240,    80,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "4",     	'4'}, 

    {  0,   130,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "5",     	'5'},
    { 80,   130,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "6",     	'6'}, 
    {160,   130,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "7",     	'7'}, 
    {240,   130,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "8",     	'8'}, 

    {  0,   180,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "Return", 	GX_KEY_RETURN_PREV_PAGE},
    { 80,   180,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "9",     	'9'}, 
    {160,   180,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "0",     	'0'},
    {240,   180,  GX_PIXELMAP_ID_B_KEYBOARD,  GX_PIXELMAP_ID_B_KEYBOARD_H,  0,   "Enter",  	GX_KEY_SELECT}, 

    {  0,     0,                          0,                            0,  0,   NULL,       0}
};

//-------------------------------------------------------------------------
static VOID  PopulateKeyboardButtons(GX_WINDOW *frame, KEY_LAYOUT_ENTRY *layout);
static VOID key_widget_draw(KEY_WIDGET *key);
static VOID key_widget_select(GX_WIDGET *widget);
static VOID key_widget_deselect(GX_WIDGET *key);
static void key_widget_create(KEY_WIDGET *key_widget, USHORT id, KEY_LAYOUT_ENTRY *entry, GX_WIDGET *frame);
#endif

//-------------------------------------------------------------------------
UINT show_window(GX_WINDOW * p_new, GX_WIDGET * p_widget, bool detach_old)
{
    UINT err = GX_SUCCESS;

    if (!p_new->gx_widget_parent)
    {
        err = gx_widget_attach(p_window_root, p_new);
    }
    else
    {
        err = gx_widget_show(p_new);
    }

    gx_system_focus_claim(p_new);

    GX_WIDGET * p_old = p_widget;
    if (p_old && detach_old)
    {
        if (p_old != (GX_WIDGET*)p_new)
        {
            gx_widget_detach(p_old);
        }
    }
		
    return err;
}

//-------------------------------------------------------------------------
// Init Screen Event Handler
UINT InitScreenEventHandler (GX_WINDOW * widget, GX_EVENT * event_ptr)
{
//    UINT status;


    switch (event_ptr->gx_event_type)
    {

        case Display_Information_Screen:
            show_window((GX_WINDOW*)&information_screen, (GX_WIDGET*)widget, true);
            page_information_screen_flag = 1;
            function_set_flag = 0;
            
            sprintf(itos_string, "V%u.%u.%u", prop_ver1, prop_ver2, prop_ver3);
  					gx_prompt_text_set(prop_ver_pmpt_text, itos_string); 
  					
            gx_prompt_text_set(hhp_ver_pmpt_text, version_string3);
            
        break;
           
        default:
            return gx_window_event_process(widget, event_ptr);
        
    }
    
    return 0;
    
}

//-------------------------------------------------------------------------
// Information Screen Event Handler
UINT InformationScreenEventHandler (GX_WINDOW * widget, GX_EVENT * event_ptr)
{
  	uint8_t i;
	#ifdef using_keyboard
		UINT err;
	#endif
	

    switch (event_ptr->gx_event_type)
    {

        case GX_SIGNAL(ID_SETTING_FUNCTION_BTN, GX_EVENT_CLICKED):
						if(LCD_off_flag == 1) {
        			LCD_ON();
        			break;
        		}
						LCD_ON();
						
						i = 0;  
  					while(m250_setup_mode(1) != 0) {  //Enter setting mode
  					  R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(10);  
  					  if(i > 3) break;
  					  i++;
  					}

						function_set_flag = 1;
						Shut_down_display_timeout2 = shut_down_timer2;
					
					#ifdef using_keyboard	
						err = show_window((GX_WINDOW*)&keyboard_screen, (GX_WIDGET*)widget, true);	//setting_screen
						//debug
						if(err != GX_SUCCESS) g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
					#endif
						
						page_information_screen_flag = 2;
					
					#ifdef using_keyboard	
						// populate buttons into keyboard frame 
   					PopulateKeyboardButtons( (GX_WINDOW*)&keyboard_screen, key_layout_1 );

   					if(populate_keyboard_flag == 0) gx_single_line_text_input_style_set(&keyboard_screen.keyboard_screen_keyboard_input_field, 
   		                                   					GX_STYLE_CURSOR_BLINK | GX_STYLE_CURSOR_ALWAYS);
						
						populate_keyboard_flag = 1;
					#else
						show_window((GX_WINDOW*)&popup_screen, (GX_WIDGET*)widget, true); 
  					gx_multi_line_text_view_text_set(popup_text_view, "Are you sure to go to setting mode?");
					#endif
							
        break;
    
				case GX_EVENT_PEN_DOWN:
						if(LCD_off_flag == 1) {
        			LCD_ON();
        		}
				break;
				
        default:
            return gx_window_event_process(widget, event_ptr);
        
    }
    
    return 0;
    
}

//-------------------------------------------------------------------------
#ifdef using_keyboard
static VOID key_widget_draw(KEY_WIDGET *key)
{
    INT x_offset = 0;
    INT y_offset = 1;
    GX_RESOURCE_ID font_id = GX_FONT_ID_MIDSIZE;
    GX_PIXELMAP *map;


  	gx_pixelmap_button_draw((GX_PIXELMAP_BUTTON *) key);	 
  	   
    if (key->text)
    {
        if (strlen(key->text) > 1)
        {
            font_id = GX_FONT_ID_DEFAULT;
        }
        if (key ->gx_widget_style & GX_STYLE_BUTTON_PUSHED)
        {
            x_offset++;
            y_offset++;
        }

					  gx_widget_text_draw(key, GX_COLOR_ID_WHITE,
            font_id, key->text,
            x_offset, y_offset);
   			
    }
    else
    {
        if (key->icon)
        {
            UINT err = gx_context_pixelmap_get(key->icon, &map);
						
						if(err != GX_SUCCESS) {
							g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
   					}
   		
            if (map)
            {
                x_offset = key->gx_widget_size.gx_rectangle_right - key->gx_widget_size.gx_rectangle_left + 1;
                x_offset -= map ->gx_pixelmap_width;
                x_offset /=2;
                x_offset += key->gx_widget_size.gx_rectangle_left;

                y_offset = key->gx_widget_size.gx_rectangle_bottom - key->gx_widget_size.gx_rectangle_top + 1;
                y_offset -= map ->gx_pixelmap_height;
                y_offset /=2;
                y_offset += key->gx_widget_size.gx_rectangle_top;
            
                err = gx_canvas_pixelmap_draw( (GX_VALUE)x_offset, (GX_VALUE)y_offset, map );
                
								if(err != GX_SUCCESS) {
									g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
   							}
   		
            }
        }
    }
}

//-------------------------------------------------------------------------
static VOID key_widget_select(GX_WIDGET *widget)
{
	GX_BUTTON *button = (GX_BUTTON *) widget;
	KEY_WIDGET *key = (KEY_WIDGET *) widget;
	GX_EVENT notification;
	

    // Set style for pen down. 
    button -> gx_widget_style |=  GX_STYLE_BUTTON_PUSHED;
    gx_widget_front_move(widget, NULL);
    
    // Mark as dirty.
    gx_system_dirty_mark(widget);

    // send notification to parent keyboard frame
    notification.gx_event_type = KEY_PRESS_EVENT;
    notification.gx_event_payload.gx_event_ushortdata[0] = key->key_val;
    notification.gx_event_target = 0;
    widget->gx_widget_parent->gx_widget_event_process_function(widget->gx_widget_parent, &notification);
}

//-------------------------------------------------------------------------
static VOID key_widget_deselect(GX_WIDGET *key)
{
    // Set style for pen down.  
    key -> gx_widget_style &=  ~GX_STYLE_BUTTON_PUSHED;
    
    // Mark as dirty. 
    gx_system_dirty_mark(key);
}

//-------------------------------------------------------------------------
static VOID key_widget_create(KEY_WIDGET *key_widget, USHORT id, KEY_LAYOUT_ENTRY *entry, GX_WIDGET *frame)
{
		UINT err;
    GX_RECTANGLE size;
    GX_VALUE left;
    GX_VALUE top;
    const GX_THEME *theme_ptr;
    GX_PIXELMAP *map;
    
    
    theme_ptr = main_display_theme_table[0];

		if(populate_keyboard_flag == 0) {  	
    	map = theme_ptr->theme_pixelmap_table[entry->selected_background];
    	left = (GX_VALUE)(frame->gx_widget_size.gx_rectangle_left + entry->xoffset);
    	top =  (GX_VALUE)(frame->gx_widget_size.gx_rectangle_top + entry->yoffset);
    	
 		
     gx_utility_rectangle_define( &size, left, top,
             (GX_VALUE)(left + map->gx_pixelmap_width - 1),
             (GX_VALUE)(top + map->gx_pixelmap_height - 1) );

    	// first create the base pixelmap button
    	err = gx_pixelmap_button_create((GX_PIXELMAP_BUTTON *) key_widget,
            NULL, frame,
            entry->normal_background,
            entry->selected_background,
            0, GX_STYLE_HALIGN_CENTER | GX_STYLE_VALIGN_CENTER | GX_STYLE_ENABLED,
            id, &size);
    
    	//debug
			if(err != GX_SUCCESS) {
				g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);
   		}
   		
		}
						   
    // now initialize the extended fields
    key_widget->icon = entry->icon;
    key_widget->text = entry->text;
    key_widget->key_val = entry->key_val;
    gx_widget_draw_set(key_widget, key_widget_draw);
    key_widget->gx_button_select_handler = key_widget_select;
    key_widget->gx_button_deselect_handler = key_widget_deselect;
}

//-------------------------------------------------------------------------
static VOID PopulateKeyboardButtons(GX_WINDOW *frame, KEY_LAYOUT_ENTRY *entry)
{
    INT index = 0;	
    //GX_BOOL moved_flag;
    //UINT err;
    
    while(entry ->key_val)
    {
        key_widget_create(&key_control_blocks[index], (USHORT)(index + 1), entry, (GX_WIDGET *) frame);
        entry++;
        index++;
    }
}

//-------------------------------------------------------------------------
UINT keyboard_frame_event_handler(GX_WINDOW *frame, GX_EVENT *event_ptr)
{
	GX_EVENT key_event;
	GX_CHAR * buffer_address;
	UINT string_size, buffer_size;
//	GX_CHAR buffer_string[21];
	int i;
//	uint8_t data_tmp, cfg_tmp;
	

    switch(event_ptr->gx_event_type)
    {
    		case KEY_PRESS_EVENT:
    			Shut_down_display_timeout2 = shut_down_timer2;
      	  // these events arrive from the key buttons. generate keyboard input
      	  //   events sent to the input field. The key value is held in the gx_event_ushortdata[0] field
      	  //
      	  key_event.gx_event_target = (GX_WIDGET *)&keyboard_screen.keyboard_screen_keyboard_input_field;
      	  key_event.gx_event_sender = frame ->gx_widget_id;
      	  key_event.gx_event_payload.gx_event_ushortdata[0] = event_ptr->gx_event_payload.gx_event_ushortdata[0];
      	  key_event.gx_event_type = GX_EVENT_KEY_DOWN;
      	  gx_system_event_send(&key_event);
      	  key_event.gx_event_type = GX_EVENT_KEY_UP;
      	  gx_system_event_send(&key_event);
      	  
      	  if( event_ptr->gx_event_payload.gx_event_ushortdata[0] == GX_KEY_SELECT ) {
      	  	//gx_prompt_text_set(second_pmpt_text, "Enter Password");
      	  	gx_single_line_text_input_buffer_get(my_text_input, &buffer_address, &string_size, &buffer_size);
      	  	
      	  	i = atoi (buffer_address);
      	 
      	  
      	  	if( i == 1234 || i == 1458 || i == 2390 ) {

      	  		gx_single_line_text_input_buffer_clear(my_text_input);
      	  	
      	  		//for ASL
      	  		show_window((GX_WINDOW*)&edit_screen, (GX_WIDGET*)frame, true); //(GX_WIDGET*)widget, true); 
  					  page_information_screen_flag = 19;
  					  gx_prompt_text_set(edit_pmpt_text, "Edit"); 
      	  		//for ASL end
      	  		
      	  	}
						else {
							gx_prompt_text_set(second_pmpt_text, "Re-enter Password");
							gx_single_line_text_input_buffer_clear(my_text_input);
						}
      	
      	  }
      	  else if( event_ptr->gx_event_payload.gx_event_ushortdata[0] == GX_KEY_RETURN_PREV_PAGE ) {
      	  	gx_prompt_text_set(second_pmpt_text, "Enter Password");
      	  	gx_single_line_text_input_buffer_clear(my_text_input);
      	  	
      	  	i = 0;  
  					while(m250_setup_mode(0) != 0) {  //Exit setting mode
  					  R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(10);  
  					  if(i > 3) break;
  					  i++;
  					}
  					
      	  	Shut_down_display_timeout = shut_down_timer1;
      	  	
      	  	show_window((GX_WINDOW*)&information_screen, (GX_WIDGET*)frame, true);
						page_information_screen_flag = 1;
						Shut_down_display_timeout = shut_down_timer1;
  					Shut_down_display_timeout2 = 0;
						function_set_flag = 0;
						
      	  }
        
        	break;

    		case Return_InforScr_Event:
      	  	gx_prompt_text_set(second_pmpt_text, "Enter Password");
      	  	gx_single_line_text_input_buffer_clear(my_text_input);
      	  	
      	  	i = 0;  
  					while(m250_setup_mode(0) != 0) {  //Exit setting mode
  					  R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);//delay_ms(10);  
  					  if(i > 3) break;
  					  i++;
  					}
  					
      	  	show_window((GX_WINDOW*)&information_screen, (GX_WIDGET*)frame, true);
						page_information_screen_flag = 1;
						function_set_flag = 0;
    				
        	break;

    	default:
        return gx_window_event_process(frame, event_ptr);
    }
    
    return 0;
    
}

//-------------------------------------------------------------------------
UINT input_field_event_process(GX_SINGLE_LINE_TEXT_INPUT *text_input, GX_EVENT *event_ptr)
{
    if (event_ptr ->gx_event_type == GX_SIGNAL(IDB_BACKSPACE, GX_EVENT_CLICKED))
    {
        gx_single_line_text_input_backspace(text_input);
        return 0;
    }

    return gx_single_line_text_input_event_process(text_input, event_ptr);
}
//-------------------------------------------------------------------------
#else
//-------------------------------------------------------------------------
UINT keyboard_frame_event_handler(GX_WINDOW *frame, GX_EVENT *event_ptr)
{
    switch(event_ptr->gx_event_type)
    {
    	default:
        return gx_window_event_process(frame, event_ptr);
    }
    
    return 0;
    
}

//-------------------------------------------------------------------------
UINT input_field_event_process(GX_SINGLE_LINE_TEXT_INPUT *text_input, GX_EVENT *event_ptr)
{
		(void)text_input;
		(void)event_ptr;
		
    return 0;
}
//-------------------------------------------------------------------------
#endif //end #ifdef using_keyboard
//-------------------------------------------------------------------------
// DateTime Screen Event Handler
UINT DateTimeScreenEventHandler (GX_WINDOW * widget, GX_EVENT * event_ptr)
{
	GX_VERTICAL_LIST    *list;
	int     row, i;
	GX_PIXELMAP_BUTTON *button1;
	GX_PIXELMAP_BUTTON *button2;
	
  					    					
    switch (event_ptr->gx_event_type)
    {

    		case GX_SIGNAL(ID_DATETIME_PGUP_BTN, GX_EVENT_CLICKED):
    				Shut_down_display_timeout2 = shut_down_timer2;
						
						date_time_page_num = 0;
						
        		gx_prompt_text_set(DateTime_list_pos[0], "Hour");
        		sprintf(text_temp[0], "%02x", rtcc_data[3]&0x1f);
      			gx_prompt_text_set(DateTime_pmpt_pos[0], text_temp[0]);
      			
        		gx_prompt_text_set(DateTime_list_pos[1], "Minute");
        		sprintf(text_temp[1], "%02x", rtcc_data[2]);
      			gx_prompt_text_set(DateTime_pmpt_pos[1], text_temp[1]);
      			
      			gx_prompt_text_set(DateTime_list_pos[2], "Second");
        		sprintf(text_temp[2], "%02x", rtcc_data[1]&0x7f);
      			gx_prompt_text_set(DateTime_pmpt_pos[2], text_temp[2]);
      					
  					gx_prompt_text_set(DateTime_list_pos[3], "AM/PM");
        		if( (rtcc_data[3]&0x20) == 0x20 ) gx_prompt_text_set(DateTime_pmpt_pos[3], "PM");
        		else gx_prompt_text_set(DateTime_pmpt_pos[3], "AM");
    				
    				date_time_disp_num = date_time_row_num;
    				
    				do_not_save = 0; 
  					gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
  					
  					button1 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgUp_button;
  					button2 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgDn_button;
  					
  					gx_pixelmap_button_pixelmap_set(button1,
																						GX_PIXELMAP_ID_PAGESTOP,
																						GX_PIXELMAP_ID_PAGESTOP,
																						GX_PIXELMAP_ID_PAGESTOP);

  					gx_pixelmap_button_pixelmap_set(button2,
																						GX_PIXELMAP_ID_PAGEDOWN,
																						GX_PIXELMAP_ID_PAGEDOWN,
																						GX_PIXELMAP_ID_PAGEDOWN);
          			
        	break;

    		case GX_SIGNAL(ID_DATETIME_PGDN_BTN, GX_EVENT_CLICKED):
    				Shut_down_display_timeout2 = shut_down_timer2;
						
						date_time_page_num = 1;
						
        		gx_prompt_text_set(DateTime_list_pos[0], "Year");
        		sprintf(text_temp[0], "%02x", rtcc_data[7]);
      			gx_prompt_text_set(DateTime_pmpt_pos[0], text_temp[0]);
      			
        		gx_prompt_text_set(DateTime_list_pos[1], "Month");
        		sprintf(text_temp[1], "%02x", rtcc_data[6]&0x1f);
      			gx_prompt_text_set(DateTime_pmpt_pos[1], text_temp[1]);
      			
      			gx_prompt_text_set(DateTime_list_pos[2], "Day");
        		sprintf(text_temp[2], "%02x", rtcc_data[5]&0x3f);
      			gx_prompt_text_set(DateTime_pmpt_pos[2], text_temp[2]);
      					
  					gx_prompt_text_set(DateTime_list_pos[3], "");
        		gx_prompt_text_set(DateTime_pmpt_pos[3], "");
    				
    				date_time_disp_num = (uint8_t)(date_time_row_num+4);
    				
    				do_not_save = 0; 
  					gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
  					      
  					button1 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgUp_button;
  					button2 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgDn_button;
  					
  					gx_pixelmap_button_pixelmap_set(button1,
																						GX_PIXELMAP_ID_PAGEUP,
																						GX_PIXELMAP_ID_PAGEUP,
																						GX_PIXELMAP_ID_PAGEUP);

  					gx_pixelmap_button_pixelmap_set(button2,
																						GX_PIXELMAP_ID_PAGESTOP,
																						GX_PIXELMAP_ID_PAGESTOP,
																						GX_PIXELMAP_ID_PAGESTOP);
          			
        	break;

				case GX_SIGNAL(ID_DATETIME_LIST, GX_EVENT_LIST_SELECT):
						Shut_down_display_timeout2 = shut_down_timer2;
						list = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_list;
						gx_vertical_list_selected_index_get(list, &row);
						
						date_time_disp_num = (uint8_t)(row+date_time_page_num*4);
						date_time_row_num = (uint8_t)row;
						
						for(i = 0; i < 4; i++) gx_prompt_text_color_set(DateTime_pmpt_pos[i], GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
						gx_prompt_text_color_set(DateTime_pmpt_pos[row], GX_COLOR_ID_BTN_BORDER, GX_COLOR_ID_BTN_BORDER);
						
						do_not_save = 0; 
  					gx_prompt_text_set(DateTime_pmpt_text, "DateTime"); 
  					      	
					break;

				case GX_SIGNAL(ID_DATETIME_SCREEN_R_BTN, GX_EVENT_CLICKED):
						Shut_down_display_timeout2 = shut_down_timer2;
						if(change_flag == 1 && do_not_save == 0) {
							gx_prompt_text_set(DateTime_pmpt_text, "havn't save!"); 
							do_not_save = 1;
						}
						else {
							list = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_list;
							gx_vertical_list_selected_set(list, 0);

  						button1 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgUp_button;
  						button2 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgDn_button;
  						
  						gx_pixelmap_button_pixelmap_set(button1,
																							GX_PIXELMAP_ID_PAGESTOP,
																							GX_PIXELMAP_ID_PAGESTOP,
																							GX_PIXELMAP_ID_PAGESTOP);
            	
  						gx_pixelmap_button_pixelmap_set(button2,
																							GX_PIXELMAP_ID_PAGEDOWN,
																							GX_PIXELMAP_ID_PAGEDOWN,
																							GX_PIXELMAP_ID_PAGEDOWN);

							show_window((GX_WINDOW*)&edit_screen, (GX_WIDGET*)widget, true);
							page_information_screen_flag = 3;
							
						}
					
					break;
           
    		case Return_SettingScr_Event:
							list = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_list;
							gx_vertical_list_selected_set(list, 0);

  						button1 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgUp_button;
  						button2 = &((DATETIME_SCREEN_CONTROL_BLOCK *)widget) -> DateTime_screen_DateTime_PgDn_button;
  						
  						gx_pixelmap_button_pixelmap_set(button1,
																							GX_PIXELMAP_ID_PAGESTOP,
																							GX_PIXELMAP_ID_PAGESTOP,
																							GX_PIXELMAP_ID_PAGESTOP);
            	
  						gx_pixelmap_button_pixelmap_set(button2,
																							GX_PIXELMAP_ID_PAGEDOWN,
																							GX_PIXELMAP_ID_PAGEDOWN,
																							GX_PIXELMAP_ID_PAGEDOWN);

							show_window((GX_WINDOW*)&edit_screen, (GX_WIDGET*)widget, true);
							page_information_screen_flag = 3;
    				
        	break;

				case GX_SIGNAL(ID_DATETIME_SAVE, GX_EVENT_CLICKED):
						Shut_down_display_timeout2 = shut_down_timer2;
						if(change_flag == 1) {

              change_flag = 0; 
              gx_prompt_text_set(DateTime_pmpt_text, "Ok!");

            }
            else gx_prompt_text_set(DateTime_pmpt_text, "No change!");

					break;
					
        default:
        	return gx_window_event_process(widget, event_ptr);
        
    }
    
    return 0;
    
}

//-------------------------------------------------------------------------
// Edit Screen Event Handler
UINT EditScreenEventHandler (GX_WINDOW * widget, GX_EVENT * event_ptr)
{
  					    					
    switch (event_ptr->gx_event_type)
    {

				case GX_SIGNAL(ID_EDIT_SCREEN_R_BTN, GX_EVENT_CLICKED):
						Shut_down_display_timeout2 = shut_down_timer2;
								
						//for ASL
						show_window((GX_WINDOW*)&information_screen, (GX_WIDGET*)widget, true);
						page_information_screen_flag = 1;
						function_set_flag = 0;
						//for ASL end
					
					break;
           
    		case Return_SettingScr_Event:
						show_window((GX_WINDOW*)&information_screen, (GX_WIDGET*)widget, true);
						page_information_screen_flag = 1;
					//	function_set_flag = 0;
    				
        	break;
 
        default:
            return gx_window_event_process(widget, event_ptr);
        
    }
    
    return 0;
    
}

//-------------------------------------------------------------------------
