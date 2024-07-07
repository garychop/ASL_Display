//*****************************************************************************
// Filename: ION_SNP_Calibrate_Screen.cpp
// Description: This file supports the ION Programming Main Screen.
//
// Date: Oct 24, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "DeviceInfo.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Defines, Macros
//*************************************************************************************

// The first two steps represent the Puff and Sip thresholds for the SNP Head Array
typedef enum {SNP_SIP = 0, SNP_PUFF, SNP_SOFT_SIP, SNP_HARD_SIP, SNP_SOFT_PUFF, SNP_HARD_PUFF, SNP_CALIBRATE_END} SNP_STEP_NUM;

//*************************************************************************************
// Local Var's
//*************************************************************************************

int8_t g_SNP_Nozzle_Value;
int g_SNP_Calibrate_Value;
char g_SNP_Calibrate_Value_Str[16];
char g_SNP_Slider_Value_Str[16];

SNP_STEP_NUM g_SNP_Calibration_Step;

/**************************************************************************************
 * external declarations.
 */
extern bool g_SNP_CalibrationIsActive;  // Used to coordinate getting SNP Nozzle data.

//*************************************************************************************
// Forward declarations
//*************************************************************************************

void SetupCalibrationStep (SNP_STEP_NUM calStep);
void PositionPointers(DEVICE_INFO_STRUCT* device, ION_SNP_CALIBRATE_SCREEN_CONTROL_BLOCK* SNP_Window);
void DisplayCalibrationValue(void);
static void SendCalibrationThresholdsToHub (void);

//*************************************************************************************
//
//*************************************************************************************

UINT SNP_Calibrate_Slider_event_function(GX_PIXELMAP_SLIDER *widget, GX_EVENT *event_ptr)
{
//	int currentValue;
//
    gx_pixelmap_slider_event_process(widget, event_ptr);
//
//	switch (event_ptr->gx_event_type)
//	{
//	case GX_EVENT_SHOW:
//	case GX_EVENT_PEN_DOWN:
//	case GX_EVENT_PEN_DRAG:
//	case GX_EVENT_PEN_UP:
//		currentValue = widget->gx_slider_info.gx_slider_info_current_val;
//		g_SNP_Calibrate_Value = currentValue;
//		break;
//	default:
//		break;
//	} // end swtich


	return GX_SUCCESS;
}

//*************************************************************************************
// This function puts the SNP Nub along the slider based upon the current SNP pressure/vacuum
//*************************************************************************************

VOID SNP_Calibrate_Slider_Draw_Function (GX_PIXELMAP_SLIDER *slider)
{
	//int myVal, totalSteps;
	//int increment;

	//totalSteps = slider->gx_slider_info.gx_slider_info_max_val - slider->gx_slider_info.gx_slider_info_min_val;

	//increment = 128 / totalSteps / 2;	// This is how many going Plus and Minus.
	//myVal = g_SNP_Calibrate_Value / increment;

	slider->gx_slider_info.gx_slider_info_current_val = g_SNP_Nozzle_Value;
	sprintf (g_SNP_Slider_Value_Str, "%2d", g_SNP_Nozzle_Value);
	gx_prompt_text_set ((GX_PROMPT*) slider->gx_widget_first_child, g_SNP_Slider_Value_Str);
	gx_pixelmap_slider_draw (slider);
	
}

//*************************************************************************************
// This function locates the 4 Pointers
//*************************************************************************************

