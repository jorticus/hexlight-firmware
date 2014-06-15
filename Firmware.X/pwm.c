/*
 * PWM output control
 * Author: Jared Sanson
 *
 *
 */


#include <p32xxxx.h>
#include <peripheral/timer.h>
#include <peripheral/outcompare.h>
#include <dsplib_dsp.h>
#include <mchp_dsp_wrapper.h>
#include "common.h"
#include "hardware.h"
#include "pwm.h"

// These map PWM channels to OC outputs
//volatile unsigned int* OCRS[] = {&OC1RS, &OC2RS, &OC3RS, &OC4RS};
volatile unsigned int* OCRS[] = {&OC2RS, &OC3RS, &OC4RS, &OC1RS};

#define OCxCON (OC_OFF | OC_IDLE_CON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE)

//#define PWM_PR 0xFFFF // 16-bit resolution
//#define PWM_PR 0x3FFF // 14-bit resolution
//#define PWM_PR 0x0FFF // 12-bit resolution
#define PWM_PR 0x03FF // 10-bit resolution
//#define PWM_PR 0x00FF // 8-bit resolution



static inline UINT16 calc_duty(INT16 value) {
    // Input range is 0-32767, output range is 0-PWM_PR
    // Using signed ints for compatibility with Q15 fractional datatype
    return (value > 0) ? (PWM_PR * (INT32)value) / 32767 : 0;
}


void PWMInitialize() {

    // Set up IO
    _TRIS(PIO_OC1) = OUTPUT;
    _TRIS(PIO_OC2) = OUTPUT;
    _TRIS(PIO_OC3) = OUTPUT;
    _TRIS(PIO_OC4) = OUTPUT;

    // Initialize the timebase (frequency = pb_clock/postscaler/PR2)
    OpenTimer2(T2_OFF | T2_PS_1_1 | T2_SOURCE_INT, PWM_PR);

    // Initialize the OC modules
    OpenOC1(OCxCON, 0, 0);
    OpenOC2(OCxCON, 0, 0);
    OpenOC3(OCxCON, 0, 0);
    OpenOC4(OCxCON, 0, 0);

    PWMSetChannel(PWM1, Q15(0.1));

    PWMSetChannel(PWM2, Q15(0.1));
    PWMSetChannel(PWM3, Q15(0.1));
    PWMSetChannel(PWM4, Q15(0.1));
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


void PWMSetChannel(pwm_channel_t channel, INT16 value) {
    *OCRS[channel] = calc_duty(value);
}

void PWMUpdate(INT16 ch1, INT16 ch2, INT16 ch3, INT16 ch4) {
    *OCRS[PWM1] = calc_duty(ch1);
    *OCRS[PWM2] = calc_duty(ch2);
    *OCRS[PWM3] = calc_duty(ch3);
    *OCRS[PWM4] = calc_duty(ch4);
}

void PWMAllOn() {
    OC1RS = OC2RS = OC3RS = OC4RS = PR2;
}
void PWMAllOff() {
    OC1RS = OC2RS = OC3RS = OC4RS = 0;
}