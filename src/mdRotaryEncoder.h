/*
 * Rotary encoder library for Arduino 
 * an extension of Rotary Library by Ben Buxton
 * <https://github.com/buxtronix/arduino/tree/master/libraries/Rotary>
 * 
 * Added callback functions, a position field, and position min and max range
 * 
 * Can be used in conjuction with the mdPushButton library
 * <https://github.com/sigmdel/mdPushButon to handle KY-040 type rotary encoders
 * with push button.
 * 
 * Michel Deslierres
 * 2020-12-23
 * 
 * 
 * Original Ben Buxton notice
 * 
 * Rotary encoder handler for arduino. v1.1
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 * 
 * See the full notice in mdRotaryEncoder.cpp
 */

 // SPDX-License-Identifier: GPL-3.0-only

#ifndef MDROTARYENCODER_H
#define MDROTARYENCODER_H

#include "Arduino.h"  // for PlatformIO

// Enable this to emit codes twice per step.
//#define HALF_STEP

// Values returned by process()
enum rotation_t { 
   DIR_NONE = 0x00,  // No complete step yet.
   DIR_CW = 0x10,    // button rotated right (clockwise step)
   DIR_CCW = 0x20    // button rotated left (counter-clockwise step)
};

// Callback type definition for handler such as buttonClicked(int clicks)
typedef void (*callback)();
typedef void (*callback_int)(int);

class mdRotary
{

  public:
    mdRotary(uint8_t ClockPin, uint8_t DataPin, bool enableInternalPullups = false);

    rotation_t process(void);
   
    // Returns the GPIO DataPin connected to the rotary encoder.
    // Used to identify the object in a callback function.
    uint8_t id(); // returns DataPin

    // Returns the current position of the encoder.
    // The default position when the object is 
    // instantiated is 0. See setPosition();
    int32_t getPosition(void);  

    // Returns the minimum allowed position. By default min is 0.
    // See setLimits().
    int32_t getMin(void);

    // Returns the maximum allowed position. By default max is 0.
    // See setLimits().
    int32_t getMax(void);

    // Returns true if rollover from max to min or min to max
    // is allowed. By default rollover is true. However by default
    // min = max = 0 so rollover has no effect until min is 
    // set lower than max. See setLimits()
    bool getRollover(void);

    // Set the minimum and maximum position and if position roll over is
    // allowed or not. If necessary position will be clamped between
    // min and max. Returns position.
    //
    // If min > max, then only rollover will be set.
    //
    // If min = max then the min, max and rollover will be ignored. 
    // Position is then a 32 bit integer with a range from -2147483648
    // to 2147483647 and roll over will occur.
    // 
    // Roll over on clockwise rotation means position will go from 
    // max to min. If roll over is not allowed, position will stay
    // at max.
    //
    // Roll over on counter-clockwise rotation means position will go
    // from min to max.If roll over is not allowed, position will stay
    // at min.
    //
    int32_t setLimits(int32_t max, int32_t min = 0, bool rollover = true);

    // Set the current position. Note that position will be clamped between
    // min and max if min < max.
    int32_t setPosition(int32_t value);
 
    // Set a callback function to be called when the button has been rotated  
    // in the specified direction. 
    void onButtonRotatedLeft(callback);
    void onButtonRotatedRight(callback);

    // Set a handler for rotation in any direction
    void onButtonRotated(callback_int);
  protected:
    callback _OnCbLeft;
    callback _OnCbRight;
    callback_int _OnCbRotated;
  private:
    uint8_t _clock;
    uint8_t _data;
    uint8_t _state;
    uint8_t _pressed;
    int32_t _position;
    int32_t _min;
    int32_t _max;
    bool _rollover;
    bool _incPos(void);
    bool _decPos(void);
    uint8_t _updateRotary(void);
  };

#endif
 
