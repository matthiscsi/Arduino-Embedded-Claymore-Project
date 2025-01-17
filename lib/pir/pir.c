#define __DELAY_BACKWARD_COMPATIBLE__
#include <avr/io.h>
#include <pir.h>
#include <util/delay.h>

#define PIR_PIN PD2

void initPIR() {
  // Set PIR pin as input
  DDRD &= ~(1 << PIR_PIN);
  // Enable pull-up resistor
  PORTD |= (1 << PIR_PIN);
}