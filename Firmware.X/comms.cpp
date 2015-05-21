
#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"
#include "comms.h"

#include "pwm.h"
#include "colourengine.hpp"

extern bool enableUsbAudio;

static int CommsProcessCommand(byte command, byte* payload_data, byte payload_len, byte* reply_data, uint* reply_len) {
    // NOTE: reply_data is assumed to contain at least MAX_PAYLOAD_SIZE bytes
    *reply_len = 0;
    //toggle(PIO_LED3);

    switch (command) {
        case CMD_TEST:
            if (payload_len != 0) return ERROR_INVALID_PAYLOAD;
            reply_data[0] = 123;
            *reply_len = 1;
            return RESULT_SUCCESS;
            break;

        case CMD_POWER_OFF:
            if (payload_len != 0) return ERROR_INVALID_PAYLOAD;
            ColourEngine::PowerOff();
            return RESULT_SUCCESS;
            break;

        case CMD_POWER_ON:
            if (payload_len != 0) return ERROR_INVALID_PAYLOAD;
            ColourEngine::PowerOn();
            return RESULT_SUCCESS;
            break;

        case CMD_SET_MODE:
            //pl_mode mode = *(pl_mode*)payload_data;
            break;

        // Set the raw RGBW values
        case CMD_SET_PWM:
            if (payload_len == sizeof(pl_pwm_t)) {
                pl_pwm_t* payload = (pl_pwm_t*)payload_data;
                ColourEngine::SetRGBW(RGBWColour(
                    payload->ch1,
                    payload->ch2,
                    payload->ch3,
                    payload->ch4
                ));
                return RESULT_SUCCESS;
            } else return ERROR_INVALID_PAYLOAD;
            break;

        // Update the outputs using device-independant colour
//        case CMD_SET_XYY:
//            if (payload_len == sizeof(XYYColour)) {
//                XYYColour* payload = (XYYColour*)payload_data;
//                ColourEngine::SetXYY(*payload);
//                return RESULT_SUCCESS;
//            } else return ERROR_INVALID_PAYLOAD;
//            break;
//
//        // Update the outputs using device-independant colour
//        case CMD_SET_XYZ:
//            if (payload_len == sizeof(XYZColour)) {
//                XYZColour* payload = (XYZColour*)payload_data;
//                ColourEngine::SetXYZ(*payload);
//                return RESULT_SUCCESS;
//            } else return ERROR_INVALID_PAYLOAD;
//            break;

        /*case CMD_GET_XYY:
            XYYColour* payload = (XYYColour*)reply_data;
            *reply_len = sizeof(XYYColour);
            *payload = ColourEngine::GetXYY();
            return RESULT_SUCCESS;
            break;*/

        case CMD_ENABLE_USBAUDIO:
            #ifdef USB_USE_AUDIO_CLASS
            if (payload_len == 1) {
                enableUsbAudio = (bool)payload_data[0];
                _LAT(PIO_LED1) = enableUsbAudio;
                return RESULT_SUCCESS;
            } else return ERROR_INVALID_PAYLOAD;
            #else
            return RESULT_SUCCESS;
            #endif
            break;


    }
    return ERROR_INVALID_COMMAND;
}

int CommsProcessFrame(byte* rx_buf, byte* tx_buf, uint len) {
    packet_header_t* header = (packet_header_t*)rx_buf;

    if (header->length + MIN_PACKET_SIZE > len)
        return ERROR_PAYLOAD_TOO_SMALL; // Not enough bytes in frame for the payload? Unlikely.

    // Check the CRC
    packet_footer_t* footer = (packet_footer_t*)(rx_buf + sizeof(packet_header_t) + header->length);
    UINT16 crc = 0;
    //TODO: Calculate CRC
    if (crc != footer->crc)
        return ERROR_INVALID_CRC;

    byte tx_payload[MAX_PACKET_SIZE];
    uint payload_size;

    int result = CommsProcessCommand(header->command,
        (rx_buf + sizeof(packet_header_t)), header->length,
        tx_payload, &payload_size);

    // Prepare TX data
    if (result == RESULT_SUCCESS) {
        int tx_size = CommsPreparePacket(tx_buf, header->command, tx_payload, payload_size);
        return tx_size;
    }

    return result;
}

int CommsPreparePacket(byte* tx_buf, byte command, byte* payload, uint payload_len) {
    byte* buf;
    uint tx_idx = 0;
    uint tx_size = 0;

    packet_header_t header;
    header.command = command;
    header.length = (payload != NULL) ? payload_len : 0;

    // Write header
    buf = (byte*)&header;
    for (int i=0; i<sizeof(packet_footer_t); i++) {
        tx_buf[tx_idx++] = buf[i];
    }

    // Write payload
    if (payload != NULL && payload_len > 0) {
        for (int i=0; i<payload_len; i++) {
            tx_buf[tx_idx++] = payload[i];
        }
    }

    packet_footer_t footer;
    footer.crc = 0;

    // Write footer
    buf = (byte*)&footer;
    for (int i=0; i<sizeof(packet_footer_t); i++) {
        tx_buf[tx_idx++] = buf[i];
    }

    return tx_idx; // tx_size
}
