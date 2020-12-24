/*
 * Example using the mdRotary library, dumping the encoder position to the serial
 * port. The position increases or decreases depending on the direction
 * of rotation.
 *
 * This example uses interrupts instead of polling.
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

// Counter that will be incremented or decremented by rotation.
int oldPosition = 0;

// rotate is the interrupt service routine (ISR) called anytime the rotary inputs change state.
// It is best to avoid Serial.print statements in an ISR.
#if defined(ESP8266)
ICACHE_RAM_ATTR 
#endif
void rotate() {
  rotary.process();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), rotate, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DATA_PIN), rotate, CHANGE);
}

void loop() {
  if ( rotary.getPosition() != oldPosition ) {
    oldPosition = rotary.getPosition();
    // Safer to print changes here than in the ISR
    Serial.println(oldPosition);
  }
  delay(50);
}
