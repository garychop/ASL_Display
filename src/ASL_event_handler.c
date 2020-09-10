//-------------------------------------------------------------------------
#include <stdio.h>
#include "tx_api.h"
#include "gx_api.h"
#include "ASL_HHP_Display_GUIX_resources.h"
#include "ASL_HHP_Display_GUIX_specifications.h"
#include "hal_data.h"
#include "my_gui_thread.h"
#include <my_gui_thread_entry.h>
#include "ASL165_System.h"

//-------------------------------------------------------------------------
extern GX_WINDOW_ROOT * p_window_root;

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

//******************************************************************************************
// Detach one window and attach another window to root. This function checks if the
// parent window is already attached and attaches itself to it. If is already attached
// it simply displays the widget.
//
// Used it to "Change Screens".
//
//******************************************************************************************

VOID screen_toggle(GX_WINDOW *new_win, GX_WINDOW *old_win)
{
    if (!new_win->gx_widget_parent)
    {
        gx_widget_attach(p_window_root, (GX_WIDGET *)new_win);
    }
    else
    {
        gx_widget_show((GX_WIDGET *)new_win);
    }
    gx_widget_detach((GX_WIDGET *)old_win);
}

