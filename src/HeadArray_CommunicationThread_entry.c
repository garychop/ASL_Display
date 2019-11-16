//***************************************************************
//   The communication protocol I am using is not the real I2C.
//   I only use I2C start bit and 8 bit data trans method.
//   The trans stop controlled by length byte.
//   After data transmition stop, set io and clk to high, then set io and
//   clk as input. only when sending data can set io and clk
//   as output. no stop bit and ack bit.
//
//   I2C communication procedure:
//      1.    check if I2C_RES is high. if not, can not reset I2C_CS
//      2.    if I2C_RES is high, reset I2C_CS, so that
//            slave(sigma) can goes into interrupt routing
//      3.    before sending package, check if I2C_RES is low.
//      4.    if I2C_RES is low, send package.
//      5.    before receiving package, check if I2C_RES is high.
//      6.    if I2C_RES is high, receive response
//*******************************************************************************

#include "my_gui_thread.h"
#include <my_gui_thread_entry.h>

#include "HeadArray_CommunicationThread.h"

//******************************************************************************
// Defines and Macros
//******************************************************************************

#define HEARTBEAT_CMD 0x44

//******************************************************************************
// Forward declarations
//******************************************************************************

static void write_i2c_byte(uint8_t i2cbyte);
static uint8_t send_i2c_package(uint8_t *i2c_pack, uint8_t pack_len);
static uint8_t read_i2c_byte(void);
static uint8_t read_i2c_package(void);
uint8_t ExecuteHeartBeat(void);
uint8_t CalculateChecksum (uint8_t *, uint8_t);

//******************************************************************************
// Global Variables
//******************************************************************************

uint8_t g_HeartBeatCounter = 0;

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
        cs += (uint8_t) myData;
    }
    return cs;
}
//******************************************************************************
//
//******************************************************************************

static void write_i2c_byte(uint8_t i2cbyte)
{
    uint8_t i;

    // Send a bit at a time.
    for(i = 0; i < 8; i++)
    {
        if( (i2cbyte<<i) & 0x80 )
        {
            g_ioport_on_ioport.pinWrite(i2c_io, IOPORT_LEVEL_HIGH); //output_high(i2c_io);
        }
        else
        {
            g_ioport_on_ioport.pinWrite(i2c_io, IOPORT_LEVEL_LOW);  //output_low(i2c_io);
        }
        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);   //delay_us(1);  //2
        g_ioport_on_ioport.pinWrite(i2c_clk, IOPORT_LEVEL_HIGH);    //output_high(i2c_clk);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MICROSECONDS);   //delay_us(3); //5
        g_ioport_on_ioport.pinWrite(i2c_clk, IOPORT_LEVEL_LOW); //output_low(i2c_clk);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MICROSECONDS);   //delay_us(1);  //2
    }
}

//******************************************************************************

//******************************************************************************

static uint8_t send_i2c_package(uint8_t *i2c_pack, uint8_t pack_len)
{
    uint8_t i;
    uint16_t wait;
    ioport_level_t pin_state;

    TX_DISABLE;             // Turn off interrupts

    g_ioport_on_ioport.pinCfg( i2c_io,(IOPORT_CFG_DRIVE_MID | IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_LEVEL_HIGH) );
    g_ioport_on_ioport.pinCfg( i2c_clk,(IOPORT_CFG_DRIVE_MID | IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_LEVEL_HIGH) );

    //R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
    g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_LOW);

    // Wait for the i2c_res goes LOW
    wait = 46000;   //(104ms)
    do
    {    // if i2c_res == 1, wait
        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
        if(wait < 2)
        {
            //set_tris_e(0x33);
            g_ioport_on_ioport.pinDirectionSet(i2c_io, IOPORT_DIRECTION_INPUT);
            g_ioport_on_ioport.pinDirectionSet(i2c_clk, IOPORT_DIRECTION_INPUT);

            //output_high(i2c_cs);
            g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);

            TX_RESTORE;             // turn interrupts back on
            return 1;    // time out error
        }

        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;
    } while(pin_state == IOPORT_LEVEL_HIGH);

    // Wait for the i2c_res goes LOW
    wait = 46000;   //(104ms)
    do
    {    // if i2c_res == 1, wait
        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
        if(wait < 2)
        {
            //set_tris_e(0x33);
            g_ioport_on_ioport.pinDirectionSet(i2c_io, IOPORT_DIRECTION_INPUT);
            g_ioport_on_ioport.pinDirectionSet(i2c_clk, IOPORT_DIRECTION_INPUT);

            //output_high(i2c_cs);
            g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);

            TX_RESTORE;             // turn interrupts back on
            return 1;    // time out error
        }

        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;
    } while(pin_state == IOPORT_LEVEL_LOW);

    //debug
    //g_ioport.p_api->pinWrite(TEST_PIN, IOPORT_LEVEL_LOW);

    //issue i2c start
