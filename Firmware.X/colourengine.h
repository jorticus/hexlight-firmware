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


/*class ColourEngine {
public:
    static void SetColour(RGB colour);
    static void SetWhiteChannel(float value);
    static void SetBrightness(float value);

private:
    static void Update();

    static RGB colour;
    static float white;
    static float brightness;
};*/

namespace ColourEngine {
    void Initialize();
    void Tick1ms();


    // Set the colour and update the PWM outputs. Corrected for linear perception
    // Not recommended, RGB doesn't say anything about the LEDs used.
    void SetRGB(RGBColour colour);
    RGBColour GetRGB();

    // xyY or XYZ are the best options for setting colour. Corrected for linear perception
    void SetXYY(XYYColour colour);
    XYYColour GetXYY();

    void SetXYZ(XYZColour colour);
    XYZColour GetXYZ();

    void CalibrateChannel(uint channel, XYYColour colour_point);

    // Turn the PWM outputs on or off
    void PowerOn(uint fade=0);
    void PowerOff(uint fade=0);
}

#endif	/* COLOURENGINE_H */

