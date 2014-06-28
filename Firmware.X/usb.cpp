

#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"

extern "C" {
    #include "USB/usb.h"
    //#include "USB/usb_function_cdc.h"
    #include "USB/usb_function_hid.h"
}
#include "usb.h"

#include "colourengine.h"
#include "comms.h"

bool usb_sleeping = FALSE;

USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;

byte rx_buffer[64];
byte tx_buffer[64];


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
    int numBytesRead;
    static int numBytesToWrite = 0;

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) {
        _LAT(PIO_LED_USB) = LOW;
        return;
    }
    _LAT(PIO_LED_USB) = HIGH;

    if (!HIDRxHandleBusy(USBOutHandle)) { //Check if data was received from the host.

        int result = cdcProtocolFramer.ProcessData(rx_buffer, sizeof(rx_buffer));

        if (cdcProtocolFramer.tx_idx > 0) {
            numBytesToWrite = cdcProtocolFramer.tx_idx;
            for (int i=0; i<numBytesToWrite; i++)
                tx_buffer[i] = cdcProtocolFramer.tx_buffer[i];

            if (!HIDTxHandleBusy(USBInHandle)) {
                USBInHandle = HIDTxPacket(HID_EP, (byte*)tx_buffer, numBytesToWrite);
            }
        }

        //Re-arm the OUT endpoint for the next packet
        USBOutHandle = HIDRxPacket(HID_EP, (byte*)rx_buffer, sizeof(rx_buffer));
    }
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

void USBCBInitEP(void)
{
    //enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    //Re-arm the OUT endpoint for the next packet
    USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&rx_buffer,sizeof(rx_buffer));
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
            //CDCInitEP();
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            //USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            //USBCBCheckOtherReq();
            //USBCheckCDCRequest();
            USBCheckHIDRequest();
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