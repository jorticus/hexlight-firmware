
#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"
#include "comms.h"

#include "pwm.h"
#include "colourengine.h"

extern bool enableUsbAudio;

int ProcessCommand(byte command, byte* payload_data, byte payload_len, byte* reply_data, uint* reply_len) {
    // NOTE: reply_data is assumed to contain at least MAX_PAYLOAD_SIZE bytes
    *reply_len = 0;
    toggle(PIO_LED3);

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

        // Directly set the raw linear PWM values (0-32767), for each of the four channels.
        case CMD_SET_PWM:
            if (payload_len == sizeof(pl_pwm_t)) {
                pl_pwm_t* payload = (pl_pwm_t*)payload_data;
                PWMUpdate(payload->ch1, payload->ch2, payload->ch3, payload->ch4);
                return RESULT_SUCCESS;
            } else return ERROR_INVALID_PAYLOAD;
            break;

        // Update the outputs using device-independant colour
        case CMD_SET_XYY:
            if (payload_len == sizeof(XYYColour)) {
                XYYColour* payload = (XYYColour*)payload_data;
                ColourEngine::SetXYY(*payload);
                return RESULT_SUCCESS;
            } else return ERROR_INVALID_PAYLOAD;
            break;

        // Update the outputs using device-independant colour
        case CMD_SET_XYZ:
            if (payload_len == sizeof(XYZColour)) {
                XYZColour* payload = (XYZColour*)payload_data;
                ColourEngine::SetXYZ(*payload);
                return RESULT_SUCCESS;
            } else return ERROR_INVALID_PAYLOAD;
            break;

        /*case CMD_GET_XYY:
            XYYColour* payload = (XYYColour*)reply_data;
            *reply_len = sizeof(XYYColour);
            *payload = ColourEngine::GetXYY();
            return RESULT_SUCCESS;
            break;*/

        case CMD_ENABLE_USBAUDIO:
            if (payload_len == 1) {
                enableUsbAudio = (bool)reply_data[0];
                _LAT(PIO_LED1) = enableUsbAudio;
                return RESULT_SUCCESS;
            } else return ERROR_INVALID_PAYLOAD;
            break;

    }
    return ERROR_INVALID_COMMAND;
}

int UnescapeData(byte* src_buf, byte* dst_buf, uint src_len, uint dst_len) {
    int srci, dsti;
    for (srci=0, dsti=0; srci<src_len; srci++, dsti++) {
        if (dsti == dst_len)
            return -1; // Not enough dest space

        byte data = src_buf[srci];

        if (data == HLDC_FRAME_DELIMITER)
            return -1;  // Invalid byte

        if (data == HLDC_ESCAPE && srci < src_len-1)
            data = src_buf[++srci] ^ HLDC_ESCAPE_MASK;

        dst_buf[dsti] = data;
    }
    return dsti;
}

int EscapeData(byte* src_buf, byte* dst_buf, uint src_len, uint dst_len) {
    // NOTE: Worst case is that every byte needs to be escaped,
    // so make dst_buf twice as large as the src_buf.
    
    int srci, dsti=0;
    for (srci=0; srci<src_len; srci++) {
        byte data = src_buf[srci];

        if (data == HLDC_ESCAPE || data == HLDC_FRAME_DELIMITER) {
            dst_buf[dsti++] = HLDC_ESCAPE;
            if (dsti == dst_len)
                return -1; // Not enough dest space

            data ^= HLDC_ESCAPE_MASK;
        }

        dst_buf[dsti++] = data;
        if (dsti == dst_len)
            return -1; // Not enough dest space
    }
    return dsti;
}


int ProtocolFramer::ProcessData(byte* buf, int len) {
    tx_size = 0;
    int result;

    for (int i=0; i<len; i++) {
        result = ProcessByte(buf[i]);
        if (result < 0)
            return result; // Error
    }
    return RESULT_SUCCESS;
}

