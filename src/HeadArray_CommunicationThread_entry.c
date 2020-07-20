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

//#define FORCE_OK_FOR_GUI_DEBUGGING      // comment this out to run "non-debug" code.

//#define USE_ORIGINAL_WIRING

#ifdef USE_ORIGINAL_WIRING // This uses the wire harness with Brown, Blue, White, Red, Black, Green wires
#define I2C_CS_PIN      IOPORT_PORT_05_PIN_01   //IOPORT_PORT_04_PIN_10
#define I2C_RES_PIN     IOPORT_PORT_05_PIN_02   //IOPORT_PORT_04_PIN_11
#define I2C_IO_PIN      IOPORT_PORT_05_PIN_03   //IOPORT_PORT_04_PIN_12
#define I2C_CLK_PIN     IOPORT_PORT_05_PIN_04   //IOPORT_PORT_04_PIN_13
#else   // This uses the wire harness with the Yellow, Pink, White, Gray, Brown, Green wires, so called "Standard" wire harness.
#define I2C_CS_PIN      IOPORT_PORT_05_PIN_02
#define I2C_RES_PIN     IOPORT_PORT_05_PIN_01   //IOPORT_PORT_04_PIN_11
#define I2C_IO_PIN      IOPORT_PORT_05_PIN_04   //IOPORT_PORT_04_PIN_12
#define I2C_CLK_PIN     IOPORT_PORT_05_PIN_03   //IOPORT_PORT_04_PIN_13
#endif


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
uint8_t GetPadData(void);

//******************************************************************************
// Global Variables
//******************************************************************************

uint8_t g_HeartBeatCounter = 0;
uint8_t g_GetAllPadData = false;
uint8_t g_GetDataActive = 0;
PHYSICAL_PAD_ENUM g_ActivePadID = INVALID_PAD;

//****************************************************************************
// External References
//****************************************************************************

extern uint8_t g_HA_Version_Major, g_HA_Version_Minor, g_HA_Version_Build, g_HA_EEPROM_Version;

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
PAD_DIRECTION_ENUM myPadDirection[] = {LEFT_DIRECTION, RIGHT_DIRECTION, FORWARD_DIRECTION};
FEATURE_ID_ENUM g_myMode = POWER_ONOFF_ID;
uint8_t g_HeadArray_Status;
PAD_TYPE_ENUM g_MyPadTypes[] = {PROPORTIONAL_PADTYPE, PROPORTIONAL_PADTYPE, PROPORTIONAL_PADTYPE};
uint16_t g_RawData, g_DriveDemand;
uint8_t g_TimeoutValue;            // 1.5 seconds in 100 milli second increments.
uint8_t g_ActiveFeatureSet = 0x1f;    // "1f" is all features active plus Clicks on.
#endif

const int16_t gDEBUG_NeutralDAC_Constant = 2048;
int16_t gDEBUG_NeutralDAC_Value = 2020;
const int16_t gDEBUG_RangeValue = 410;


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

    // If this far, we may as well set the CS pin high.
    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

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
    for(i = 0; i < msgLength-1; i++)
    {
        myCS = (uint8_t)(responseMsg[i] + myCS);
    }
    if (myCS != responseMsg[msgLength-1])
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
    uint8_t HB_Message[16];
    uint8_t cs;
    uint8_t msgStatus;
    uint8_t HB_Response[16];
    HHP_HA_MSG_STRUCT HeadArrayMsg;

    HB_Message[0] = 0x04;     // msg length
    HB_Message[1] = HHP_HA_HEART_BEAT;
    HB_Message[2] = ++g_HeartBeatCounter;
    cs = CalculateChecksum(HB_Message, (uint8_t) (HB_Message[0]-1));
    HB_Message[HB_Message[0]-1] = cs;
    msgStatus = Send_I2C_Package(HB_Message, HB_Message[0]);

    if (msgStatus == MSG_OK)
    {
        msgStatus = Read_I2C_Package(HB_Response);
    }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
    HB_Response[1] = HB_Message[2];     // Heart Beat incrementing value.
    HB_Response[2] = (uint8_t) TranslateFeature_EnumToChar (g_myMode);
    HB_Response[3] = g_HeadArray_Status;