//    g_ioport_on_ioport.pinWrite(i2c_io, IOPORT_LEVEL_LOW);  //output_low(i2c_io);
//    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);   //delay_us(5);
//    g_ioport_on_ioport.pinWrite(i2c_clk, IOPORT_LEVEL_LOW); //output_low(i2c_clk);

    //send data
    //  delay_us(20);
    //R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);
    for(i = 0; i < pack_len; i++)
    {
        write_i2c_byte(i2c_pack[i]);
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MICROSECONDS);  //delay_us(10);
    }

    g_ioport_on_ioport.pinWrite(i2c_io, IOPORT_LEVEL_HIGH);
    //R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);
    g_ioport_on_ioport.pinWrite(i2c_clk, IOPORT_LEVEL_HIGH);
    //R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);

    g_ioport_on_ioport.pinDirectionSet(i2c_io, IOPORT_DIRECTION_INPUT);
    g_ioport_on_ioport.pinDirectionSet(i2c_clk, IOPORT_DIRECTION_INPUT);

    g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);

    // Wait for the i2c_res goes LOW
    wait = 46000;   //(104ms)
    do
    {    // if i2c_res == 1, wait
        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
        if(wait < 2)
        {
            //set_tris_e(0x33);
            g_ioport_on_ioport.pinDirectionSet(i2c_io, IOPORT_DIRECTION_INPUT);
            g_ioport_on_ioport.pinDirectionSet(i2c_clk, IOPORT_DIRECTION_INPUT);

            //output_high(i2c_cs);
            g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);

            TX_RESTORE;             // turn interrupts back on
            return 1;    // time out error
        }

        R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MICROSECONDS);   //add
        wait--;
    } while(pin_state == IOPORT_LEVEL_HIGH);

    return 0;
}

//******************************************************************************
// before receiving data, must set i2c_io and i2c_clk as input
//first wait CLK=1
//while CLK=1; check if IO =0; at the same time check time out
//if IO = 0; then we get the start bite; else failed
// if we get the start bit, must wait CLK low
//-------------------------------------------------------------------------
// read data is in globle para i2c_data_read
// function return 0 : time out
//******************************************************************************

static uint8_t read_i2c_byte(void)
{
    uint8_t i, wait;
    ioport_level_t pin_state;

    i2c_data_read = 0;

    for(i = 0; i < 8; i++)
    {
        wait = 200; //max waiting about 45us
        do
        {    // wait clk high
            g_ioport_on_ioport.pinRead(i2c_clk, &pin_state);
            R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);   //add
            wait--;
            if(wait == 0)
                return 1;
        } while(pin_state == IOPORT_LEVEL_LOW);

        //delay_us(1);
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);   //add
        wait = 200; //max waiting about 45us
        do
        { // wait clk low
            g_ioport_on_ioport.pinRead(i2c_clk, &pin_state);
            R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);   //add
            wait--;
            if(wait == 0)
                return 1;    // time out
        } while(pin_state == IOPORT_LEVEL_HIGH);    //input(i2c_clk) == 1
        //if(input(i2c_io))
        g_ioport_on_ioport.pinRead(i2c_io, &pin_state);
        if(pin_state == IOPORT_LEVEL_HIGH)
            i2c_data_read = ( (uint8_t)(i2c_data_read<<1)|0x01 );
        else
            i2c_data_read = ( (uint8_t)(i2c_data_read<<1)&0xfe );

    }
    return 0;
}

