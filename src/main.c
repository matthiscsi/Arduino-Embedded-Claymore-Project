#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <display.h>
#include <led.h>
#include <button.h>
#include <buzzer.h>
#include <potentiometer.h>
#include <usart.h>


#define PIRPIN 5
#define DEFAULTBUZZERFREQUENCY 100
#define DEFAULTBUZZERDURATION 100
#define ONESECONDMS 1000

struct BombSettings {
  bool usingCountdown;
  bool usingPIR;
  volatile uint16_t countdown;
};

char countdownStr[3];
bool timerSet = false;
bool pirSet = false;

uint16_t potValue = 0;
uint16_t prevPotValue = 0;

volatile uint8_t* button_pressed;
int* patternIndex;

int pattern[] = {1, 2, 3}; // Secret code pattern

//todo test of volatile wegmag
// Struct to hold bomb settings


struct BombSettings bombSettings; 

void initializePIR();
bool isMotionDetected();
void laatTimer2DeBuzzerEenToonhoogteGenereren(int toonhoogte, double actiefTijdInSeconden);
void disarmBomb();
void knopSequentieUpdate(int button);
void initTimer();
void startCountdown(uint16_t duration);
void initBombSettings();


ISR(TIMER1_COMPA_vect) {
  if (bombSettings.countdown > 0) {
    bombSettings.countdown--;
    sprintf(countdownStr, "%d", bombSettings.countdown);
    printf("%s\n", countdownStr);
    if(bombSettings.countdown > 4){
    playTone(DEFAULTBUZZERFREQUENCY,DEFAULTBUZZERDURATION);
    writeNumberAndWait(bombSettings.countdown, 900);
    }
    if (bombSettings.countdown <= 4) {
      switch (bombSettings.countdown) {
        case 4:
          playTone(500, DEFAULTBUZZERDURATION);
          writeNumberAndWait(bombSettings.countdown, 900);
          break;
        case 3:
          lightUpLed(3);
          playTone(1000, DEFAULTBUZZERDURATION);
          writeNumberAndWait(bombSettings.countdown, 900);
          break;
        case 2:
          lightUpLed(2);
          playTone(2000, DEFAULTBUZZERDURATION);
          writeNumberAndWait(bombSettings.countdown, 900);
          break;
        case 1:
          lightUpLed(1);
          playTone(4000, DEFAULTBUZZERDURATION);
          writeNumberAndWait(bombSettings.countdown, 900);
          break;
        case 0:
          lightUpLed(0);
          playTone(50, DEFAULTBUZZERDURATION);
          writeNumberAndWait(bombSettings.countdown, 900);

          timerSet = false;

          explodeBomb();
        }
        
      }
  } 
} 

void initBombSettings() {
  bombSettings.usingCountdown = false;
  bombSettings.usingPIR = false;
  bombSettings.countdown = 0;
}

void explodeBomb(){
          //bom explosie animatie
          clearDisplay();
          lightUpAllLeds();
          SendDataToSegment(1, 0x7F);  // Activate dot on segment 2 (geen nood voor een loop hier)
          printf("\nBomb exploding!!!\n");
          clearDisplay();

          SendDataToSegment(1, 0x7F);  // Activate dot on segment 2 (geen nood voor een loop hier)
         _delay_ms(ONESECONDMS);

          clearDisplay();
          for (int i = 0; i < 1000; i++) {
          SendDataToSegment(1, 0x7B); 
          SendDataToSegment(2, 0xEF);
          _delay_ms(1);
          }

          clearDisplay();

          for (int i = 0; i < 1000; i++) {
            SendDataToSegment(1, 0x4B);  
            SendDataToSegment(2, 0xE9);
            _delay_ms(1);
          }

          for (int i = 0; i < 1000; i++) {
            SendDataToSegment(1, 0x4B);  
            SendDataToSegment(2, 0xE9);
            SendDataToSegment(0, 0x80); 
            SendDataToSegment(3, 0x80);
            _delay_ms(1);
          }

          for (int i = 0; i < 1000; i++) {
            SendDataToSegment(0, 0x00);
            SendDataToSegment(1, 0x00);
            SendDataToSegment(2, 0x00);
            SendDataToSegment(3, 0x00);
            _delay_ms(1);
          }

          lightDownAllLeds(); //leds stonden nog aan van de 5 seconden countdown!

          timerSet = false;

          free(button_pressed);
          free(patternIndex);

          while(!timerSet){
            writeStringAndWait("    ", ONESECONDMS);
          }
}

ISR(PCINT1_vect) {
  if (bit_is_clear(PINC, PC1)) {
    _delay_us(500);
    if (bit_is_clear(PINC, PC1)) {
      *button_pressed = 1;
      knopSequentieUpdate(1);
    }
  } else if (bit_is_clear(PINC, PC2)) {
    _delay_us(500);
    if (bit_is_clear(PINC, PC2)) {
      *button_pressed = 2;
      knopSequentieUpdate(2);
    }
  } else if (bit_is_clear(PINC, PC3)) {
    _delay_us(500);
    if (bit_is_clear(PINC, PC3)) {
      *button_pressed = 3;
      knopSequentieUpdate(3);
    }
  } else {
    *button_pressed = 0;
  }
}

