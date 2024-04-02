//*****************************************************************************
// Filename: ION_BT_SupportFunctions.cpp
//
// Date: Sept 5, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "custom_checkbox.h"
#include "BluetoothDeviceInfo.h"

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

int g_SelectedBTDevice_ToProgram = 0;
int g_SelectedBluetoothDeviceIndex;

// The following is the Bluetooth Device information.
BLUETOOTH_DEVICE_DATA g_BluetoothDeviceSettings[MAX_BLUETOOTH_DEVICES];

// The following hold the Bluetooth Data for the User Bluetooth Screen.
BLUETOOTH_SCREEN_DATA g_BTDeviceSetup_ScreenInfo[MAX_BLUETOOTH_DEVICES];

const char g_BT_DESCRIPTION_STRINGS [BT_TYPE_END][DESCRIPTION_STRING_WIDTH+1] =  
{	//" MOUSE ",
	" 3-SWITCH ",
	" 4-SWITCH ",
	" AAC DEVICE ",
	" FEEDER ",
	" PHONE ",
	" TABLET ",
	" LEGACY ",
	" SEATING ",
	" Bluetooth A",
	" Bluetooth B",
	" Bluetooth C",
	" Bluetooth D"
	/*" Bluetooth E",
	" Bluetooth F",
	" Bluetooth G"*/};

const char g_BT_COLOR_TEXT [][BT_COLOR_STRING_WIDTH] = 
{ "WHITE", "BLUE", "ORANGE", "RED", "GREEN", "YELLOW", "NONE" };

//*************************************************************************************
// Forward Declarations
//*************************************************************************************

//*************************************************************************************
// This is called by the main routine at startup to retrieve the Bluetooth Device Setup information.
//*************************************************************************************

void InitializeBluetoothDeviceInformation(void)
{
	BT_SetDeviceTypeInformation (0, BT_MOUSE_TYPE);
	BT_SetDeviceColor (0, BT_GREEN);
	BT_SetDeviceStatus (0, BT_PAIRED);

	BT_SetDeviceTypeInformation (1, BT_3_SWITCH_TYPE);
	BT_SetDeviceColor (1, BT_BLUE);
	BT_SetDeviceStatus (1, BT_PAIRED);

	BT_SetDeviceTypeInformation (2, BT_SEATING_TYPE);
	BT_SetDeviceColor (2, BT_ORANGE);
	BT_SetDeviceStatus (2, BT_PAIRED);

	BT_SetDeviceTypeInformation (3, BT_AUX_COMM_TYPE);
	BT_SetDeviceColor (3, BT_RED);
	BT_SetDeviceStatus (3, BT_PAIRED);

	BT_SetDeviceTypeInformation (4, BT_FEEDER_TYPE);
	BT_SetDeviceColor (4, BT_YELLOW);
	BT_SetDeviceStatus (4, BT_PAIRED);

	BT_SetDeviceTypeInformation (5, BT_LEGACY_TYPE);
	BT_SetDeviceColor (5, BT_YELLOW);
	BT_SetDeviceStatus (5, BT_PAIRED);

	BT_SetDeviceTypeInformation (6, BT_PHONE_TYPE);
	BT_SetDeviceColor (6, BT_BLUE);
	BT_SetDeviceStatus (6, BT_PAIRED);

	BT_SetDeviceTypeInformation (7, BT_ACU_TYPE);
	BT_SetDeviceColor (7, BT_WHITE);
	BT_SetDeviceStatus (7, BT_PAIRED);

}

//*************************************************************************************
// This function sets the Type and Icons in the Bluetooth Device Information.
//*************************************************************************************

