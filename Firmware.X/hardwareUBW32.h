/* 
 * File:   hardwareUBW32.h
 * Author: Jared
 *
 * Created on 13 June 2014, 7:41 PM
 */

#ifndef HARDWAREUBW32_H
#define	HARDWAREUBW32_H

#define _TRIS(pin)            _TRIS_F(pin)
#define _TRIS_F(alpha,bit)    (TRIS ## alpha ## bits.TRIS ## alpha ## bit)
#define _PORT(pin)            _PORT_F(pin)
#define _PORT_F(alpha,bit)    (PORT ## alpha ## bits.R ## alpha ## bit)
#define _LAT(pin)            _LAT_F(pin)
#define _LAT_F(alpha,bit)    (LAT ## alpha ## bits.LAT ## alpha ## bit)
#define toggle(pin)            _TOGGLE_F(pin)
#define _TOGGLE_F(alpha,bit)    (LAT ## alpha ## INV = (1<<bit))
#define R(alpha,bit) alpha,bit

#define INPUT 1
#define OUTPUT 0

#define ANALOG 0
#define DIGITAL 1

///// Configuration //////

#define F_OSC 4000000L  // 8MHz oscillator / 2
extern unsigned int sys_clock;
extern unsigned int pb_clock;


#define PIO_LED_USB     R(E,3)  // Dim Yellow
#define PIO_LED1        R(E,2)  // White
#define PIO_LED2        R(E,1)  // Red
#define PIO_LED3        R(E,0)  // Yellow

#define PIO_BTN_PGM     R(E,7)
#define PIO_BTN_USR     R(E,6)

// UBW32 uses active-low logic for LEDs and buttons
#define HIGH 0
#define LOW 1


#define PIO_OC1 R(D,0)
#define PIO_OC2 R(D,1)
#define PIO_OC3 R(D,2)
#define PIO_OC4 R(D,3)


#endif	/* HARDWAREUBW32_H */

