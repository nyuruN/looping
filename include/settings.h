#pragma once

#include <Arduino.h>

namespace EEPROMSettings {
  struct Timestamp {
    uint16_t height;
    uint16_t mass;
    uint16_t velocity;
  };

  extern uint16_t massPresets[3];
  extern Timestamp timestamps[10];

  void load();

  void save();
}
