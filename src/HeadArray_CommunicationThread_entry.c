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
#include "ASL165_System.h"
#include "DeviceInfo.h"

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

#define NACK_RESPONSE 0x15          // NACK response
#define MSG_OK  0                   // Indicates message as processed OK.
#define MSG_STATUS_TIMEOUT 1        // indicates message processing failed to receive proper status, i.e. NO I/O Line or CS
#define MSG_INVALID_FORMAT 2        // Indicates message was formatted improperly or invalid data.
#define MSG_NAK 3                   // Indicates that the Head Array did not like the message

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
uint8_t SendAttendantData (void);
uint32_t Request_SNP_Data(void);

//******************************************************************************
// Global Variables
//******************************************************************************

uint8_t g_HeartBeatCounter = 0;
uint8_t g_GetAllPadData = false;
uint8_t g_GetDataActive = 0;
bool g_SNP_CalibrationIsActive = false;
PHYSICAL_PAD_ENUM g_ActivePadID = END_OF_PAD_ENUM;
// The following are used with then Attendant Screen is active.
uint8_t g_AttendantActive = 0;
int8_t g_AttendantSpeedDemand = 0;
int8_t g_AttentantDirectionDemand = 0;
uint8_t g_PadSensorStatus = 0x00;         // Used to determine if the Pad Status has changed.

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
extern int8_t g_SNP_Nozzle_Value;

extern void StoreAuditorySettings (uint8_t*);

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

    // Check for NACK response.
    if (responseMsg[1] == NACK_RESPONSE)
    {
        if (responseMsg[0] == 3)    // Check to ensure it's just the NAK response and not a valid response
            return MSG_NAK;
    }

    return MSG_OK;
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
    if (g_HA_EEPROM_Version < 8)
        HB_Response[4] = 0; // Since this is older ASL110 firmware, clear the Sensor Status since it's garbage and causing excessive delays.
    HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus = HB_Response[4];   // Transfer the Sensor Status's.
    HeadArrayMsg.HeartBeatMsg.m_SubIndex = HB_Response[5];          // Transfer the Sub-Index
    HeadArrayMsg.HeartBeatMsg.m_ActiveDriverControl = HB_Response[6]; // Active Driver Control enum

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
        if (g_ActivePadID == END_OF_PAD_ENUM)
            g_ActivePadID = (PHYSICAL_PAD_ENUM) 0;  // Roll over beethoven.
    }

    return msgStatus;
}

//******************************************************************************
// Function: SendAttendantData
// Description: This function assembles a message to send the Attendant Information
//       and sends it to the Head Array.
//******************************************************************************

uint8_t SendAttendantData (void)
{
    uint8_t status, cs;
    uint8_t HA_Msg[16];
    uint8_t HB_Response[16];

    // Assemble message to send to the Head Array
    HA_Msg[0] = 0x07;     // msg length
    HA_Msg[1] = HHP_HA_ATTENDANT_CONTROLS_CMD;
    HA_Msg[2] = g_AttendantActive;
    HA_Msg[3] = (uint8_t) g_AttendantSpeedDemand;
    HA_Msg[4] = (uint8_t) g_AttentantDirectionDemand;
    HA_Msg[5] = 0;      // Heartbeat... TODO... change to send incremented value.
    cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
    HA_Msg[HA_Msg[0]-1] = cs;
    status = Send_I2C_Package(HA_Msg, HA_Msg[0]);
    if (status == MSG_OK)
    {
        status = Read_I2C_Package(HB_Response);
    }
    return status;
}

/******************************************************************************
 * This function gets the SNP data from the ION HUB and stores it in a global var.
 * @param GUI_Msg
 * @return
 */
uint32_t Request_SNP_Data(void)
{
    uint8_t HA_Msg[16];
    uint8_t cs;
    uint8_t msgStatus;
    uint8_t HB_Response[16];
    HHP_HA_MSG_STRUCT HeadArrayMsg;

    HA_Msg[0] = 0x03;     // msg length
    HA_Msg[1] = HHP_HA_SNP_GET_RAWDATA_CMD;
    cs = CalculateChecksum(HA_Msg, (uint8_t) (HA_Msg[0]-1));
    HA_Msg[HA_Msg[0]-1] = cs;
    msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);

    if (msgStatus == MSG_OK)
    {
        msgStatus = Read_I2C_Package(HB_Response);
        if (msgStatus == MSG_OK)
        {
            g_SNP_Nozzle_Value = (int8_t) HB_Response[2];
        }
    }


    // Prepare and send the Pad Data message to the GUI via the queue. We do this because it's the process that is
    // handling the Screen Updates.
    HeadArrayMsg.m_MsgType = HHP_HA_SNP_GET_RAWDATA_CMD;

    tx_queue_send(&q_COMM_to_GUI_Queue, &HeadArrayMsg, TX_NO_WAIT);

    return msgStatus;
}

