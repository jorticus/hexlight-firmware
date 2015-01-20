/* 
 * File:   fade.h
 * Author: Jared
 *
 * Created on 19 January 2015, 11:40 PM
 */

#ifndef FADE_H
#define	FADE_H

#include "fixedpoint.hpp"

typedef void (*finished_cb)(void);

class Fader {
public:
    Fader(q15* variable, q15 speed, finished_cb on_finished = NULL);

    void set_target(q15 target);
    
    void start(q15 target);
    void stop();

    void update();

    q15     speed;
    q15     target;

    bool    inc;
    bool    is_fading;

    finished_cb on_finished;

private:
    q15     cmp;
    q15*    current;
};


#endif	/* FADE_H */

