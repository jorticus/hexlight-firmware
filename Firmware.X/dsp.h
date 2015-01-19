/* 
 * File:   dsp.h
 * Author: Jared
 *
 * Created on 15 June 2014, 4:53 PM
 */

#ifndef DSP_H
#define	DSP_H

#include <stdint.h>
#include <math.h>                /* fabs, sin, cos, atan, sqrt */

typedef int16_t q15;
typedef q15 fractional;

#define Q15(X) \
   ((X < 0.0) ? (INT16)(32768*(X) - 0.5) : (INT16)(32767*(X) + 0.5))



#ifdef	__cplusplus
extern "C" {
#endif

//extern fractional Float2Fract (         /* Converts float into fractional */
//   float aVal                           /* float value in range [-1, 1) */
//);
//extern float Fract2Float (        /* Converts fractional into float */
//   fractional aVal         /* fract value in range {-1, 1-2^-15} */
//);

#ifdef	__cplusplus
}
#endif

#endif	/* DSP_H */