//******************************************************************************
// Function: Process_GUI_Messages
// Description: This function processes messages sent by the GUI.
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
#ifdef FUSION
            HA_Msg[2] = (uint8_t) TranslateFeature_EnumToChar (GUI_Msg.ModeChangeMsg.m_Mode);
#else
            HA_Msg[2] = (uint8_t) (GUI_Msg.ModeChangeMsg.m_Mode);
#endif
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
            g_ActivePadID = GUI_Msg.GetDataMsg.m_PadID;     // Start with the Left Pad
            g_GetDataActive = GUI_Msg.GetDataMsg.m_Start;   // non0 = Start getting data, 0 = Stop getting data.
            if (g_ActivePadID == END_OF_PAD_ENUM)               // If INVALID_PAD, then we are doing diagnostics and we want to get data from all of the pads.
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
                HB_Response[3] = g
            }
#endif
            // We are going to determine if the returned message contains the 2nd byte of features.
            // If not, we are going to set a default.
            // We need to check for "0" because the EEPROM Version may not be processed yet.
            if ((g_HA_EEPROM_Version > 0) && (g_HA_EEPROM_Version < 6))
                HB_Response[3] = 0;     // Force NO features

            if (msgStatus == MSG_OK)
            {
                SendFeatureGet (HB_Response[1], HB_Response[2], HB_Response[3]);
            }
            break;

        case HHP_HA_FEATURE_SET:
            HA_Msg[0] = 0x05;     // msg length
            HA_Msg[1] = HHP_HA_FEATURE_SET;
            HA_Msg[2] = GUI_Msg.SendFeatureActiveList.m_FeatureActiveList;
            HA_Msg[3] = GUI_Msg.SendFeatureActiveList.m_Timeout;
            if (g_HA_EEPROM_Version >= 6)
            {
                HA_Msg[0] = 0x06;     // msg length
                HA_Msg[4] = GUI_Msg.SendFeatureActiveList.m_FeatureListByte2;
            }
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

        case HHP_HA_ATTENDANT_CONTROLS_CMD:
            // This is sent by the GUI when the Attendant changes between inactive and active.
            // It is also sent when the position of the Speed and Direction changes.

            // We are going to capture this information so we can send it down to the Head Array periodically.
            g_AttendantActive = GUI_Msg.SendAttendantControl.m_AttendantActive;
            g_AttendantSpeedDemand = GUI_Msg.SendAttendantControl.m_SpeedDemand;
            g_AttentantDirectionDemand = GUI_Msg.SendAttendantControl.m_DirectionDemand;

            // Assemble message to send to the Head Array
            msgStatus = SendAttendantData();
            break;

        case HHP_HA_ATTENDANT_SETTINGS_GET:
            HA_Msg[1] = HHP_HA_ATTENDANT_SETTINGS_GET;
            if (g_HA_EEPROM_Version >= 7)   // Older ASL110 firmware?
            {
                HA_Msg[0] = 0x03;     // msg length
                cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
                HA_Msg[HA_Msg[0]-1] = cs;
                msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
                if (msgStatus == MSG_OK)
                {
                    msgStatus = Read_I2C_Package(HB_Response);
                    if (msgStatus == MSG_OK)
                    {
                        g_AttendantSettings = HB_Response[1];
                        g_AttendantTimeout = HB_Response[2];
                    }
                }
            }
            else
            {
                msgSent = false;
            }
            break;

        case HHP_HA_ATTENDANT_SETTINGS_SET:
            if (g_HA_EEPROM_Version >= 7)   // Older ASL110 firmware?
            {
                HA_Msg[0] = 0x05;     // msg length
                HA_Msg[1] = HHP_HA_ATTENDANT_SETTINGS_SET;
                HA_Msg[2] = g_AttendantSettings;
                HA_Msg[3] = g_AttendantTimeout;
                cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
                HA_Msg[HA_Msg[0]-1] = cs;
                msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
                if (msgStatus == MSG_OK)
                {
                    msgStatus = Read_I2C_Package(HB_Response);
                }
            }
            else
            {
                msgSent = false;
            }
            break;

        case HHP_HA_WHO_ARE_YOU_CMD:
            HA_Msg[0] = 0x03;     // msg length
            HA_Msg[1] = HHP_HA_WHO_ARE_YOU_CMD;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    g_WhoAmi = 0x01;
                }
            }
            break;

        /*----------------------------------
         * Request Drive Control Enable / Disable msg
         */
        case HHP_HA_GET_DRIVER_CONTROL_ENABLE:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_GET_DRIVER_CONTROL_ENABLE;
            HA_Msg[2] = GUI_Msg.DriverControlEnable.m_DeviceID;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    SendDriverEnableToGUI (HB_Response[1], HB_Response[2]);
                }
            }
            break;

        /*----------------------------------
         * Send Drive Control Enable / Disable msg
         */
        case HHP_HA_SET_DRIVER_CONTROL_ENABLE:
            HA_Msg[0] = 0x05;     // msg length
            HA_Msg[1] = HHP_HA_SET_DRIVER_CONTROL_ENABLE;
            HA_Msg[2] = GUI_Msg.DriverControlEnable.m_DeviceID;
            HA_Msg[3] = GUI_Msg.DriverControlEnable.m_Enabled;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ;
                }
            }
            break;

        /*-----------------------------------------
         * Get ION Driver Control 4 Pad Assignments.
         */
        case HHP_HA_GET_DRIVER_CONTROL_INPUT_ASSIGNMENT:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_GET_DRIVER_CONTROL_INPUT_ASSIGNMENT;
            HA_Msg[2] = GUI_Msg.ION_GetPadAssignment.m_DeviceID;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ProcessDriveControlPadAssignemnt_Response (HB_Response[1], HB_Response[2], HB_Response[3], HB_Response[4], HB_Response[5], HB_Response[6]);
                }
