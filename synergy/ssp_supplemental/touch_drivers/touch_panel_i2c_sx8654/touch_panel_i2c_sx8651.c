/***********************************************************************************************************************
 * Copyright [2015] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : touch_panel_i2c_sx8651.c
 * Description  : I2C touch panel framework chip specific implementation for the SX8651 touch panel chip.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_touch_panel_i2c.h"

/*******************************************************************************************************************//**
 * @ingroup SF_TOUCH_PANEL_I2C
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @}
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** Macro for error logger. */
#ifndef SF_TOUCH_PANEL_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_TOUCH_PANEL_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_version)
#endif

#define extract_x(t) ((int16_t) (((t).x_msb << 8) | ((t).x_lsb)))
#define extract_y(t) ((int16_t) (((t).y_msb << 8) | ((t).y_lsb)))

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Driver-specific touch point register mapping */
typedef struct st_SX8651_touch
{
    uint8_t  x_msb  : 4;
    uint8_t  x_chan : 3;
    uint8_t         : 1;
    uint8_t  x_lsb;

    uint8_t  y_msb  : 4;
    uint8_t  y_chan : 3;
    uint8_t         : 1;
    uint8_t  y_lsb  : 8;
} SX8651_touch_t;

/* Touch controller register addresses */
#define SX8651_REGTOUCH0    (0x00)
#define SX8651_REGCHANMSK   (0x04)
//#define SX8651_REGPROX0     (0x0B)
//#define SX8651_REGIRQMSK    (0x22)
//#define SX8651_REGIRQSRC    (0x23)

/* Commands */
#define SX8651_CMD_PENTRG   (0xe0)
#define SX8651_CMD_READ_REG (0x40)

/* Bits for RegTouch0 (Address 0x00) */
#define SX8651_REG_TOUCH0_TOUCHRATE_200CPS  (0x7 << 4)
#define SX8651_REG_TOUCH0_POWDLY_8_9US      (0x4 << 0)

/* Pen detection circuit's pull-up resistor value */
#define SX8651_REG_TOUCH1_RPNDT_114KOHM     (0x0 << 2)
#define SX8651_REG_TOUCH1_RPNDT_228KOHM     (0x1 << 2)
#define SX8651_REG_TOUCH1_RPNDT_57KOHM      (0x2 << 2)
#define SX8651_REG_TOUCH1_RPNDT_28KOHM      (0x3 << 2)

/* Bits for RegTouch1 (Address 0x01) */
#define SX8651_REG_TOUCH1_RESERVED                   (0x1 << 5)

/* Change this configuration to one of the resistor value, based on the panel used*/
#define SX8651_REG_TOUCH1_RPDNT_RESISTOR_VALUE_CFG   (SX8651_REG_TOUCH1_RPNDT_228KOHM)
#define SX8651_REG_TOUCH1_FILT_NFILT3                (0x3 << 0)

/* Bits for RegTouch2 (Address 0x02) */
#define SX8651_REG_TOUCH2_SETDLY_8_9US      (0x4 << 0)

/* Bits for RegChanMsk (Address 0x4) */
#define SX8651_REGCHANMSK_XCONV             (0x1 << 7)
#define SX8651_REGCHANMSK_YCONV             (0x1 << 6)

/* Bits for RegProx0 (Address 0x0B) */
//#define SX8651_REGPROX0_PROXSCANPERIOD_OFF  (0x0 << 0)

/* Bits for RegIrqMsk (Address 0x22) */
//#define SX8651_REGIRQMSK_PENRELEASE         (0x1 << 2)
//#define SX8651_REGIRQMSK_PENTOUCH_TOUCHCONVDONE (0x1 << 3)

/* Bits for RegIrqSrc (Address 0x23) */
//#define SX8651_REGIRQSRC_PENRELEASEIRQ      (0x1 << 2)

/* ADC max output value (12-bit resolution) */
#define SX8651_MAX_ADC_OUTPUT_VALUE         (4095L)

/* I2C communication retry times */
#define SX8651_I2C_RETRY_TIMES              (10)

#define TOUCH_THRESHOLD (2)

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

static ssp_err_t SX8651_payload_get (sf_touch_panel_ctrl_t * const    p_ctrl,
                                     sf_touch_panel_payload_t * const p_payload);

static ssp_err_t SX8651_reset (sf_touch_panel_ctrl_t * const p_ctrl);

