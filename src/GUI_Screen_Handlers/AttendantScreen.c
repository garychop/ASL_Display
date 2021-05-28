//*****************************************************************************
// Filename: AttendantScreen.c
//
// Date: April 28, 2021
//
// Author: G. Chopcinski, Kg Solutions, LLC
// 
//*****************************************************************************

#include "math.h"

#include "ASL165_System.h"
#include "QueueDefinition.h"

//*************************************************************************************
// Local Macros
//*************************************************************************************

#define INNER_CIRCLE_DIAMETER (30)
#define INNER_CIRCLE_DIAMETER_FLOAT (30.0f)

//*************************************************************************************
// External References
//*************************************************************************************

//*************************************************************************************
// Local/Global variables
//*************************************************************************************

uint8_t g_AttendantStatus = 0;  // D0 = 1 = Attendant Active, D1 = 1 if ESTOP active

//*************************************************************************************
// Function Name: AttendantScreen_event_process
//
// Description: This functions handles the Attendant screen and dispatches
//		to the Diagnostic Screen or the Reset System screen.
//
//*************************************************************************************

//char gPositionString[16];
//char gDriveDemandString[16];

UINT AttendantScreen_event_process (GX_WINDOW *window, GX_EVENT *event_ptr)
{
	GX_POINT myPoint;
	double myDistance;
	double diffX, diffY;
	int8_t speedDemand = 0, directionDemand = 0;
    float speed, direction;
	bool calculateDistance = false;

	switch (event_ptr->gx_event_type)
	{
	case GX_EVENT_SHOW:
	    g_ActiveScreen = (GX_WIDGET*) window;
		calculateDistance = false;
		g_AttendantStatus = 0x01;           // D0 = 1 = Attendant Active
		SendAttendantControl_toHeadArray (g_AttendantStatus, 0, 0);
		break;

	case GX_SIGNAL (OK_BTN_ID, GX_EVENT_CLICKED):
        calculateDistance = false;
        g_AttendantStatus = 0x0;            // D0 = 0 = Attendant NOT Active
        gx_icon_button_pixelmap_set (&AttendantScreen.AttendantScreen_StopButton, GX_PIXELMAP_ID_HEADARRAY_88X70);
        SendAttendantControl_toHeadArray (0, 0, 0);     // Indicate Attendant NOT active and NOT in e-stop.
        screen_toggle((GX_WINDOW *)&MainUserScreen, window);
		break;

	case GX_SIGNAL (HA_POWERON_BTN_ID, GX_EVENT_CLICKED):
        if (g_AttendantStatus & 2)      // are we in e-stop?
            gx_icon_button_pixelmap_set (&AttendantScreen.AttendantScreen_StopButton, GX_PIXELMAP_ID_HEADARRAY_DISABLED_88X70);
        else
            gx_icon_button_pixelmap_set (&AttendantScreen.AttendantScreen_StopButton, GX_PIXELMAP_ID_HEADARRAY_88X70);
	    break;

	case GX_SIGNAL (HA_POWEROFF_BTN_ID, GX_EVENT_CLICKED):
        gx_icon_button_pixelmap_set (&AttendantScreen.AttendantScreen_StopButton, GX_PIXELMAP_ID_HEADARRAY_POWEROFF_88X70);
	    break;

	case GX_EVENT_PEN_DOWN:
        calculateDistance = true;
        if (event_ptr->gx_event_target->gx_widget_id == STOP_BUTTON_ID)
        {
            if (g_AttendantStatus & 2)      // are we in e-stop?
            {
                g_AttendantStatus &= 0xfd;  // Make D1 = 0 for NOT in e-stop.
                gx_icon_button_pixelmap_set (&AttendantScreen.AttendantScreen_StopButton, GX_PIXELMAP_ID_HEADARRAY_88X70);
            }
            else
            {
                g_AttendantStatus |= 0x02;  // Make D1 = 1 for IN e-stop
                gx_icon_button_pixelmap_set (&AttendantScreen.AttendantScreen_StopButton, GX_PIXELMAP_ID_HEADARRAY_DISABLED_88X70);
            }
        }
		break;

	case GX_EVENT_PEN_DRAG:
		if (event_ptr->gx_event_target->gx_widget_id == ATTENDANT_DRIVER_ID)
		{
			calculateDistance = true;
		}
		break;

	case GX_EVENT_PEN_UP:
        calculateDistance = false;
        SendAttendantControl_toHeadArray (g_AttendantStatus, 0, 0);
        break;

	default:
	    break;
	} // end switch

	if (calculateDistance)
	{
		// distance = square root of the sum of the squares
		// Calculate the distance for the center of the Circle.
		diffX = (double) (event_ptr->gx_event_payload.gx_event_pointdata.gx_point_x) - 120.0f;
		diffY = (double) (event_ptr->gx_event_payload.gx_event_pointdata.gx_point_y) - 120.0f;
		diffX *= diffX;
		diffY *= diffY;
		myDistance = sqrt (diffX + diffY);

		// Check to see if the position is within the valid area.
		if ((myDistance > INNER_CIRCLE_DIAMETER_FLOAT) && (myDistance < 100.0f))
		{
			// touch is within valid zone
			//gx_numeric_prompt_value_set (&AttendantScreen.AttendantScreen_Distance_Prompt, (int) myDistance);
            // Calculate speed demand
            // First determine if it's in the "dead zone" which is within the inner circle.
            if ((event_ptr->gx_event_payload.gx_event_pointdata.gx_point_x < (120-INNER_CIRCLE_DIAMETER)) || (event_ptr->gx_event_payload.gx_event_pointdata.gx_point_x > (120+INNER_CIRCLE_DIAMETER)))
            {
                speed = (float) event_ptr->gx_event_payload.gx_event_pointdata.gx_point_x - 120;    // 120 = screen center of circle
                if (speed > 0)  // Forward direction?
                {
                    speed -= INNER_CIRCLE_DIAMETER_FLOAT;   // eliminate inner circle offset
                    speed *= (100.0f / (100.0f - INNER_CIRCLE_DIAMETER_FLOAT));                     // convert to 0-100
                    if (speed > 75)
                        speed = 100;
                    // If Digital Option is set, force a full demand
                    if (g_AttendantSettings & 0x02) // D1 = 1 if Digital
                        speed = 100;
                }
                else                    // Must be reverse
                {
                    speed += INNER_CIRCLE_DIAMETER; // adjust for inner circle offset
                    speed *= (100.0f / (100.0f - INNER_CIRCLE_DIAMETER_FLOAT));                     // convert to 0-100
                    if (speed < -65)
                        speed = -100;
                    // If Digital Option is set, force a full demand
                    if (g_AttendantSettings & 0x02) // D1 = 1 if Digital
                        speed = -100;
                }
                speedDemand = (int) speed;
            }

            // Calculate direction demand
            // First determine if it's in the "dead zone" which is within the circle.
            if ((event_ptr->gx_event_payload.gx_event_pointdata.gx_point_y < (120-INNER_CIRCLE_DIAMETER)) || (event_ptr->gx_event_payload.gx_event_pointdata.gx_point_y > (120+INNER_CIRCLE_DIAMETER)))
            {
                direction = event_ptr->gx_event_payload.gx_event_pointdata.gx_point_y - 120;
                if (direction > 0)  // Right
                {
                    direction -= INNER_CIRCLE_DIAMETER_FLOAT;   // eliminate inner circle offset
                    direction *= (100.0f / (100.0f - INNER_CIRCLE_DIAMETER_FLOAT));                     // convert to 0-100
                    if (direction > 40)
                        direction = 100;
                    // If Digital Option is set, force a full demand
                    if (g_AttendantSettings & 0x02) // D1 = 1 if Digital
                        direction = 100;
                }
                else                    // Must be left
                {
                    direction += INNER_CIRCLE_DIAMETER; // adjust for inner circle offset
                    direction *= (100.0f / (100.0f - INNER_CIRCLE_DIAMETER_FLOAT));                     // convert to 0-100
                    if (direction < -40)
                        direction = -100;
                    // If Digital Option is set, force a full demand
                    if (g_AttendantSettings & 0x02) // D1 = 1 if Digital
                        direction = -100;
                }
                directionDemand = direction;
            }

	        SendAttendantControl_toHeadArray (g_AttendantStatus, speedDemand, directionDemand);
		}
		else	// touch is outside of valid zone
		{
            SendAttendantControl_toHeadArray (g_AttendantStatus, speedDemand, directionDemand);
		}
	}

    gx_window_event_process(window, event_ptr);

	return (GX_SUCCESS);
}


