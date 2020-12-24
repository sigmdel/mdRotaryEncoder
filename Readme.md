# mdRotaryEncoder Library

A rotary encoder library for Arduino. In this fork, callback functions for the following
events 

    OnButtonRotatedRight   // encoder rotated clockwise, callback function with no parameters
    OnButtonRotatedLeft    // encoder rotated counter-clockwise, callback function with no parameters
    OnButtonRotated        // encoder rotated, callback function with a rotation_t parameter giving the direction

have been added to the Ben Buxton Rotary library
<https://github.com/buxtronix/arduino/tree/master/libraries/Rotary>
 
Also added a `position` indicator which is an integer that is increased when the 
encoder is turned clockwise and decreased when the encoder is turned counter clockwise.
The position indicator can be limited to a range with or without rollover when it would
reach a value beyond the minimum or maximum values.

# Original README by Ben Buxton

Rotary encoder handler for arduino. v1.1

Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
Contact: bb@cactii.net

A typical mechanical rotary encoder emits a two bit gray code
on 3 output pins. Every step in the output (often accompanied
by a physical 'click') generates a specific sequence of output
codes on the pins.

There are 3 pins used for the rotary encoding - one common and
two 'bit' pins.

The following is the typical sequence of code on the output when
moving from one step to the next:

  Position   Bit1   Bit2
  ----------------------
    Step1     0      0
     1/4      1      0
     1/2      1      1
     3/4      0      1
    Step2     0      0

From this table, we can see that when moving from one 'click' to
the next, there are 4 changes in the output code.

- From an initial 0 - 0, Bit1 goes high, Bit0 stays low.
- Then both bits are high, halfway through the step.
- Then Bit1 goes low, but Bit2 stays high.
- Finally at the end of the step, both bits return to 0.

>*Note*: `Bit1` is called `ClockPin` and `Bit2` is called `DataPin` in 
the forked library. This corresponds more or less to the labelling on 
readily available KY-040 type rotary encoders.

Detecting the direction is easy - the table simply goes in the other
direction (read up instead of down).

To decode this, we use a simple state machine. Every time the output
code changes, it follows state, until finally a full steps worth of
code is received (in the correct order). At the final 0-0, it returns
a value indicating a step in one direction or the other.

It's also possible to use 'half-step' mode. This just emits an event
at both the 0-0 and 1-1 positions. This might be useful for some
encoders where you want to detect all positions.

If an invalid state happens (for example we go from '0-1' straight
to '1-0'), the state machine resets to the start until 0-0 and the
next valid codes occur.

The biggest advantage of using a state machine over other algorithms
is that this has inherent debounce built in. Other algorithms emit spurious
output with switch bounce, but this one will simply flip between
sub-states until the bounce settles, then continue along the state
machine.

A side effect of debounce is that fast rotations can cause steps to
be skipped. By not requiring debounce, fast rotations can be accurately
measured.

Another advantage is the ability to properly handle bad state, such
as due to EMI, etc.

It is also a lot simpler than others - a static state table and less
than 10 lines of logic.

# Changes

While the original library used primarily the `(unsigned) char` data type, in this version, `uint8_t` 
is used with one exception. The result of the `process()` function is now an enumerated type.

    enum rotation_t { 
      DIR_NONE = 0x00,  // No complete step yet.
      DIR_CW = 0x10,    // button rotated right (clockwise step)
      DIR_CCW = 0x20    // button rotated left (counter-clockwise step)
    };

Otherwise there is no change to the state machine.

# References

There are many other similar libraries. 

[Rotary](https://github.com/CarlosSiles67/Rotary) by Carlos Siles using a modified 
two-bit encoding scheme is recent and has links to a more extensive description of 
the algorithm used by Ben Buxton.

[Rotary_Button](https://bitbucket.org/Dershum/rotary_button/src/master/) by Phil Fisk
handles the push button included in KY-040 type rotary encoders. Of course, that push
button can be managed with a separate library such as 
[mdPushButton](https://github.com/sigmdel/mdPushButton).


# Licence

The **GNU GPL Version 3** ([SPDX](https://spdx.dev/): [GPL-3.0-only](https://spdx.org/licenses/GPL-3.0-only.html)) licence applies as set by the original author.

