/*
 * PWM output control
 * Author: Jared Sanson
 *
 *
 */


#include <p32xxxx.h>
#include <peripheral/outcompare.h>
#include "common.h"
#include "hardware.h"
#include "pwm.h"

#define NUM_CHANNELS 4

uint channels[NUM_CHANNELS];

void PWMInitialize() {
    // Initialize the OC modules

    // For the HexRGB hardware, set up peripheral remapping
}

void PWMEnable() {
    // Enable all OC channels
}
void PWMDisable() {
    // Disable all OC channels
}

void PWMSetChannel(pwm_channel_t channel, uint value) {
    channels[channel] = value;
    // Update OCx
}

uint PWMGetChannel(pwm_channel_t channel) {
    return channels[channel];
}
