

#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"

extern "C" {
    #include "USB/usb.h"
    #include "USB/usb_function_cdc.h"
}
#include "usb.h"

#include "colourengine.h"

bool usb_sleeping = FALSE;



int USBUserProcess(void) {
    //TODO: How can we abstract USB/UART comms a bit better?
    static char rx_buffer[64];
    static char tx_buffer[64];
    int numBytesRead;

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) {
        _LAT(PIO_LED_USB) = LOW;
        return 0;
    }
    _LAT(PIO_LED_USB) = HIGH;


    numBytesRead = getsUSBUSART(rx_buffer, 64);
    if (numBytesRead != 0) {

        // Simple packet format: ['X'][R][G][B]
        if (rx_buffer[0] == 'X') {
            byte r = rx_buffer[1];
            byte g = rx_buffer[2];
            byte b = rx_buffer[3];

            RGB colour(r/255.0, g/255.0, b/255.0);
            ColourEngine::SetColour(colour);
        }
    }

    /*if (USBUSARTIsTxTrfReady()) {
        putUSBUSART(tx_buffer, 64);
    }*/

    CDCTxService();

    return numBytesRead;
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