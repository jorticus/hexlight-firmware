

#include <cstdlib>
#include "common.h"
#include "pwm.h"
#include "dsp.h"

#include "colourengine.h"


namespace ColourEngine {

    RGB current_colour;
    float current_white = 0.0f;
    float current_brightness = 1.0f;

    static void CIE1931(float lum) {

    }

    static void Update() {
        float r = current_colour.cr * current_brightness;
        float g = current_colour.cg * current_brightness;
        float b = current_colour.cb * current_brightness;
        float w = current_white * current_brightness;

        PWMUpdate(Q15(r), Q15(g), Q15(b), Q15(w));
    }




    void Initialize() {
        Update();
    }

    void SetColour(RGB _colour) {
        current_colour = _colour;
        Update();
    }

    void SetWhiteChannel(float value) {
        current_white = value;
        Update();
    }

    void SetBrightness(float value) {
        current_brightness = value;
        Update();
    }


    void PowerOn() {
        PWMEnable();
        Update();
    }
    void PowerOff() {
        PWMDisable();
    }

}