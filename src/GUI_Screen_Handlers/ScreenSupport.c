//*****************************************************************************
// Filename: ScreenSupport.c
// Description: This file has general usage screen functions.
//
// Author: G. Chopcinski, Kg Solutions, LLC
// Date: Jan 28, 2024
// 
//*****************************************************************************

//#include "custom_checkbox.h"
#include "ASL165_System.h"
#include "ScreenSupport.h"

PROGRAMMING_SCREEN_INFO g_ProgrammingScreenInfoStruct[MAX_PROGRAMMING_SCREEN_STRUCTURES];

//*****************************************************************************

void InitializeScreenInfoStruct(PROGRAMMING_SCREEN_INFO* info)
{
	int idx;

	for (idx = 0; idx < MAX_PROGRAMMING_SCREEN_STRUCTURES; ++idx)
	{
		//info->m_ButtonWidget;	//  = NULL;
		//info->m_Checkbox;		//  = NULL;
		info->m_Enabled = false;
		//info->m_ItemWidget;		//  = NULL;
		info->m_LargeDescriptionID = GX_STRING_ID_BLANK;
		//info->m_MultiLineButtonWidget; //  = NULL;
		//info->m_PromptWidget;	//  = NULL;

		++info;	// Point to next object.
	}
}

//*****************************************************************************

void CleanupInfoStruct(PROGRAMMING_SCREEN_INFO *info, GX_VERTICAL_LIST* list)
{
	int idx;
	GX_BOOL result;

	for (idx = 0; idx < MAX_PROGRAMMING_SCREEN_STRUCTURES; ++idx)
	{
		gx_widget_created_test(&info->m_ButtonWidget, &result);
		if (result == GX_TRUE)
			gx_widget_delete((GX_WIDGET*)&info->m_ButtonWidget);

		gx_widget_created_test(&info->m_Checkbox, &result);
		if (result == GX_TRUE)
			gx_widget_delete((GX_WIDGET*)&info->m_Checkbox);

		gx_widget_created_test(&info->m_MultiLineButtonWidget, &result);
		if (result == GX_TRUE)
			gx_widget_delete((GX_WIDGET*)&info->m_MultiLineButtonWidget);

		gx_widget_created_test(&info->m_PromptWidget, &result);
		if (result == GX_TRUE)
			gx_widget_delete((GX_WIDGET*)&info->m_PromptWidget);

		gx_widget_created_test(&info->m_ItemWidget, &result);
		if (result == GX_TRUE)
			gx_widget_delete((GX_WIDGET*)&info->m_ItemWidget);

		info->m_Enabled = false;

		++info;
	}
	list->gx_vertical_list_child_count = 0;

}



