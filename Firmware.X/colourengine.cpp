

#include <p32xxxx.h>
#include <cstdlib>
#include "common.h"
#include "fixedpoint.hpp"
#include "math.hpp"
#include "pwm.h"
#include "hardware.h"

#include "colourengine.hpp"
#include "fader.hpp"

///// Static Member Initialization /////

q15                         ColourEngine::mode_param            = Q15(0.0);
power_t                     ColourEngine::current_power         = OFF;
ColourEngine::mode_t        ColourEngine::current_mode          = mManual;
ColourEngine::colourspace_t ColourEngine::current_colourspace   = clSRGBW;

q15                         ColourEngine::current_brightness    = Q15(0.0);
RGBWColour                  ColourEngine::current_rgbw          = RGBWColour(Q15(1.0), Q15(1.0), Q15(1.0), Q15(1.0));
HSVColour                   ColourEngine::current_hsv           = HSVColour(Q15(0.0), Q15(1.0), Q15(1.0));

Fader                       ColourEngine::brightness_fader(&current_brightness, 1, NULL);

///// Public Methods /////

void ColourEngine::Initialize() {
    // Nothing to initialize
}

void ColourEngine::SetBrightness(q15 brightness) {
    if (brightness_fader.is_fading) {
        brightness_fader.set_target(brightness);
    }
    else {
        current_brightness = brightness;
    }
}

void ColourEngine::SetRGBW(const RGBWColour& colour) {
    current_rgbw = colour;
    current_colourspace = (colour.space == RGBWColour::sRGB) ? clSRGBW : clRawRGBW;
}

const RGBWColour& ColourEngine::GetRGBW() {
    return current_rgbw;
}

void ColourEngine::PowerOn(uint fade) {
    if (fade > 0) {
        brightness_fader.speed = q15(max(Q15_MAXINT / (fade/4), 1));  // Equivalent to (1.0f / fade)
        brightness_fader.on_finished = NULL;
        brightness_fader.start(Q15(1.0));
    }
    
    Update();
    PWMEnable();
}

void ColourEngine::PowerOff(uint fade) {
    if (fade == 0) {
        PWMDisable();
    }
    else {
        brightness_fader.speed = q15(max(Q15_MAXINT / (fade/4), 1));  // Equivalent to (1.0f / fade)
        
        brightness_fader.on_finished = [](void) {
            brightness_fader.on_finished = NULL;
            PWMDisable();
        };
        
        brightness_fader.start(Q15(0.0));
    }
}

void ColourEngine::SetPower(power_t power, uint fade) {
    if (current_power != power) {
        if (power == ON) {
            PowerOn(fade);
        }
        else {
            PowerOff(fade);
        }
        current_power = power;
    }
}

void ColourEngine::SetMode(mode_t mode, q15 param) {
    current_mode = mode;

    // Reset colour in manual mode
    if (mode == mManual) {
        current_rgbw = RGBWColour(Q15(1.0), Q15(1.0), Q15(1.0), Q15(1.0));
    }

    mode_param = param;
}

void ColourEngine::Tick() {
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

///// Private Methods /////

void ColourEngine::Update() {
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
