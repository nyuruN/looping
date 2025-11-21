#include <Arduino.h>

#include <EEPROM.h>

#include "../include/settings.h"

uint16_t EEPROMSettings::masses[];
uint16_t EEPROMSettings::height;

void EEPROMSettings::load() {
  for (uint8_t i = 0; i < sizeof(masses); ++i) {
    *((uint8_t*) masses + i) = EEPROM.read(i);
  }
  height = EEPROM.read(sizeof(masses));
}

void EEPROMSettings::save() {
  for (uint8_t i = 0; i < sizeof(masses); ++i) {
    EEPROM.update(i, *((uint8_t*) masses + i));
  }
  EEPROM.update(sizeof(masses), height);
}
