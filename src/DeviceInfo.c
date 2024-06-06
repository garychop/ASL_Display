//*****************************************************************************
// Filename: DeviceInfo.c
// Description: This file supports the Devices.
//
// Date: Oct 15, 2023
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "ASL165_System.h"
#include "DeviceInfo.h"
#include "QueueDefinition.h"

//*************************************************************************************

//extern int g_SNP_Nozzle_Value;

//*************************************************************************************

DEVICE_INFO_STRUCT g_DeviceSettings[MAX_DEVICES];
DEVICE_INFO_STRUCT *gp_ProgrammingDevice = NULL;
DEVICE_NUMBER_ENUM g_ActiveDriverControlIdx = HEAD_ARRY_DEVICE_IDX;
DEVICE_INFO_STRUCT* gp_ActiveDriverControl = NULL;
DEVICE_NUMBER_ENUM g_ActiveDriverControl;

/*************************************************************************************
 * Forawrd Declarations
 */
void SetDefaultDriverControl (void);
VOID Initialize_HeadArray_Settings();
VOID Initialize_Driver4Quad_Settings();
VOID Initialize_SipNPUff_Settings();
VOID Initialize_2_Switch_Settings();
VOID Initialize_SNP_HeadArray_Settings();

//*************************************************************************************

void SetDefaultDriverControl (void)
{
	gp_ActiveDriverControl = &g_DeviceSettings[HEAD_ARRY_DEVICE_IDX];
	g_ActiveDriverControlIdx = HEAD_ARRY_DEVICE_IDX;
}

//*************************************************************************************

void SetProgrammingDriverControl (DEVICE_INFO_STRUCT *device)
{
	gp_ProgrammingDevice = device;
}

//*************************************************************************************
// Advance to the next enabled Driver Control
//*************************************************************************************

void AdvanceToNextDriverControl()
{
	int idx;

	for (idx = 0; idx < ENDOF_DEVICES_IDX; ++idx)
	{
		if (++g_ActiveDriverControlIdx >= ENDOF_DEVICES_IDX)
			g_ActiveDriverControlIdx = (DEVICE_NUMBER_ENUM)0;
		if (g_DeviceSettings[g_ActiveDriverControlIdx].m_Enabled == ENABLED)
			break;
	}
	gp_ActiveDriverControl = &g_DeviceSettings[g_ActiveDriverControlIdx];
}

//*************************************************************************************

VOID Initialize_HeadArray_Settings()
{
	// Populate the first Device, which is the Head Array or 4-quadrant Driver Control
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_DriverConfiguration = HEAD_ARRY_DEVICE_IDX;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_DriverQuadrantSetting = DRIVER_3_QUADRANT;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_Mode_Switch_Schema = DRV_MODE_SWITCH_PIN5;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_Enabled = ENABLED;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_DeviceNameStringID = GX_STRING_ID_QUADRANT_3;

	// Populate the Head Array's Pad Information, starting with LEFT
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadDirection = PAD_DIRECTION_LEFT;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Button;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_SNP_Threshold = 30; // soft puff

	// Set the Head Array's Right Pad defaults.
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadDirection = PAD_DIRECTION_RIGHT;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimuDriveString,  "20%");
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Button;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_SNP_Threshold = -30; // soft sip
	
	// Set the Head Array's Center/Forward Pad defaults.
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadDirection = PAD_DIRECTION_FORWARD;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_SNP_Threshold = 60; // hard puff
	
	// Set the Head Array's Reverse Pad Defaulats.
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadDirection = PAD_DIRECTION_OFF;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[HEAD_ARRY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_SNP_Threshold = -60; // hard sip

	/*
     * Get Enabled setting from ION Hub
     */
    RequestDriverEnableStatus (HEAD_ARRY_DEVICE_IDX);
}

//*************************************************************************************

VOID Initialize_Driver4Quad_Settings()
{
	// Populate the 4-quadrant Driver Control
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_DriverConfiguration = DRIVER_4_QUAD_IDX;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_DriverQuadrantSetting = DRIVER_4_QUADRANT;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_Mode_Switch_Schema = DRV_MODE_SWITCH_PIN5;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_Enabled = DISABLED;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_DeviceNameStringID = GX_STRING_ID_QUADRANT_4;

	// Populate the 4-quadrant Driver Control Pad Information, starting with LEFT
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_PadDirection = PAD_DIRECTION_LEFT;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Button;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[LEFT_PAD].m_SNP_Threshold = 30; // soft puff
	
	// Set the 4-quadrant Driver Control Right Pad defaults.
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_PadDirection = PAD_DIRECTION_RIGHT;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Button;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[RIGHT_PAD].m_SNP_Threshold = -30; // soft sip
	
	// Set the 4-quadrant Driver Control Center/Forward Pad defaults.
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_PadDirection = PAD_DIRECTION_FORWARD;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[CENTER_PAD].m_SNP_Threshold = 60; // hard puff
	
	// Set the 4-quadrant Driver Control Reverse Pad Defaulats.
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_PadDirection = PAD_DIRECTION_REVERSE;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[DRIVER_4_QUAD_IDX].m_PadInfo[REVERSE_PAD].m_SNP_Threshold = -60; // hard sip

	/*
     * Get Enabled setting from ION Hub
     */
    RequestDriverEnableStatus (DRIVER_4_QUAD_IDX);
}

