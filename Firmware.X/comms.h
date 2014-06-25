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


#define MIN_PACKET_SIZE (sizeof(packet_header_t) + sizeof(packet_footer_t))
#define MAX_PACKET_SIZE 64
#define MAX_PAYLOAD_SIZE (MAX_PACKET_SIZE - MIN_PACKET_SIZE)

#define RX_BUFFER_SIZE 256

#define HLDC_FRAME_DELIMITER 0x7E
#define HLDC_ESCAPE 0x7D
#define HLDC_ESCAPE_MASK 0x20


class ProtocolFramer {
public:
    ProtocolFramer() :
        rx_idx(0), waitingForEOF(FALSE)
        {};


    bool ProcessData(byte* buf, int len);

    bool waitingForEOF;
    byte rx_buffer[RX_BUFFER_SIZE];
    int rx_idx;

private:
    int FindFrame(byte** buf, int* len);
    //int ParseData(byte* buf, byte len);
    int ProcessFrame();
};





typedef struct {
    UINT16 ch1;
    UINT16 ch2;
    UINT16 ch3;
    UINT16 ch4;
} pl_pwm_t;

typedef struct {
    float X;
    float Y;
    float Z;
} pl_xyz_t;

typedef struct {
    float x;
    float y;
    float Y;
} pl_xyy_t;

typedef struct {
    byte ch;
    pl_xyy_t point;
} pl_xyz_cal_t;
typedef enum { XYZ_CH1, XYZ_CH2, XYZ_CH3, XYZ_CH4, XYZ_WHITE } pl_xyz_cal_ch;

typedef enum { MODE_HOST_CONTROL, MODE_TRIG, MODE_CYCLE, MODE_AUDIO } pl_mode;


#define CMD_POWER_ON 0x01
#define CMD_POWER_OFF 0x02
#define CMD_SET_MODE 0x03
#define CMD_GET_LAST_ERROR 0x04

#define CMD_SET_PWM 0x10
#define CMD_GET_PWM 0x11
#define CMD_SET_XYZ 0x12
#define CMD_GET_XYZ 0x13
#define CMD_SET_XYY 0x14
#define CMD_GET_XYY 0x15

#define CMD_SET_XYZ_CAL 0x20
#define CMD_GET_XYZ_CAL 0x21

#define CMD_ACK 0xFF


#endif	/* COMMS_H */