int ProtocolFramer::ProcessByte(byte b) {
    switch (rx_state) {
        // Wait until a frame is detected
        case stWaitingForSOF:
            if (b == HLDC_FRAME_DELIMITER)
                rx_state = stWaitingForHeader;
            break;

        // To make sure we're not detecting the end of a frame,
        // make sure the next byte is not a frame delimiter
        case stWaitingForHeader:
            if (b != HLDC_FRAME_DELIMITER) {
                rx_idx = 0;
                rx_buffer[rx_idx++] = b;
                rx_state = stReadingFrame;
            } else {
                // Another SOF received, stay in this state.
            }
            break;

        // Now we know we're in a frame, process until the end of frame
        // is found, or we run out of buffer space.
        case stReadingFrame:
            if (b != HLDC_FRAME_DELIMITER) {

                rx_buffer[rx_idx++] = b;

                // Buffer full (no delimiter received before buffer filled up)
                if (rx_idx == MAX_PACKET_SIZE) {
                    rx_state = stWaitingForSOF;
                    return ERROR_PACKET_TOO_BIG;
                }

            } else { // EOF received

                // Not enough bytes for a complete frame
                if (rx_idx < MIN_PACKET_SIZE) {
                    rx_state = stWaitingForSOF;
                    return ERROR_PACKET_TOO_SMALL;
                }

                int result = ProcessFrame(rx_buffer, rx_idx);
                rx_idx = 0;

                rx_state = stWaitingForSOF;
                return result;
            }
            break;

        default:
            rx_state = stWaitingForSOF;
    }
    return RESULT_PROCESSING;
}


int ProtocolFramer::ProcessFrame(byte* buf, uint len) {
    // Unescape the data
    byte data[MAX_PACKET_SIZE];
    int numBytes = UnescapeData(buf, data, len, MAX_PACKET_SIZE);
    if (numBytes < 0)
        return ERROR_UNESCAPE_ERROR; // Error escaping data (numBytes contains error code)

    packet_header_t* header = (packet_header_t*)data;

    if (header->length + MIN_PACKET_SIZE > numBytes)
        return ERROR_PAYLOAD_TOO_SMALL; // Not enough bytes in frame for the payload? Unlikely.

    // Check the CRC
    packet_footer_t* footer = (packet_footer_t*)(data + header->length);
    UINT16 crc = 0;
    //TODO: Calculate CRC
    if (crc != footer->crc)
        return ERROR_INVALID_CRC;

    byte tx_payload[MAX_PACKET_SIZE];
    uint tx_size;

    int result = ProcessCommand(header->command,
        (data + sizeof(packet_header_t)), header->length,
        tx_payload, &tx_size);

    // Prepare TX data
    if (result == RESULT_SUCCESS) {
        if (!PreparePacket(header->command, tx_payload, tx_size))
            return ERROR_FORMING_RESPONSE_PACKET;
    }

    return result;
}

bool ProtocolFramer::PreparePacket(byte command, byte* payload, uint len) {
    byte* buf;
    tx_idx = 0;
    tx_size = 0;
    tx_buffer[tx_idx++] = HLDC_FRAME_DELIMITER;

    packet_header_t header;
    header.command = command;
    header.length = (payload != NULL) ? len : 0;

    // Write header
    buf = (byte*)&header;
    for (int i=0; i<sizeof(packet_footer_t); i++) {
        if (!TxWrite(buf[i]))
            return FALSE;
    }

    // Write payload
    if (payload != NULL && len > 0) {
        for (int i=0; i<len; i++) {
            if (!TxWrite(payload[i]))
                return FALSE;
        }
    }

    packet_footer_t footer;
    footer.crc = 0;

    // Write footer
    buf = (byte*)&footer;
    for (int i=0; i<sizeof(packet_footer_t); i++) {
        if (!TxWrite(buf[i]))
            return FALSE;
    }

    tx_buffer[tx_idx++] = HLDC_FRAME_DELIMITER;
    tx_size = tx_idx;
    return TRUE;
}

bool ProtocolFramer::TxWrite(byte b) {
    if (b == HLDC_FRAME_DELIMITER || b == HLDC_ESCAPE) {
        tx_buffer[tx_idx++] = HLDC_ESCAPE;

        if (tx_idx == MAX_PACKET_SIZE)
            return FALSE;

        b ^= HLDC_ESCAPE_MASK;
    }

    tx_buffer[tx_idx++] = b;
    if (tx_idx == MAX_PACKET_SIZE)
        return FALSE;

    return TRUE;
}
