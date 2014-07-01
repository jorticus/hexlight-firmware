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
    RGBColour(float red=0, float green=0, float blue=0) :
        red(red), green(green), blue(blue)   {};

    float red, green, blue;
};

class XYYColour {
public:
    XYYColour(float x=0, float y=0, float Y=0) :
        x(x), y(y), Y(Y) {};

    float x, y, Y;
};

class XYZColour {
public:
    XYZColour(float x=0, float y=0, float z=0) :
        x(x), y(y), z(z) {};

    float x, y, z;
};

#endif	/* COLOURSPACES_H */