//void PositionPointers(DEVICE_TYPE_STRUCT* device, ION_SNP_CALIBRATE_SCREEN_CONTROL_BLOCK* SNP_Window)
void PositionPointers(DEVICE_INFO_STRUCT* device, ION_SNP_CALIBRATE_SCREEN_CONTROL_BLOCK* SNP_Window)
{
	GX_RECTANGLE sliderRectangle;
	int sliderHeight;
	float sliderIncrement;
	int threshold;
	float thresholdPixels_f;
	int thresholdPixels_i;
	GX_RECTANGLE pointerRectangle;
	int pointerHeight;

	sliderRectangle = SNP_Window->ION_SNP_Calibrate_Screen_SNP_Calibrate_Slider.gx_widget_size;
	sliderHeight = sliderRectangle.gx_rectangle_bottom - sliderRectangle.gx_rectangle_top;
	sliderIncrement = (float)sliderHeight / 200.0f;

	// Position the Sof Sip pointer based upon the Right Pad threshold value.
	if (device->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
	{
		gx_widget_hide((GX_WIDGET*)&SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftSip_Icon);
	}
	else
	{
		gx_widget_show((GX_WIDGET*)&SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftSip_Icon);
		// Position Soft Sip
		threshold = device->m_PadInfo[RIGHT_PAD].m_SNP_Threshold;
		// convert from -100 to 100 => 0 to sliderHeight
		thresholdPixels_f = (float)(threshold + 100) * sliderIncrement;
		thresholdPixels_i = sliderHeight - (int)thresholdPixels_f;
		// Adjust Pointer position
		pointerRectangle = SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftSip_Icon.gx_widget_size;
		pointerHeight = pointerRectangle.gx_rectangle_bottom - pointerRectangle.gx_rectangle_top;
		// Set pointer's new coordinates
		pointerRectangle.gx_rectangle_top = (GX_VALUE) (sliderRectangle.gx_rectangle_top + thresholdPixels_i - (pointerHeight / 2));
		pointerRectangle.gx_rectangle_bottom = (GX_VALUE) (pointerRectangle.gx_rectangle_top + pointerHeight);
		gx_widget_resize((GX_WIDGET*)&SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftSip_Icon, &pointerRectangle);
	}

	// Position Hard Sip or SIP for SNP Head Array
	if (device->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
	{
		gx_icon_pixelmap_set(&SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardSip_Icon, GX_PIXELMAP_ID_SNP_SIP_POINTER, GX_PIXELMAP_ID_SNP_SIP_POINTER);
	}
	else
	{
		gx_icon_pixelmap_set(&SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardSip_Icon, GX_PIXELMAP_ID_SNP_HARDSIP_POINTER, GX_PIXELMAP_ID_SNP_HARDSIP_POINTER);
	}
	threshold = device->m_PadInfo[REVERSE_PAD].m_SNP_Threshold;
	// convert from -100 to 100 => 0 to sliderHeight
	thresholdPixels_f = (float)(threshold + 100) * sliderIncrement;
	thresholdPixels_i = sliderHeight - (int) thresholdPixels_f;
	// Adjust Pointer position
	pointerRectangle = SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardSip_Icon.gx_widget_size;
	pointerHeight = pointerRectangle.gx_rectangle_bottom - pointerRectangle.gx_rectangle_top;
	// Set pointer's new coordinates
	pointerRectangle.gx_rectangle_top = (GX_VALUE) (sliderRectangle.gx_rectangle_top + thresholdPixels_i - (pointerHeight / 2));
	pointerRectangle.gx_rectangle_bottom = (GX_VALUE) (pointerRectangle.gx_rectangle_top + pointerHeight);
	gx_widget_resize ((GX_WIDGET*) &SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardSip_Icon, &pointerRectangle);

	// Position Soft Puff pointer based upon the Left Pad threshold
	if (device->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
	{
		gx_widget_hide((GX_WIDGET*)&SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftPuff_Icon);
	}
	else
	{
		gx_widget_show((GX_WIDGET*)&SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftPuff_Icon);
		threshold = device->m_PadInfo[LEFT_PAD].m_SNP_Threshold;
		// convert from -100 to 100 => 0 to sliderHeight
		thresholdPixels_f = (float)(threshold + 100) * sliderIncrement;
		thresholdPixels_i = sliderHeight - (int)thresholdPixels_f;
		// Adjust Pointer position
		pointerRectangle = SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftPuff_Icon.gx_widget_size;
		pointerHeight = pointerRectangle.gx_rectangle_bottom - pointerRectangle.gx_rectangle_top;
		// Set pointer's new coordinates
		pointerRectangle.gx_rectangle_top = (GX_VALUE) (sliderRectangle.gx_rectangle_top + thresholdPixels_i - (pointerHeight / 2));
		pointerRectangle.gx_rectangle_bottom = (GX_VALUE) (pointerRectangle.gx_rectangle_top + pointerHeight);
		gx_widget_resize((GX_WIDGET*)&SNP_Window->ION_SNP_Calibrate_Screen_SNP_SoftPuff_Icon, &pointerRectangle);
	}

	// Position Hard Puff
	if (device->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
	{
		gx_icon_pixelmap_set(&SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardPuff_Icon, GX_PIXELMAP_ID_SNP_PUFF_POINTER, GX_PIXELMAP_ID_SNP_PUFF_POINTER);
	}
	else
	{
		gx_icon_pixelmap_set(&SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardPuff_Icon, GX_PIXELMAP_ID_SNP_HARDPUFF_POINTER, GX_PIXELMAP_ID_SNP_HARDPUFF_POINTER);
	}
	threshold = device->m_PadInfo[CENTER_PAD].m_SNP_Threshold;
	// convert from -100 to 100 => 0 to sliderHeight
	thresholdPixels_f = (float)(threshold + 100) * sliderIncrement;
	thresholdPixels_i = sliderHeight - (int) thresholdPixels_f;
	// Adjust Pointer position
	pointerRectangle = SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardPuff_Icon.gx_widget_size;
	pointerHeight = pointerRectangle.gx_rectangle_bottom - pointerRectangle.gx_rectangle_top;
	// Set pointer's new coordinates
	pointerRectangle.gx_rectangle_top = (GX_VALUE) (sliderRectangle.gx_rectangle_top + thresholdPixels_i - (pointerHeight / 2));
	pointerRectangle.gx_rectangle_bottom = (GX_VALUE) (pointerRectangle.gx_rectangle_top + pointerHeight);
	gx_widget_resize ((GX_WIDGET*) &SNP_Window->ION_SNP_Calibrate_Screen_SNP_HardPuff_Icon, &pointerRectangle);
}

//*************************************************************************************
//
//*************************************************************************************

void SetupCalibrationStep (SNP_STEP_NUM calStep)
{
	g_SNP_Calibration_Step = calStep;

	switch (g_SNP_Calibration_Step)
	{
	case SNP_SIP:	// SNP Head Array
		gx_prompt_text_id_set(&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_SNP_SET_SIP);
		gx_prompt_text_color_set(&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_YELLOW, GX_COLOR_ID_YELLOW); // , GX_COLOR_ID_YELLOW);
		break;
	case SNP_PUFF:	// SNP Head Array
		gx_prompt_text_id_set(&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_SNP_SET_PUFF);
		gx_prompt_text_color_set(&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_BRIGHT_ORANGE, GX_COLOR_ID_BRIGHT_ORANGE);
		break;
	case SNP_SOFT_SIP:
		gx_prompt_text_id_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_SNP_SET_SOFT_SIP);
		gx_prompt_text_color_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_YELLOW, GX_COLOR_ID_YELLOW);
		break;
	case SNP_HARD_SIP:
		gx_prompt_text_id_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_SNP_SET_HARD_SIP);
		gx_prompt_text_color_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_BRIGHT_ORANGE, GX_COLOR_ID_BRIGHT_ORANGE);
		break;
	case SNP_SOFT_PUFF:
		gx_prompt_text_id_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_SNP_SET_SOFT_PUFF);
		gx_prompt_text_color_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_YELLOW, GX_COLOR_ID_YELLOW);
		break;
	case SNP_HARD_PUFF:
		gx_prompt_text_id_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_SNP_SET_HARD_PUFF);
		gx_prompt_text_color_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_BRIGHT_ORANGE, GX_COLOR_ID_BRIGHT_ORANGE);
		break;
	default:
		gx_prompt_text_id_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_STRING_ID_BLANK);
		gx_prompt_text_color_set (&ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Calibration_Prompt, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
		strcpy (g_SNP_Calibrate_Value_Str, "--");
		gx_prompt_text_set ((GX_PROMPT*) &ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Value_Prompt, g_SNP_Calibrate_Value_Str);
		break;
	} // end switch
}

