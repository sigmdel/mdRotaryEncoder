/* Rotary encoder handler for arduino. v1.1
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 * <https://github.com/buxtronix/arduino/tree/master/libraries/Rotary>
 * 
 * A typical mechanical rotary encoder emits a two bit gray code
 * on 3 output pins. Every step in the output (often accompanied
 * by a physical 'click') generates a specific sequence of output
 * codes on the pins.
 *
 * There are 3 pins used for the rotary encoding - one common and
 * two 'bit' pins.
 *
 * The following is the typical sequence of code on the output when
 * moving from one step to the next:
 *
 *   Position   Bit1   Bit2
 *   ----------------------
 *     Step1     0      0
 *      1/4      1      0
 *      1/2      1      1
 *      3/4      0      1
 *     Step2     0      0
 *
 * From this table, we can see that when moving from one 'click' to
 * the next, there are 4 changes in the output code.
 *
 * - From an initial 0 - 0, Bit1 goes high, Bit0 stays low.
 * - Then both bits are high, halfway through the step.
 * - Then Bit1 goes low, but Bit2 stays high.
 * - Finally at the end of the step, both bits return to 0.
 *
 * Detecting the direction is easy - the table simply goes in the other
 * direction (read up instead of down).
 *
 * To decode this, we use a simple state machine. Every time the output
 * code changes, it follows state, until finally a full steps worth of
 * code is received (in the correct order). At the final 0-0, it returns
 * a value indicating a step in one direction or the other.
 *
 * It's also possible to use 'half-step' mode. This just emits an event
 * at both the 0-0 and 1-1 positions. This might be useful for some
 * encoders where you want to detect all positions.
 *
 * If an invalid state happens (for example we go from '0-1' straight
 * to '1-0'), the state machine resets to the start until 0-0 and the
 * next valid codes occur.
 *
 * The biggest advantage of using a state machine over other algorithms
 * is that this has inherent debounce built in. Other algorithms emit spurious
 * output with switch bounce, but this one will simply flip between
 * sub-states until the bounce settles, then continue along the state
 * machine.
 * A side effect of debounce is that fast rotations can cause steps to
 * be skipped. By not requiring debounce, fast rotations can be accurately
 * measured.
 * Another advantage is the ability to properly handle bad state, such
 * as due to EMI, etc.
 * It is also a lot simpler than others - a static state table and less
 * than 10 lines of logic.
 */

// SPDX-License-Identifier: GPL-3.0-only

#include "Arduino.h"  // for PlatformIO
#include "mdRotaryEncoder.h"


/*
 * The below state table has, for each state (row), the new state
 * to set based on the next encoder output. From left to right in,
 * the table, the encoder outputs are 00, 01, 10, 11, and the value
 * in that position is the new state to set.
 */

#define R_START 0x0

#ifdef HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const uint8_t ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const uint8_t ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif


// Constructor

mdRotary::mdRotary(uint8_t ClockPin, uint8_t DataPin, bool enableInternalPullups) {
  // Assign variables.
  _rollover = true;
  _min = 0;
  _max = 0;
  _position = 0;
  _clock = ClockPin;
  _data = DataPin;

  int mode = (enableInternalPullups) ? INPUT_PULLUP : INPUT;
  pinMode(_clock, mode);
  pinMode(_data, mode);
  // Initialise state.
  _state = R_START;
}

// The state machine "pump"

rotation_t mdRotary::process(void) {
  rotation_t res = (rotation_t) _updateRotary();	
  // call handlers, but only if a step has been completed
  // in other words only if res != DIR_NONE.
  switch (res) {
    case DIR_NONE: /* do nothing and stop warning */ break;
    case DIR_CW:  if (_incPos()) {
        if (this->_OnCbRight) this->_OnCbRight();
        if (this->_OnCbRotated) this->_OnCbRotated(_position);
      }
      break;
    case DIR_CCW:  if (_decPos()) {
      if (this->_OnCbLeft) this->_OnCbLeft();
      if (this->_OnCbRotated) this->_OnCbRotated(_position);
      }
      break;
  }  
  return res;
}

// This was the original process() in buxtronix. It implements the state machine

uint8_t mdRotary::_updateRotary(void) {
  // Grab state of input pins.
  uint8_t pinstate = (digitalRead(_data) << 1) | digitalRead(_clock);
  // Determine new state from the pins and state table.
  _state = ttable[_state & 0xf][pinstate];
  // Return emit bits, ie the generated event.
  return (rotation_t) _state & 0x30; 
}

uint8_t mdRotary::id() {
  return _data;
}

bool mdRotary::_incPos() {
  _position++;
  if (_min < _max && _position > _max ) {
    if (_rollover)
      _position = _min;
    else {
      _position = _max;
      return false; // no callbacks when position already at max
    } 
  }      
  return true;
}

bool mdRotary::_decPos() {
  _position--;
  if (_min < _max && _position < _min ) {
    if (_rollover) 
      _position = _max;
    else {
      _position = _min;
      return false;  // no callbacks when position already at min
    } 
  }      
  return true;
}


int32_t mdRotary::getPosition(void) {
  return _position;
}

int32_t mdRotary::getMin(void) {
  return _min;
}

int32_t mdRotary::getMax(void) {
  return _max;
}

bool mdRotary::getRollover(void) {
  return _rollover;
}

int32_t mdRotary::setPosition(int32_t value) {
  if (_min < _max) {
    if (value < _min) { 
      if (_rollover) value = _max;
      else value = _min;
    }  
    if (value > _max) {
      if (_rollover) value = _min;
      else value = _max;
    }  
  }
  _position = value;
  return _position;
}

int32_t mdRotary::setLimits(int32_t max, int32_t min, bool Rollover) {
  _rollover = Rollover;
  if (min <= max) {
    _min = min;
    _max = max;
    return setPosition(_position);
  }
  return _position;
}

void mdRotary::onButtonRotatedLeft( callback cb ) {
  this->_OnCbLeft = cb;
}

void mdRotary::onButtonRotatedRight( callback cb ) {
  this->_OnCbRight = cb;
}

void mdRotary::onButtonRotated( callback_int cb ) {
  this->_OnCbRotated = cb;
}


