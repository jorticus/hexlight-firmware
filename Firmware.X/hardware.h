/* 
 * File:   hardware.h
 * Author: Jared
 *
 * Created on 10 June 2014, 8:57 PM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H

#if defined(BOARD_HEXLIGHT)
    #include "hardwareHexRGB.h"
    #define BOARD_HEXRGB
#elif defined(BOARD_UBW32)
    #include "hardwareUBW32.h"
#else
    #error Unsupported board
#endif

#endif	/* HARDWARE_H */

