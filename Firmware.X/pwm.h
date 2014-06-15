/* 
 * File:   pwm.h
 * Author: Jared
 *
 * Created on 14 June 2014, 4:41 PM
 */

#ifndef PWM_H
#define	PWM_H

#include <GenericTypeDefs.h>

typedef enum { PWM1=0, PWM2, PWM3, PWM4 } pwm_channel_t;

typedef enum { PWM_PS_1=0, PWM_PS_2, PWM_PS_4, PWM_PS_8, PWM_PS_16, PWM_PS_32, PWM_PS_64, PWM_PS_256 } pwm_prescaler_t;


void PWMInitialize();

void PWMEnable();
void PWMDisable();

//void PWMSetFrequency(uint frequency);

// Sets the timebase post-scaler, dividing the output frequency. (Default 1:1)
void PWMSetScaler(pwm_prescaler_t ps);

// Sets the output resolution, in bits (range 2-16 bits)
void PWMSetResolution(uint res);

// Adjust the PWM duty cycle
void PWMSetChannel(pwm_channel_t channel, UINT16 value);
UINT16 PWMGetChannel(pwm_channel_t channel);
void PWMUpdate(UINT16 ch1, UINT16 ch2, UINT16 ch3, UINT16 ch4);

void PWMAllOn();
void PWMAllOff();


#endif	/* PWM_H */

