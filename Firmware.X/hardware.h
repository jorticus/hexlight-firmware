/* 
 * File:   hardware.h
 * Author: Jared
 *
 * Created on 10 June 2014, 8:57 PM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H

#if defined(__32MX220F032B__)
    #include "hardwareHexRGB.h"
    #define BOARD_HEXRGB
#elif defined(__32MX460F512L__)
    #include "hardwareUBW32.h"
    #define BOARD_UBW32
#else
    #error Unsupported part
#endif

#endif	/* HARDWARE_H */