//******************************************************************************

//******************************************************************************

static uint8_t read_i2c_package(void)
{
    uint8_t i, i2c_get_len;
    uint16_t wait;
    ioport_level_t pin_state;

    i2c_data[0] = 0;

    // Wait for the RES line to go LOW
    wait = 40000;  //max waiting about 88ms  //(1000 max waiting about 2.2ms)
    do
    {    // if i2c_res == 0, can not receive package
        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
        if(wait == 0)
        {
            TX_RESTORE;
            return 1;    // time out error
        }
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;
    } while(pin_state == IOPORT_LEVEL_HIGH);

    // We need the CLK and DATA lines to be HIGH at this point
    g_ioport_on_ioport.pinRead(i2c_clk, &pin_state);
    if(pin_state == IOPORT_LEVEL_LOW)
    {   //input(i2c_clk) == 0
        TX_RESTORE;
        return 1;    // error
    }
    g_ioport_on_ioport.pinRead(i2c_io, &pin_state);
    if(pin_state == IOPORT_LEVEL_LOW)
    {   //input(i2c_clk) == 0
        TX_RESTORE;
        return 1;    // error
    }

//    // Wait for the clock to go HIGH
//    wait = 90; //max waiting about 200us
//    do
//    { //wait i2c_io = 0
//        g_ioport_on_ioport.pinRead(i2c_io, &pin_state);
//        if(wait == 0)
//        {
//            TX_RESTORE;
//            return 2;    // time out error
//        }
//        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);   //add
//        wait--;
//        //R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS); //delay_us(1);
//    } while(pin_state == IOPORT_LEVEL_LOW);
//
//    wait = 45; //max waiting about 100us
//    do
//    {  //wait i2c_clk = 0
//        g_ioport_on_ioport.pinRead(i2c_clk, &pin_state);
//        if(wait == 0)
//        {
//            TX_RESTORE;
//            return 2;    // time out error
//        }
//        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);   //add
//        wait--;
//    } while(pin_state == IOPORT_LEVEL_HIGH);

    if(read_i2c_byte() != 0)
    {
        TX_RESTORE;
        return 1;    // error
    }

    i2c_get_len = i2c_data_read;
    i2c_data[0] = i2c_get_len;

    for(i = 0; i < i2c_get_len-1; i++)
    {
        if(read_i2c_byte() != 0)
        {
            TX_RESTORE;
            return 1;    // error
        }
        else
        {
            i2c_data[i+1] = i2c_data_read;
        }
    }

    TX_RESTORE;
  return 0;
}

