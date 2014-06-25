

#include <p32xxxx.h>
#include <cstdlib>
#include "common.h"
#include "pwm.h"
#include "dsp.h"
#include "hardware.h"

#include "colourengine.h"


namespace ColourEngine {

    RGB current_colour;
    //float current_white = 0.0f;
    float current_brightness = 1.0f;

    namespace Fade {
        void (*_update_cb)(float) = NULL;
        void (*_finished_cb)(void) = NULL;

        float _speed = 0.0f;
        float _cmp = 0.0f;
        float _target = 0.0f;
        float _current = 0.0f;
        bool _inc = TRUE;
        bool is_fading = FALSE;

        void Update() {
            if (_inc) {
                _current += _speed;
                if (_current >= _cmp) {
                    is_fading = FALSE;
                    _current = _target;
                    if (_finished_cb)
                        _finished_cb();
                }
            } else {
                _current -= _speed;
                if (_current <= _cmp) {
                    is_fading = FALSE;
                    _current = _target;
                    if (_finished_cb)
                        _finished_cb();
                }
            }
            if (_update_cb)
                _update_cb(_current);
        }

        void Fade(float from, float to, float speed, void (*update_cb)(float)=NULL, void (*finished_cb)(void)=NULL) {
            _current = from;
            _target = to;
            _speed = speed;
            _inc = (from < to);
            _cmp = _target;
            /*if (_inc) {
                _cmp = _target - _speed;
            } else {
                _cmp = _target + _speed;
            }*/
            _update_cb = update_cb;
            _finished_cb = finished_cb;

            if (_update_cb)
                _update_cb(_current);

            is_fading = TRUE;
        }
    }

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
        /*float r = cielum(current_colour.cr * current_brightness);
        float g = cielum(current_colour.cg * current_brightness);
        float b = cielum(current_colour.cb * current_brightness);*/
        float r = current_colour.cr;
        float g = current_colour.cg;
        float b = current_colour.cb;

        PWMUpdate(Q15(r), Q15(g), Q15(b), 0);
        _LAT(PIO_LED2) = LOW;
    }



    void Initialize() {
        // Set initial PWM values
        Update();
    }

    void Tick1ms() {
        if (Fade::is_fading) {
            Fade::Update();
        }
    }


    void SetColour(RGB _colour) {
        current_colour = _colour;
        Update();
    }

    /*void SetColourXYZ(XYZ _colour) {
        _LAT(PIO_LED2) = HIGH;
        float r = cielum(current_colour.cr * current_brightness);
        float g = cielum(current_colour.cg * current_brightness);
        float b = cielum(current_colour.cb * current_brightness);


        PWMUpdate(Q15(r), Q15(g), Q15(b), 0);
        _LAT(PIO_LED2) = LOW;
    }*/

    /*void SetWhiteChannel(float value) {
        current_white = value;
        Update();
    }*/

    void SetBrightness(float value) {
        current_brightness = value;
        Update();
    }


    void PowerOn(uint fade) {
        PWMEnable();
        if (fade == 0) {
            Update();
        } else {
            Fade::Fade(0.0f, 1.0f, 1.0f / (float)fade, SetBrightness);
        }
    }
    static void _PowerOffFadeFinished() {
        PWMDisable();
    }
    void PowerOff(uint fade) {
        if (fade == 0) {
            PWMDisable();
        } else {
            Fade::Fade(current_brightness, 0.0f, 1.0f / (float)fade, SetBrightness, _PowerOffFadeFinished);
        }
    }




}