#endif

    // Prepare and send Heart Message to GUI.
    HeadArrayMsg.HeartBeatMsg.m_HB_OK = false;
    HeadArrayMsg.HeartBeatMsg.m_HB_Count = HB_Response[1];
    HeadArrayMsg.HeartBeatMsg.m_ActiveMode = TranslateFeature_CharToEnum ((char) HB_Response[2]);// Translate the Active Feature from COMM protocol (1-based) to GUI Array position (0-based).
    HeadArrayMsg.HeartBeatMsg.m_HA_Status = HB_Response[3];

    HeadArrayMsg.m_MsgType = HHP_HA_HEART_BEAT;
    if (msgStatus == MSG_OK)
        HeadArrayMsg.HeartBeatMsg.m_HB_OK = true;
    else
        HeadArrayMsg.HeartBeatMsg.m_HB_OK = false;
    ++HeadArrayMsg.HeartBeatMsg.m_HB_Count;

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
    if (HeadArrayMsg.HeartBeatMsg.m_HB_Count > 60)
    {
        g_HeartBeatCounter = 61;
        g_HeadArray_Status = 0x01;
        HeadArrayMsg.HeartBeatMsg.m_HA_Status = g_HeadArray_Status;
    }
    else if (HeadArrayMsg.HeartBeatMsg.m_HB_Count > 40)
    {
        //g_HeartBeatCounter = 45;
        g_HeadArray_Status = 0x21;
        HeadArrayMsg.HeartBeatMsg.m_HA_Status = g_HeadArray_Status;
    }
//    else if (HeadArrayMsg.HeartBeatMsg.m_HB_Count > 40)
//    {
//        //g_HeartBeatCounter = 40;
//        g_HeadArray_Status = 0x00;
//        HeadArrayMsg.HeartBeatMsg.m_HA_Status = g_HeadArray_Status;
//    }
    if (HeadArrayMsg.HeartBeatMsg.m_HB_Count > 20)
    {
        //g_HeartBeatCounter = 20;
        HeadArrayMsg.HeartBeatMsg.m_HB_OK = true;
    }
#endif

    // Send message to.... let's say... the GUI task.
    tx_queue_send(&q_COMM_to_GUI_Queue, &HeadArrayMsg, TX_NO_WAIT);

    return msgStatus;
}

//******************************************************************************
// Function: GetPadData
// Description: This function requests the Pad Data from the head array
//      and then passes it back to the GUI.
//
//******************************************************************************