//                else // debugging
//                {
//                    ProcessDriveControlPadAssignemnt_Response (TWO_SWITCH_DEVICE_IDX, PAD_DIRECTION_LEFT, PAD_DIRECTION_OFF, PAD_DIRECTION_REVERSE, PAD_DIRECTION_OFF);
//                }
            }
            break;

        case HHP_HA_SET_DRIVER_CONTROL_INPUT_ASSIGNMENT:
            HA_Msg[0] = 0x09;     // msg length
            HA_Msg[1] = HHP_HA_SET_DRIVER_CONTROL_INPUT_ASSIGNMENT;
            HA_Msg[2] = GUI_Msg.ION_SetPadAssignment.m_DeviceID;
            HA_Msg[3] = GUI_Msg.ION_SetPadAssignment.m_ForwardPadAssignment;
            HA_Msg[4] = GUI_Msg.ION_SetPadAssignment.m_LeftPadAssignemnt;
            HA_Msg[5] = GUI_Msg.ION_SetPadAssignment.m_RightPadAssignment;
            HA_Msg[6] = GUI_Msg.ION_SetPadAssignment.m_ReversePadAssignment;
            HA_Msg[7] = GUI_Msg.ION_SetPadAssignment.m_ModeSwitchSchema;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ;
                }
            }
            break;

        case HHP_HA_BLUETOOTH_SETUP_GET_CMD:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_BLUETOOTH_SETUP_GET_CMD;
            HA_Msg[2] = GUI_Msg.Send_BT_DeviceDefinition.m_SlotNumber;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    // parametes are BT Device Type [2], Slot Number [1], Color [3], status [4];
                    Send_Get_BT_DeviceDefinitions_Response (HB_Response[1], HB_Response[2], HB_Response[3], HB_Response[4]);
                }
            }
            break;

        case HHP_HA_BLUETOOTH_SETUP_SET_CMD:
            HA_Msg[0] = 0x07;     // msg length
            HA_Msg[1] = HHP_HA_BLUETOOTH_SETUP_SET_CMD;
            HA_Msg[2] = GUI_Msg.Send_BT_DeviceDefinition.m_SlotNumber;
            HA_Msg[3] = GUI_Msg.Send_BT_DeviceDefinition.m_DeviceIdenfication;
            HA_Msg[4] = GUI_Msg.Send_BT_DeviceDefinition.m_Color;
            HA_Msg[5] = GUI_Msg.Send_BT_DeviceDefinition.m_Status;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ; // Process response and do what?!
                }
            }
            break;

        case HHP_HA_DIAGNOSTIC_CMD:
            HA_Msg[0] = 0x05;     // msg length
            HA_Msg[1] = HHP_HA_DIAGNOSTIC_CMD;
            HA_Msg[2] = 0;
            HA_Msg[3] = GUI_Msg.DriverControlEnable.m_Enabled;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ; // Process response and do what?!
                }
            }
            break;

        case HHP_HA_SNP_THRESHOLDS_GET:
            HA_Msg[0] = 0x04;     // msg length
            HA_Msg[1] = HHP_HA_SNP_THRESHOLDS_GET;
            HA_Msg[2] = GUI_Msg.ION_SNP_Calibration_CMD_s.m_DeviceID;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    // Save the Thresholds in the Device Settings.
                    g_DeviceSettings[HA_Msg[2]].m_PadInfo[RIGHT_PAD].m_SNP_Threshold = (int8_t) HB_Response[1];
                    g_DeviceSettings[HA_Msg[2]].m_PadInfo[REVERSE_PAD].m_SNP_Threshold = (int8_t) HB_Response[2];
                    g_DeviceSettings[HA_Msg[2]].m_PadInfo[LEFT_PAD].m_SNP_Threshold = (int8_t) HB_Response[3];
                    g_DeviceSettings[HA_Msg[2]].m_PadInfo[CENTER_PAD].m_SNP_Threshold = (int8_t) HB_Response[4];
                }
            }
            break;

        case HHP_HA_SNP_THRESHOLDS_SET:
            HA_Msg[0] = 0x08;     // msg length from LENGTH to CHECKSUM, inclusive
            HA_Msg[1] = HHP_HA_SNP_THRESHOLDS_SET;
            HA_Msg[2] = GUI_Msg.ION_SNP_Calibration_CMD_s.m_DeviceID;
            HA_Msg[3] = (uint8_t) GUI_Msg.ION_SNP_Calibration_CMD_s.m_SoftSip;
            HA_Msg[4] = (uint8_t) GUI_Msg.ION_SNP_Calibration_CMD_s.m_HardSip;
            HA_Msg[5] = (uint8_t) GUI_Msg.ION_SNP_Calibration_CMD_s.m_SoftPuff;
            HA_Msg[6] = (uint8_t) GUI_Msg.ION_SNP_Calibration_CMD_s.m_HardPuff;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ;
                }
            }
            break;

        case HHP_HA_AUDITORY_SETTINGS_GET_CMD:
            HA_Msg[0] = 0x03;     // msg length from LENGTH to CHECKSUM, inclusive
            HA_Msg[1] = HHP_HA_AUDITORY_SETTINGS_GET_CMD;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    Send_Auditory_Setting_ToGUI (&HB_Response[1]);
                }
            }
            break;

        case HHP_HA_AUDITORY_SETTINGS_SET_CMD:
            HA_Msg[0] = 0x09;     // msg length from LENGTH to CHECKSUM, inclusive
            HA_Msg[1] = HHP_HA_AUDITORY_SETTINGS_SET_CMD;
            HA_Msg[2] = GUI_Msg.ION_Auditory_Struct.m_AuditorySetting;
            HA_Msg[3] = GUI_Msg.ION_Auditory_Struct.m_Volume;
            HA_Msg[4] = GUI_Msg.ION_Auditory_Struct.m_AP1;
            HA_Msg[5] = GUI_Msg.ION_Auditory_Struct.m_AP2;
            HA_Msg[6] = GUI_Msg.ION_Auditory_Struct.m_AP3;
            HA_Msg[7] = GUI_Msg.ION_Auditory_Struct.m_AP4;
            cs = CalculateChecksum(HA_Msg, (uint8_t)(HA_Msg[0]-1));
            HA_Msg[HA_Msg[0]-1] = cs;
            msgStatus = Send_I2C_Package(HA_Msg, HA_Msg[0]);
            if (msgStatus == MSG_OK)
            {
                msgStatus = Read_I2C_Package(HB_Response);
                if (msgStatus == MSG_OK)
                {
                    ;
                }
            }
            break;

        default:
            msgSent = false;
            break;
    } // end switch

    return msgSent;
}

