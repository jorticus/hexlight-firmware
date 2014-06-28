/*
 * HexRGB Firmware v0.1
 * Author: Jared Sanson
 *
 * HexRGB is a PIC32-based USB LED driver, with four output channels
 * capable of 12-bit resolution at >1kHz (TODO: Specify the actual frequency).
 * Additionally it has an RS485 interface and a microphone pre-amp for
 * sound-light applications.
 *
 * Built for:
 *   PIC32MX220F032B (HexRGB hardware)
 *   PIC32MX460F512L (UBW32 dev board)
 */

#include <p32xxxx.h>
#include <plib.h>
#include <peripheral/timer.h>
#include "common.h"
#include "hardware.h"

extern "C" {
    #include "USB/usb.h"
    #include "USB/usb_function_cdc.h"
}

#include "adc.h"
#include "pwm.h"
#include "usb.h"
#include "colourengine.h"


#define TICK_FREQUENCY 1000

unsigned int sys_clock = F_OSC;
unsigned int pb_clock = F_OSC;


void SystickInit() {
    OpenTimer4(T4_ON | T4_IDLE_CON | T4_PS_1_8 | T4_32BIT_MODE_OFF | T4_SOURCE_INT, pb_clock/8/TICK_FREQUENCY);
    INTSetVectorPriority(INT_TIMER_4_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_TIMER_4_VECTOR, INT_SUB_PRIORITY_LEVEL_2);
    INTEnable(INT_T4, INT_ENABLED);
}

void InitializeSystem() {

    // Disable ADC port (allows PORTB to be used for digital I/O)
    AD1PCFG = 0xFFFF;

    TRISE = 0x0000;
    TRISB = 0x0000;
    TRISC = 0x0000;
    TRISD = 0x0000;
    LATE = 0x0000;
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;

    // Force disconnect of USB bootloader
    U1CON = 0x00000000;
    U1PWRC = 0x00000000;

    // LEDs
    _TRIS(PIO_LED1) = OUTPUT;
    _TRIS(PIO_LED2) = OUTPUT;
    _TRIS(PIO_LED3) = OUTPUT;
    _TRIS(PIO_LED_USB) = OUTPUT;
    _TRIS(PIO_BTN_PGM) = 1;
    _TRIS(PIO_BTN_USR) = 1;

    _TRIS(PIO_USBP) = INPUT;
    _TRIS(PIO_USBN) = INPUT;

    _LAT(PIO_LED1) = LOW;
    _LAT(PIO_LED2) = LOW;
    _LAT(PIO_LED3) = HIGH;
    _LAT(PIO_LED_USB) = LOW;

    mJTAGPortEnable(0);

    // Initializethe PIC32 core
    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_1, OSC_FRC_POST_1);
    sys_clock = F_OSC * 20 / 1; // Assumes the above configuration
    mOSCSetPBDIV(OSC_PB_DIV_1);
    pb_clock = SYSTEMConfig(sys_clock, SYS_CFG_ALL);


    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();

    // Initialize core time base
    SystickInit();
}


int main(void) {
    InitializeSystem();
    ADCInitialize();
    PWMInitialize();
    USBDeviceInit();

    //ADCStartCapture();

    ColourEngine::Initialize();
    ColourEngine::PowerOn(1000);

    _LAT(PIO_LED3) = LOW;

    {
        RGBColour colour(1.0, 1.0, 1.0);

        //ColourEngine::SetBrightness(1.0f);
        ColourEngine::SetRGB(colour);
        

    }

    //_LAT(PIO_LED1) = HIGH;

    while (1) {
        USBDeviceTasks();
        USBUserProcess();
 


        // Convenience - pressing PGM also resets the device
        if (_PORT(PIO_BTN_PGM) == HIGH) {
            SoftReset();
        }
    }

    return 0;
}

extern "C" {
    void __ISR(_TIMER_4_VECTOR, IPL2SOFT) tick_timer_isr() {
        INTClearFlag(INT_T4);
        ColourEngine::Tick1ms();
        //toggle(PIO_LED2);
    }
}
