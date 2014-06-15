/* 
 * File:   pwm.h
 * Author: Jared
 *
 * Created on 14 June 2014, 4:41 PM
 */

#ifndef PWM_H
#define	PWM_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum { PWM1=0, PWM2, PWM3, PWM4 } pwm_channel_t;

typedef enum { PWM_PS_1=0, PWM_PS_2, PWM_PS_4, PWM_PS_8, PWM_PS_16, PWM_PS_32, PWM_PS_64, PWM_PS_256 } pwm_prescaler_t;


void PWMInitialize();

void PWMEnable();
void PWMDisable();

//void PWMSetFrequency(uint frequency);

// Sets the timebase post-scaler, dividing the output frequency. (Default 1:1)
void PWMSetScaler(pwm_prescaler_t ps);

// Adjust the PWM duty cycle
void PWMSetChannel(pwm_channel_t channel, INT16 value);
void PWMUpdate(INT16 ch1, INT16 ch2, INT16 ch3, INT16 ch4);

void PWMAllOn();
void PWMAllOff();

#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */

