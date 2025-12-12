#include <Arduino.h>

#include <Encoder.h>

#include "../include/app.h"
#include "../include/display.h"
#include "../include/settings.h"
#include "../include/menu.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

App app = App();
Encoder encoder(2, 4);

namespace EncoderButton {
  constexpr uint8_t encoderBtn = 3;
  volatile unsigned long lastPress = millis();
  bool pressed = false;

  void buttonISR(){
    lastPress = millis();
  }

  inline void setup() {
    pinMode(encoderBtn, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderBtn), buttonISR, CHANGE);
  }

  inline void update() {
    if ((millis() - lastPress) > 20) {
      bool b = digitalRead(encoderBtn);
      if (pressed != b && !b) {
        app.press();
      }
      pressed = b;
    }
  }
}

void setup() {
  TCCR0A = B10100011;
  TCCR0B = B00000001;
  OCR0A = 0xef;
  OCR0B = 0xef;
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  EncoderButton::setup();
  LightBarrier::setup();

  EEPROMSettings::load();

  app.setup();

  // Serial.begin(9600);
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

  Menu::interruptUI.update();
  EncoderButton::update();

  if (Menu::interruptUI.anim)
    Menu::interruptUI.render();
  else
    app.render();
}