//*************************************************************************************
// This function displays the appropriate calibration values based upon the 
// calibration step
//*************************************************************************************

void DisplayCalibrationValue()
{
	switch (g_SNP_Calibration_Step)
	{
	case SNP_SOFT_SIP:
		sprintf (g_SNP_Calibrate_Value_Str, "%2d", gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_SNP_Threshold);
		break;
	case SNP_HARD_SIP:
	case SNP_SIP:		// SNP Head Array 
		sprintf (g_SNP_Calibrate_Value_Str, "%2d", gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_SNP_Threshold);
		break;
	case SNP_SOFT_PUFF:
		sprintf (g_SNP_Calibrate_Value_Str, "%2d", gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_SNP_Threshold);
		break;
	case SNP_HARD_PUFF:
	case SNP_PUFF:		// SNP Head Array
		sprintf (g_SNP_Calibrate_Value_Str, "%2d", gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_SNP_Threshold);
		break;
	default:
		strcpy (g_SNP_Calibrate_Value_Str, "--");
		break;
	} // end switch
	gx_prompt_text_set ((GX_PROMPT*) &ION_SNP_Calibrate_Screen.ION_SNP_Calibrate_Screen_SNP_Value_Prompt, g_SNP_Calibrate_Value_Str);
}

/******************************************************************************
 * This function sends the Calibration Data to the ION HUB
 */

static void SendCalibrationThresholdsToHub (void)
{
//    void SendSNPThresholdSet (DEVICE_NUMBER_ENUM device, int8_t soft_sip, int8_t hard_sip, int8_t soft_puff, int8_t hard_puff)
    SendSNPThresholdSet (gp_ProgrammingDevice->m_DriverConfiguration,
                         gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_SNP_Threshold,    // soft_sip
                         gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_SNP_Threshold,  // hard sip
                         gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_SNP_Threshold,     // soft puff
                         gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_SNP_Threshold);  // hard puff
}


