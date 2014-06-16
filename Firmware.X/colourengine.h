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


// Represents RGB(W) colour which can be used to update the PWM channels
class RGB {
public:
    RGB(float red=0, float green=0, float blue=0) :
        cr(red), cg(green), cb(blue)  {};

    float cr, cg, cb;
};

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

    // Set the colour and update the PWM outputs
    void SetColour(RGB colour);
    void SetWhiteChannel(float value);
    void SetBrightness(float value);

    // Turn the PWM outputs on or off, optionally fading.
    // fade specifies the amount of time to fade over, in milliseconds.
    // These calls are non-blocking
    void PowerOn(uint fade=0);
    void PowerOff(uint fade=0);
}

#endif	/* COLOURENGINE_H */

