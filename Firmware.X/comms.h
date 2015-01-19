/* 
 * File:   comms.h
 * Author: Jared
 *
 * Created on 25 June 2014, 1:37 PM
 */

#ifndef COMMS_H
#define	COMMS_H


typedef struct __attribute__((packed)) {
    //byte sof;
    byte command;
    byte length;  // Length in bytes, including escape chars
} packet_header_t;

typedef struct __attribute__((packed)) {
    UINT16 crc;
   // byte eof;
} packet_footer_t;

#define RESULT_PROCESSING 0
#define RESULT_SUCCESS 1
#define RESULT_ERROR -1

#define MIN_PACKET_SIZE (sizeof(packet_header_t) + sizeof(packet_footer_t))
#define MAX_PACKET_SIZE 64
#define MAX_PAYLOAD_SIZE (MAX_PACKET_SIZE - MIN_PACKET_SIZE)

#define RX_BUFFER_SIZE 256

#define HLDC_FRAME_DELIMITER 0x7E
#define HLDC_ESCAPE 0x7D
#define HLDC_ESCAPE_MASK 0x20

#define ERROR_UNKNOWN -1
#define ERROR_PACKET_TOO_BIG -2
#define ERROR_PACKET_TOO_SMALL -3
#define ERROR_PAYLOAD_TOO_SMALL -4
#define ERROR_UNESCAPE_ERROR -5
#define ERROR_INVALID_CRC -6
#define ERROR_FORMING_RESPONSE_PACKET -7
#define ERROR_INVALID_COMMAND -8
#define ERROR_INVALID_PAYLOAD -9

class ProtocolFramer {
public:
    ProtocolFramer() :
        rx_idx(0), tx_idx(0), rx_state(stWaitingForSOF)
        {};


    int ProcessData(byte* buf, int len);
    bool PreparePacket(byte command, byte* payload, uint len);
    
    byte tx_buffer[MAX_PACKET_SIZE];
    byte tx_size;

private:
    int ProcessByte(byte b);
    int ProcessFrame(byte* data, uint len);
    bool TxWrite(byte b);

    typedef enum { stWaitingForSOF, stWaitingForHeader, stReadingFrame } state_t;

    byte rx_buffer[MAX_PACKET_SIZE];
    
    uint rx_idx;
    uint tx_idx;
    

    state_t rx_state;
};



#include "colourspaces.h"


typedef struct {
    UINT16 ch1;
    UINT16 ch2;
    UINT16 ch3;
    UINT16 ch4;
} pl_pwm_t;

/*typedef struct {
    float X;
    float Y;
    float Z;
} pl_xyz_t;

typedef struct {
    float x;
    float y;
    float Y;
} pl_xyy_t;*/

//typedef struct {
//    byte ch;
//    //pl_xyy_t point;
//    XYYColour colour_point;
//} pl_xyz_cal_t;
//typedef enum { XYZ_CH1, XYZ_CH2, XYZ_CH3, XYZ_CH4, XYZ_WHITE } pl_xyz_cal_ch;

typedef enum { MODE_HOST_CONTROL, MODE_TRIG, MODE_CYCLE, MODE_AUDIO } pl_mode;


#define CMD_TEST 0x00
#define CMD_POWER_ON 0x01
#define CMD_POWER_OFF 0x02
#define CMD_SET_MODE 0x03
#define CMD_GET_LAST_ERROR 0x04

#define CMD_SET_PWM 0x10
#define CMD_SET_RGB 0x12
#define CMD_GET_RGB 0x13
#define CMD_SET_XYZ 0x14
#define CMD_GET_XYZ 0x15
#define CMD_SET_XYY 0x16
#define CMD_GET_XYY 0x17

#define CMD_SET_XYZ_CAL 0x20
#define CMD_GET_XYZ_CAL 0x21

#define CMD_ENABLE_USBAUDIO 0x30

#define CMD_ERROR 0xFF


#endif	/* COMMS_H */

