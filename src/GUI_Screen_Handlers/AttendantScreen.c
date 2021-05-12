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

int g_DefaultTrapCounter;

//*************************************************************************************
// Function Name: AttendantScreen_event_process
//
// Description: This functions handles the Attendant screen and dispatches
//		to the Diagnostic Screen or the Reset System screen.
//
//*************************************************************************************

char gPositionString[16];
char gDriveDemandString[16];

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
		calculateDistance = false;
		gx_prompt_text_set (&AttendantScreen.AttendantScreen_DriveDemand_Prompt, "---");
		SendAttendantControl_toHeadArray (true, 0, 0);
		break;

	case GX_SIGNAL (OK_BTN_ID, GX_EVENT_CLICKED):
        SendAttendantControl_toHeadArray (false, 0, 0);
        screen_toggle((GX_WINDOW *)&MainUserScreen, window);
		break;

	case GX_EVENT_PEN_DOWN:
		myPoint = event_ptr->gx_event_payload.gx_event_pointdata;
		sprintf (gPositionString, "%d,%d", myPoint.gx_point_x, myPoint.gx_point_y);
		gx_prompt_text_set (&AttendantScreen.AttendantScreen_RawPosition_Prompt, gPositionString);

		if (event_ptr->gx_event_target->gx_widget_id == ATTENDANT_DRIVER_ID)
		{
			//gx_prompt_text_color_set ((GX_PROMPT*)&AttendantScreen.AttendantScreen_RawPosition_Prompt, GX_COLOR_GREEN, GX_COLOR_GREEN, GX_COLOR_GREEN);
			calculateDistance = true;
		}
		//else
		//{
		//	gx_prompt_text_color_set ((GX_PROMPT*)&AttendantScreen.AttendantScreen_RawPosition_Prompt, GX_COLOR_YELLOW, GX_COLOR_YELLOW, GX_COLOR_YELLOW);
		//}
		break;

	case GX_EVENT_PEN_DRAG:
		myPoint = event_ptr->gx_event_payload.gx_event_pointdata;
		sprintf (gPositionString, "%d,%d", myPoint.gx_point_x, myPoint.gx_point_y);
		gx_prompt_text_set (&AttendantScreen.AttendantScreen_RawPosition_Prompt, gPositionString);

		if (event_ptr->gx_event_target->gx_widget_id == ATTENDANT_DRIVER_ID)
		{
			//gx_prompt_text_color_set ((GX_PROMPT*)&AttendantScreen.AttendantScreen_RawPosition_Prompt, GX_COLOR_GREEN, GX_COLOR_GREEN, GX_COLOR_GREEN);
			calculateDistance = true;
		}
		//else
		//{
		//	gx_prompt_text_color_set ((GX_PROMPT*)&AttendantScreen.AttendantScreen_RawPosition_Prompt, GX_COLOR_YELLOW, GX_COLOR_YELLOW, GX_COLOR_YELLOW);
		//}
		break;

	case GX_EVENT_PEN_UP:
        calculateDistance = false;
        gx_prompt_text_set (&AttendantScreen.AttendantScreen_DriveDemand_Prompt, "---");
        gx_numeric_prompt_value_set (&AttendantScreen.AttendantScreen_Distance_Prompt, 0);
        SendAttendantControl_toHeadArray (true, 0, 0);
        break;

	default:
	    ++g_DefaultTrapCounter;
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
			gx_numeric_prompt_value_set (&AttendantScreen.AttendantScreen_Distance_Prompt, (int) myDistance);
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
                }
                else                    // Must be reverse
                {
                    speed += INNER_CIRCLE_DIAMETER; // adjust for inner circle offset
                    speed *= (100.0f / (100.0f - INNER_CIRCLE_DIAMETER_FLOAT));                     // convert to 0-100
                    if (speed < -65)
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
                }
                else                    // Must be left
                {
                    direction += INNER_CIRCLE_DIAMETER; // adjust for inner circle offset
                    direction *= (100.0f / (100.0f - INNER_CIRCLE_DIAMETER_FLOAT));                     // convert to 0-100
                    if (direction < -40)
                        direction = -100;
                }
                directionDemand = direction;
            }

			sprintf (gDriveDemandString, "%d,%d", speedDemand, directionDemand);
			gx_prompt_text_set (&AttendantScreen.AttendantScreen_DriveDemand_Prompt, gDriveDemandString);
	        SendAttendantControl_toHeadArray (true, speedDemand, directionDemand);
		}
		else	// touch is outside of valid zone
		{
			gx_prompt_text_set (&AttendantScreen.AttendantScreen_DriveDemand_Prompt, "---");
			gx_numeric_prompt_value_set (&AttendantScreen.AttendantScreen_Distance_Prompt, 0);
            SendAttendantControl_toHeadArray (true, speedDemand, directionDemand);
		}
	}
//	else
//	{
//        gx_prompt_text_set (&AttendantScreen.AttendantScreen_DriveDemand_Prompt, "---");
//        gx_numeric_prompt_value_set (&AttendantScreen.AttendantScreen_Distance_Prompt, 0);
//	}

    gx_window_event_process(window, event_ptr);

	return (GX_SUCCESS);
}


