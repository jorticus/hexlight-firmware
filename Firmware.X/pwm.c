/*
 * PWM output control
 * Author: Jared Sanson
 *
 *
 */


#include <p32xxxx.h>
#include <peripheral/timer.h>
#include <peripheral/outcompare.h>
#include "common.h"
#include "hardware.h"
#include "pwm.h"


#define NUM_CHANNELS 4 // Do not change


//uint channels[NUM_CHANNELS];

volatile unsigned int* OCR[] = {&OC1R, &OC2R, &OC3R, &OC4R};
volatile unsigned int* OCRS[] = {&OC1RS, &OC2RS, &OC3RS, &OC4RS};


#define OCxCON (OC_OFF | OC_IDLE_CON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE)

#define PWMPR 0x03FF // 10-bit resolution


void PWMInitialize() {

    // Set up IO
    _TRIS(PIO_OC1) = OUTPUT;
    _TRIS(PIO_OC2) = OUTPUT;
    _TRIS(PIO_OC3) = OUTPUT;
    _TRIS(PIO_OC4) = OUTPUT;

    // Initialize the timebase (frequency = pb_clock/postscaler/PR2)
    OpenTimer2(T2_OFF | T2_PS_1_1 | T2_SOURCE_INT, 0x03FF);

    // Initialize the OC modules
    OpenOC1(OCxCON, 0, 0);
    OpenOC2(OCxCON, 0, 0);
    OpenOC3(OCxCON, 0, 0);
    OpenOC4(OCxCON, 0, 0);


    PWMSetChannel(PWM1, 100);

    PWMSetChannel(PWM2, 1000);
    PWMSetChannel(PWM3, 1000);
    PWMSetChannel(PWM4, 1000);
}

void PWMEnable() {
    // Enable all OC channels
    OC1CONbits.ON = 1;
    OC2CONbits.ON = 1;
    OC3CONbits.ON = 1;
    OC4CONbits.ON = 1;
    T2CONbits.ON = 1;
}
void PWMDisable() {
    // Disable all OC channels
    T2CONbits.ON = 0;
    OC1CONbits.ON = 0;
    OC2CONbits.ON = 0;
    OC3CONbits.ON = 0;
    OC4CONbits.ON = 0;
}

/*void PWMSetFrequency(uint frequency) {
    PR2 = pb_clock / frequency;
}*/

void PWMSetScaler(pwm_prescaler_t ps) {
    T2CONbits.TCKPS = ps;
}


void PWMSetChannel(pwm_channel_t channel, UINT16 value) {
    *OCRS[channel] = value;
}

UINT16 PWMGetChannel(pwm_channel_t channel) {
    return *OCRS[channel];
}

void PWMUpdate(UINT16 ch1, UINT16 ch2, UINT16 ch3, UINT16 ch4) {
    OC1R = ch1;
    OC2R = ch2;
    OC3R = ch3;
    OC4R = ch4;
}

void PWMAllOn() {
    OC1RS = OC2RS = OC3RS = OC4RS = PR2;
}
void PWMAllOff() {
    OC1RS = OC2RS = OC3RS = OC4RS = 0;
}