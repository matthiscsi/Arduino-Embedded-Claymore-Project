#define __DELAY_BACKWARD_COMPATIBLE__
#include <avr/io.h>
#include <led.h>
#include <util/delay.h>

void enableLed(int led) {
  DDRB |= (1 << (PB2 + led));
  lightDownLed(led);
}

void enableLeds(uint8_t leds) {
  DDRB |= (leds << 2);
  lightDownLeds(leds);
}

void enableAllLeds() {
  DDRB |= 0b00111100;
  lightDownAllLeds();
}

void lightUpLed(int led) { PORTB &= ~(1 << (PB2 + led)); }

void lightUpLeds(uint8_t leds) { PORTB &= ~(leds << 2); }

void lightUpAllLeds() { PORTB &= 0b11000011; }

void lightDownLed(int led) { PORTB |= (1 << (PB2 + led)); }

void lightDownLeds(uint8_t leds) { PORTB |= (leds << 2); }

void lightDownAllLeds() { PORTB |= 0b00111100; }

void dimLed(int lednumber, int percentage, int duration) {
  for (double i = 0; i < (double)duration / 10; i++) {
    lightUpLed(lednumber);
    _delay_ms((double)percentage / 10);
    lightDownLed(lednumber);
    _delay_ms(10 - ((double)percentage / 10));
  }
}

void fadeInLed(int led, int duration) {
  for (int i = 0; i < 101; i++) {
    dimLed(led, i, duration / 100);
  }
}