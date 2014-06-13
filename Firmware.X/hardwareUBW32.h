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
#define _TOGGLE_F(alpha,bit)    (TRIS ## alpha ## INV = (1<<bit))
#define R(alpha,bit) alpha,bit

#define INPUT 1
#define OUTPUT 0

#define ANALOG 0
#define DIGITAL 1

///// Configuration //////

#define F_CPU 16000000



#define PIO_LED1 R(B,0) //TODO: Which pins are the LEDs on
#define PIO_LED2 R(B,1)
#define PIO_LED3 R(B,3)


#endif	/* HARDWAREUBW32_H */

