

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

    q15 mode_param;
    power_t current_power = OFF;
    mode_t current_mode = mManual;
    colourspace_t current_colourspace = clSRGBW;

    q15 current_brightness = Q15(0.0);
    RGBWColour current_rgbw = RGBWColour(Q15(1.0), Q15(1.0), Q15(1.0), Q15(1.0));
    HSVColour current_hsv = HSVColour(Q15(0.0), Q15(1.0), Q15(1.0));

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

    }

    void SetBrightness(q15 brightness) {
        if (brightness_fader.is_fading) {
            brightness_fader.set_target(brightness);
        }
        else {
            current_brightness = brightness;
            //Update();
        }
    }

    void SetRGBW(const RGBWColour& colour) {
        current_rgbw = colour;
        current_colourspace = (colour.space == RGBWColour::sRGB) ? clSRGBW : clRawRGBW;
    }

    const RGBWColour& GetRGB() {
        return current_rgbw;
    }

    void PowerOn(uint fade) {
        PWMEnable();
        if (fade == 0) {
            Update();
        }
        else {
            brightness_fader.speed = q15(max(Q15_MAXINT / (fade/4), 1));  // Equivalent to (1.0f / fade)
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
            brightness_fader.speed = q15(max(Q15_MAXINT / (fade/4), 1));  // Equivalent to (1.0f / fade)
            brightness_fader.on_finished = &BrightnessFaderFinished;
            brightness_fader.start(Q15(0.0));
        }
    }

    void SetPower(power_t power) {
        if (current_power != power) {
            if (power == ON) {
                PowerOn(1000);
            }
            else {
                PowerOff(1000);
            }
            current_power = power;
        }
    }

    void SetMode(mode_t mode, q15 param) {
        current_mode = mode;
        mode_param = param;
    }

    void Tick() {
        // Animation
        if (brightness_fader.is_fading) {
            brightness_fader.update();
        }

        switch (current_mode) {
            case mManual:
                break; // Do nothing, let SetRGBW control the colour

            case mHsvFade: {
                current_hsv.hue += mode_param;
                current_rgbw = current_hsv.to_rgbw();
                break;
            }

            case mStrobe: {
                current_hsv.hue += Q15(0.01);
                current_rgbw = current_hsv.to_rgbw();
                break;
            }

        }

        // Update the colour every tick.
        // We don't need to update it any faster than this
        Update();
    }
}

