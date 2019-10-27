/***********************************************************************************************************************
 * Copyright [2015] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * Unless otherwise expressly agreed in writing between Renesas and you: 1) you may not use, copy, modify, distribute,
 * display, or perform the contents; 2) you may not use any name or mark of Renesas for advertising or publicity
 * purposes or in connection with your use of the contents; 3) RENESAS MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE
 * SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF CONTRACT OR TORT, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents included in this file may
 * be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : lcd_setup.c
 * Description  : Definition for the SK-S7 LCD Panel setup function through SPI interface
***********************************************************************************************************************/

#include "my_gui_thread.h"
#include "lcd.h"
#include "my_gui_thread_entry.h"


static void lcd_write(uint8_t cmd, uint8_t * data ,uint32_t len);

static void lcd_write(uint8_t cmd, uint8_t * data ,uint32_t len)
{
    ssp_err_t err;

    g_ioport_on_ioport.pinWrite(LCD_CMD,IOPORT_LEVEL_LOW);
    g_ioport_on_ioport.pinWrite(LCD_CS,IOPORT_LEVEL_LOW);

    err = g_rspi_lcdc.p_api->write(g_rspi_lcdc.p_ctrl, &cmd, 1, SPI_BIT_WIDTH_8_BITS);
    if (SSP_SUCCESS != err)
    {
        g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);//while(1);
    }

    tx_semaphore_get(&g_my_gui_semaphore,TX_WAIT_FOREVER);

    if (len)
    {
        g_ioport_on_ioport.pinWrite(LCD_CMD,IOPORT_LEVEL_HIGH);

        err = g_rspi_lcdc.p_api->write(g_rspi_lcdc.p_ctrl, data, len,SPI_BIT_WIDTH_8_BITS);
        if (SSP_SUCCESS != err)
        {
            g_ioport.p_api->pinWrite(GRNLED, IOPORT_LEVEL_LOW);//while(1);
        }

        tx_semaphore_get(&g_my_gui_semaphore,TX_WAIT_FOREVER);
    }
    g_ioport_on_ioport.pinWrite(LCD_CS,IOPORT_LEVEL_HIGH);

}

void ILI9341V_Init(void)
{
    uint8_t data[8];
    int i;

    g_ioport_on_ioport.pinWrite(LCD_CS,IOPORT_LEVEL_HIGH);
    g_ioport_on_ioport.pinWrite(LCD_RESET,IOPORT_LEVEL_HIGH);
    g_ioport_on_ioport.pinWrite(LCD_RESET,IOPORT_LEVEL_LOW);
    tx_thread_sleep(1);
    g_ioport_on_ioport.pinWrite(LCD_RESET,IOPORT_LEVEL_HIGH);

    lcd_write(ILI9341_SW_RESET, (uint8_t *)"\x0", 0);
    tx_thread_sleep(1);
    for (i=0; i<4; i++)
    {
        data[0] = (uint8_t) (0x10 + i);
        lcd_write(0xD9,data,1);
    }

    lcd_write(ILI9341_MAC,          (uint8_t *) "\x00", 1);
    
    lcd_write(ILI9341_DISP_INV_ON, (uint8_t *) "\x0", 0);
    
    lcd_write(ILI9341_PIXEL_FORMAT, (uint8_t *) "\x55", 1); 
    
    lcd_write(ST7789V_RAM_CTRL,     (uint8_t *) "\x11\xC0", 2); 
    lcd_write(ST7789V_RGB_INTERFACE,(uint8_t *) "\x42\x02\x14", 3);
    
    lcd_write(ILI9341_COLUMN_ADDR,  (uint8_t *) "\x00\x00\x00\xEF", 4);
    lcd_write(ILI9341_PAGE_ADDR,    (uint8_t *) "\x00\x00\x01\x3F", 4);
    lcd_write(ILI9341_GAMMA,        (uint8_t *) "\x01", 1);
		
    lcd_write(ILI9341_SLEEP_OUT,    (uint8_t *) "\x00", 1);
    tx_thread_sleep(2);
    lcd_write(ILI9341_DISP_ON,      (uint8_t *) "\x00", 1);
}
