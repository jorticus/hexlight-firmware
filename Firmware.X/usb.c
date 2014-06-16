

#include <p32xxxx.h>
#include "hardware.h"
#include "USB/usb.h"
#include "USB/usb_function_cdc.h"




void USBCBSuspend() {

}

void USBCBWakeFromSuspend() {

}

void USBCB_SOF_Handler() {

}

void USBCBErrorHandler() {

}

void USBCBCheckOtherReq() {
    USBCheckCDCRequest();
}

void USBCBStdSetDscHandler() {

}

void USBCBInitEP() {
    CDCInitEP();
}

void USBCBSendResume() {
    static WORD delay_count;

    USBResumeControl = 1;                // Start RESUME signaling

    delay_count = 1800U;                // Set RESUME line for 1-13 ms
    do
    {
        delay_count--;
    }while(delay_count);
    USBResumeControl = 0;
}

BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
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

