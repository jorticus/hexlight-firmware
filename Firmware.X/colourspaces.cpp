
#include "common.h"
#include "fixedpoint.hpp"

#include "colourspaces.h"
#include "colourengine.h"


static q15 cielum(q15 intensity) {
    // Converts raw linear intensity to perceptually-linear luminance,
    // using the CIE1931 perceptual luminance model
    if (intensity <= Q15(0.08)) {
        return intensity * Q15(0.110705);
    } else {
        q15 a = (intensity * Q15(0.86207)) + Q15(0.1379312);
        return a*a*a;
    }
}


RGBWColour RGBWColour::to_linear() {
    return RGBWColour(
        cielum(this->red),
        cielum(this->green),
        cielum(this->blue),
        cielum(this->white)
    );
}

RGBWColour operator*(const RGBWColour& lhs, q15 rhs) {
    return RGBWColour(
        lhs.red * rhs,
        lhs.green * rhs,
        lhs.blue * rhs,
        lhs.white * rhs
    );
}

RGBWColour operator*(q15 lhs, const RGBWColour& rhs) {
    return RGBWColour(
        lhs * rhs.red,
        lhs * rhs.green,
        lhs * rhs.blue,
        lhs * rhs.white
    );
}