//*************************************************************************************
// Function: Process_Communication_Msgs
//
//*************************************************************************************

void ProcessIncomingMessages ()
{
    GX_EVENT gxe;
    uint32_t qStatus;
    HHP_HA_MSG_STRUCT HeadArrayMsg;
    ULONG numMsgs;
    PHYSICAL_PAD_ENUM myPad;
//    uint8_t temp8;

    // Is there anything to process, i.e. Is there anything from the Head Array Comm Process?
    tx_queue_info_get (&q_COMM_to_GUI_Queue, NULL, &numMsgs, NULL, NULL, NULL, NULL);
    if (numMsgs == 0)
    {
        return;
    }

    // Get message or return if error.
    qStatus = tx_queue_receive (&q_COMM_to_GUI_Queue, &HeadArrayMsg, TX_NO_WAIT);
    if (qStatus != TX_SUCCESS)
        return;

    switch (HeadArrayMsg.m_MsgType)
    {
        case HHP_HA_HEART_BEAT:
            if (HeadArrayMsg.HeartBeatMsg.m_HB_OK)
            {
                // Store the Pad Status
                g_PadSettings[LEFT_PAD].m_PadStatus = (HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x04) ? PAD_STATUS_OK : PAD_STATUS_ERROR;
                g_PadSettings[RIGHT_PAD].m_PadStatus = (HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x08) ? PAD_STATUS_OK : PAD_STATUS_ERROR;
                g_PadSettings[CENTER_PAD].m_PadStatus = (HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x10) ? PAD_STATUS_OK : PAD_STATUS_ERROR;

                // Store the Pad Sensor Status (D0, D1 = Center Pad; D2 and D3 = Right Pad; D4 and D5 = Left Pad
                g_PadSettings[CENTER_PAD].m_PadSensorStatus = HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus & 0x03;     // Only store D0 and D1
                g_PadSettings[RIGHT_PAD].m_PadSensorStatus = (HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus >> 2) & 0x03; // store only d2 and d3
                g_PadSettings[LEFT_PAD].m_PadSensorStatus = (HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus >> 4) & 0x03; // store only d4 and d5
                g_PadSettings[REVERSE_PAD].m_PadSensorStatus = (HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus >> 6) & 0x03; // store only D6 and D7

                // This will update the screen's depiction of the Driver Control.
                if ((g_PadSensorStatus != HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus)
                    || (g_HeadArrayStatus1 != HeadArrayMsg.HeartBeatMsg.m_HA_Status))
                {
                    g_HeadArrayStatus1 = HeadArrayMsg.HeartBeatMsg.m_HA_Status;
                    g_PadSensorStatus = HeadArrayMsg.HeartBeatMsg.m_HA_SensorStatus;
                    gxe.gx_event_type = GX_EVENT_REDRAW;
                    gxe.gx_event_sender = GX_ID_NONE;
                    gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                    gxe.gx_event_display_handle = 0;
                    gx_system_event_send(&gxe);
                }

                if (g_ActiveScreen->gx_widget_id == STARTUP_SPLASH_SCREEN_ID)
                {
                    AdjustActiveFeaturePositions ((FEATURE_ID_ENUM)(HeadArrayMsg.HeartBeatMsg.m_ActiveMode));   // This function also store "g_ActiveFeature" if appropriate.
                    gxe.gx_event_type = GX_SIGNAL (HB_OK_ID, GX_EVENT_CLICKED);
                    gxe.gx_event_sender = GX_ID_NONE;
                    gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                    gxe.gx_event_display_handle = 0;
                    gx_system_event_send(&gxe);
                }
                else if (g_ActiveScreen->gx_widget_id == MAIN_USER_SCREEN_ID)
                {
                    if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x01)   // Bit0 = 1 if Head Array in "Ready", Power On mode.
                    {
                        if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x20) == 0x20)// Out of Neutral?
                        {
                            gxe.gx_event_type = GX_SIGNAL (HB_OON_ID, GX_EVENT_CLICKED);
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                        else if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x1c) != 0x1c)
                        {   // Check if any pad is in error which is 0x04, 0x08 or 0x10, OK when "1"
                            gxe.gx_event_type = GX_SIGNAL (PAD_ERROR_ID, GX_EVENT_CLICKED);
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                        // This triggers redrawing the main screen if the mode changes.
                        else if (g_ActiveFeature != HeadArrayMsg.HeartBeatMsg.m_ActiveMode)
                        {
                            AdjustActiveFeaturePositions ((FEATURE_ID_ENUM)(HeadArrayMsg.HeartBeatMsg.m_ActiveMode));   // This function also store "g_ActiveFeature" if appropriate.
                            gxe.gx_event_type = GX_EVENT_REDRAW;
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                        // Determine if the Driver Control has changed. If so, redraw the User Main Screen.
                        else if (g_ActiveDriverControlIdx != HeadArrayMsg.HeartBeatMsg.m_ActiveDriverControl)
                        {
                            g_ActiveDriverControlIdx = HeadArrayMsg.HeartBeatMsg.m_ActiveDriverControl;
                            gp_ActiveDriverControl = &g_DeviceSettings[g_ActiveDriverControlIdx];
                            gxe.gx_event_type = GX_EVENT_REDRAW;
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                        // Let's see if the Bluetooth Feature is selected in the User Main Menu
                        // ... and a submenu is requested.
                        if (g_ActiveFeature == BLUETOOTH_FEATURE_HB_ID)
                        {
                            if (g_BluetoothSubIndex != HeadArrayMsg.HeartBeatMsg.m_SubIndex)
                            {
                                g_BluetoothSubIndex = (int8_t) HeadArrayMsg.HeartBeatMsg.m_SubIndex;
                                gxe.gx_event_type = GX_SIGNAL (GOTO_BT_SUBMENU_ID, GX_EVENT_CLICKED);
                                gxe.gx_event_sender = GX_ID_NONE;
                                gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                                gxe.gx_event_display_handle = 0;
                                gx_system_event_send(&gxe);
                            }
                        }
                    }
                    else if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x00)// Bit 0 = 0; means Head Array in Power Off, Idle Mode, we are recommending to change screens.
                    {
                        gxe.gx_event_type = GX_SIGNAL (POWER_OFF_ID, GX_EVENT_CLICKED);
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == READY_SCREEN_ID)
                {
                    if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x01)   // Bit0 = 1 if Head Array in "Ready", Power On mode.
                    {
                        gxe.gx_event_type = GX_SIGNAL (POWER_ON_ID, GX_EVENT_CLICKED);
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == OON_SCREEN_ID)
                {
                    if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x20) == 0x00)   // We are OK to go... Neutral Test passed.
                    {
                        gxe.gx_event_type = GX_SIGNAL (OON_OK_BTN_ID, GX_EVENT_CLICKED);
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == ION_BT_DEVICE_SELECT_SCREEN_ID)
                {
                    if (g_BluetoothSubIndex != (int8_t) HeadArrayMsg.HeartBeatMsg.m_SubIndex)
                    {
                        //AdjustActiveFeaturePositions ((FEATURE_ID_ENUM)(HeadArrayMsg.HeartBeatMsg.m_ActiveMode));   // This function also store "g_ActiveFeature" if appropriate.
                        g_BluetoothSubIndex = (int8_t) HeadArrayMsg.HeartBeatMsg.m_SubIndex;
                        gxe.gx_event_type = GX_SIGNAL (BT_SUBMENU_CHANGED_ID, GX_EVENT_CLICKED);;
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == ION_BT_ACTIVE_SCREEN_ID)
                {
                    if (g_BluetoothSubIndex != (int8_t) HeadArrayMsg.HeartBeatMsg.m_SubIndex)
                    {
                        g_BluetoothSubIndex = (int8_t) HeadArrayMsg.HeartBeatMsg.m_SubIndex;
                        gxe.gx_event_type = GX_SIGNAL (BT_SUBMENU_CHANGED_ID, GX_EVENT_CLICKED);;
                        gxe.gx_event_sender = GX_ID_NONE;
                        gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                        gxe.gx_event_display_handle = 0;
                        gx_system_event_send(&gxe);
                    }
                }
                else if (g_ActiveScreen->gx_widget_id == ATTENDANT_SCREEN_ID)
                {
                    if ((g_HeadArrayStatus1 & 0x01) != (HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01))
                    {
                        if ((HeadArrayMsg.HeartBeatMsg.m_HA_Status & 0x01) == 0x01)   // Bit0 = 1 if Head Array in "Ready", Power On mode.
                        {
                            gxe.gx_event_type = GX_SIGNAL (HA_POWERON_BTN_ID, GX_EVENT_CLICKED);
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }
                        else
                        {
                            gxe.gx_event_type = GX_SIGNAL (HA_POWEROFF_BTN_ID, GX_EVENT_CLICKED);
                            gxe.gx_event_sender = GX_ID_NONE;
                            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                            gxe.gx_event_display_handle = 0;
                            gx_system_event_send(&gxe);
                        }

                    }
                }
            }
            else    // Failed Heart Beat
            {
                if (g_ActiveScreen->gx_widget_id == MAIN_USER_SCREEN_ID)
                {
                    gxe.gx_event_type = GX_SIGNAL (HB_TIMEOUT_ID, GX_EVENT_CLICKED);
                    gxe.gx_event_sender = GX_ID_NONE;
                    gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
                    gxe.gx_event_display_handle = 0;
                    gx_system_event_send(&gxe);
                }
            }
            break;

        case HHP_HA_PAD_ASSIGMENT_GET:  // Yes, the COMM task is responding with a "set" command.
            myPad = HeadArrayMsg.PadAssignmentResponseMsg.m_PhysicalPadNumber;
            if (myPad != END_OF_PAD_ENUM)
            {
                g_PadSettings[myPad].m_PadDirection = HeadArrayMsg.PadAssignmentResponseMsg.m_LogicalDirection;
                g_PadSettings[myPad].m_PadType = HeadArrayMsg.PadAssignmentResponseMsg.m_PadType;
            }
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_VERSION_GET:
            //sprintf (g_HeadArrayVersionString, "ASL110: %d.%d.%d", HeadArrayMsg.Version.m_Major, HeadArrayMsg.Version.m_Minor, HeadArrayMsg.Version.m_Build);
            g_HA_Version_Major = HeadArrayMsg.Version.m_Major;
            g_HA_Version_Minor = HeadArrayMsg.Version.m_Minor;
            g_HA_Version_Build = HeadArrayMsg.Version.m_Build;
            g_HA_EEPROM_Version = HeadArrayMsg.Version.m_EEPROM_Version;

            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_PAD_DATA_GET:
            myPad = HeadArrayMsg.GetDataMsg.m_PadID;        // Get Physical Pad ID
            if (myPad < END_OF_PAD_ENUM)
            {
                g_PadSettings[myPad].m_Proportional_RawValue = HeadArrayMsg.GetDataMsg.m_RawData;
                g_PadSettings[myPad].m_Proportional_DriveDemand = HeadArrayMsg.GetDataMsg.m_DriveDemand;
            }
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_CALIBRATE_RANGE_GET:
            myPad = HeadArrayMsg.GetDataMsg.m_PadID;        // Get Physical Pad ID
            if (myPad < END_OF_PAD_ENUM)
            {
                g_PadSettings[myPad].m_Minimum_ADC_Threshold = HeadArrayMsg.CalibrationDataResponse.m_MinADC;
                g_PadSettings[myPad].m_Maximum_ADC_Threshold = HeadArrayMsg.CalibrationDataResponse.m_MaxADC;
                g_PadSettings[myPad].m_PadMinimumCalibrationValue = (int16_t) (HeadArrayMsg.CalibrationDataResponse.m_MinThreshold);
                g_PadSettings[myPad].m_PadMaximumCalibrationValue = (int16_t) (HeadArrayMsg.CalibrationDataResponse.m_MaxThreshold);
            }
            break;

        case HHP_HA_FEATURE_GET:
            g_TimeoutValue = HeadArrayMsg.GetFeatureResponse.m_Timeout;
            g_MainScreenFeatureInfo[POWER_ONOFF_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_POWER_ON_OFF_BIT_MASK ? true : false); // Power On/Off
            g_MainScreenFeatureInfo[BLUETOOTH_FEATURE_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_OUT_CTRL_TO_BT_MODULE_BIT_MASK ? true : false); // Bluetooth
            g_MainScreenFeatureInfo[NEXT_FUNCTION_OR_TOGGLE_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_NEXT_FUNCTION_BIT_MASK ? true : false); // Next Function
            g_MainScreenFeatureInfo[NEXT_PROFILE_OR_USER_MENU_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_NEXT_PROFILE_BIT_MASK ? true : false); // Next Profile
            g_ClicksActive = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_SOUND_ENABLED_BIT_MASK ? true : false);              // Clicks on/off
            g_PowerUpInIdle = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_POWER_UP_IN_IDLE_BIT_MASK ? true : false);          // Power up in idle
            g_RNet_Active = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet & FUNC_FEATURE_RNET_SEATING_MASK ? true : false); // Process RNet
            g_MainScreenFeatureInfo[RNET_SEATING_ID].m_Enabled = g_RNet_Active;

            // Process the second feature byte... if available from the Head Array
            if (g_HA_EEPROM_Version >= 6)
            {
                g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Enabled = (HeadArrayMsg.GetFeatureResponse.m_FeatureSet2 & FUNC_FEATURE2_RNET_SLEEP_BIT_MASK ? true : false);
                g_Mode_Switch_Schema = ((HeadArrayMsg.GetFeatureResponse.m_FeatureSet2 & FUNC_FEATURE2_MODE_REVERSE_BIT_MASK) ? HUB_MODE_SWITCH_REVERSE : HUB_MODE_SWITCH_PIN5);
            }
            else
            {
                g_MainScreenFeatureInfo[RNET_SLEEP_FEATURE_ID].m_Enabled = false;
                g_Mode_Switch_Schema = HUB_MODE_SWITCH_PIN5;
            }

            // EEPROM Version 7
            //      - Added PAD SENSOR status to protocol
            if (g_HA_EEPROM_Version >= 7)
            {
                g_ShowPadsOnMainScreen = ((HeadArrayMsg.GetFeatureResponse.m_FeatureSet2 & FUNC_FEATURE2_SHOW_PADS_BIT_MASK) ? true : false); // FEATURE_ENABLED : FEATURE_DISABLED);
            }
            else
            {
                g_ShowPadsOnMainScreen = false;
            }

            // Add Driving Mode
            g_MainScreenFeatureInfo[DRIVE_FEATURE_ID].m_Enabled = ((HeadArrayMsg.GetFeatureResponse.m_FeatureSet2 & FUNC_FEATURE2_DRIVING_MODE_BIT_MASK) ? true : false);

            AdjustActiveFeaturePositions (g_ActiveFeature);   // This function also store "g_ActiveFeature" if appropriate.

            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_NEUTRAL_DAC_GET:
            g_NeutralDAC_Constant = HeadArrayMsg.NeutralDAC_Get_Response.m_DAC_Constant;
            g_NeutralDAC_Setting = HeadArrayMsg.NeutralDAC_Get_Response.m_NeutralDAC_Value;
            g_NeutralDAC_Range= HeadArrayMsg.NeutralDAC_Get_Response.m_Range;
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            g_WaitingForVeerResponse = false;
            break;

        case HHP_HA_DRIVE_OFFSET_GET:
            g_PadSettings[CENTER_PAD].m_MinimumDriveValue = HeadArrayMsg.DriveOffset_Get_Response.m_CenterPad_DriveOffset;
            g_PadSettings[LEFT_PAD].m_MinimumDriveValue = HeadArrayMsg.DriveOffset_Get_Response.m_LeftPad_DriveOffset;
            g_PadSettings[RIGHT_PAD].m_MinimumDriveValue = HeadArrayMsg.DriveOffset_Get_Response.m_RightPad_DriveOffset;
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_GET_DRIVER_CONTROL_INPUT_ASSIGNMENT:
            if (HeadArrayMsg.DriverControlPadAssignemt.m_DeviceID < ENDOF_DEVICES_IDX)
            {
                g_DeviceSettings[HeadArrayMsg.DriverControlPadAssignemt.m_DeviceID].m_PadInfo[LEFT_PAD].m_PadDirection = HeadArrayMsg.DriverControlPadAssignemt.m_LeftPad;
                g_DeviceSettings[HeadArrayMsg.DriverControlPadAssignemt.m_DeviceID].m_PadInfo[RIGHT_PAD].m_PadDirection = HeadArrayMsg.DriverControlPadAssignemt.m_RightPad;
                g_DeviceSettings[HeadArrayMsg.DriverControlPadAssignemt.m_DeviceID].m_PadInfo[CENTER_PAD].m_PadDirection = HeadArrayMsg.DriverControlPadAssignemt.m_FowardPad;
                g_DeviceSettings[HeadArrayMsg.DriverControlPadAssignemt.m_DeviceID].m_PadInfo[REVERSE_PAD].m_PadDirection = HeadArrayMsg.DriverControlPadAssignemt.m_ReversePad;
                g_DeviceSettings[HeadArrayMsg.DriverControlPadAssignemt.m_DeviceID].m_Mode_Switch_Schema = HeadArrayMsg.DriverControlPadAssignemt.m_ModeSwitchSchema;
            }
            break;

        case HHP_HA_BLUETOOTH_SETUP_GET_CMD:
            //void BT_Process_HUB_DeviceDefintion (uint8_t slotNumber, BT_DEVICE_TYPE deviceType, BT_COLOR color, BT_STATUS_ENUM bt_status)
            BT_Process_HUB_DeviceDefintion (HeadArrayMsg.BT_DeviceDefinition.m_SlotNumber, HeadArrayMsg.BT_DeviceDefinition.m_DeviceIdenfication,
                                            HeadArrayMsg.BT_DeviceDefinition.m_Color, HeadArrayMsg.BT_DeviceDefinition.m_Status);
            break;

        case HHP_HA_GET_DRIVER_CONTROL_ENABLE:
            // Store the Enabled Status in the Device's Settings Structure.
            g_DeviceSettings[HeadArrayMsg.DriverControlEnable.m_DeviceID].m_Enabled = HeadArrayMsg.DriverControlEnable.m_Enabled;
            break;

