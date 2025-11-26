#include <Arduino.h>

#include <EEPROM.h>

#include "../include/settings.h"


uint16_t EEPROMSettings::massPresets[3];
EEPROMSettings::Timestamp EEPROMSettings::timestamps[10];

void EEPROMSettings::load() {
  for (uint8_t i = 0; i < sizeof(massPresets); ++i) {
    *((uint8_t*) massPresets + i) = EEPROM.read(i);
  }

  for (uint8_t i = 0; i < sizeof(timestamps); ++i) {
    *((uint8_t*) timestamps + i) = EEPROM.read(i + sizeof(massPresets));
  }
}

void EEPROMSettings::save() {
  for (uint8_t i = 0; i < sizeof(massPresets); ++i) {
    EEPROM.update(i, *((uint8_t*) massPresets + i));
  }

  for (uint8_t i = 0; i < sizeof(timestamps); ++i) {
    EEPROM.update(i + sizeof(massPresets), *((uint8_t*) timestamps + i));
  }
}
