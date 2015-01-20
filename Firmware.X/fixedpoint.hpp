/* 
 * File:   fixedpoint.hpp
 * Author: Jared
 *
 * Created on 20 January 2015, 7:06 PM
 */

#ifndef FIXEDPOINT_HPP
#define	FIXEDPOINT_HPP

#include <stdint.h>

/**
 * Fixed-point numeric type
 *
 * I - number of integer bits (including sign)
 * F - number of fractional bits
 * Both bits must sum to 16
 */
template <uint8_t I, uint8_t F>
class fractional {
public:
    // Default constructor
    fractional() { }

    // Implicit conversion from uint16_t
    fractional(int16_t frac) : _value(frac) { }

    // Copy constructor
    fractional(fractional<I,F> const& rhs) : _value(rhs._value) { }

    // Converting copy constructor
    template<uint8_t I2, uint8_t F2>
    fractional(fractional<I2, F2> const& rhs) : _value(rhs._value) {
        if (I-I2 > 0)
            _value >>= I-I2;
        if (I2<I > 0)
            _value <<= I2-I;
    }

    uint16_t value() const {
        return _value;
    }

    // Implicit conversion to uint16_t
//    operator int16_t() const {
//        return value;
//    }

    // Assignment operator
    fractional<I, F>& operator= (fractional<I, F> const& rhs) {
        this->_value = rhs._value;
        return *this;
    }

    // Converting assignment operator
    template<uint8_t I2, uint8_t F2>
    fractional<I, F>& operator= (fractional<I2, F2> const& rhs);

    // Comparison
    bool operator< (fractional<I, F> const& rhs) const {
        return _value < rhs._value;
    }
    bool operator== (fractional<I, F> const& rhs) const {
        return _value == rhs._value;
    }
    bool operator> (fractional<I, F> const& rhs) const {
        return _value > rhs._value;
    }
    bool operator>= (fractional<I, F> const& rhs) const {
        return _value >= rhs._value;
    }
    bool operator<= (fractional<I, F> const& rhs) const {
        return _value <= rhs._value;
    }
    bool operator!= (fractional<I, F> const& rhs) const {
        return _value != rhs._value;
    }

    // Unary negation operator (signed types only!)
    fractional<I, F> operator-() const {
        return fractional<I, F>(-value);
    }

//    // Increment
//    fractional<I, F>& operator++() {
//        value += 1; // TODO
//        return *this;
//    }
//    // Decrement
//    fractional<I, F>& operator--() {
//        value -= 1; // TODO
//        return *this;
//    }

    // Addition-assignment
    fractional<I, F>& operator+=(fractional<I, F> const& frac) {
        _value += frac._value;
        return *this;
    }
    // Subtraction-assignment
    fractional<I, F>& operator-=(fractional<I, F> const& frac) {
        _value -= frac._value;
        return *this;
    }
    // Multiplication-assignment
    fractional<I, F>& operator*=(fractional<I, F> const& frac) {
        _value = (static_cast<uint32_t>(_value) * frac._value) >> F;
        return *this;
    }
    // Division-assignment
    fractional<I, F>& operator/=(fractional<I, F> const& frac) {
        _value = (static_cast<uint32_t>(_value) << F) / frac._value;
        return *this;
    }

    // Addition
    fractional<I, F> operator+(fractional<I, F> const& rhs) const {
        return fractional<I, F>(this->_value + rhs._value);
    }
    // Subtraction
    fractional<I, F> operator-(fractional<I, F> const& rhs) const {
        return fractional<I, F>(this->_value - rhs._value);
    }
    // Multiplication
    fractional<I, F> operator*(fractional<I, F> const& rhs) const {
        return fractional<I, F>( (static_cast<uint32_t>(_value) * rhs._value) >> F );
    }
    // Division
    fractional<I, F> operator/(fractional<I, F> const& rhs) const {
        return fractional<I, F>( (static_cast<uint32_t>(_value) << F) / rhs._value );
    }


protected:
    int16_t _value;
};

#define FRACTIONAL(I,F,X) fractional<I,F>( (X < 0.0) ? (INT16)((1<<F)*(X)-0.5) : (INT16)(((1<<F)-1)*(X)+0.5) )


// Convenience typedefs
typedef fractional<1,15> q15; // -1.0 .. 1.0
typedef fractional<2,14> q14; // -2.0 .. 2.0

// Macros for converting between floating point and fixed point at compile-time
#define Q15(X) FRACTIONAL(1,15,X)
#define Q14(X) FRACTIONAL(2,14,X)

#define Q15_MAXINT  32767
#define Q15_MININT -32768


#endif	/* FIXEDPOINT_HPP */

