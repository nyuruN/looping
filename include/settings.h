#pragma once

#include <Arduino.h>

namespace EEPROMSettings {
  extern uint16_t masses[4];

  void load();

  void save();
}
