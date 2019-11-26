//***************************************************************
//   The communication protocol I am using is not the real I2C.
//   I only use I2C start bit and 8 bit data trans method.
//   The trans stop controlled by length byte.
//   After data transmition stop, set io and clk to high, then set io and
//   clk as input. only when sending data can set io and clk
//   as output. no stop bit and ack bit.
//
//   I2C communication procedure:
//      1.    check if I2C_RES_PIN is high. if not, can not reset I2C_CS_PIN
//      2.    if I2C_RES_PIN is high, reset I2C_CS_PIN, so that
//            slave(sigma) can goes into interrupt routing
//      3.    before sending package, check if I2C_RES_PIN is low.
//      4.    if I2C_RES_PIN is low, send package.
//      5.    before receiving package, check if I2C_RES_PIN is high.
//      6.    if I2C_RES_PIN is high, receive response
//*******************************************************************************

#include "my_gui_thread.h"
#include <my_gui_thread_entry.h>

#include "HeadArray_CommunicationThread.h"

#include "QueueDefinition.h"

#define FORCE_OK_FOR_GUI_DEBUGGING


//******************************************************************************
// Defines and Macros
//******************************************************************************
#define MSG_OK  0                   // Indicates message as processed OK.
#define MSG_STATUS_TIMEOUT 1        // indicates message processing failed to receive proper status, i.e. NO I/O Line or CS
#define MSG_INVALID_FORMAT 2        // Indicates message was formatted improperly or invalid data.

//******************************************************************************
// Forward Declarations
//******************************************************************************

static void Write_I2C_Byte(uint8_t i2cbyte);
static uint8_t Send_I2C_Package (uint8_t *i2c_pack, uint8_t pack_len);
static uint8_t Read_I2C_Byte (uint8_t *);
static uint8_t Read_I2C_Package(uint8_t *);
uint8_t ExecuteHeartBeat(void);
uint8_t CalculateChecksum (uint8_t *, uint8_t);
uint32_t Process_GUI_Messages (GUI_MSG_STRUCT);

//******************************************************************************
// Global Variables
//******************************************************************************

uint8_t g_HeartBeatCounter = 0;
void (*g_MyState)(void);

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
char myPadDirection = 'L';
uint8_t g_myMode;
#endif

//******************************************************************************
// Function:CalculateChecksum
// Description: Calculates the checks and populates the array.
//******************************************************************************

uint8_t CalculateChecksum (uint8_t *data, uint8_t msgLen)
{
    uint8_t cs;
    uint8_t counter;
    uint8_t myData;

    cs = 0;
    for (counter = 0; counter < msgLen; ++counter)
    {
        myData = data[counter];
        cs = (uint8_t)(myData + cs);
    }
    return cs;
}

//******************************************************************************
//
//******************************************************************************

static void Write_I2C_Byte(uint8_t i2cbyte)
{
    uint8_t i;

    // Send a bit at a time.
    for(i = 0; i < 8; i++)
    {
        // Set Data (IO) Pin based upon bit in byte.
        if( (i2cbyte<<i) & 0x80 )
        {
            g_ioport_on_ioport.pinWrite(I2C_IO_PIN, IOPORT_LEVEL_HIGH);
        }
        else
        {
            g_ioport_on_ioport.pinWrite(I2C_IO_PIN, IOPORT_LEVEL_LOW);
        }
        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);           // Delay are short period of time
        g_ioport_on_ioport.pinWrite(I2C_CLK_PIN, IOPORT_LEVEL_HIGH);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MICROSECONDS);          // Delay about 50 microseconds so Head Array will recongize it.
        g_ioport_on_ioport.pinWrite(I2C_CLK_PIN, IOPORT_LEVEL_LOW);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MICROSECONDS);          // Delay about 50 microseconds so Head Array will recongize it.
    }
}

//******************************************************************************
// Function: Send_I2C_Package
// Description: This function processes the I/O signals so as to send
//      an entire packet to the ASL110 Head Array.
// Returns: MSG_OK or MSG_STATUS_TIMEOUT
//******************************************************************************

