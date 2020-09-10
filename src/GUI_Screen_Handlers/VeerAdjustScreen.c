//*****************************************************************************
// Filename: VeerAdjustScreen.c
// Description: This file handles the Veer Adjustment screen.
//
// Date: Sept 9, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
//
//*****************************************************************************

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local Variables
//*************************************************************************************

char g_SliderValue[8] = "1234567";

//*************************************************************************************
// Function Name: VeerAdjust_Screen_event_handler and Slider_Draw_function
//
// Description: These handle the Veer Adjust Screen.
//
//*************************************************************************************

VOID VeerAdjust_Screen_draw_function (GX_WINDOW *window)
{
    gx_window_draw(window);
}

//*************************************************************************************

UINT VeerSlider_event_function(GX_PIXELMAP_SLIDER *widget, GX_EVENT *event_ptr)
{
    int16_t newVal;
    int16_t totalSteps;
    int16_t increment;
    int16_t sliderAsFound;
    int16_t minimumDAC;

    gx_pixelmap_slider_event_process(widget, event_ptr);    // This must be before the processing
                                                            // to allow the values to be stored in the widget.

    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_SHOW:
        break;
    case GX_EVENT_PEN_DOWN:
    case GX_EVENT_PEN_DRAG:
    case GX_EVENT_PEN_MOVE:
        sliderAsFound = (int16_t) widget->gx_slider_info.gx_slider_info_current_val;
        totalSteps = (int16_t)(widget->gx_slider_info.gx_slider_info_max_val - widget->gx_slider_info.gx_slider_info_min_val);
        increment = 5;      // This means that each slider step is worth 5 DAC counts.
        minimumDAC = (int16_t)(g_NeutralDAC_Constant - (increment * (totalSteps/2)));       // lowest value.
        newVal = (int16_t) (minimumDAC + (increment * sliderAsFound));
        // Send the new data to the Head Array, but only when the process of Pen Down, Drag and Up are done. No sense sending it multiple times.
        if (!g_WaitingForVeerResponse)
        {
            SendNeutralDAC_Set(newVal);
            SendNeutralDAC_GetCommand();
            g_WaitingForVeerResponse = true;
        }
        break;
    default:
        break;
    } // end switch

    return GX_SUCCESS;
}

//*************************************************************************************

VOID Slider_Draw_Function (GX_PIXELMAP_SLIDER *slider)
{
    int myVal, totalSteps; // offset, increment,
    int DACCountsPerStep, MinimumDAC_Counts, MaximumDAC_Counts;

    // We need to convert the target DAC Counts Setting to the number of Slider Steps.
    // The slider is 0-20 representing the extreme left to the extreme right of the slider.

    // Get the number of steps in the Screen's Slider widget.
    totalSteps = (int16_t)(slider->gx_slider_info.gx_slider_info_max_val - slider->gx_slider_info.gx_slider_info_min_val);
    // Get the number of DAC Counts between the new setting and the midpoint.
    DACCountsPerStep = 5;
    MinimumDAC_Counts = g_NeutralDAC_Constant - (DACCountsPerStep * (totalSteps/2));
    MaximumDAC_Counts = g_NeutralDAC_Constant + (DACCountsPerStep * (totalSteps/2));
    // Check for DAC Setting outside of displayable range and set to min or max slider position.
    if (g_NeutralDAC_Setting < MinimumDAC_Counts)
        myVal = slider->gx_slider_info.gx_slider_info_min_val;
    else if (g_NeutralDAC_Setting > MaximumDAC_Counts)
        myVal = slider->gx_slider_info.gx_slider_info_max_val;
    else
        myVal = (g_NeutralDAC_Setting - MinimumDAC_Counts) / DACCountsPerStep;


    slider->gx_slider_info.gx_slider_info_current_val = myVal;
    gx_pixelmap_slider_draw (slider);
    myVal = slider->gx_slider_info.gx_slider_info_current_val - (slider->gx_slider_info.gx_slider_info_max_val/2);
    sprintf (g_SliderValue, "%2d", myVal);

    gx_text_button_text_set (&VeerAdjustScreen.VeerAdjustScreen_SliderValue_Button, g_SliderValue);
}

//*************************************************************************************

UINT VeerAdjust_Screen_event_handler (GX_WINDOW *window, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_SIGNAL(OK_BTN_ID, GX_EVENT_CLICKED):
        screen_toggle((GX_WINDOW *)&PerformanceSelectionScreen, window);
        break;

    case GX_EVENT_SHOW:
        g_WaitingForVeerResponse = false;
        SendNeutralDAC_GetCommand();
        break;

    case GX_EVENT_PEN_UP:
        break;
    }

    gx_window_event_process(window, event_ptr);

    return GX_SUCCESS;
}

