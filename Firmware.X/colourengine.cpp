

#include <cstdlib>
#include "common.h"
#include "pwm.h"
#include "dsp.h"

#include "colourengine.h"


namespace ColourEngine {

    RGB current_colour;
    float current_white = 0.0f;
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

    void SetWhiteChannel(float value) {
        current_white = value;
        Update();
    }

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