static uint8_t Send_I2C_Package(uint8_t *i2c_pack, uint8_t pack_len)
{
    uint8_t i;
    uint16_t wait;
    ioport_level_t pin_state;

    TX_DISABLE;             // Turn off interrupts

    g_ioport_on_ioport.pinCfg( I2C_IO_PIN,(IOPORT_CFG_DRIVE_MID | IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_LEVEL_HIGH) );
    g_ioport_on_ioport.pinCfg( I2C_CLK_PIN,(IOPORT_CFG_DRIVE_MID | IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_LEVEL_HIGH) );
    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_LOW);

    // Wait for the I2C_RES_PIN goes LOW
    wait = 5000;     // Was 46000 but 5000 should be 10 milliseconds in 2 microsecond increments.
    do
    {    // if I2C_RES_PIN == 1, wait
        g_ioport_on_ioport.pinRead(I2C_RES_PIN, &pin_state);
        if(wait < 2)
        {
            //set_tris_e(0x33);
            g_ioport_on_ioport.pinDirectionSet(I2C_IO_PIN, IOPORT_DIRECTION_INPUT);
            g_ioport_on_ioport.pinDirectionSet(I2C_CLK_PIN, IOPORT_DIRECTION_INPUT);

            //output_high(I2C_CS_PIN);
            g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

            TX_RESTORE;             // turn interrupts back on
            return MSG_STATUS_TIMEOUT;    // time out error
        }

        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;
    } while(pin_state == IOPORT_LEVEL_HIGH);

    // Wait for the I2C_RES_PIN goes LOW
    wait = 500;   // This should be a 1 millisecond wait.
    do
    {    // if I2C_RES_PIN == 1, wait
        g_ioport_on_ioport.pinRead(I2C_RES_PIN, &pin_state);
        if(wait < 2)
        {
            //set_tris_e(0x33);
            g_ioport_on_ioport.pinDirectionSet(I2C_IO_PIN, IOPORT_DIRECTION_INPUT);
            g_ioport_on_ioport.pinDirectionSet(I2C_CLK_PIN, IOPORT_DIRECTION_INPUT);

            //output_high(I2C_CS_PIN);
            g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

            TX_RESTORE;             // turn interrupts back on
            return MSG_STATUS_TIMEOUT;    // time out error
        }

        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;
    } while(pin_state == IOPORT_LEVEL_LOW);

    //send data
    for(i = 0; i < pack_len; i++)
    {
        Write_I2C_Byte(i2c_pack[i]);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MICROSECONDS);  //delay_us(10);
    }

    g_ioport_on_ioport.pinWrite(I2C_IO_PIN, IOPORT_LEVEL_HIGH);
    //R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
    g_ioport_on_ioport.pinWrite(I2C_CLK_PIN, IOPORT_LEVEL_HIGH);
    //R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);

    g_ioport_on_ioport.pinDirectionSet(I2C_IO_PIN, IOPORT_DIRECTION_INPUT);
    g_ioport_on_ioport.pinDirectionSet(I2C_CLK_PIN, IOPORT_DIRECTION_INPUT);

    // Wait for the I2C_RES_PIN goes LOW
    wait = 500;   // This should be a 1 millisecond wait.
    do
    {
        g_ioport_on_ioport.pinRead(I2C_RES_PIN, &pin_state);
        if(wait < 2)
        {
            g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

            TX_RESTORE;                     // turn interrupts back on
            return MSG_STATUS_TIMEOUT;      // time out error
        }

        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);   //add
        wait--;
    } while(pin_state == IOPORT_LEVEL_HIGH);

    // If this far, we may as well set the CS pin high.
    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

    return MSG_OK;
}

//******************************************************************************
// Function: Read_I2C_Byte
// Description: Read a byte into the passed parameter
//      Wait for clock to go high then low. Data is available on High-to-low transition.
// Returns: MSG_OK if byte is read.
//      else MSG_STATUS_TIMEOUT if clock input did not change in time.
//******************************************************************************

static uint8_t Read_I2C_Byte(uint8_t *readByte)
{
    uint8_t i, wait, myData;
    ioport_level_t pin_state;

    *readByte = 0;
    myData = 0;     // this is stored locally until all 8 bits are read, the transferred back to the calling procedure.

    for(i = 0; i < 8; i++)
    {
        // Wait for clock to go high.
        wait = 200; //max waiting about 200 us
        do
        {
            g_ioport_on_ioport.pinRead(I2C_CLK_PIN, &pin_state);        // Get Port state
            R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);       // This about 1 microsecond
            wait--;
            if(wait < 1)
                return MSG_STATUS_TIMEOUT;
        } while(pin_state == IOPORT_LEVEL_LOW);

        // Wait for Clock to go low.
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
        wait = 200; //max waiting about 200 us
        do
        {
            g_ioport_on_ioport.pinRead(I2C_CLK_PIN, &pin_state);
            R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
            wait--;
            if(wait < 1)
                return MSG_STATUS_TIMEOUT;    // time out
        } while(pin_state == IOPORT_LEVEL_HIGH);    //input(I2C_CLK_PIN) == 1

        g_ioport_on_ioport.pinRead(I2C_IO_PIN, &pin_state);
        if(pin_state == IOPORT_LEVEL_HIGH)
            myData = ( (uint8_t)(myData<<1)|0x01 );
        else
            myData = ( (uint8_t)(myData<<1)&0xfe );
    }

    *readByte = myData;     // returned process byte

    return MSG_OK;
}

