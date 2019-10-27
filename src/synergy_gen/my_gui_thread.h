/* generated thread header file - do not edit */
#ifndef MY_GUI_THREAD_H_
#define MY_GUI_THREAD_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void my_gui_thread_entry(void);
#else
extern void my_gui_thread_entry(void);
#endif
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sci_spi.h"
#include "r_spi_api.h"
#include "r_riic.h"
#include "r_i2c_api.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_adc.h"
#include "r_adc_api.h"
#include "r_wdt.h"
#include "r_wdt_api.h"
#include "r_icu.h"
#include "r_external_irq_api.h"
#include "sf_external_irq.h"
#include "sf_touch_panel_i2c.h"
#include "sf_touch_panel_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer1;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
extern const spi_cfg_t g_rspi_lcdc_cfg;
/** SPI on SCI Instance. */
extern const spi_instance_t g_rspi_lcdc;
extern sci_spi_instance_ctrl_t g_rspi_lcdc_ctrl;
extern const sci_spi_extended_cfg g_rspi_lcdc_cfg_extend;

#ifndef g_lcd_spi_callback
void g_lcd_spi_callback(spi_callback_args_t *p_args);
#endif

#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer0)
#define g_rspi_lcdc_P_TRANSFER_TX (NULL)
#else
#define g_rspi_lcdc_P_TRANSFER_TX (&g_transfer0)
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer1)
#define g_rspi_lcdc_P_TRANSFER_RX (NULL)
#else
#define g_rspi_lcdc_P_TRANSFER_RX (&g_transfer1)
#endif
#undef SYNERGY_NOT_DEFINED

#define g_rspi_lcdc_P_EXTEND (&g_rspi_lcdc_cfg_extend)
extern const i2c_cfg_t g_i2c1_cfg;
/** I2C on RIIC Instance. */
extern const i2c_master_instance_t g_i2c1;
#ifndef NULL
void NULL(i2c_callback_args_t *p_args);
#endif

extern riic_instance_ctrl_t g_i2c1_ctrl;
extern const riic_extended_cfg g_i2c1_extend;
#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED)
#define g_i2c1_P_TRANSFER_TX (NULL)
#else
#define g_i2c1_P_TRANSFER_TX (&SYNERGY_NOT_DEFINED)
#endif
#if (SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED)
#define g_i2c1_P_TRANSFER_RX (NULL)
#else
#define g_i2c1_P_TRANSFER_RX (&SYNERGY_NOT_DEFINED)
#endif
#undef SYNERGY_NOT_DEFINED
#define g_i2c1_P_EXTEND (&g_i2c1_extend)
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer2;
#ifndef g_timer2_callback
void g_timer2_callback(timer_callback_args_t *p_args);
#endif
/** ADC on ADC Instance. */
extern const adc_instance_t g_adc0;
#ifndef NULL
void NULL(adc_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer1;
#ifndef g_timer1_callback
void g_timer1_callback(timer_callback_args_t *p_args);
#endif
/** WDT on WDT Instance. */
extern const wdt_instance_t g_wdt;
#ifndef NULL
void NULL(wdt_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer0;
#ifndef g_timer0_callback
void g_timer0_callback(timer_callback_args_t *p_args);
#endif
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t g_external_irq0;
#ifndef NULL
void NULL(external_irq_callback_args_t *p_args);
#endif
/** SF External IRQ on SF External IRQ Instance. */
extern const sf_external_irq_instance_t g_sf_external_irq0;
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer3;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer2;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
extern const i2c_cfg_t g_i2c0_cfg;
/** I2C on RIIC Instance. */
extern const i2c_master_instance_t g_i2c0;
#ifndef NULL
void NULL(i2c_callback_args_t *p_args);
#endif

extern riic_instance_ctrl_t g_i2c0_ctrl;
extern const riic_extended_cfg g_i2c0_extend;
#define SYNERGY_NOT_DEFINED (1)            
#if (SYNERGY_NOT_DEFINED == g_transfer2)
#define g_i2c0_P_TRANSFER_TX (NULL)
#else
#define g_i2c0_P_TRANSFER_TX (&g_transfer2)
#endif
#if (SYNERGY_NOT_DEFINED == g_transfer3)
#define g_i2c0_P_TRANSFER_RX (NULL)
#else
#define g_i2c0_P_TRANSFER_RX (&g_transfer3)
#endif
#undef SYNERGY_NOT_DEFINED
#define g_i2c0_P_EXTEND (&g_i2c0_extend)
/** SF Touch Panel on SF Touch Panel I2C Instance. */
extern const sf_touch_panel_instance_t g_sf_touch_panel_i2c0;
/** Messaging Framework Instance */
extern const sf_message_instance_t g_sf_message0;
void g_sf_touch_panel_i2c0_err_callback(void *p_instance, void *p_data);
void sf_touch_panel_i2c_init0(void);
extern TX_SEMAPHORE g_my_gui_semaphore;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* MY_GUI_THREAD_H_ */
