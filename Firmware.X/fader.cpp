/* 
 * File:   fade.cpp
 * Author: Jared
 *
 * Created on 19 January 2015, 11:40 PM
 */

#include "common.h"
#include "fixedpoint.hpp"
#include "fader.hpp"

Fader::Fader(q15* variable, q15 speed, finished_cb on_finished)
    : current(variable), target(*variable), speed(speed), cmp(*variable), inc(true), is_fading(false),
    on_finished(on_finished) { }

void Fader::set_target(q15 _target) {
    target = _target;
    cmp = _target;
    inc = (*current < _target);

    // Correct cmp to prevent overflow. (eg, when target=1.0 or 0.0 the speed delta may cause current to overflow)
    if (inc) {
        // If there's not enough range left,
        // change the speed so it'll take exactly one cycle.
        if (cmp < speed)
            speed = cmp;
        else
            cmp -= speed;
    }
    else {
        if (cmp > Q15(1.0)-speed)
            speed = Q15(1.0)-speed;
        else
            cmp += speed;
    }
}

void Fader::start(q15 _target) {
    is_fading = false;
    set_target(_target);
    is_fading = true;
}

void Fader::stop() {
    is_fading = false;
}

void Fader::update() {
    if (is_fading) {
        if (inc) {
            *current += speed;
            if (*current >= cmp) {
                is_fading = false;
                *current = target;
                if (on_finished)
                    on_finished();
            }
        } else {
            *current -= speed;
            if (*current <= cmp) {
                is_fading = FALSE;
                *current = target;
                if (on_finished)
                    on_finished();
            }
        }
    }
}
