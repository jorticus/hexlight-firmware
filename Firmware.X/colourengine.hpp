/* 
 * File:   colourengine.hpp
 * Author: Jared
 *
 * Created on 15 June 2014, 5:37 PM
 *
 * Provides colour manipulation and conversion routines.
 */

#ifndef COLOURENGINE_HPP
#define	COLOURENGINE_HPP

#include "colourspaces.hpp"

typedef enum { OFF=0, ON=1 } power_t;


class RGBWColour;
class HSVColour;
class Fader;

class ColourEngine {
public:
    typedef enum { mManual, mHsvFade, mStrobe, NUM_MODES } mode_t;
    typedef enum { clRawRGBW, clSRGBW, clXYZ, clXYY } colourspace_t;

    static void Initialize();

    // Update the colour engine colours
    static void Tick();

    // Set the colour engine mode (see mode_t enum)
    static void SetMode(mode_t mode, q15 param = Q15(0.0));

    // Set the global brightness
    // Note: not updated until Tick() is called.
    static void SetBrightness(q15 brightness);

    // Set the colour and update the PWM outputs. Corrected for linear perception
    // Not recommended, RGB doesn't say anything about the LEDs used.
    static void SetRGBW(const RGBWColour& colour);
    static const RGBWColour& GetRGBW();

    // xyY or XYZ are the best options for setting colour. Corrected for linear perception
//    static void SetXYY(XYYColour colour);
//    static XYYColour GetXYY();
//
//    static void SetXYZ(XYZColour colour);
//    static XYZColour GetXYZ();

    //static void CalibrateChannel(uint channel, XYYColour colour_point);

    // Turn the PWM outputs on or off
    static void PowerOn(uint fade=0);
    static void PowerOff(uint fade=0);
    static void SetPower(power_t power, uint fade=0);

protected:
    static q15              mode_param;
    static power_t          current_power;
    static mode_t           current_mode;
    static colourspace_t    current_colourspace;
    static q15              current_brightness;
    static RGBWColour       current_rgbw;
    static HSVColour        current_hsv;

    static Fader            brightness_fader;

    static void Update();
};

#endif	/* COLOURENGINE_HPP */

