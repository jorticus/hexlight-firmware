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

#define BUILD_NO_BOOTLOADER


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


#ifdef BUILD_NO_BOOTLOADER
#pragma config UPLLEN   = ON        // USB PLL Enabled
#pragma config UPLLIDIV = DIV_4     // USB PLL Input Divider = Divide by 1
#pragma config FUSBIDIO = OFF
#pragma config FVBUSONIO = OFF

#pragma config JTAGEN   = OFF
#pragma config DEBUG    = OFF           // Background Debugger disabled
#pragma config FPLLMUL  = MUL_20         // PLL Multiplier: Multiply by 20
#pragma config FPLLIDIV = DIV_4         // PLL Input Divider:  Divide by 4
#pragma config FPLLODIV = DIV_2         // PLL Output Divider: Divide by 2

#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)

#pragma config FWDTEN = OFF             // WD timer: OFF
#pragma config POSCMOD = HS             // Primary Oscillator Mode: High Speed xtal
#pragma config FNOSC = PRIPLL           // Oscillator Selection: Primary oscillator  w/ PLL
//#pragma config FNOSC = FRC
#pragma config FPBDIV = DIV_1           // Peripheral Bus Clock: Divide by 1
#pragma config BWP = OFF                // Boot write protect: OFF


#pragma config IESO = ON
#pragma config FCKSM = CSECME

#pragma config ICESEL = ICS_PGx2    // ICE pins configured on PGx1 (PGx2 is multiplexed with USB D+ and D- pins).
#endif


// TODO - Verify tick frequency
#define TICK_FREQUENCY 250

unsigned int sys_clock = F_SYSCLK;
unsigned int pb_clock = F_SYSCLK;

bool power = false;
uint8_t mode = 0;
static bool last_btn2 = false;

void SystickInit() {
    OpenTimer4(T4_ON | T4_IDLE_CON | T4_PS_1_8 | T4_32BIT_MODE_OFF | T4_SOURCE_INT, pb_clock/8/TICK_FREQUENCY);
    INTSetVectorPriority(INT_TIMER_4_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_TIMER_4_VECTOR, INT_SUB_PRIORITY_LEVEL_2);
    INTEnable(INT_T4, INT_ENABLED);
}

void InitializeSystem() {

#ifdef BOARD_UBW32
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
#endif
#ifdef BOARD_HEXLIGHT
    ANSELA = 0x0000;
    ANSELB = 0x0000;
#endif
    LATA = 0x0000;
    LATB = 0x0000;

    // Ensure LED drivers are driven low as soon as possible
//    _TRIS(PIO_OC1) = 0;
//    _TRIS(PIO_OC2) = 0;
//    _TRIS(PIO_OC3) = 0;
//    _TRIS(PIO_OC4) = 0;
//    _LAT(PIO_OC1) = OUTPUT;
//    _LAT(PIO_OC2) = OUTPUT;
//    _LAT(PIO_OC3) = OUTPUT;
//    _LAT(PIO_OC4) = OUTPUT;

    // Force disconnect of USB bootloader
    U1CON = 0x00000000;
    U1PWRC = 0x00000000;

    // LEDs
//    _TRIS(PIO_LED1) = OUTPUT;
//    _TRIS(PIO_LED2) = OUTPUT;
#ifdef BOARD_UBW32
    _TRIS(PIO_LED3) = OUTPUT;
    _TRIS(PIO_LED_USB) = OUTPUT;
    _TRIS(PIO_BTN_PGM) = 1;
    _TRIS(PIO_BTN_USR) = 1;
#elif BOARD_HEXLIGHT
    _TRIS(PIO_BTN1) = INPUT;
    _TRIS(PIO_BTN2) = INPUT;
#endif

//    _TRIS(PIO_USBP) = INPUT;
//    _TRIS(PIO_USBN) = INPUT;

//    _LAT(PIO_LED1) = LOW;
//    _LAT(PIO_LED2) = LOW;
#ifdef BOARD_UBW32
    _LAT(PIO_LED3) = HIGH;
    _LAT(PIO_LED_USB) = LOW;
#endif

    mJTAGPortEnable(0);

    // Initializethe PIC32 core
    //OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_2, OSC_FRC_POST_2);
    sys_clock = F_SYSCLK;
    mOSCSetPBDIV(OSC_PB_DIV_1);
    pb_clock = SYSTEMConfig(sys_clock, SYS_CFG_ALL);


    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();

    // Initialize core time base
    SystickInit();
}


int main(void) {
    InitializeSystem();

    // Wait for reset button to be released
    while (_PORT(PIO_BTN1) == HIGH) { }

    //ADCInitialize();
    PWMInitialize();
    PWMEnable();
    USBDeviceInit();

    //ADCStartCapture();
    _LAT(PIO_LED1) = HIGH;

    ColourEngine::Initialize();
    ColourEngine::PowerOn(1000); // Fade in

    //_LAT(PIO_LED2) = HIGH;

    while (1) {
        USBDeviceTasks();
        USBUserProcess();
    }

    return 0;
}

extern "C" {
    void __ISR(_TIMER_4_VECTOR, IPL2SOFT) tick_timer_isr() {
        INTClearFlag(INT_T4);
        ColourEngine::Tick();
        //toggle(PIO_LED2);

        // Switch mode
        if (_PORT(PIO_BTN2) == HIGH) {
            last_btn2 = HIGH;
        }
        else if (last_btn2 == HIGH) {
            last_btn2 = LOW;
            if (mode++ == (int)ColourEngine::NUM_MODES-1)
                mode = 0;
            ColourEngine::SetMode(static_cast<ColourEngine::mode_t>(mode), Q15(0.0001));
        }
    }
}
