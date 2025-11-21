#pragma once

#include <Arduino.h>

namespace EEPROMSettings {
  extern uint16_t masses[4]; // 0 .. 3
  extern uint16_t height; // 4

  void load();

  void save();
}