//******************************************************************************

//******************************************************************************

static uint8_t Read_I2C_Package(uint8_t *responseMsg)
{
    uint8_t i, msgLength, myByte;
    uint16_t wait;
    ioport_level_t pin_state;
    uint8_t myCS;

    i2c_data[0] = 0;

    // Wait for the RES line to go LOW
    wait = 2500;       // wait about 50 milliseconds.
    do
    {    // if I2C_RES_PIN == 0, can not receive package
        g_ioport_on_ioport.pinRead(I2C_RES_PIN, &pin_state);
        if(wait == 0)
        {
            TX_RESTORE;
            return MSG_STATUS_TIMEOUT;    // time out error
        }
        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;
    } while(pin_state == IOPORT_LEVEL_HIGH);

    // We need the CLK and DATA lines to be HIGH at this point
    g_ioport_on_ioport.pinRead(I2C_CLK_PIN, &pin_state);
    if(pin_state == IOPORT_LEVEL_LOW)
    {
        TX_RESTORE;
        return MSG_STATUS_TIMEOUT;
    }
    g_ioport_on_ioport.pinRead(I2C_IO_PIN, &pin_state);
    if(pin_state == IOPORT_LEVEL_LOW)
    {
        TX_RESTORE;
        return MSG_STATUS_TIMEOUT;
    }

    // First byte is the message length.
    if(Read_I2C_Byte(&msgLength) != MSG_OK)
    {
        TX_RESTORE;
        return MSG_INVALID_FORMAT;
    }
    responseMsg[0] = msgLength;

    // Read the rest of the bytes.
    for(i = 0; i < msgLength-1; i++)
    {
        myByte = 0;
        if(Read_I2C_Byte(&myByte) != MSG_OK)
        {
            TX_RESTORE;
            return MSG_INVALID_FORMAT;
        }
        else
        {
            responseMsg[i+1] = myByte;
        }
    }

    TX_RESTORE;             // Turn on ThreadX interrupts

    // Determine if Checksum is correct and return error if not.
    myCS = 0;
    for(i = 0; i < msgLength-2; i++)
    {
        myCS = (uint8_t)(responseMsg[i+1] + myCS);
    }
    if (myCS != responseMsg[msgLength])
        return MSG_INVALID_FORMAT;

    return MSG_OK;
}

//******************************************************************************
//
//*******************************************************************************/

uint8_t get_PROP_version(void)
{

   uint8_t s_dat[3] = {3, 0x34, 0x37};
   uint16_t wait;
   ioport_level_t pin_state;
   uint8_t myVersionData[10];

   g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);  //output_high(I2C_CS_PIN);

    wait = 20; // GC 53000;  //520ms
    do
    {    // if I2C_RES_PIN == 0, can not send package
        g_ioport_on_ioport.pinRead(I2C_RES_PIN, &pin_state);
        if (--wait < 2)
        {
            return 1;    // time out error
        }
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
    } while(pin_state == IOPORT_LEVEL_LOW);

    if( Send_I2C_Package(s_dat, 3) )
        return 1;

    // now wait and check response
    if(Read_I2C_Package(myVersionData) == MSG_OK)
    {
        if(myVersionData[0]!=6)
            return MSG_INVALID_FORMAT;     // length error
        if(myVersionData[1]!=0x74)
            return MSG_INVALID_FORMAT;  //cmd error or setup fail (NACK)
        if( myVersionData[5] != (uint8_t)(myVersionData[0]+myVersionData[1]+myVersionData[2]+myVersionData[3]+myVersionData[4]) )
            return MSG_INVALID_FORMAT;     // crc error
        g_HA_MajorVersion = myVersionData[2];
        g_HA_MinorVersion = myVersionData[3];
        g_HA_BuildVersion = myVersionData[4];

        return MSG_OK;
    }

   return MSG_INVALID_FORMAT;      // error
}

//******************************************************************************
// Function: ExecuteHearBeat(void)
//
// Description: This function send the Heart Beat to the ASL110 Head Array and
//      listens for a response.
//      If successful, the OK status is sent to the main task.
//      If Not successful, an event is sent to the main task.
//
//******************************************************************************

