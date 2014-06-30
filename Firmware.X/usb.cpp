

#include <p32xxxx.h>
#include "hardware.h"
#include "common.h"

extern "C" {
    #include "USB/usb.h"
    #include "USB/usb_function_cdc.h"
    #include "USB/usb_function_hid.h"
    #include "USB/usb_function_audio.h"
    #include "test_audio.h"
}
#include "usb.h"

#include "colourengine.h"
#include "comms.h"

bool usb_sleeping = FALSE;

#ifdef USB_USE_HID
USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;
#endif

#ifdef USB_USE_AUDIO_CLASS
USB_HANDLE USBAudioTxHandle = 0;
USB_HANDLE USBAudioRxHandle = 0;
int transferUnderProgress  =0;
unsigned long FrameCounter =0;
unsigned char FrameData[16];

bool enableUsbAudio = false;

#endif


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
    static byte test = 0;

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) {
        _LAT(PIO_LED_USB) = LOW;
        return;
    }
    _LAT(PIO_LED_USB) = HIGH;

#ifdef USB_USE_HID

    if (!HIDRxHandleBusy(USBOutHandle) && !HIDTxHandleBusy(USBInHandle)) { //Check if data was received from the host.

        int result = cdcProtocolFramer.ProcessData(rx_buffer, sizeof(rx_buffer));

        if (result < 0) {
            byte error_code = -result;
            cdcProtocolFramer.PreparePacket(CMD_ERROR, &error_code, 1);
        }

        // Clear TX buffer
        for (int i=0; i<sizeof(tx_buffer); i++)
           tx_buffer[i] = 0;

        // Copy TX data
        if (cdcProtocolFramer.tx_size > 0) {
            for (int i=0; i<cdcProtocolFramer.tx_size; i++)
                tx_buffer[i] = cdcProtocolFramer.tx_buffer[i];
        }
        
        USBInHandle = HIDTxPacket(HID_EP, (byte*)tx_buffer, sizeof(tx_buffer));

        //Re-arm the OUT endpoint for the next packet
        USBOutHandle = HIDRxPacket(HID_EP, (byte*)rx_buffer, sizeof(rx_buffer));
    }

#endif
#ifdef USB_USE_CDC
    
    numBytesRead = getsUSBUSART((char*)rx_buffer, 64);
    if (numBytesRead != 0) {
        int result = cdcProtocolFramer.ProcessData(rx_buffer, numBytesRead);

        if (result < 0) {
            byte error_code = -result;
            cdcProtocolFramer.PreparePacket(CMD_ERROR, &error_code, 1);
        }


        // Clear TX buffer
        for (int i=0; i<sizeof(tx_buffer); i++)
           tx_buffer[i] = 0;

        // Copy TX data
        if (cdcProtocolFramer.tx_size > 0) {
            for (int i=0; i<cdcProtocolFramer.tx_size; i++)
                tx_buffer[i] = cdcProtocolFramer.tx_buffer[i];
            numBytesToWrite = cdcProtocolFramer.tx_size;
        }
    }

    if (numBytesToWrite > 0 && USBUSARTIsTxTrfReady()) {
        putUSBUSART((char*)tx_buffer, numBytesToWrite);
        numBytesToWrite = 0;
    }

    CDCTxService();

#endif
#ifdef USB_USE_AUDIO_CLASS

    if (enableUsbAudio) {
        unsigned char sampleCounter;
        if(!USBHandleBusy(USBAudioTxHandle)) {
            for(sampleCounter =0; sampleCounter < NO_OF_BYTES_TRANSFRED_IN_A_USB_FRAME; sampleCounter=sampleCounter+2) {
                FrameData[sampleCounter] =   audioSamples[FrameCounter+sampleCounter];
                FrameData[sampleCounter+1] = audioSamples[FrameCounter+sampleCounter+1];
            }
            FrameCounter += NO_OF_BYTES_TRANSFRED_IN_A_USB_FRAME;
            if (FrameCounter >= sizeof(audioSamples)/sizeof(UINT)) {
                FrameCounter = 0;
                transferUnderProgress = 0;
            }
            USBAudioTxHandle = USBTxOnePacket(AS_EP , FrameData,NO_OF_BYTES_TRANSFRED_IN_A_USB_FRAME);
        }
    }
     
#endif
}

void USBCBInitEP(void)
{
#ifdef USB_USE_HID
    //enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    //Re-arm the OUT endpoint for the next packet
    USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&rx_buffer,sizeof(rx_buffer));
#endif
#ifdef USB_USE_CDC
    CDCInitEP();
#endif
#ifdef USB_USE_AUDIO_CLASS
    //enable the Audio Streaming(Isochronous) endpoint
    USBEnableEndpoint(AS_EP ,USB_OUT_ENABLED|USB_IN_ENABLED|USB_DISALLOW_SETUP);
#endif
}

void USBCBCheckOtherReq() {
#ifdef USB_USE_CDC
    USBCheckCDCRequest();
#endif
#ifdef USB_USE_HID
    USBCheckHIDRequest();
#endif
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
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            //USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
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



