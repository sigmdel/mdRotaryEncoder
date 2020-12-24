/*
 * Example using the mdRotary library, displaying the encoder position and the 
 * direction of rotation of the encoder at each step.
 *
 * This example uses polling and callback functions rather than interrupts.
 */

#include <Arduino.h>
#include <mdRotaryEncoder.h>

#ifndef SERIAL_BAUD
  #define SERIAL_BAUD 9600
#endif

#ifndef CLOCK_PIN
  #define CLOCK_PIN 7
#endif

#ifndef DATA_PIN
  #define DATA_PIN 8
#endif

// A three-wire rotary encoder is wired with the common to ground and the clock and data pins
// to two distinct GPIO pins. If external pull up resistors are note used then
// enable internal pull up resistors
//   mdRotary rotary = mdRotary(CLOCK_PIN, DATA_PIN, true);
//
// If a four-wire rotary encoder such as the KY-040 type rotary encoder is used, connect the 
// encoder GND pin to ground, the + pin to Vcc (take care, no more than 3.3 volts with many
// newer micro controllers) and the DT to a GPIO pin (DATA_PIN) and CLK to a GPIO pin (CLOCK_PIN)
// 
mdRotary rotary = mdRotary(CLOCK_PIN, DATA_PIN);

void ButtonRotatedLeft(void) {
  Serial.print("Button rotated counter clockwise"); 
}

void ButtonRotatedRight(void) {
  Serial.print("Button rotated clockwise");
}

void ButtonRotated(int32_t pos) {
  Serial.print(", position: ");
  Serial.println(pos);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  rotary.onButtonRotated(ButtonRotated);
  rotary.onButtonRotatedLeft(ButtonRotatedLeft);
  rotary.onButtonRotatedRight(ButtonRotatedRight);
}

void loop() {
  rotary.process();
}

