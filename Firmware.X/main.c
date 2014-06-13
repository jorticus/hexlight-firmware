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
#include "common.h"
#include "hardware.h"
#include "sound.h"


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

    mJTAGPortEnable(0);

    // Initialize the PIC32 core
    SYSTEMConfig(F_CPU, SYS_CFG_ALL);
    INTEnableInterrupts();
    INTEnableSystemMultiVectoredInt();

    // LEDs
    _TRIS(PIO_LED1) = OUTPUT;
    _TRIS(PIO_LED2) = OUTPUT;
    _TRIS(PIO_LED3) = OUTPUT;
}


int main(void) {
    InitializeSystem();
    SndInitialize();

    SndStartCapture();

    while (1) {
        SndProcess();
    }

    return 0;
}
