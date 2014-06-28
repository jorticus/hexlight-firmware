/* 
 * File:   hardwareHexRGB.h
 * Author: Jared
 *
 * Created on 13 June 2014, 7:41 PM
 */

#ifndef HARDWAREHEXRGB_H
#define	HARDWAREHEXRGB_H

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

#define PIO_LED_STATUS ??

#define PIO_BTN_PRG ??
#define PIO_BTN1 ??
#define PIO_BTN2 ??

#define PIO_TRIG ??

#define HIGH 1
#define LOW 0


// Remappable I/O
#define PIO_OC1 R(B,3)
#define PIO_OC2 R(B,1)
#define PIO_OC3 R(B,0)
#define PIO_OC4 R(B,2)

#define PIO_USBP ??
#define PIO_USBN ??

#define self_power 1
#define USB_BUS_SENSE 1


#endif	/* HARDWAREHEXRGB_H */

