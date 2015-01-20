

#include <p32xxxx.h>
#include <cstdlib>
#include "common.h"
#include "fixedpoint.hpp"
#include "math.hpp"
#include "pwm.h"
#include "hardware.h"

#include "colourengine.h"
#include "fade.h"


namespace ColourEngine {

    typedef enum { clRawRGBW, clSRGBW, clXYZ, clXYY } colourspace_t;

    colourspace_t current_colourspace = clSRGBW;

    q15 current_brightness = Q15(0.0);
    RGBWColour current_rgbw = RGBWColour(Q15(1.0), Q15(1.0), Q15(1.0), Q15(1.0));
//    XYZColour current_xyz;
//    XYYColour current_xyy;

    Fader brightness_fader(&current_brightness, 1, NULL);

    static void Update() {
        _LAT(PIO_LED2) = HIGH;

        // Adjust current colour by brightness
        RGBWColour colour = current_rgbw * current_brightness;

        // Raw RGBW values are passed directly to the PWM outputs.
        // Note that scaling by brightness will probably not look right, so refrain from using brightness control in this case!

        // sRGBW values operate in a perceptually-uniform colour space
        // ie, 0.5 = half brightness (not half power!)
        // These values can be scaled while still appearing linear.
        if (colour.space == RGBWColour::sRGB) {
            colour = colour.to_linear();
        }

        PWMUpdate(colour.red, colour.green, colour.blue, colour.white);
        _LAT(PIO_LED2) = LOW;
    }

    void Initialize() {
        // Set initial PWM values
        current_rgbw = RGBWColour(0, 0, 0, 0);
        Update();
    }

    void Tick1ms() {
        // Animation
        if (brightness_fader.is_fading) {
            brightness_fader.update();
        }
        Update();
    }

    void SetBrightness(q15 brightness) {
        if (brightness_fader.is_fading) {
            brightness_fader.set_target(brightness);
        }
        else {
            current_brightness = brightness;
            Update();
        }
    }

    void SetRGBW(const RGBWColour& colour) {
        current_rgbw = colour;
        current_colourspace = (colour.space == RGBWColour::sRGB) ? clSRGBW : clRawRGBW;
        //Update();
    }
//    void SetXYY(XYYColour colour) {
//        current_xyy = colour;
//        current_colourspace = clXYY;
//        Update();
//    }
//    void SetXYZ(XYZColour colour) {
//        current_xyz = colour;
//        current_colourspace = clXYZ;
//        Update();
//    }

    const RGBWColour& GetRGB() {
        return current_rgbw;
    }
//    XYYColour GetXYY() {
//        return current_xyy;
//    }
//    XYZColour GetXYZ() {
//        return current_xyz;
//    }

//    void CalibrateChannel(uint channel, XYYColour colour_point) {
//        //TODO
//    }

    void PowerOn(uint fade) {
        PWMEnable();
        if (fade == 0) {
            Update();
        }
        else {
            brightness_fader.speed = q15(max(Q15_MAXINT / fade, 1));  // Equivalent to (1.0f / fade)
            brightness_fader.start(Q15(1.0));
        }
    }

    static void BrightnessFaderFinished() {
        brightness_fader.on_finished = NULL;
        PWMDisable();
    }

    void PowerOff(uint fade) {
        if (fade == 0) {
            PWMDisable();
        }
        else {
            brightness_fader.speed = q15(max(Q15_MAXINT / fade, 1));  // Equivalent to (1.0f / fade)
            brightness_fader.on_finished = &BrightnessFaderFinished;
            brightness_fader.start(Q15(0.0));
        }
    }


}

