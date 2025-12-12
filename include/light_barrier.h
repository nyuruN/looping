#pragma once

#include <Arduino.h>

namespace LightBarrier {
  extern volatile uint16_t time;
  extern volatile enum class State {
    Idle,
    Measuring,
    Failed,
    Success
  } state;

  void setup();
}