//*************************************************************************************
// Event processing
//*************************************************************************************

UINT ION_SNP_Calibrate_Screen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    ION_SNP_CALIBRATE_SCREEN_CONTROL_BLOCK *SNP_Window = (ION_SNP_CALIBRATE_SCREEN_CONTROL_BLOCK *)window;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
		if (gp_ProgrammingDevice->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
		{
			SetupCalibrationStep(SNP_SIP);
		}
		else
		{
			SetupCalibrationStep(SNP_SOFT_SIP);
		}
		g_SNP_CalibrationIsActive = true;   // This tells the COMM task to periodically request SNP nozzle data.
        Send_DiagnosticCommand (1); // "1" disables commands to the Wheelchair
		DisplayCalibrationValue();
		PositionPointers (gp_ProgrammingDevice, SNP_Window);
		break;

	case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
		if (gp_ProgrammingDevice->m_DriverConfiguration == SNP_HEAD_ARRAY_DEVICE_IDX)
		{
			if (g_SNP_Calibration_Step == SNP_SIP)
			{
				g_SNP_Calibration_Step = SNP_PUFF;
				SetupCalibrationStep(SNP_PUFF);
			}
			else
			{
	            Send_DiagnosticCommand (0); // "0" enables commands to the Wheelchair
	            g_SNP_CalibrationIsActive = false;   // This tells the COMM task to NOT request SNP nozzle data.
	            SendCalibrationThresholdsToHub ();
				screen_toggle(PopPushedWindow(), window);
			}
		}
		else // Must be Sip-N-Puff device
		{
			if (++g_SNP_Calibration_Step >= SNP_CALIBRATE_END)
			{
                Send_DiagnosticCommand (0); // "0" enables commands to the Wheelchair
                g_SNP_CalibrationIsActive = false;   // This tells the COMM task to NOT request SNP nozzle data.
                SendCalibrationThresholdsToHub ();
				screen_toggle(PopPushedWindow(), window);
			}
			else
			{
				SetupCalibrationStep(g_SNP_Calibration_Step);
			}
		}
		DisplayCalibrationValue();
		PositionPointers (gp_ProgrammingDevice, SNP_Window);
		break;

	case GX_SIGNAL (PLUS_BTN_ID, GX_EVENT_CLICKED):
		switch (g_SNP_Calibration_Step)
		{
		case SNP_SOFT_SIP:
			if (gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_SNP_Threshold < -5)
				++gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_SNP_Threshold;
			break;
		case SNP_HARD_SIP:
		case SNP_SIP:
			if (gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_SNP_Threshold < -20)
				++gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_SNP_Threshold;
			break;
		case SNP_SOFT_PUFF:
			if (gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_SNP_Threshold < 50)
				++gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_SNP_Threshold;
			break;
		case SNP_HARD_PUFF:
		case SNP_PUFF:
			if (gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_SNP_Threshold < 99)
				++gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_SNP_Threshold;
			break;
		default:
			break;
		}
		PositionPointers (gp_ProgrammingDevice, SNP_Window);
		DisplayCalibrationValue();
		break;

	case GX_SIGNAL (MINUS_BTN_ID, GX_EVENT_CLICKED):
		switch (g_SNP_Calibration_Step)
		{
		case SNP_SOFT_SIP:
			if (gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_SNP_Threshold > -50)
				--gp_ProgrammingDevice->m_PadInfo[RIGHT_PAD].m_SNP_Threshold;
			break;
		case SNP_HARD_SIP:
		case SNP_SIP:
			if (gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_SNP_Threshold > -99)
				--gp_ProgrammingDevice->m_PadInfo[REVERSE_PAD].m_SNP_Threshold;
			break;
		case SNP_SOFT_PUFF:
			if (gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_SNP_Threshold > 5)
				--gp_ProgrammingDevice->m_PadInfo[LEFT_PAD].m_SNP_Threshold;
			break;
		case SNP_HARD_PUFF:
		case SNP_PUFF:
			if (gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_SNP_Threshold > 20)
				--gp_ProgrammingDevice->m_PadInfo[CENTER_PAD].m_SNP_Threshold;
			break;
		default:
			break;
		}
		PositionPointers (gp_ProgrammingDevice, SNP_Window);
		DisplayCalibrationValue();
		break;
	}

    gx_window_event_process(window, event_ptr);

	return GX_SUCCESS;
}

