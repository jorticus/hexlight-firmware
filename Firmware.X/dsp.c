
#include "common.h"
#include "dsp.h"

#define	SCALE		1L<<15			/* 2^15 */
#define	RANGE		1L<<16			/* 2^16 */

float Fract2Float (fractional aVal) {
   /* Local declarations. */
   double scale = pow (2.0, -15.0);		/* 2^(-15) */
   long int fullRange = 1L<<16;			/* 2^(16) */
   long int halfRange = 1L<<15;			/* 2^(15) */
   double decimalVal = 0.0;

   /* Convert. */
   if (aVal >= halfRange) {
      aVal -= fullRange;
   }
   decimalVal = ((double) aVal)*scale;

   /* Return decimal value in floating point. */
   return ((float) decimalVal);
}

fractional Float2Fract (float aVal) {
   /* Local declarations. */
   long int scale = SCALE;
   long int fractVal = 0.0;
   double decimalVal = 0.0;
   double dummy = 0.0;
   int isOdd = 0;

   /* Convert with convergent rounding and saturation. */
   decimalVal = aVal*scale;
   if (aVal >= 0) {
      fractVal = floor (decimalVal);
      dummy = fractVal/2.0;
      isOdd = (int) ((dummy - floor (dummy))*2.0);
      dummy = decimalVal -fractVal;
      if ((dummy > 0.5) || ((dummy == 0.5) && isOdd)) {
         fractVal += 1.0;
      }
      if (fractVal >= scale) {
         fractVal = scale - 1.0;
      }
   } else {	/* aVal < 0 */
      fractVal = ceil (decimalVal);
      if (fractVal != decimalVal) {
         fractVal -= 1.0;
      }
      dummy = fractVal/2.0;
      isOdd = (int) ((dummy - floor (dummy))*2.0);
      dummy = decimalVal -fractVal;
      if ((dummy > 0.5) || ((dummy == 0.5) && isOdd)) {
         fractVal += 1.0;
      }
      if (fractVal < -scale) {
         fractVal = -scale;
      }
   }

   /* Return fractional value. */
   return ((fractional) fractVal);

}