void initTimer() {
  // Set timer1 to CTC mode and enable compare interrupt
  TCCR1B |= (1 << WGM12);
  TIMSK1 |= (1 << OCIE1A);

  // Set the prescaler to 64
  TCCR1B |= (1 << CS11) | (1 << CS10);

  // Set the compare value for 1ms intervals
  OCR1A = 249;
}

void startCountdown(uint16_t duration) {
  bombSettings.countdown = duration + 1;
  timerSet = true;
  TCNT1 = 0;  // Reset the timer

}

void disarmBomb() {
  printf("Bomb has been disarmed.");
  writeStringAndWait("BOMB", ONESECONDMS);
  writeStringAndWait("OFF", ONESECONDMS);

  for (int i = 0; i <= 4; i++) {
    lightUpLed(i);
    _delay_ms(50);
    lightDownLed(i);
    _delay_ms(50);
    lightUpLed(i);
    _delay_ms(50);
    lightDownLed(i);
    _delay_ms(50);
  }

  timerSet = false;
  pirSet = false;

  free(button_pressed);
  free(patternIndex);

  while(!timerSet){
    writeStringAndWait("    ", ONESECONDMS);
  }
}

void knopSequentieUpdate(int button) {
  if (button == pattern[*patternIndex]) {
    // Button pressed matches the expected value
    (*patternIndex)++;

    if (*patternIndex == 3) {
      disarmBomb();
    }
  } else {
    // Wrong button pressed, reset the pattern index
    *patternIndex = 0;
  }

  // Delay to debounce the button press
  _delay_ms(100);
}

void initializePIR() {
  PORTD |= (1 << PIRPIN);
  DDRD &= ~(1 << PIRPIN); // Set PIR pin as input
}

bool isMotionDetected() {
  if (PIND & (1 << PIRPIN)) {
    return true;
  }
  else {
     // No motion
     return false;
  }
}

int main() {
  enableButton(1);
  enableButton(2);
  enableButton(3);
  enableAllButtonInterrupts();
  enableAllLeds();
  initUSART();
  initPotmeter();
  initTimer();
  initDisplay();
  sei();

  button_pressed = (uint8_t*)calloc(1, sizeof(uint8_t));
  patternIndex = (int*)calloc(1, sizeof(int));

  clearDisplay();
  
      while(1){
      printf("Do you want to make the bomb function using a set countdown or infrared motion detection?\n\n");
      printf("Press button 1 for potentiometer set countdown detonation.\n");
      printf("Press button 3 for infrared motion detection detonation.\n\n");

      while(!bombSettings.usingCountdown && !bombSettings.usingPIR){
        
        if(*button_pressed == 1){
          bombSettings.usingCountdown = true;
        }
        else if(*button_pressed == 3){
          bombSettings.usingPIR = true;
        }
      }

      if(bombSettings.usingCountdown == true){
        printf("Countdown Timer:\n");
        printf("Set the time using the potentiometer.\n");
        printf("Press button 2 to start the timer.\n\n");
        writeStringAndWait("SET", ONESECONDMS);
        writeStringAndWait("TIME", ONESECONDMS);
        writeStringAndWait("IN  ", ONESECONDMS);
        writeStringAndWait("SEC.", ONESECONDMS);

        prevPotValue = 0;

        while (*button_pressed != 2) {
          potValue = readPotmeterValue();
          // Check if the potentiometer value is within the range [10, 240]
          if (potValue < 10) {
            potValue = 10;
          } else if (potValue > 240) {
            potValue = 240;
          }

          writeNumberAndWait((int)potValue, 200);
          if (potValue != prevPotValue) {
            printf("Potentiometer value: %d\n", potValue);
            prevPotValue = potValue;
          }
        }
          
        printf("The bomb has been activated.\n\n");
        writeStringAndWait("RUN", ONESECONDMS);
        startCountdown(potValue); // +1 omdat de timer anders op 1 waarde minder begint.
        enableBuzzer();

        while (timerSet)
        {
          
        }
      }

      if(bombSettings.usingPIR){

        printf("Infrared motion sensor:\n");
        printf("Press button 2 to start the sensor and bomb.\n\n");

        while(*button_pressed != 2){

        }

        initializePIR();
        printf("Initialising sensor and arming bomb...\n");
        printf("This will take ten seconds.\n");
        printf("Get away from the bomb!\n");
        for(int i = 0; i<=5; i++){
          writeStringAndWait("PIR", ONESECONDMS);
          writeStringAndWait("INIT", ONESECONDMS);
        }
        pirSet = true;


        for (int i = 0; i <= 4; i++) {
          lightUpLed(i);
          _delay_ms(50);
          lightDownLed(i);
          _delay_ms(50);
          lightUpLed(i);
          _delay_ms(50);
          lightDownLed(i);
          _delay_ms(50);
        }

        printf("Bomb armed.\n\n");

        while(pirSet){
          for(int i = 0; i <= 4; i++){
            SendDataToSegment(i, 0x7F);
            _delay_ms(50);
          }
          for(int i = 4; i >= 0; i--){
            SendDataToSegment(i, 0x7F);
            _delay_ms(50);
          }
          if(isMotionDetected() == true){
            enableBuzzer();
            printf("Motion detected! Detonation imminent.\n");
            startCountdown(5); //wanneer de PIR iets detecteert, geeft men nog 5 seconden om de bom nog mogelijk te ontmantelen
            pirSet = false;
          }
        }
    }
  }
}




