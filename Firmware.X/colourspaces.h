/* 
 * File:   colourspaces.h
 * Author: Jared
 *
 * Created on 28 June 2014, 2:38 PM
 */

#ifndef COLOURSPACES_H
#define	COLOURSPACES_H

#include "fixedpoint.hpp"

class Colour {
    // Abstract base class
};

// Represents RGB(W) colour (raw or perceptually-linear)
class RGBWColour : Colour {
public:
    typedef enum { rawRGB, sRGB } space_t;
    
    RGBWColour(q15 red=0, q15 green=0, q15 blue=0, q15 white=0, space_t space=sRGB) :
        red(red), green(green), blue(blue), white(white), space(space) { }

    const RGBWColour to_linear();

    friend RGBWColour operator*(const RGBWColour& lhs, q15 rhs);
    friend RGBWColour operator*(q15 lhs, const RGBWColour& rhs);

    q15 red, green, blue, white;
    space_t space;
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

#endif	/* COLOURSPACES_H */