uint8_t GetPadData(void)
{
    uint8_t HB_Message[16];
    uint8_t cs;
    uint8_t msgStatus;
    uint8_t HB_Response[16];
    HHP_HA_MSG_STRUCT HeadArrayMsg;

    HB_Message[0] = 0x04;     // msg length
    HB_Message[1] = HHP_HA_PAD_DATA_GET;
    HB_Message[2] = (uint8_t) TranslatePad_EnumToChar (g_ActivePadID);
    cs = CalculateChecksum(HB_Message, (uint8_t) (HB_Message[0]-1));
    HB_Message[3] = cs;
    msgStatus = Send_I2C_Package(HB_Message, HB_Message[0]);

    if (msgStatus == MSG_OK)
    {
        msgStatus = Read_I2C_Package(HB_Response);
    }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
    HB_Response[1] = (uint8_t) ((g_RawData >> 8) & 0xff);
    HB_Response[2] = (uint8_t) (g_RawData & 0xff);
    HB_Response[3] = (uint8_t) ((g_DriveDemand >> 8) & 0xff);
    HB_Response[4] = (uint8_t) (g_DriveDemand & 0xff);
    g_RawData = (uint16_t)(g_RawData + 5);
    if (g_DriveDemand > 100)
        g_DriveDemand = 0;
    if (g_RawData > 980)
        g_RawData = 0;

    msgStatus = MSG_OK;     // Fool it for now.
#endif

    // Prepare and send the Pad Data message to the GUI via the queue.
    HeadArrayMsg.m_MsgType = HHP_HA_PAD_DATA_GET;
    HeadArrayMsg.GetDataMsg.m_PadID = g_ActivePadID;
    HeadArrayMsg.GetDataMsg.m_RawData = (uint16_t) ((HB_Response[1] << 8) + HB_Response[2]);
    HeadArrayMsg.GetDataMsg.m_DriveDemand = (uint16_t) ((HB_Response[3] << 8) + HB_Response[4]);

    tx_queue_send(&q_COMM_to_GUI_Queue, &HeadArrayMsg, TX_NO_WAIT);

    // Determine next pad data to get.
    if (g_GetAllPadData)    // If the GUI requested All Pads, then advance to the next pad.
    {
        ++g_ActivePadID;
        if (g_ActivePadID == INVALID_PAD)
            g_ActivePadID = (PHYSICAL_PAD_ENUM) 0;  // Roll over beethoven.
    }

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
    uint8_t padType;
    uint8_t physicalPad;
    uint8_t padDirection;
    int16_t NeutralDAC_Constant, NeutralDAC_Value, RangeValue;

    switch (GUI_Msg.m_MsgType)
    {
        case HHP_HA_PAD_ASSIGMENT_GET:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_PAD_ASSIGMENT_GET;
            HA_Msg[2] = (uint8_t) TranslatePad_EnumToChar (GUI_Msg.PadAssignmentRequestMsg.m_PhysicalPadNumber);
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[3] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    if (HB_Response[1] != NAK)
                    {
                        physicalPad = GUI_Msg.PadAssignmentRequestMsg.m_PhysicalPadNumber;         // I have to use data from the Request and not the response.
                        padDirection = TranslatePadDirection_CharToEnum ((char) HB_Response[1]);
                        padType = TranslatePadType_CharToEnum ((char) HB_Response[2]);
                        SendPadGetResponse (physicalPad, padDirection, padType);
                    }
                }
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            // Regardless of what happens above.
            if (msgStatus != MSG_OK)
            {
                physicalPad = GUI_Msg.PadAssignmentRequestMsg.m_PhysicalPadNumber;
                SendPadGetResponse (physicalPad, myPadDirection[physicalPad], g_MyPadTypes[physicalPad]);
            }
#endif
            break;

        case HHP_HA_ACTIVE_FEATURE_SET:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_ACTIVE_FEATURE_SET;
            HA_Msg[2] = (uint8_t) TranslateFeature_EnumToChar (GUI_Msg.ModeChangeMsg.m_Mode);
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[3] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            g_myMode = (uint8_t) GUI_Msg.ModeChangeMsg.m_Mode;  // Set global data for debugging. Refer to Heart Beat msg processing.
#endif
            break;

        case HHP_HA_PAD_ASSIGMENT_SET:
            HA_Msg[0] = 0x06;     // msg length
            HA_Msg[1] = HHP_HA_PAD_ASSIGMENT_SET;
            HA_Msg[2] = (uint8_t) TranslatePad_EnumToChar (GUI_Msg.PadAssignmentSetMsg.m_PhysicalPadNumber);
            HA_Msg[3] = (uint8_t) TranslatePadDirection_EnumToChar (GUI_Msg.PadAssignmentSetMsg.m_LogicalDirection);
            HA_Msg[4] = (uint8_t) TranslatePadType_EnumToChar (GUI_Msg.PadAssignmentSetMsg.m_PadType);
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[5] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);  // This is either an ACK or NAK
            }
#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            physicalPad = GUI_Msg.PadAssignmentSetMsg.m_PhysicalPadNumber;
            myPadDirection[physicalPad] = GUI_Msg.PadAssignmentSetMsg.m_LogicalDirection;
            g_MyPadTypes[physicalPad] = GUI_Msg.PadAssignmentSetMsg.m_PadType;
#endif
            break;

        case HHP_HA_VERSION_GET:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_VERSION_GET;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[2] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                SendVersionToGUI (HB_Response[1], HB_Response[2], HB_Response[3], HB_Response[4]);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            if (msgStatus != MSG_OK)    // Just in case the message WAS received.
            {
                SendVersionToGUI (0,1,0,1);
            }
#endif
            break;

        case HHP_HA_PAD_DATA_GET:
            g_ActivePadID = GUI_Msg.GetDataMsg.m_PadID;          // Start with the Left Pad
            g_GetDataActive = GUI_Msg.GetDataMsg.m_Start;   // non0 = Start getting data, 0 = Stop getting data.
            if (g_ActivePadID == INVALID_PAD)
            {
                g_ActivePadID = (PHYSICAL_PAD_ENUM) 0;
                g_GetAllPadData = true;
            }
            else
                g_GetAllPadData = false;

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            g_RawData = 400;
            g_DriveDemand = 0;
#endif
            break;

        case HHP_HA_CALIBRATE_RANGE_GET:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_CALIBRATE_RANGE_GET;
            HA_Msg[2] = (uint8_t) TranslatePad_EnumToChar (GUI_Msg.GetCalibrationData.m_PadID);
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            if (msgStatus != MSG_OK)
            {
                HB_Response[1] = (2 >> 8) & 0xff;       // Min ADC
                HB_Response[2] = (2 & 0xff);
                HB_Response[3] = (1000 >> 8) & 0xff;    // Max ADC
                HB_Response[4] = (1000 & 0xff);
                HB_Response[5] = (5 >> 8) & 0xff;      // Min Threshold
                HB_Response[6] = (5 & 0xff);
                HB_Response[7] = (95 >> 8) & 0xff;     // Max Threshold
                HB_Response[8] = (95 & 0xff);
                msgStatus = MSG_OK;
            }
