
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


RGBWColour RGBWColour::to_linear() const {
    if (this->space == RGBWColour::sRGB) {
        // Convert sRGB to rawRGB
        return RGBWColour(
            cielum(this->red),
            cielum(this->green),
            cielum(this->blue),
            cielum(this->white),
            RGBWColour::rawRGB
        );
    }
    else {
        // RGBW value is already luminance-corrected!
        return *this;
    }
}

RGBWColour operator*(const RGBWColour& lhs, q15 rhs) {
    return RGBWColour(
        lhs.red * rhs,
        lhs.green * rhs,
        lhs.blue * rhs,
        lhs.white * rhs,
        lhs.space
    );
}

RGBWColour operator*(q15 lhs, const RGBWColour& rhs) {
    return RGBWColour(
        lhs * rhs.red,
        lhs * rhs.green,
        lhs * rhs.blue,
        lhs * rhs.white,
        rhs.space
    );
}


RGBWColour HSVColour::to_rgbw(RGBWColour::space_t space) const {
    q15 red, green, blue;

    // Edge case
    if (saturation == Q15(0.0)) {
        return RGBWColour(Q15(0.0), Q15(0.0), Q15(0.0), Q15(0.0), space);
    }

    // TODO - Replace the following integer arithmetic with fractional (will require 32-bit fractional types!)
    uint32_t region = ((uint32_t)hue.value() / 5461) % 6; // 60 degrees
    q15 remainder = q15((hue.value() - (region * 5461)) * 6);

    q15 v = value;
    q15 p = value * (Q15(1.0) - saturation);
    q15 q = value * (Q15(1.0) - remainder * saturation);
    q15 t = value * (Q15(1.0) - (Q15(1.0) - remainder) * saturation);

    switch (region) {
        case 0: red = v; green = t; blue = p; break;
        case 1: red = q; green = v; blue = p; break;
        case 2: red = p; green = v; blue = t; break;
        case 3: red = p; green = q; blue = v; break;
        case 4: red = t; green = p; blue = v; break;
        case 5: red = v; green = p; blue = q; break;
    }

//    red = remainder;
//    green = q15(region * 5461);
//    blue = Q15(0.0);

    q15 white = Q15(0.0); // White channel unused.
    return RGBWColour(red, green, blue, white, space);
}