static ssp_err_t SX8651_i2c_read (i2c_api_master_t const * const p_i2c_api, i2c_ctrl_t * const p_i2c_ctrl,
                                      uint8_t * const p_dest, uint32_t const bytes);

static ssp_err_t SX8651_i2c_write (i2c_api_master_t const * const p_i2c_api, i2c_ctrl_t * const p_i2c_ctrl,
                                      uint8_t * const p_src, uint32_t const bytes, bool const restart );

//static ssp_err_t SX8651_i2c_write_followed_by_read (i2c_api_master_t const * const p_i2c_api,
//                                      i2c_ctrl_t * const p_i2c_ctrl, uint8_t * const p_data, uint32_t const bytes);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_touch_panel_i2c_SX8651";
#endif


const sf_touch_panel_i2c_chip_t g_sf_touch_panel_i2c_chip_sx8651 =
{
    .payloadGet = SX8651_payload_get,
    .reset      = SX8651_reset
};

/** Version data structure used by error logger macro. */
#if BSP_CFG_ERROR_LOG != 0
#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static const ssp_version_t g_version =
{
    .api_version_minor  = SF_TOUCH_PANEL_API_VERSION_MINOR,
    .api_version_major  = SF_TOUCH_PANEL_API_VERSION_MAJOR,
    .code_version_major = SF_TOUCH_PANEL_I2C_CODE_VERSION_MAJOR,
    .code_version_minor = SF_TOUCH_PANEL_I2C_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif
#endif


/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Reads data from the touch controller through the I2C-BUS interface.
 **********************************************************************************************************************/
static ssp_err_t SX8651_i2c_read (i2c_api_master_t const * const p_i2c_api, i2c_ctrl_t * const p_i2c_ctrl,
                                      uint8_t * const p_dest, uint32_t const bytes)
{
    ssp_err_t err;

    /** Performs I2C read operation. Retry some time if failed in the communication. */
    for (int i = 0; i < SX8651_I2C_RETRY_TIMES; i++)
    {
        err = p_i2c_api->read(p_i2c_ctrl, p_dest, bytes, false);
        if (SSP_SUCCESS == err)
        {
            break;
        }
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Writes data to the touch controller through the I2C-BUS interface.
 **********************************************************************************************************************/
static ssp_err_t SX8651_i2c_write (i2c_api_master_t const * const p_i2c_api, i2c_ctrl_t * const p_i2c_ctrl,
                                       uint8_t * const p_src, uint32_t const bytes, bool const restart)
{
    ssp_err_t err;

    /** Performs I2C write operation. Retry some time if failed in the communication. */
    for (int i = 0; i < SX8651_I2C_RETRY_TIMES; i++)
    {
        err = p_i2c_api->write(p_i2c_ctrl, p_src, bytes, restart);
        if (SSP_SUCCESS == err)
        {
            break;
        }
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Write data to and read data from the touch controller through the I2C-BUS interface.
 **********************************************************************************************************************/
/*
static ssp_err_t SX8651_i2c_write_followed_by_read (i2c_api_master_t const * const p_i2c_api,
                     i2c_ctrl_t * const p_i2c_ctrl, uint8_t * const p_data, uint32_t const bytes)
{
    ssp_err_t err;

    // Performs I2C write followed by read. Retry some time if failed in the communication. 
    for (int i = 0; i < SX8651_I2C_RETRY_TIMES; i++)
    {
        // Performs I2C write operation with requesting restart condition. 
        err = p_i2c_api->write(p_i2c_ctrl, p_data, bytes, true);
        if (SSP_SUCCESS == err)
        {
            // Performs I2C read operation. This starts from the restart condition. 
            err = p_i2c_api->read(p_i2c_ctrl, p_data, bytes, false);
            if (SSP_SUCCESS == err)
            {
                break;
            }
        }
    }
    return err;
}
*/

/*******************************************************************************************************************//**
 * @brief   Reads the touch event data from the touch controller.  Implements sf_touch_panel_i2c_chip_t::payloadGet.
 * @param[in,out] p_api_ctrl Pointer to a structure allocated by user. This control structure is initialized in
 *                           this function.
 * @param[out]    p_payload  Pointer to the payload data structure. Touch data provided should be processed to
 *                           logical pixel values.
**********************************************************************************************************************/

ssp_err_t SX8651_payload_get (sf_touch_panel_ctrl_t * const p_api_ctrl, sf_touch_panel_payload_t * const p_payload)
{
    sf_touch_panel_i2c_instance_ctrl_t * p_ctrl = (sf_touch_panel_i2c_instance_ctrl_t *) p_api_ctrl;

    i2c_ctrl_t                     * p_i2c_ctrl    = p_ctrl->p_lower_lvl_i2c->p_ctrl;
    sf_external_irq_ctrl_t         * p_irq_ctrl    = p_ctrl->p_lower_lvl_irq->p_ctrl;
    i2c_api_master_t         const * p_i2c_api     = p_ctrl->p_lower_lvl_i2c->p_api;
    sf_external_irq_api_t    const * p_irq_api     = p_ctrl->p_lower_lvl_irq->p_api;
    ssp_err_t err = SSP_SUCCESS;
    SX8651_touch_t touchData[1];

    //----------------------------------------------------------------------------------------------------
    // Every time we encounter a 'state' change, we exit this function otherwise, we're going to stay in this function.
    //
    // Process the initialize state or when a button push/release has been completed.

    if ((p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_NONE)
        || (p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_UP))
    {
        err = p_irq_api->wait(p_irq_ctrl, TX_WAIT_FOREVER);     // Wait for a touch
        if (SSP_SUCCESS != SSP_SUCCESS)                                 // Process the error
        {
            SF_TOUCH_PANEL_ERROR_RETURN(false, err);            // Log the error for some reason.
        }
        // Gets X/Y coordinate data
        err = SX8651_i2c_read(p_i2c_api, p_i2c_ctrl, (uint8_t *) &touchData[0], sizeof(touchData));
        if (SSP_SUCCESS != err)
        {
            p_i2c_api->reset(p_i2c_ctrl);
            SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
        }
        // Processes the raw data for the touch point(s) into useful data
        p_payload->x = (int16_t) (((int32_t) p_ctrl->hsize_pixels * (int32_t) extract_x(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);
        p_payload->y = (int16_t) (((int32_t) p_ctrl->vsize_pixels * (int32_t) extract_y(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);
        p_ctrl->last_payload.event_type = p_payload->event_type = SF_TOUCH_PANEL_EVENT_DOWN;
        p_ctrl->last_payload.x = p_payload->x;
        p_ctrl->last_payload.y = p_payload->y;
        return err;
    }

    //----------------------------------------------------------------------------------------------------
    // Process the "DOWN" state. We either change to "MOVE" or "HOLD" based upon the current x/y position.
    else if (p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_DOWN)
    {
        err = p_irq_api->wait(p_irq_ctrl, 11);                   // This returns if touching is still occurring.
        if (SSP_SUCCESS != err)                                 // Process the error
        {
            p_ctrl->last_payload.event_type = p_payload->event_type = SF_TOUCH_PANEL_EVENT_UP;
            p_payload->x = p_ctrl->last_payload.x;  /* SX8651 returns coordinates with the max value if PEN UP event happens. */
            p_payload->y = p_ctrl->last_payload.y;  /* Use the coordinates obtained at last PEN DOWN and do not save the one got this time. */
            return SSP_SUCCESS;
        }
        // Gets X/Y coordinate data
        err = SX8651_i2c_read(p_i2c_api, p_i2c_ctrl, (uint8_t *) &touchData[0], sizeof(touchData));
        if (SSP_SUCCESS != err)
        {
            p_i2c_api->reset(p_i2c_ctrl);
            SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
        }
        p_payload->x = (int16_t) (((int32_t) p_ctrl->hsize_pixels * (int32_t) extract_x(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);
        p_payload->y = (int16_t) (((int32_t) p_ctrl->vsize_pixels * (int32_t) extract_y(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);

        if ((abs (p_ctrl->last_payload.x - p_payload->x) < TOUCH_THRESHOLD) && (abs(p_ctrl->last_payload.y - p_payload->y) < TOUCH_THRESHOLD))
        {
            p_payload->event_type = SF_TOUCH_PANEL_EVENT_HOLD;
        }
        else
        {
            p_payload->event_type = SF_TOUCH_PANEL_EVENT_MOVE;
        }
        p_ctrl->last_payload.event_type = p_payload->event_type;
        p_ctrl->last_payload.x = p_payload->x;
        p_ctrl->last_payload.y = p_payload->y;
        return err;
    }

    //----------------------------------------------------------------------------------------------------
    // Process HOLD state. We either stay in this state if the the x/y positions have NOT changed or we
    // change to UP or MOVE.
    else if (p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_HOLD)
    {
        while (p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_HOLD)
        {
            err = p_irq_api->wait(p_irq_ctrl, 11);                   // This returns if touching is still occurring.
            if (SSP_SUCCESS != err)                                 // Process the error
            {
                p_ctrl->last_payload.event_type = p_payload->event_type = SF_TOUCH_PANEL_EVENT_UP;
                p_payload->x = p_ctrl->last_payload.x;  /* SX8651 returns coordinates with the max value if PEN UP event happens. */
                p_payload->y = p_ctrl->last_payload.y;  /* Use the coordinates obtained at last PEN DOWN and do not save the one got this time. */
                return SSP_SUCCESS;
            }
            // Gets X/Y coordinate data
            err = SX8651_i2c_read(p_i2c_api, p_i2c_ctrl, (uint8_t *) &touchData[0], sizeof(touchData));
            if (SSP_SUCCESS != err)
            {
                p_i2c_api->reset(p_i2c_ctrl);
                SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
            }
            p_payload->x = (int16_t) (((int32_t) p_ctrl->hsize_pixels * (int32_t) extract_x(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);
            p_payload->y = (int16_t) (((int32_t) p_ctrl->vsize_pixels * (int32_t) extract_y(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);

            if ((abs (p_ctrl->last_payload.x - p_payload->x) >= TOUCH_THRESHOLD) || (abs(p_ctrl->last_payload.y - p_payload->y) >= TOUCH_THRESHOLD))
            {
                p_ctrl->last_payload.event_type = p_payload->event_type = SF_TOUCH_PANEL_EVENT_DOWN;
            }
            tx_thread_sleep(2);     // This is needed to allow the GUI to process the command.
        } // end while "HOLD"
        tx_thread_sleep(2);     // This is needed to allow the GUI to process the command.
        p_ctrl->last_payload.x = p_payload->x;
        p_ctrl->last_payload.y = p_payload->y;
        return err;
    }

    //----------------------------------------------------------------------------------------------------
    // Process MOVE state. We either stay in this state if we are continuously moving or we change
    // to UP or HOLD.
    else if (p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_MOVE)
    {
        while (p_ctrl->last_payload.event_type == SF_TOUCH_PANEL_EVENT_MOVE)
        {
            err = p_irq_api->wait(p_irq_ctrl, 11);                   // This returns if touching is still occurring.
            if (SSP_SUCCESS != err)                                 // Process the error
            {
                p_ctrl->last_payload.event_type = p_payload->event_type = SF_TOUCH_PANEL_EVENT_UP;
                p_payload->x = p_ctrl->last_payload.x;  /* SX8651 returns coordinates with the max value if PEN UP event happens. */
                p_payload->y = p_ctrl->last_payload.y;  /* Use the coordinates obtained at last PEN DOWN and do not save the one got this time. */
                return SSP_SUCCESS;
            }
            // Gets X/Y coordinate data
            err = SX8651_i2c_read(p_i2c_api, p_i2c_ctrl, (uint8_t *) &touchData[0], sizeof(touchData));
            if (SSP_SUCCESS != err)
            {
                p_i2c_api->reset(p_i2c_ctrl);
                SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
            }
            p_payload->x = (int16_t) (((int32_t) p_ctrl->hsize_pixels * (int32_t) extract_x(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);
            p_payload->y = (int16_t) (((int32_t) p_ctrl->vsize_pixels * (int32_t) extract_y(touchData[0])) / SX8651_MAX_ADC_OUTPUT_VALUE);

            if ((abs (p_ctrl->last_payload.x - p_payload->x) < TOUCH_THRESHOLD) && (abs(p_ctrl->last_payload.y - p_payload->y) < TOUCH_THRESHOLD))
            {
                p_payload->event_type = SF_TOUCH_PANEL_EVENT_DOWN;      // We eventually move to HOLD, but the GUI seems to want a MOVE before a HOLD.
            }
            else
            {
                break;      // Send another "MOVE" to the system
            }
            tx_thread_sleep(2);     // This is needed to allow the GUI to process the command.
        } // end while "MOVE"
        p_ctrl->last_payload.event_type = p_payload->event_type;
        p_ctrl->last_payload.x = p_payload->x;
        p_ctrl->last_payload.y = p_payload->y;

        return err;
    }

    //----------------------------------------------------------------------------------------------------
    // Process "INVALID" state
    else    // First time through the event_type is SF_TOUCH_PANEL_EVENT_INVALID
    {
        p_i2c_api->reset(p_i2c_ctrl);
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Resets the touch chip.  Implements sf_touch_panel_i2c_chip_t::reset.
 *
 * @param[in]  p_api_ctrl   Pointer to control block from touch panel framework.
 **********************************************************************************************************************/
static ssp_err_t SX8651_reset (sf_touch_panel_ctrl_t * const p_api_ctrl)
{
    sf_touch_panel_i2c_instance_ctrl_t * p_ctrl = (sf_touch_panel_i2c_instance_ctrl_t *) p_api_ctrl;

    /* Parameter checking done in touch panel framework. */

    i2c_api_master_t const * const p_i2c_api       = p_ctrl->p_lower_lvl_i2c->p_api;
    i2c_ctrl_t * const             p_i2c_ctrl      = p_ctrl->p_lower_lvl_i2c->p_ctrl;
    uint8_t                        command[4];

    /** Resets touch chip by setting GPIO reset pin low. */
    g_ioport_on_ioport.pinWrite(p_ctrl->pin, IOPORT_LEVEL_LOW);

    /** Waits for a while (keep the reset signal low longer than 1ms) */
    tx_thread_sleep(2);

    /** Resets the I2C peripheral. */
    ssp_err_t err = p_i2c_api->reset(p_i2c_ctrl);

    /** Releases touch chip from reset */
    g_ioport_on_ioport.pinWrite(p_ctrl->pin, IOPORT_LEVEL_HIGH);

    /** Waits just for a while before accessing touch chip */
    tx_thread_sleep(2);

    /** Writes a complete configuration generated by the SX8651 evaluation software */
    command[0] = SX8651_REGTOUCH0;
    command[1] = SX8651_REG_TOUCH0_TOUCHRATE_200CPS|SX8651_REG_TOUCH0_POWDLY_8_9US;
    command[2] = SX8651_REG_TOUCH1_RESERVED|SX8651_REG_TOUCH1_RPDNT_RESISTOR_VALUE_CFG|SX8651_REG_TOUCH1_FILT_NFILT3;
    command[3] = SX8651_REG_TOUCH2_SETDLY_8_9US;
    err        = SX8651_i2c_write(p_i2c_api, p_i2c_ctrl, command, 4, false);
    if (SSP_SUCCESS != err)
    {
        p_i2c_api->reset(p_i2c_ctrl);
        SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
    }

    /** The generated configuration enables too many conversion channels, ensure only
     * channels X and Y are enabled
     */
    command[0] = SX8651_REGCHANMSK;
    command[1] = SX8651_REGCHANMSK_XCONV | SX8651_REGCHANMSK_YCONV;
    err        = SX8651_i2c_write(p_i2c_api, p_i2c_ctrl, command, 2, false);
    if (SSP_SUCCESS != err)
    {
        p_i2c_api->reset(p_i2c_ctrl);
        SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
    }

    /** Enables the PenTouch/TouchConvDone and PenRelease interrupts */
 	/*   
    command[0] = SX8651_REGIRQMSK;
    command[1] = SX8651_REGIRQMSK_PENTOUCH_TOUCHCONVDONE | SX8651_REGIRQMSK_PENRELEASE;
    err        = SX8651_i2c_write(p_i2c_api, p_i2c_ctrl, command, 2, false);
    if (SSP_SUCCESS != err)
    {
        p_i2c_api->reset(p_i2c_ctrl);
        SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
    }
	*/
    /** Defines the proximity scan period - Turn off proximity as we don't currently use it */
  /*  
    command[0] = SX8651_REGPROX0;
    command[1] = SX8651_REGPROX0_PROXSCANPERIOD_OFF;
    err        = SX8651_i2c_write(p_i2c_api, p_i2c_ctrl, command, 2, false);
    if (SSP_SUCCESS != err)
    {
        p_i2c_api->reset(p_i2c_ctrl);
        SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
    }
	*/
    /** Enables pen trigger mode */
    command[0] = SX8651_CMD_PENTRG;
    err        = SX8651_i2c_write(p_i2c_api, p_i2c_ctrl, command, 1, false);
    if (SSP_SUCCESS != err)
    {
        p_i2c_api->reset(p_i2c_ctrl);
        SF_TOUCH_PANEL_ERROR_RETURN(false, SSP_ERR_INTERNAL);
    }

    /** Initializes the last touch event info. */
    p_ctrl->last_payload.event_type = SF_TOUCH_PANEL_EVENT_NONE;

    return SSP_SUCCESS;
}
