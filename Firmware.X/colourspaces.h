/* 
 * File:   colourspaces.h
 * Author: Jared
 *
 * Created on 28 June 2014, 2:38 PM
 */

#ifndef COLOURSPACES_H
#define	COLOURSPACES_H

// Represents RGB(W) colour which can be used to update the PWM channels
class RGBColour {
public:
    RGBColour(uint red=0, uint green=0, uint blue=0) :
        red(red), green(green), blue(blue)   {};

    uint red, green, blue;
};

class XYYColour {
public:
    XYYColour(uint x=0, uint y=0, uint Y=0) :
        x(x), y(y), Y(Y) {};

    uint x, y, Y;
};

class XYZColour {
public:
    XYZColour(uint x=0, uint y=0, uint z=0) :
        x(x), y(y), z(z) {};

    uint x, y, z;
};

#endif	/* COLOURSPACES_H */

