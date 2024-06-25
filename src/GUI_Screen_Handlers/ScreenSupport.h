//*****************************************************************************
// Filename: ScreenSupport.h
// Description: This file declares function, typedefs and macros to support
//		the User's Main Screen
//
// Author: G. Chopcinski, Kg Solutions, LLC
// Date: Jan 28, 2024
// 
//*****************************************************************************

#ifndef SCREEN_SUPPORT_H
#define SCREEN_SUPPORT_H

#include "custom_checkbox.h"

#define MAX_PROGRAMMING_SCREEN_STRUCTURES (8)

// This structure is used by each screen.
typedef struct
{
	int m_Enabled;
	GX_RESOURCE_ID m_LargeDescriptionID;
	GX_WIDGET m_ItemWidget;
	GX_PROMPT m_PromptWidget;
	GX_TEXT_BUTTON m_ButtonWidget;
	GX_MULTI_LINE_TEXT_BUTTON m_MultiLineButtonWidget;
	CUSTOM_CHECKBOX m_Checkbox;
} PROGRAMMING_SCREEN_INFO;

extern PROGRAMMING_SCREEN_INFO g_ProgrammingScreenInfoStruct[MAX_PROGRAMMING_SCREEN_STRUCTURES];

void InitializeScreenInfoStruct(PROGRAMMING_SCREEN_INFO* info);
void CleanupInfoStruct(PROGRAMMING_SCREEN_INFO* info, GX_VERTICAL_LIST* list);


#endif // SCREEN_SUPPORT_H


