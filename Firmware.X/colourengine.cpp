

#include <p32xxxx.h>
#include <cstdlib>
#include "common.h"
#include "pwm.h"
#include "dsp.h"
#include "hardware.h"

#include "colourengine.h"


namespace ColourEngine {

    typedef enum { clRGB, clXYZ, clXYY } colourspace_t;

    colourspace_t current_colourspace = clRGB;
    bool linear_colour = false; // If true, the colours are processed linearly, ie. no perceptual correction is done.

    RGBColour current_rgb;
    XYZColour current_xyz;
    XYYColour current_xyy;



    static float cielum(float intensity) {
        // Converts raw linear intensity to perceptually-linear luminance,
        // using the CIE1931 perceptual luminance model
        if (intensity <= 0.08) {
            return intensity * 0.110705;
        } else {
            float a = (intensity + 0.16) * 0.86207;
            return a*a*a;
        }
    }



    static void Update() {
        _LAT(PIO_LED2) = HIGH;
        float a=0, b=0, c=0, d=0;

        switch (current_colourspace) {
            case clRGB:
                a = current_rgb.red;
                b = current_rgb.green;
                c = current_rgb.blue;
                d = 0;
                break;

            case clXYZ:
                //TODO: XYZ to RGB
                break;

            case clXYY:
                //TODO: xyY to RGB
                break;
        }

        if (!linear_colour) {
            a = cielum(a);
            b = cielum(b);
            c = cielum(c);
            d = cielum(d);
        }

        PWMUpdate(Q15(a), Q15(b), Q15(c), Q15(d));
        _LAT(PIO_LED2) = LOW;
    }



    void Initialize() {
        // Set initial PWM values
        Update();
    }

    void Tick1ms() {

    }


    void SetRGB(RGBColour colour) {
        current_rgb = colour;
        current_colourspace = clRGB;
        Update();
    }
    void SetXYY(XYYColour colour) {
        current_xyy = colour;
        current_colourspace = clXYY;
        Update();
    }
    void SetXYZ(XYZColour colour) {
        current_xyz = colour;
        current_colourspace = clXYZ;
        Update();
    }

    RGBColour GetRGB() {
        return current_rgb;
    }
    XYYColour GetXYY() {
        return current_xyy;
    }
    XYZColour GetXYZ() {
        return current_xyz;
    }

    void CalibrateChannel(uint channel, XYYColour colour_point) {
        //TODO
    }

    void PowerOn(uint fade) {
        PWMEnable();
    }
    void PowerOff(uint fade) {
        PWMDisable();
    }

}

