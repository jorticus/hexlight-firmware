/* 
 * File:   colourspaces.hpp
 * Author: Jared
 *
 * Created on 28 June 2014, 2:38 PM
 */

#ifndef COLOURSPACES_HPP
#define	COLOURSPACES_HPP

#include "fixedpoint.hpp"

class Colour {
    // Abstract base class
};

// Represents RGB(W) colour (raw or perceptually-linear)
// Default RGB space is sRGB, so 0.5 will appear half bright for PWM'd LEDs.
class RGBWColour : Colour {
public:
    typedef enum { rawRGB, sRGB } space_t;
    
    RGBWColour(q15 red=0, q15 green=0, q15 blue=0, q15 white=0, space_t space=sRGB) :
        red(red), green(green), blue(blue), white(white), space(space) { }

    // Apply luminance correction (converts sRGB to rawRGB)
    RGBWColour to_linear() const;

    friend RGBWColour operator*(const RGBWColour& lhs, q15 rhs);
    friend RGBWColour operator*(q15 lhs, const RGBWColour& rhs);

    q15 red, green, blue, white;
    space_t space;
};

// Represents HSV colour (white channel is unused)
// Hue:             0.0 (red) to 1.0 (red)
// Saturation:      0.0 (white) to 1.0 (fully saturated)
// Value:           0.0 (black) to 1.0 (full value)
class HSVColour : Colour {
public:
    HSVColour(q15 hue, q15 saturation, q15 value) :
        hue(hue), saturation(saturation), value(value) { }

    // Convert to RGBW (default sRGB colourspace)
    RGBWColour to_rgbw(RGBWColour::space_t space = RGBWColour::sRGB) const;

    q15 hue;
    q15 saturation;
    q15 value;
};

// Represents a RGB(W) colour that shall be perceptually corrected, where 0.5 = half brightness.
//class SRGBWColour : Colour {
//    SRGBWColour(q15 red=0, q15 green=0, q15 blue=0, q15 white=0) :
//        red(red), green(green), blue(blue), white(white) { }
//
//    // Convert to linear RGBW
//    const RGBWColour& to_linear();
//
//    q15 red, green, blue, white;
//};

//class XYYColour {
//public:
//    XYYColour(q15 x=0, q15 y=0, q15 Y=0) :
//        x(x), y(y), Y(Y) { }
//
//    q15 x, y, Y;
//};
//
//class XYZColour {
//public:
//    XYZColour(q15 x=0, q15 y=0, q15 z=0) :
//        x(x), y(y), z(z) { }
//
//    q15 x, y, z;
//};

#endif	/* COLOURSPACES_HPP */