//        case HHP_HA_ATTENDANT_SETTINGS_GET:
//            g_AttendantSettings = HeadArrayMsg.AttendantSettings_Get_Response.m_AttendantSettings;
//            g_AttendantTimeout = HeadArrayMsg.AttendantSettings_Get_Response.m_AttendantTimeout;
//            // Don't know if need to redraw this.
//            gxe.gx_event_type = GX_EVENT_REDRAW;
//            gxe.gx_event_sender = GX_ID_NONE;
//            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
//            gxe.gx_event_display_handle = 0;
//            gx_system_event_send(&gxe);
//            break;

        // Process the SNP Get Data by updating the current display.
        case HHP_HA_SNP_GET_RAWDATA_CMD:
            // Redraw the current window.
            gxe.gx_event_type = GX_EVENT_REDRAW;
            gxe.gx_event_sender = GX_ID_NONE;
            gxe.gx_event_target = 0;  /* the event to be routed to the widget that has input focus */
            gxe.gx_event_display_handle = 0;
            gx_system_event_send(&gxe);
            break;

        case HHP_HA_AUDITORY_SETTINGS_GET_CMD:
            StoreAuditorySettings ((uint8_t*) &HeadArrayMsg.WholeMsg.m_MsgArray);
            break;

        default:
            break;
    } // end switch
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
    uint8_t msgCounter = 0;

