#pragma once

#include <Arduino.h>

namespace LightBarrier {
  extern volatile bool measuring;
  extern volatile uint16_t time;
  extern volatile bool success; // Indicates that no overflow happened

  void setup();
}