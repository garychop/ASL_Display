//*****************************************************************************
// Filename: Custom_Checkbox.c
//
// Date: Sept 1, 2020
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
// This file was derived from the Express Logic Smart Watch demo code supplied
// with GUIX.
//
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"

#define CUSTOM_CHECKBOX_TIMER 2

VOID custom_checkbox_draw(CUSTOM_CHECKBOX *checkbox);
UINT custom_checkbox_event_process(CUSTOM_CHECKBOX *checkbox, GX_EVENT *event_ptr);
VOID custom_checkbox_select(GX_CHECKBOX *checkbox);

//*****************************************************************************
// This is used to create Checkbox sliders.

CUSTOM_CHECKBOX_INFO DefaultCheckboxInfo =
{
    TOGGLE_BUTTON_ID,
    GX_PIXELMAP_ID_SWITCH_BG,
    GX_PIXELMAP_ID_SWITCH_ACTIVE,
    GX_PIXELMAP_ID_SWITCH_DISACTIVE,
    4, // this parameter locates the Black and Green buttons in the background
    24 // 24... This item does change anything
};

/*************************************************************************************/

VOID custom_checkbox_create(CUSTOM_CHECKBOX *checkbox, GX_WIDGET *parent, CUSTOM_CHECKBOX_INFO *info, GX_RECTANGLE *size, int enabled)
{
    gx_checkbox_create((GX_CHECKBOX *)checkbox, "", parent, GX_NULL, GX_STYLE_ENABLED | GX_STYLE_TRANSPARENT, (USHORT) info->widget_id, size);
	if (enabled)
        checkbox->gx_widget_style |= GX_STYLE_BUTTON_PUSHED;

    checkbox->gx_button_select_handler = (VOID (*)(GX_WIDGET *))custom_checkbox_select;
    checkbox->gx_widget_draw_function = (VOID (*)(GX_WIDGET *))custom_checkbox_draw;
    checkbox->gx_widget_event_process_function = (UINT (*)(GX_WIDGET *, GX_EVENT *))custom_checkbox_event_process;
    checkbox->background_id = info->background_id;
    checkbox->cur_offset = info->start_offset;
    checkbox->start_offset = info->start_offset;
    checkbox->end_offset = info->end_offset;
    checkbox->gx_checkbox_checked_pixelmap_id = info->checked_map_id;
    checkbox->gx_checkbox_unchecked_pixelmap_id = info->unchecked_map_id;
}

/*************************************************************************************/
VOID custom_checkbox_draw(CUSTOM_CHECKBOX *checkbox)
{
    GX_PIXELMAP *map;
    INT ypos;

    gx_context_pixelmap_get(checkbox->background_id, &map);

    if (map)
    {
        gx_canvas_pixelmap_draw(checkbox->gx_widget_size.gx_rectangle_left, checkbox->gx_widget_size.gx_rectangle_top, map);
    }


    if (checkbox->gx_widget_style & GX_STYLE_BUTTON_PUSHED)
    {
        gx_context_pixelmap_get(checkbox->gx_checkbox_checked_pixelmap_id, &map);

        if (map)
        {
            ypos = (checkbox->gx_widget_size.gx_rectangle_bottom - checkbox->gx_widget_size.gx_rectangle_top + 1);
            ypos -= map->gx_pixelmap_width;
            ypos /= 2;
            ypos += checkbox->gx_widget_size.gx_rectangle_top;
			--ypos;

            gx_canvas_pixelmap_draw((GX_VALUE) (checkbox->gx_widget_size.gx_rectangle_right - map->gx_pixelmap_width - checkbox->cur_offset), (GX_VALUE) ypos, map);
        }
    }
    else
    {
        gx_context_pixelmap_get(checkbox->gx_checkbox_unchecked_pixelmap_id, &map);

        if (map)
        {
            ypos = (checkbox->gx_widget_size.gx_rectangle_bottom - checkbox->gx_widget_size.gx_rectangle_top); //  + 1);
            ypos -= map->gx_pixelmap_width;
            ypos /= 2;
            ypos += checkbox->gx_widget_size.gx_rectangle_top;
			--ypos;
            gx_canvas_pixelmap_draw((GX_VALUE) (checkbox->gx_widget_size.gx_rectangle_left + checkbox->cur_offset), (GX_VALUE) ypos, map);
        }
    }
}

/*************************************************************************************/
VOID  custom_checkbox_select(GX_CHECKBOX *checkbox)
{
    if (!(checkbox->gx_widget_style & GX_STYLE_BUTTON_PUSHED))
    {
        checkbox->gx_widget_style |= GX_STYLE_BUTTON_PUSHED;
    }
    else
    {
        checkbox->gx_widget_style &= ~GX_STYLE_BUTTON_PUSHED;
    }
}

/*************************************************************************************/
UINT custom_checkbox_event_process(CUSTOM_CHECKBOX *checkbox, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type)
    {
    case GX_EVENT_PEN_DOWN:
        checkbox->gx_button_select_handler((GX_WIDGET *)checkbox);
		checkbox->cur_offset = checkbox->start_offset;
        gx_system_dirty_mark(checkbox);
        return gx_widget_event_process((GX_WIDGET *)checkbox, event_ptr);
		break;

	case GX_EVENT_PEN_UP:
		break;

    default:
        return gx_widget_event_process((GX_WIDGET *)checkbox, event_ptr);
    }

    return 0;
}