#ifdef FORCE_OK_FOR_GUI_DEBUGGING
    g_HeadArray_Status = 0x00;
    g_TimeoutValue = 15;
#endif

    g_ioport_on_ioport.pinWrite(I2C_CS_PIN, IOPORT_LEVEL_HIGH);

    while (1)
    {
        msgSent = false;
        // Process requests from the GUI.
        tx_queue_info_get (&g_GUI_to_COMM_queue, NULL, &numMsgs, NULL, NULL, NULL, NULL);
        if (numMsgs)
        {
            tx_queue_receive (&g_GUI_to_COMM_queue, &GUI_Msg, TX_NO_WAIT);
            msgSent = Process_GUI_Messages (GUI_Msg);
        }
        else // We have no messages from the GUI to process.
        {
            if (g_AttendantActive)
            {
                // The attendant screen is active, but we need to send a HB every once in a while.
                if (++msgCounter < 2)
                    msgSent = SendAttendantData();
                else
                    msgCounter = 0;
            }
            else if (g_GetDataActive)    // Are we expected to continuously get PAD data from the Head Array?
            {
                // Determine next pad data to get.
                if (g_GetAllPadData)    // If the GUI requested All Pads, then advance to the next pad, Diagnostics vs Calibration
                {
                    if (g_ActivePadID >= END_OF_PAD_ENUM)
                    {
                        g_ActivePadID = (PHYSICAL_PAD_ENUM) 0;  // Roll over beethoven.
                    }
                    else    // We want to allow the ExecuteHeartBeat() to execute at least once in a while to get the USER and MODE port switch status.
                    {
                        msgSent = GetPadData();
                        ++g_ActivePadID;        // choose the next pad.
                    }
                }
                else
                {
                    msgSent = GetPadData();
                }
            }
            else if (g_SNP_CalibrationIsActive)
            {
                if (++msgCounter > 2)
                {
                    Request_SNP_Data();
                    msgCounter = 0;
                }
            }
        }

        // If we got no messages from the GUI and we are NOT doing diagnostics, then do a HeartBeat message.
        if (msgSent == false)
            ExecuteHeartBeat();

        tx_thread_sleep (10);
    }
}