#endif
            if (msgStatus == MSG_OK)
            {
                SendCalDataResponse (GUI_Msg.GetCalibrationData.m_PadID,
                        (uint16_t) ((HB_Response[1]<<8) + HB_Response[2]),   // Min ADC
                        (uint16_t) ((HB_Response[3]<<8) + HB_Response[4]),   // Max ADC
                        (uint16_t) ((HB_Response[5]<<8) + HB_Response[6]),   // Min Threshold
                        (uint16_t) ((HB_Response[7]<<8) + HB_Response[8]));   // Max Threshold
            }
            break;

        case HHP_HA_CALIBRATE_RANGE_SET:
            HA_Msg[0] = 0x08;     // msg length
            HA_Msg[1] = HHP_HA_CALIBRATE_RANGE_SET;
            HA_Msg[2] = (uint8_t) TranslatePad_EnumToChar (GUI_Msg.SendCalibrationData.m_PadID);
            HA_Msg[3] = (uint8_t) ((GUI_Msg.SendCalibrationData.m_MinThreshold >> 8) & 0xff);
            HA_Msg[4] = (uint8_t) ((GUI_Msg.SendCalibrationData.m_MinThreshold & 0xff));
            HA_Msg[5] = (uint8_t) ((GUI_Msg.SendCalibrationData.m_MaxThreshold >> 8) & 0xff);
            HA_Msg[6] = (uint8_t) ((GUI_Msg.SendCalibrationData.m_MaxThreshold & 0xff));
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
            // Probably should process the NAK.
            break;

        case HHP_HA_CALIBRATE_START_CMD:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_CALIBRATE_START_CMD;
            HA_Msg[2] = (uint8_t) TranslatePad_EnumToChar (GUI_Msg.SendCalibrationData.m_PadID);
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
            // Probably should process the NAK.
            break;

        case HHP_HA_CALIBRATE_STOP_CMD:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_CALIBRATE_STOP_CMD;
            HA_Msg[2] = (uint8_t) TranslatePad_EnumToChar (GUI_Msg.SendCalibrationData.m_PadID);
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
            // Probably should process the NAK.
            break;

        case HHP_HA_FEATURE_GET:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_FEATURE_GET;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            if (msgStatus != MSG_OK)
            {
                msgStatus = MSG_OK;
                HB_Response[0] = HHP_HA_FEATURE_GET;
                HB_Response[1] = g_ActiveFeatureSet;   // All features enabled plus sound
                HB_Response[2] = g_TimeoutValue;     // 1.5 seconds.
            }
#endif
            if (msgStatus == MSG_OK)
            {
                SendFeatureGet (HB_Response[1], HB_Response[2]);
            }
            break;

        case HHP_HA_FEATURE_SET:
            HA_Msg[0] = 0x05;     // msg length
            HA_Msg[1] = HHP_HA_FEATURE_SET;
            HA_Msg[2] = GUI_Msg.SendFeatureActiveList.m_FeatureActiveList;
            HA_Msg[3] = GUI_Msg.SendFeatureActiveList.m_Timeout;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            if (msgStatus != MSG_OK)
            {
                g_ActiveFeatureSet = GUI_Msg.SendFeatureActiveList.m_FeatureActiveList;
                g_TimeoutValue = GUI_Msg.SendFeatureActiveList.m_Timeout;
                msgStatus = MSG_OK;
            }
#endif
            // Might want to consider processing the ACK/NAK
            break;

        case HHP_HA_NEUTRAL_DAC_GET:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_NEUTRAL_DAC_GET;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            if (msgStatus != MSG_OK)
            {
                g_ActiveFeatureSet = GUI_Msg.SendFeatureActiveList.m_FeatureActiveList;
                g_TimeoutValue = GUI_Msg.SendFeatureActiveList.m_Timeout;
                msgStatus = MSG_OK;
            }
