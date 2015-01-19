
#include "common.h"
#include "dsp.h"

#include "colourspaces.h"
#include "colourengine.h"


static uint cielum(uint intensity) {
    // Converts raw linear intensity to perceptually-linear luminance,
    // using the CIE1931 perceptual luminance model
//        if (intensity <= 0.08) {
//            return intensity * 0.110705;
//        } else {
//            float a = (intensity + 0.16) * 0.86207;
//            return a*a*a;
//        }
    //TODO
    return intensity;
}


const RGBWColour RGBWColour::to_linear() {
    return RGBWColour(
        cielum(this->red),
        cielum(this->green),
        cielum(this->blue),
        cielum(this->white)
    );
}