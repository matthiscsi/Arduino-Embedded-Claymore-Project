#define __DELAY_BACKWARD_COMPATIBLE__
#include <avr/interrupt.h>
#include <avr/io.h>
#include <buzzer.h>
#include <util/delay.h>

void enableBuzzer() { DDRD |= (1 << PD3); }

void playTone(float frequency, uint32_t duration) {
  uint32_t periodInMicro = (uint32_t)(1000000 / frequency);  // we berekenen de periode in microseconden uit de frequency
  uint32_t durationInMicro = duration * 1000;  // we willen duration in microseconden
  for (uint32_t time = 0; time < durationInMicro;
       time += periodInMicro) {    // Zie tutorial over muziek voor meer uitleg!
    PORTD &= ~(1 << PD3);          // aanzetten van de buzzer
    _delay_us(periodInMicro / 2);  // helft van de periode wachten
    PORTD |= (1 << PD3);           // afzetten van de buzzer
    _delay_us(periodInMicro / 2);  // weer helft van de periode wachten
  }
}

void disableBuzzer(){ PORTD = PORTD | 0b00001000; }