//******************************************************************************
//    cmd package format: length+cmd+data+crc; all hex bytes
//
//   length:  length of whole package, including length byte and crc byte
//   request: from HHP
//   response:from ASL-PROP
//
//   cmd:     0x30:   8.1 Pad Assignment command
//                     <LEN><MAP CMD><PHYSCIAL PAD><LOGICAL PAD><SENSOR TYPE><CHKSUM>
//                    <LEN><ACK/NAK><CHKSUM>
//
//                     request:  6+0x30+"L or R or C"+"L or R or C"+"D or A"+crc
//                    response: 3+0x60+0x63 or 3+0x61+0x64
//               ??    "D or A" -- "D or P"
//
//            0x31:   8.3 Pad Calibration (Start)
//                     <LEN><CALIBRATE START CMD><PHYSICAL PAD><CHKSUM>
//                    <LEN><ACK/NAK><CHKSUM>
//
//                     request:  4+0x31+"L or R or C"+crc
//                    response: 3+0x60+0x63 or 3+0x61+0x64
//
//            0x32:    8.3 Pad Calibration (Stop)
//                     <LEN><CALIBRATE STOP CMD><CHKSUM>
//                    <LEN><ACK/NAK><CHKSUM>
//
//                     request:  3+0x32+0x35
//                    response: 3+0x60+0x63 or 3+0x61+0x64
//
//            0x33:   8.4  Pad Diagnostics/Values
//                     <LEN><REQUEST PAD DATA CMD><PHYSICAL PAD><CHKSUM>
//                    <LEN>< PAD DATA RESPONSE><PHYSICAL PAD><LOGICAL PAD><TYPE><RAW DATA><ADJUSTED DATA><STATUS><CHKSUM>
//
//                     request:  4+0x33+"L or R or C"+crc
//                    response: 9+0x73+"L or R or C"+"L or R or C"+"D or P"+'RAW DATA'+'ADJUSTED DATA'+"0x00 or 0x01"+crc
//               ??    "D or A" -- "D or P"
//
//            0x34:   8.5  Version Information
//                     <LEN><REQUEST_VERSION_CMD><CHKSUM>
//                    <LEN><VERSION_RESPONSE><MAJOR><MINOR><BUILD><CHKSUM>
//
//                     request:  3+0x34+0x37
//                    response: 6+0x74+'MAJOR'+'MINOR'+'BUILD'+crc
//
//            0x35:   8.6  Power On Setting
//                     <LEN><REQUEST POWER SETTNG CMD><CHKSUM>
//                    <LEN><POWER SETTING RESPONSE><VALUE><CHKSUM>
//              ??    "VALUE"
//
//                     request:  3+0x35+0x38
//                    response: 4+0x75+'VALUE'+crc
//
//            0x36:   8.7  Sound Setting
//                     <LEN><REQUEST SOUND SETTNG CMD><CHKSUM>
//                    <LEN><SOUND SETTING RESPONSE><VALUE><CHKSUM>
//
//                     request:  3+0x36+0x39
//                    response: 4+0x76+"0x01 or 0x02"+crc
//
//
//            ????:   8.2  Calibration Range (ON HOLD)
//                     <LEN><REQUEST CALIBRATE RANGE CMD><CHKSUM>
//                    <LEN><CALIBRATE RANGE RESPONSE><MIN VALUE><MAX VALUE><CHKSUM>
//
//*******************************************************************************/

uint8_t m250_pwr_on(uint8_t on_off)
{
    uint8_t s_dat[10];
    uint16_t wait;
    ioport_level_t pin_state;

    g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH); //output_high(i2c_cs);

    // Wait for the i2c_res line goes HIGH
    wait = 53000;  //520ms
    do {    // if i2c_res == 0, can not send package
        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);

        if(wait == 0)
        {
            return 1;    // time out error
        }

        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
        wait--;

    } while(pin_state == IOPORT_LEVEL_LOW);

    s_dat[0] = 3;           // Set command length

    if(on_off == 1)         // DO we want to turn on the system.
    { // pwr on
        s_dat[1] = 0x30;
        s_dat[2] = 0x33;
        if( send_i2c_package(s_dat, 3) )
            return 1;   // send package
    }
    else
    {         // pwr off
        s_dat[1] = 0x31;
        s_dat[2] = 0x34;
        if( send_i2c_package(s_dat, 3) )
            return 2;   // send package
   }

   // now wait and check response
   if(read_i2c_package() == 0) {
      if(i2c_data[0]!=3) return 3;     // length error
      if(i2c_data[1]!=0x60) return 4;  //cmd error
     // if(i2c_data[2]!=0x63) return 1;     // crc error; i don't care
      return 0;
   }
   return 5;      // error

}

//-------------------------------------------------------------------------

// return 0: ok; 1: error
uint8_t m250_setup_mode(uint8_t on_off)
{
   uint8_t s_dat[10];
   uint16_t wait;
   ioport_level_t pin_state;


   g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);  //output_high(i2c_cs);


  wait = 53000;  //520ms
  do {    // if i2c_res == 0, can not send package
    g_ioport_on_ioport.pinRead(i2c_res, &pin_state);

    if(wait == 0) {
      return 1;    // time out error
    }

    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
    wait--;

  } while(pin_state == IOPORT_LEVEL_LOW);


   s_dat[0] = 3;

   if(on_off == 1) { // setup_mode on
      s_dat[1] = 0x34;
      s_dat[2] = 0x37;
      if( send_i2c_package(s_dat, 3) ) return 1;   // send package
   }

   else {         // setup_mode off
      s_dat[1] = 0x35;
      s_dat[2] = 0x38;
      if( send_i2c_package(s_dat, 3) ) return 1;   // send package
   }

   // now wait and check response
   if(read_i2c_package()==0) {
      if(i2c_data[0]!=3) return 1;     // length error
      if(i2c_data[1]!=0x60) return 1;  //cmd error or fail (NACK)
      //if(i2c_data[2]!=0x63) return 1;     // crc error; i don't care
      return 0;
   }
   return 1;      // error

}