uint8_t ExecuteHeartBeat(void)
{
    uint8_t HB_Message[4];
    uint8_t cs;
    uint8_t msgStatus;
    uint8_t HB_Response[4];
    HHP_HA_MSG_STRUCT HeadArrayMsg;

    HB_Message[0] = 0x04;     // msg length
    HB_Message[1] = HHP_HA_HEART_BEAT;
    HB_Message[2] = ++g_HeartBeatCounter;
    cs = CalculateChecksum(HB_Message, sizeof (HB_Message)-1);
    HB_Message[3] = cs;
    msgStatus = Send_I2C_Package(HB_Message, sizeof(HB_Message));

    if (msgStatus == MSG_OK)
    {
        msgStatus = Read_I2C_Package(HB_Response);
#ifdef FORCE_OK_FOR_GUI_DEBUGGING
        HB_Response[3] = g_myMode;
#endif
    }

    // Prepare and send Heart Message to GUI.
    HeadArrayMsg.HeartBeatMsg.m_HB_OK = false;
    HeadArrayMsg.HeartBeatMsg.HB_Count = g_HeartBeatCounter;
    HeadArrayMsg.HeartBeatMsg.m_ActiveMode = HB_Response[3];
    HeadArrayMsg.m_MsgType = HHP_HA_HEART_BEAT;
    if (msgStatus == MSG_OK)
        HeadArrayMsg.HeartBeatMsg.m_HB_OK = true;
    else
        HeadArrayMsg.HeartBeatMsg.m_HB_OK = false;
    ++HeadArrayMsg.HeartBeatMsg.HB_Count;

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
    if (HeadArrayMsg.HeartBeatMsg.HB_Count > 20)
    {
        g_HeartBeatCounter = 20;
        HeadArrayMsg.HeartBeatMsg.m_HB_OK = true;
    }
#endif

    // Send message to.... let's say... the GUI task.
    tx_queue_send(&q_HeadArrayCommunicationQueue, &HeadArrayMsg, TX_NO_WAIT);

//    qStatus = tx_queue_send(&q_HeadArrayCommunicationQueue, &HeadArrayMsg, TX_NO_WAIT);
//    if (qStatus == TX_SUCCESS)
//        g_ioport.p_api->pinWrite(GRNLED_PORT, IOPORT_LEVEL_LOW);        // Turn on LED

    return msgStatus;
}

//******************************************************************************
// Function: Process_GUI_Messages
// Description: This function processes messages sent by the GUI.
//
//******************************************************************************

uint32_t Process_GUI_Messages (GUI_MSG_STRUCT GUI_Msg)
{
    uint32_t msgSent = true;
    uint8_t HA_Msg[16];
    uint8_t msgStatus, cs;
    uint8_t HB_Response[16];

    switch (GUI_Msg.m_MsgType)
    {
        case HHP_HA_PAD_ASSIGMENT_GET:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_PAD_ASSIGMENT_GET;
            HA_Msg[2] = (uint8_t) GUI_Msg.PadAssignmentRequestMsg.m_PhysicalPadNumber;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[3] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            // Regardless of what happens above.
            if (msgStatus != MSG_OK)
            {
                SendPadAssignmentResponse ((char)GUI_Msg.PadAssignmentRequestMsg.m_PhysicalPadNumber, myPadDirection, &q_HeadArrayCommunicationQueue);
                switch (myPadDirection)
                {
                    case 'L': myPadDirection = 'R'; break;
                    case 'R': myPadDirection = 'F'; break;
                    case 'F': myPadDirection = 'O'; break;
                    case 'O': myPadDirection = 'L'; break;
                    default: myPadDirection = 'L'; break;
                }
            }
#endif
        case HHP_HA_MODE_CHANGE_SET:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_MODE_CHANGE_SET;
            HA_Msg[2] = (uint8_t) GUI_Msg.ModeChangeMsg.m_Mode;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[3] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            g_myMode = (uint8_t) GUI_Msg.ModeChangeMsg.m_Mode;
#endif
            break;

        default:
            msgSent = false;
            break;
    } // end switch

    return msgSent;
}

//******************************************************************************
// Function: HeadArray_CommunicationThread_entry
//  Created by Synergy software
// Description: This is the thread to handle communication to the ASL110 Head Array
//
//******************************************************************************

void HeadArray_CommunicationThread_entry(void)
{
    GUI_MSG_STRUCT GUI_Msg;
    uint32_t msgSent;
    ULONG numMsgs;

    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

    while (1)
    {
        // Process requests from the GUI.
        tx_queue_info_get (&g_GUI_queue, NULL, &numMsgs, NULL, NULL, NULL, NULL);
        if (numMsgs)
        {
            tx_queue_receive (&g_GUI_queue, &GUI_Msg, TX_NO_WAIT);
            msgSent = Process_GUI_Messages (GUI_Msg);
        }
        else
            msgSent = false;

        if (msgSent == false)
            ExecuteHeartBeat();

        tx_thread_sleep (10);
    }
}


