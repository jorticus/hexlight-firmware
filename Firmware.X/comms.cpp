
#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"
#include "comms.h"

#include "pwm.h"
#include "colourengine.h"

#define SUCCESS 0
#define ERROR -1

int ProcessCommand(byte command, byte* payload_data, byte payload_len) {
    switch (command) {
        case CMD_POWER_OFF:
            PWMDisable();
            return SUCCESS;
            break;

        case CMD_POWER_ON:
            PWMEnable();
            return SUCCESS;
            break;

        case CMD_SET_MODE:
            //pl_mode mode = *(pl_mode*)payload_data;
            break;

        case CMD_SET_PWM:
            _LAT(PIO_LED2) = HIGH;
            if (payload_len == sizeof(pl_pwm_t)) {
                pl_pwm_t* payload = (pl_pwm_t*)payload_data;
                PWMUpdate(payload->ch1, payload->ch2, payload->ch3, payload->ch4);
                return SUCCESS;
            }
            _LAT(PIO_LED2) = LOW;
            break;
    }
    return ERROR;
}


bool ProtocolFramer::ProcessData(byte* buf, int len) {
    // Process all frames in the packet
    int result;
    while ((result = FindFrame(&buf, &len) > 0) && (len > 0))
        continue;

    _LAT(PIO_LED2) = (result < 0) ? HIGH : LOW;

    return result;
}

// Scan the buffer and try find a complete frame.
// Modifies buf and len after a frame has been found, so it can be called again.
int ProtocolFramer::FindFrame(byte** buf, int* len) {
    byte* rx_data = *buf;
    int rx_len = *len;
    bool delimiterFound = FALSE;

    if (!this->waitingForEOF) {
        this->rx_idx = 0;

        // Find Start-of-Frame (SOF byte followed by non-SOF byte)
        int i;
        for (i = 0; i<rx_len-1; i++) {
            if ((rx_data[i] == HLDC_FRAME_DELIMITER) && (rx_data[i+1] != HLDC_FRAME_DELIMITER))
                break;
        }
        // SOF not found
        if (i == rx_len)
            return -1;
        
        rx_data += i;
        rx_len -= i;
    }

    // Copy data until EOF found
    for (int i=1; i<rx_len; i++) {
        byte data = rx_data[i];

        if (data == HLDC_FRAME_DELIMITER) {
            delimiterFound = TRUE;
            break;
        }

        this->rx_buffer[this->rx_idx++] = data;

        // Buffer full (no delimiter found before buffer was filled)
        if (this->rx_idx == MAX_PACKET_SIZE) {
            this->rx_idx = 0; // Discard the buffer
            this->waitingForEOF = FALSE;
            return -2;
        }
    }

    // Haven't received a complete frame yet, wait for more data
    if (!delimiterFound) {
        this->waitingForEOF = TRUE;
        return 0;
    }

    // A complete frame was found
    else {
        this->waitingForEOF = FALSE;

        if (this->rx_idx < MIN_PACKET_SIZE) {
            this->rx_idx = 0;
            return -3;
        }

        // Continue processing in case there are more frames in the buffer
        int total_len = *len - rx_len;
        *len = total_len;
        *buf = rx_data + total_len;
        return ProcessFrame();
    }
}

/*bool ProtocolFramer::ParseData(byte* buf, byte len) {
    for (i=0; i<len; i++) {
        byte data = buf[i];

        if (data == HLDC_FRAME_DELIMITER)
            return;

        else if (data == HLDC_ESCAPE) {
            data = buf[++i] ^ HLDC_ESCAPE_MASK;
        }

        this->rx_buffer[this->rx_idx++] = data;

        if (this->rx_idx == MAX_PACKET_SIZE)
            return FALSE;
    }
}*/

int ProtocolFramer::ProcessFrame() {
    byte* rx_data = this->rx_buffer;
    int rx_len = this->rx_idx;


    toggle(PIO_LED1);
    _LAT(PIO_LED3) = (rx_data[0] == 0xC3) ? HIGH : LOW;

    return 1;
}

/*int ProtocolFramer::ProcessData(byte* rx_data, byte* tx_data, uint rx_len) {
    int i;
    int sof_idx = -1;

    if (!this->waitingForEOF) {

        // Find Start-of-Frame (SOF)
        while ((*rx_data != HLDC_FRAME_DELIMITER) && (rx_len > 0)) {
            // Shift the start of the array until start of frame is found
            rx_len--;
            rx_data++;
        }
        if (rx_len == 0) // No SOF found in data
            return -1;

        // Copy and parse data until EOF found
        this->rx_idx = 0; // Reset buffer
        bool delimiterFound = this->ParseData(rx_data, rx_len);

        // Buffer full (no delimiter found before buffer was filled)
        if (this->rx_idx == MAX_PACKET_SIZE) {
            this->rx_idx = 0; // Discard the buffer
            return -1;
        }

        // Haven't received a complete frame yet
        if (!delimiterFound) {
            this->waitingForEOF = TRUE;
            return 0;
        }
        // A complete frame was found
        else {
            return ProcessFrame(tx_data);
            // What about the rest of the data left in rx_data?
        }
    }

    else { // waitingForEOF == TRUE
        bool delimiterFound = this->ParseData(rx_data, rx_len);

        // Buffer full (no delimiter found before buffer was filled)
        if (this->rx_idx == MAX_PACKET_SIZE) {
            this->waitingForEOF = FALSE;
            this->rx_idx = 0; // Discard the buffer
            return -1;
        }

        // We now have a complete frame
        if (delimiterFound) {
            this->waitingForEOF = FALSE;
            return ProcessFrame(tx_data);
        }
    }


    //return ProcessFrame(this->rx_buffer, tx_data, this->rx_idx);
}





/*int ProtocolFramer::ProcessData_old(byte* rx_data, byte* tx_data, uint rx_len) {
    uint i, j;
    byte rx_payload[MAX_PAYLOAD_SIZE];
    byte payload_len = 0;

    // Find start-of-frame
    while ((*rx_data != HLDC_FRAME_DELIMITER) && (rx_len > 0)) {
        // Shift the array until start of frame is found
        rx_len--;
        rx_data++;
    }

    // No start of frame found
    if (rx_len == 0)
        return -1;

    // Packet cut in half, abort.
    if (rx_len < MIN_PACKET_SIZE)
        return -2;

    // Parse packet header
    packet_header_t* header = (packet_header_t*)rx_data;

    // Frame is cut in half, abort.
    if ((header->length + sizeof(packet_footer_t)) > rx_len)
        return -3;

    // Parse payload (un-escape when necessary)
    UINT16 crc = 0;
    rx_data += sizeof(packet_header_t);
    for (i=0, j=0; i<header->length; i++, j++) {
        byte payload_byte;

        if (rx_data[i] == HLDC_ESCAPE) {
            payload_byte = rx_data[++i] ^ HLDC_UNESCAPE_MASK;
        } else {
            payload_byte = rx_data[i];
        }

        // Compute CRC
        // ...

        rx_payload[j] = payload_byte;
    }
    payload_len = j;

    // Check the packet footer
    rx_data += header->length;
    packet_footer_t* footer = (packet_footer_t*)rx_data;

    // Check that the packet ends with a frame delimiter
    if (footer->eof != HLDC_FRAME_DELIMITER)
        return -4;

    // Check the CRC
    //if (crc != footer->crc)
    //    return -1;


    // Send command packet to command parser
    if (ProcessCommand(header->command, rx_payload, payload_len) == ERROR) {
        return -5; // Error occurred processing the command
    }

    // If necessary, send data back
    return 0;

}*/
