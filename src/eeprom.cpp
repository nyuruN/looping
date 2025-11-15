#include <Arduino.h>

#include <EEPROM.h>

#include "../include/eeprom.h"

uint16_t EEPROMSettings::masses[];

void EEPROMSettings::load() {
  for (uint8_t i = 0; i < sizeof(masses); ++i) {
    *((uint8_t*) masses + i) = EEPROM.read(i);
  }
}

void EEPROMSettings::save() {
  for (uint8_t i = 0; i < sizeof(masses); ++i) {
    EEPROM.update(i, *((uint8_t*) masses + i));
  }
}
