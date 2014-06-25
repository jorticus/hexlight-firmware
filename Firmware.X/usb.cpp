

#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"

extern "C" {
    #include "USB/usb.h"
    #include "USB/usb_function_cdc.h"
}
#include "usb.h"

#include "colourengine.h"
#include "comms.h"

bool usb_sleeping = FALSE;


typedef struct {
    uint header;
    float r;
    float g;
    float b;
    float w;
    float l;
} hexrgb_packet_t;

typedef struct {
    uint header;
    float x;
    float y;
    float z;
    float b;
} hexrgb_packet_xyz_t;


ProtocolFramer cdcProtocolFramer;

void USBUserProcess(void) {
    //TODO: How can we abstract USB/UART comms a bit better?
    static byte rx_buffer[64];
    static byte tx_buffer[64];
    int numBytesRead;
    static int numBytesToWrite = 0;

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) {
        _LAT(PIO_LED_USB) = LOW;
        return;
    }
    _LAT(PIO_LED_USB) = HIGH;


    numBytesRead = getsUSBUSART((char*)rx_buffer, 64);
    if (numBytesRead != 0) {
        int result = cdcProtocolFramer.ProcessData(rx_buffer, numBytesRead);

        if (cdcProtocolFramer.rx_idx > 0) {
            for (int i=0; i<cdcProtocolFramer.rx_idx; i++)
                tx_buffer[i] = cdcProtocolFramer.rx_buffer[i];
            numBytesToWrite = cdcProtocolFramer.rx_idx;
            cdcProtocolFramer.rx_idx = 0;
        } else {
            tx_buffer[0] = result;
            numBytesToWrite = 1;
        }

        /*if (result > 0) {
            numBytesToWrite = result; // Need to wait for Trf to be ready before transmitting
        } else if (result < 0) {
            // Packet error
            tx_buffer[0] = (byte)-result;
            numBytesToWrite = 1;
        }*/
    }

    if (numBytesToWrite > 0 && USBUSARTIsTxTrfReady()) {
        putUSBUSART((char*)tx_buffer, numBytesToWrite);
        numBytesToWrite = 0;
    }

    CDCTxService();
}


void USBCBSuspend() {
    // Called when the PC wants to power-down this device
    _LAT(PIO_LED1) = HIGH;
    ColourEngine::PowerOff(500);
}

void USBCBWakeFromSuspend() {
    // This is sent by the PC if it wants the device to wake up from sleep
    _LAT(PIO_LED1) = LOW;
    ColourEngine::PowerOn(500);
}

void USBCB_SOF_Handler() {
    // An SOF packet is sent every 1ms by the PC when connected
    //toggle(PIO_LED2);
}

void USBCBErrorHandler() {

}


void USBCBSendResume() {
    // Call this to wake the PC up from sleep
    static WORD delay_count;

    USBResumeControl = 1;                // Start RESUME signaling

    delay_count = 1800U;                // Set RESUME line for 1-13 ms
    do
    {
        delay_count--;
    }while(delay_count);
    USBResumeControl = 0;
}



extern "C" {

BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    if (usb_sleeping) {
        // Wake up if any activity occurrs on the USB bus
        usb_sleeping = FALSE;
        USBCBWakeFromSuspend();
    }

    switch(event)
    {
        case EVENT_CONFIGURED:
            //USBCBInitEP();
            CDCInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            //USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            //USBCBCheckOtherReq();
            USBCheckCDCRequest();
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            usb_sleeping = TRUE;
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            usb_sleeping = FALSE;
            USBCBWakeFromSuspend();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER:
            Nop();
            break;
        default:
            break;
    }
    return TRUE;
}

}