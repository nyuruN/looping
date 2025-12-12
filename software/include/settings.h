#pragma once

#include <Arduino.h>

namespace EEPROMSettings {
  struct Timestamp {
    uint16_t height; // Height in 100 micro m
    uint16_t mass; // Mass in 10mg
    uint16_t time; // Time in ms
  };

  extern uint16_t massPresets[3];
  extern Timestamp timestamps[10];

  void load();

  void save();
}
