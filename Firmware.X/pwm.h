/* 
 * File:   pwm.h
 * Author: Jared
 *
 * Created on 14 June 2014, 4:41 PM
 */

#ifndef PWM_H
#define	PWM_H

typedef enum { pwm0, pwm1, pwm2, pwm3 } pwm_channel_t;

void PWMInitialize();

void PWMEnable();
void PWMDisable();

void PWMSetChannel(pwm_channel_t channel, uint value);
uint PWMGetChannel(pwm_channel_t channel);


#endif	/* PWM_H */

