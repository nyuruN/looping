#include <Arduino.h>

#include <Encoder.h>

#include "../include/app.h"
#include "../include/display.h"
#include "../include/settings.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

App app = App();
Encoder encoder(2, 4);

namespace RotaryEncoder {
  int encoderBtn = 3;
  volatile unsigned long lastPress = millis();

  void button() {
    app.press();
  }
  void buttonISR(){
    bool b = digitalRead(3);
    if (millis() - lastPress < 200)
      return;
    if (!b)
      button();
    lastPress = millis();
  }

  inline void setup() {
    pinMode(encoderBtn, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderBtn), buttonISR, CHANGE);
  }
}

namespace LightBarrier {
  bool measuring = false;

  inline void setup() {
    // Setup pin change interrupts on D8 and A2.
    PCICR |= (1 << PCIE1) | (1 << PCIE0);
    PCMSK0 |= (1 << PCINT0);
    PCMSK1 |= (1 << PCINT10);

    // Setup Timer1 (16 bit) with prescaler 64 in free-running mode.
    TCCR1A = B00000000;
    TCCR1B = B00000011;
    TCCR1C = B00000000;
    TIMSK1 |= (1 << TOIE1);
    TCNT1 = 0;
  }

  ISR(PCINT0_vect) {
    TCNT1 = 0;
    measuring = true;
  }

  ISR(PCINT1_vect) {
    if (measuring) {
      // Serial.print("Time: ");
      // Serial.print(TCNT1 * 64.0 / 16000000.0);
      // Serial.println("s");
      measuring = false;
    }
  }

  ISR(TIMER1_OVF_vect) {
    measuring = false; // Abort measurement if timer1 overflows.
  }
}

void setup() {
  app.setup();
  RotaryEncoder::setup();
  LightBarrier::setup();

  EEPROMSettings::load();
  for (uint8_t i = 0; i < sizeof(EEPROMSettings::timestamps) / sizeof(EEPROMSettings::Timestamp); ++i) {
    EEPROMSettings::timestamps[i].height = rand() % 300;
  }
}

void loop() {
  int8_t c = encoder.read();
  if (abs(c) > 3) {
    if (c < 0) {
      app.up();
      encoder.write(c + 4);
    } else {
      app.down();
      encoder.write(c - 4);
    }
  }

  app.render();
}