//-------------------------------------------------------------------------
uint8_t get_PROP_version(void)
{

   uint8_t s_dat[3] = {3, 0x34, 0x37};
   uint16_t wait;
   ioport_level_t pin_state;

   g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);  //output_high(i2c_cs);

    wait = 20; // GC 53000;  //520ms
    do
    {    // if i2c_res == 0, can not send package
        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
        if (--wait < 2)
        {
            return 1;    // time out error
        }
        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
    } while(pin_state == IOPORT_LEVEL_LOW);

    if( send_i2c_package(s_dat, 3) )
        return 1;

    // now wait and check response
    if(read_i2c_package()==0)
    {
        if(i2c_data[0]!=6)
            return 1;     // length error
        if(i2c_data[1]!=0x74)
            return 1;  //cmd error or setup fail (NACK)
        if( i2c_data[5] != (uint8_t)(i2c_data[0]+i2c_data[1]+i2c_data[2]+i2c_data[3]+i2c_data[4]) )
            return 1;     // crc error
        prop_ver1 = i2c_data[2];
        prop_ver2 = i2c_data[3];
        prop_ver3 = i2c_data[4];

        return 0;
    }

   return 1;      // error
}

uint8_t ExecuteHeartBeat(void)
{

   uint8_t s_dat[4];
   uint8_t cs;
//   ioport_level_t pin_state;

   s_dat[0] = 0x04;     // msg length
   s_dat[1] = HEARTBEAT_CMD;
   s_dat[2] = ++g_HeartBeatCounter;
   cs = CalculateChecksum(s_dat, sizeof (s_dat)-1);
   s_dat[3] = cs;
   send_i2c_package(s_dat, sizeof(s_dat));

//   g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_LOW);  //output_high(i2c_cs);
//
//   // Wait for the slave to set i2c_res line LOW
//    wait = 20; // GC 53000;  //520ms
//    do
//    {    // if i2c_res == 0, can not send package
//        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
//        if (--wait < 2)
//        {
//            return 1;    // time out error
//        }
//        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
//    } while(pin_state == IOPORT_LEVEL_HIGH);
//
//    // Wait for slave to set the i2c_res HIGH
//    wait = 20; // GC 53000;  //520ms
//    do
//    {    // if i2c_res == 0, can not send package
//        g_ioport_on_ioport.pinRead(i2c_res, &pin_state);
//        if (--wait < 2)
//        {
//            return 1;    // time out error
//        }
//        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MICROSECONDS);
//    } while(pin_state == IOPORT_LEVEL_LOW);
//
//
//   if( send_i2c_package(s_dat, sizeof(s_dat)) )
//       return 1;
//
//    // now wait and check response
//       if(read_i2c_package()==0)
//       {
//          if(i2c_data[0]!=6)
//              return 1;     // length error
//          if(i2c_data[1]!=0x74)
//              return 1;  //cmd error or setup fail (NACK)
//          if( i2c_data[5] != (uint8_t)(i2c_data[0]+i2c_data[1]+i2c_data[2]+i2c_data[3]+i2c_data[4]) )
//              return 1;     // crc error
//          prop_ver1 = i2c_data[2];
//          prop_ver2 = i2c_data[3];
//          prop_ver3 = i2c_data[4];
//
//          return 0;
//       }
//
   return 1;      // error
}


//******************************************************************************
// Function: HeadArray_CommunicationThread_entry
//  Created by Synergy software
// Description: This is the thread to handle communication to the ASL110 Head Array
//
//******************************************************************************

void HeadArray_CommunicationThread_entry(void)
{
    g_ioport_on_ioport.pinWrite(i2c_cs, IOPORT_LEVEL_HIGH);

    while (1)
    {
        ExecuteHeartBeat();
        read_i2c_package();
        tx_thread_sleep (10);
    }

}
