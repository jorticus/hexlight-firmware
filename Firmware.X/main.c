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
#include "common.h"
#include "hardware.h"

void InitializeSystem() {
    //TODO
}

int main(void) {
    InitializeSystem();

    while (1) {

    }

    return 0;
}