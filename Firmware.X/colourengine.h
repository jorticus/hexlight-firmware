/* 
 * File:   colourengine.h
 * Author: Jared
 *
 * Created on 15 June 2014, 5:37 PM
 *
 * Provides colour manipulation and conversion routines.
 */

#ifndef COLOURENGINE_H
#define	COLOURENGINE_H

#include "colourspaces.h"

namespace ColourEngine {
    typedef enum { mOff, mManual, mHsvFade, mStrobe, NUM_MODES } mode_t;

    void Initialize();
    void Tick();

    void SetMode(mode_t mode, q15 param = Q15(0.0));

    void SetBrightness(q15 brightness);

    // Set the colour and update the PWM outputs. Corrected for linear perception
    // Not recommended, RGB doesn't say anything about the LEDs used.
    void SetRGBW(const RGBWColour& colour);
    const RGBWColour& GetRGBW();

    // xyY or XYZ are the best options for setting colour. Corrected for linear perception
//    void SetXYY(XYYColour colour);
//    XYYColour GetXYY();
//
//    void SetXYZ(XYZColour colour);
//    XYZColour GetXYZ();

    //void CalibrateChannel(uint channel, XYYColour colour_point);

    // Turn the PWM outputs on or off
    void PowerOn(uint fade=0);
    void PowerOff(uint fade=0);
    
    
}

#endif	/* COLOURENGINE_H */