void BT_SetDeviceTypeInformation (UINT index, BT_DEVICE_TYPE deviceType)
{
	switch (deviceType)
	{
	case BT_MOUSE_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_MOUSE_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_MOUSE_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_MOUSE_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_MOUSE;
		break;

	case BT_3_SWITCH_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_3_SWITCH_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_3SWITCH_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_3SWITCH_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_3_SWITCH;
		break;

	case BT_4_SWITCH_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_4_SWITCH_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_4SWITCH_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_4SWITCH_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_4_SWITCH;
		break;

	case BT_AUX_COMM_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_AUX_COMM_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_ACC1_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_ACC1_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_AAC_DEVICE;
		break;

	case BT_FEEDER_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_FEEDER_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_FEEDER_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_FEEDER_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_FEEDER;
		break;
	
	case BT_PHONE_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_PHONE_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_SMARTPHONE_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_SMARTPHONE_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_PHONE;
		break;

	case BT_TABLET_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_TABLET_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_TABLET_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_TABLET_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_TABLET;
		break;

	case BT_LEGACY_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_LEGACY_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_LEGACY_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_LEGACY_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_LEGACY;
		break;

	case BT_SEATING_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_SEATING_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_WHEELCHAIR_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_WHEELCHAIR_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_SEATING;
		break;

	case BT_GENERIC_TYPE_1:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_GENERIC_TYPE_1;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_BTA_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_BTA_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_GEN_A;
		break;

	case BT_GENERIC_TYPE_2:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_GENERIC_TYPE_2;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_BTB_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_BTB_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_GEN_B;
		break;

	case BT_GENERIC_TYPE_3:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_GENERIC_TYPE_3;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_BTC_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_BTC_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_GEN_C;
		break;

	case BT_GENERIC_TYPE_4:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_GENERIC_TYPE_4;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_BTD_BLACK_60X60;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_BTD_WHITE_60X60;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_GEN_D;
		break;

	case BT_ACU_TYPE:
		g_BluetoothDeviceSettings[index].m_BT_Type = BT_ACU_TYPE;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_ATTENDANT_66X66;
		g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_ATTENDANT_66X66;
		g_BluetoothDeviceSettings[index].m_DescriptionID = GX_STRING_ID_BT_ACU;
		break;

	//case BT_GENERIC_TYPE_6:
	//	g_BluetoothDeviceSettings[index].m_BT_Type = BT_GENERIC_TYPE_6;
	//	g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_BTF_BLACK_60X60;
	//	g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_BTF_WHITE_60X60;
	//	break;

	//case BT_GENERIC_TYPE_7:
	//	g_BluetoothDeviceSettings[index].m_BT_Type = BT_GENERIC_TYPE_7;
	//	g_BluetoothDeviceSettings[index].m_BT_Icon_Selected = GX_PIXELMAP_ID_BT_ICON_BTG_BLACK_60X60;
	//	g_BluetoothDeviceSettings[index].m_BT_Icon_Normal = GX_PIXELMAP_ID_BT_ICON_BTG_WHITE_60X60;
	//	break;

	case BT_TYPE_END:
	default:
	    break;
	} // end of switch (deviceType)
}

//*************************************************************************************
// Set BT Device Color
//*************************************************************************************

void BT_SetDeviceColor (UINT index, BT_COLOR color)
{
	if (index < MAX_BLUETOOTH_DEVICES)
	{
		g_BluetoothDeviceSettings[index].m_BT_Color = color;
		switch (color)
		{
		case BT_WHITE:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_WHITE;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_WHITE;
			break;
		case BT_BLUE:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_BT_BLUE;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_BT_BLUE;
			break;
		case BT_ORANGE:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_BRIGHT_ORANGE;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_BRIGHT_ORANGE;
			break;
		case BT_RED:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_MAROON;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_MAROON;
			break;
		case BT_GREEN:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_GREEN;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_GREEN;
			break;
		case BT_YELLOW:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_YELLOW;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_YELLOW;
			break;
		default:
			g_BluetoothDeviceSettings[index].m_NormalFillColor = GX_COLOR_ID_TEXT_INPUT_FILL;
			g_BluetoothDeviceSettings[index].m_SelectedFillColor = GX_COLOR_ID_TEXT_INPUT_TEXT;
			break;
		}
	}
}

//*************************************************************************************
// This function gets or sets the status of the BT Device.
//*************************************************************************************

BT_STATUS BT_GetDeviceStatus (UINT index)
{
	return g_BluetoothDeviceSettings[index].m_Status;
}

//*************************************************************************************
void BT_SetDeviceStatus (UINT index, BT_STATUS newStatus)
{
	if (index < MAX_BLUETOOTH_DEVICES)
	{
//		if ((newStatus >= BT_DISABLED) && (newStatus < BT_STATUS_END)) // According to the compiler, this test is not needed.
			g_BluetoothDeviceSettings[index].m_Status = newStatus;
//		else
//			g_BluetoothDeviceSettings[index].m_Status = BT_DISABLED;
	}
}

//*************************************************************************************
// This function "deletes" (cleans up) the widget from the BT Screen information Structure
//*************************************************************************************

VOID BT_Screen_Widget_Cleanup (BLUETOOTH_SCREEN_DATA *bt_screen_info, INT arraySize)
{
	int i;

	for (i = 0; i < arraySize; ++i)
	{
		if (&bt_screen_info[i].m_ButtonWidget != NULL)
			gx_widget_delete ((GX_WIDGET*) &bt_screen_info[i].m_ButtonWidget);
		if (&bt_screen_info[i].m_ItemWidget != NULL)
			gx_widget_delete ((GX_WIDGET*) &bt_screen_info[i].m_ItemWidget);
		if (&bt_screen_info[i].m_IconWidget != NULL)
			gx_widget_delete ((GX_WIDGET*) &bt_screen_info[i].m_IconWidget);
		if (&bt_screen_info[i].m_MultilineBtnWidget != NULL)
			gx_widget_delete ((GX_WIDGET*) &bt_screen_info[i].m_MultilineBtnWidget);
		if (&bt_screen_info[i].m_PixelPromptWidget != NULL)
			gx_widget_delete ((GX_WIDGET*) &bt_screen_info[i].m_PixelPromptWidget);
		if (&bt_screen_info[i].m_PromptWidget != NULL)
			gx_widget_delete ((GX_WIDGET*) &bt_screen_info[i].m_PromptWidget);
	}
}