//*************************************************************************************

VOID Initialize_SipNPUff_Settings()
{
	// Populate the Device, which is the Sip-N-Puff.
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_DriverConfiguration = SIP_N_PUFF_DEVICE_IDX;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_DriverQuadrantSetting = DRIVER_4_QUADRANT;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_Mode_Switch_Schema = DRV_MODE_SWITCH_PIN5;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_Enabled = DISABLED;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_DeviceNameStringID = GX_STRING_ID_SIP_N_PUFF;

	// Populate the Sip-N-Puff Pad Information, starting with LEFT
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadDirection = PAD_DIRECTION_LEFT;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Button;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_SNP_Threshold = 30; // soft puff

	// Populate the Sip-N-Puff Pad Information right pad information.
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadDirection = PAD_DIRECTION_RIGHT;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Button;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_SNP_Threshold = -30; // soft sip
	
	// Populate the Sip-N-Puff Pad Information Center/Forward Pad defaults.
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadDirection = PAD_DIRECTION_FORWARD;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_SNP_Threshold = 60; // hard puff
	
	// Populate the Sip-N-Puff Pad Information Reverse Pad Defaulats.
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadDirection = PAD_DIRECTION_REVERSE;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SIP_N_PUFF_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_SNP_Threshold = -60; // hard sip

	/*
	 * Get Enabled setting from ION Hub
	 */
	RequestDriverEnableStatus (SIP_N_PUFF_DEVICE_IDX);
}

//*************************************************************************************

VOID Initialize_2_Switch_Settings()
{
	// Populate the Device, which is the 2-Switch.
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_DriverConfiguration = TWO_SWITCH_DEVICE_IDX;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_DriverQuadrantSetting = DRIVER_2_QUADRANT;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_Mode_Switch_Schema = DRV_MODE_SWITCH_PIN5;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_Enabled = DISABLED;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_DeviceNameStringID = GX_STRING_ID_TWO_SWITCH;

	// Populate the 2-Switch Pad Information, starting with LEFT
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadDirection = PAD_DIRECTION_LEFT;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Button;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_SNP_Threshold = 30; // soft puff

	// Populate the 2-Switch Pad Information right pad information.
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadDirection = PAD_DIRECTION_RIGHT;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Button;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_SNP_Threshold = -30; // soft sip
	
	// Populate the 2-Switch Pad Information Center/Forward Pad defaults.
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadDirection = PAD_DIRECTION_FORWARD;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_SNP_Threshold = 60; // hard puff
	
	// Populate the 2-Switch Pad Information Reverse Pad Defaulats.
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadDirection = PAD_DIRECTION_OFF;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[TWO_SWITCH_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_SNP_Threshold = -60; // hard sip

    /*
     * Get Enabled setting from ION Hub
     */
    RequestDriverEnableStatus (TWO_SWITCH_DEVICE_IDX);
}

//*************************************************************************************

VOID Initialize_SNP_HeadArray_Settings()
{
	// Populate the Device, which is the 2-Switch.
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_DriverConfiguration = SNP_HEAD_ARRAY_DEVICE_IDX;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_DriverQuadrantSetting = DRIVER_4_QUADRANT;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_Mode_Switch_Schema = DRV_MODE_SWITCH_PIN5;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_Enabled = DISABLED;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_DeviceNameStringID = GX_STRING_ID_SNP_HEAD_ARRAY;

	// Populate the SNP Head Array Information, starting with LEFT
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadDirection = PAD_DIRECTION_LEFT;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_LeftPad_Button;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[LEFT_PAD].m_SNP_Threshold = 30; // soft puff

	// Populate the SNP Head Array Information right pad information.
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadDirection = PAD_DIRECTION_RIGHT;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_RightPad_Button;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[RIGHT_PAD].m_SNP_Threshold = -30; // soft sip
	
	// Populate the SNP Head Array Information Center/Forward Pad defaults.
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadDirection = PAD_DIRECTION_FORWARD;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[CENTER_PAD].m_SNP_Threshold = 30; // hard puff
	
	// Populate the SNP Head Array Information Reverse Pad Defaulats.
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadDirection = PAD_DIRECTION_REVERSE;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadType = DIGITAL_PADTYPE;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimumDriveValue = 20;
	strcpy (g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_MinimuDriveString, "20%");
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_DirectionIcon = &SetPadDirectionScreen.SetPadDirectionScreen_CenterPad_Button;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMinimumCalibrationValue = 2;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_PadMaximumCalibrationValue = 30;
	g_DeviceSettings[SNP_HEAD_ARRAY_DEVICE_IDX].m_PadInfo[REVERSE_PAD].m_SNP_Threshold = -30; // hard sip

    /*
     * Get Enabled setting from ION Hub
     */
    RequestDriverEnableStatus (SNP_HEAD_ARRAY_DEVICE_IDX);
}

//*************************************************************************************

void InitializeDriverControls (void)
{
	Initialize_HeadArray_Settings();
	Initialize_Driver4Quad_Settings();
	Initialize_SipNPUff_Settings();
	Initialize_2_Switch_Settings();
	Initialize_SNP_HeadArray_Settings();

	SetDefaultDriverControl();
}