#endif
//            HB_Response[1] = (uint8_t) (gDEBUG_NeutralDAC_Constant >> 8);
//            HB_Response[2] = (uint8_t) (gDEBUG_NeutralDAC_Constant & 0xff);
//            HB_Response[3] = (uint8_t) (gDEBUG_NeutralDAC_Value >> 8);
//            HB_Response[4] = (uint8_t) (gDEBUG_NeutralDAC_Value & 0xff);
//            HB_Response[5] = (uint8_t) (gDEBUG_RangeValue >> 8);
//            HB_Response[6] = (uint8_t) (gDEBUG_RangeValue & 0xff);

            if (msgStatus == MSG_OK)
            {
                NeutralDAC_Constant = (int16_t) ((HB_Response[1]<<8) + HB_Response[2]);
                NeutralDAC_Value = (int16_t) ((HB_Response[3]<<8) + HB_Response[4]);
                RangeValue = (int16_t) ((HB_Response[5]<<8) + HB_Response[6]);
                SendNeutralDAC_GetResponse (NeutralDAC_Constant, NeutralDAC_Value, RangeValue);
            }
            break;

        case HHP_HA_NEUTRAL_DAC_SET:
            HA_Msg[0] = 0x05;     // msg length
            HA_Msg[1] = HHP_HA_NEUTRAL_DAC_SET;
            HA_Msg[2] = (uint8_t) ((GUI_Msg.SendNeutralDAC_Setting.m_DAC_Setting >> 8) & 0xff);
            HA_Msg[3] = (uint8_t) ((GUI_Msg.SendNeutralDAC_Setting.m_DAC_Setting & 0xff));
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
            if (msgStatus != MSG_OK)
            {
                g_ActiveFeatureSet = GUI_Msg.SendFeatureActiveList.m_FeatureActiveList;
                g_TimeoutValue = GUI_Msg.SendFeatureActiveList.m_Timeout;
                msgStatus = MSG_OK;
            }
#endif
            // Debugging ONLY
            NeutralDAC_Value = GUI_Msg.SendNeutralDAC_Setting.m_DAC_Setting;
            break;

        case HHP_HA_SAVE_PARAMETERS_CMD:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_SAVE_PARAMETERS_CMD;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
            break;

        case HHP_HA_RESET_PARAMETERS_CMD:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_RESET_PARAMETERS_CMD;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);  // This is either an ACK or NAK. Ain't much to with either.
            }
            break;

        case HHP_HA_DRIVE_OFFSET_GET:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_DRIVE_OFFSET_GET;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (HB_Response[0] == 3)        // If we are processing an older firmware version, then only one value is returned.
                {                               // .. we will use it for all 3 pads. We're using the msg length to determine the contents.
                    SendDriveOffsetGetResponse (HB_Response[1], HB_Response[1], HB_Response[1]);
                }
                else    // If newer ASL110 firmware, then all 3 pad drive offsets are returned.
                {
                    SendDriveOffsetGetResponse (HB_Response[1], HB_Response[2], HB_Response[3]);
                }
            }
            break;

        case HHP_HA_DRIVE_OFFSET_SET:
            HA_Msg[1] = HHP_HA_DRIVE_OFFSET_SET;
            if (g_HA_EEPROM_Version <= 4)   // Older ASL110 firmware?
            {
                HA_Msg[0] = 0x04;     // msg length
                HA_Msg[2] = GUI_Msg.SendDriveOffset.m_CenterPad_DriveOffset;
            }
            else
            {
                HA_Msg[0] = 0x06;     // msg length
                HA_Msg[2] = GUI_Msg.SendDriveOffset.m_CenterPad_DriveOffset;
                HA_Msg[3] = GUI_Msg.SendDriveOffset.m_LeftPad_DriveOffset;
                HA_Msg[4] = GUI_Msg.SendDriveOffset.m_RightPad_DriveOffset;

            }
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
            }
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

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
    g_HeadArray_Status = 0x00;
    g_TimeoutValue = 15;
#endif

    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

    while (1)
    {
        // Process requests from the GUI.
        tx_queue_info_get (&g_GUI_to_COMM_queue, NULL, &numMsgs, NULL, NULL, NULL, NULL);
        if (numMsgs)
        {
            tx_queue_receive (&g_GUI_to_COMM_queue, &GUI_Msg, TX_NO_WAIT);
            msgSent = Process_GUI_Messages (GUI_Msg);
        }
        else // We got messages from the GUI to process.
        {
            if (g_GetDataActive)    // Are we expected to continuously get PAD data from the Head Array?
            {
                msgSent = GetPadData();
            }
            else
            {
                msgSent = false;
            }
        }

        if (msgSent == false)
            ExecuteHeartBeat();

        tx_thread_sleep (10);
    }
}


