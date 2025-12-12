#include <Arduino.h>

#include "../include/bitmap.h"

#include "../include/menu.h"

const char Menu::Units::GRAM[] = "g";
const char Menu::Units::MILLIMETER[] = "mm";
const char Menu::Units::CENTIMETER[] = "cm";

const char Menu::Credits::STR[] PROGMEM = "Imagine dying lol";

const char Menu::Menu::LABELS0[] PROGMEM = "Back";
const char Menu::Menu::LABELS1[] PROGMEM = "Mass";
const char Menu::Menu::LABELS2[] PROGMEM = "Height";
const char Menu::Menu::LABELS3[] PROGMEM = "Settings";
const char Menu::Menu::LABELS4[] PROGMEM = "Credits";

const char* const Menu::Menu::LABELS[] PROGMEM = {
  LABELS0,
  LABELS1,
  LABELS2,
  LABELS3,
  LABELS4
};

const uint8_t* const Menu::Menu::ICONS[] PROGMEM = {
  Bitmap::BACKARROW,
  Bitmap::MASS8X8,
  Bitmap::HEIGHT8X8,
  Bitmap::GEAR,
  Bitmap::ARROW
};

const char Menu::MassSetup::FMT[] PROGMEM = "%6.1d.%02dg";

const char Menu::MassSetup::LABELS0[] PROGMEM = "Current";
const char Menu::MassSetup::LABELS1[] PROGMEM = "Light  ";
const char Menu::MassSetup::LABELS2[] PROGMEM = "Medium ";
const char Menu::MassSetup::LABELS3[] PROGMEM = "Heavy  ";
const char Menu::MassSetup::LABELS4[] PROGMEM = "Custom ";

const char* const Menu::MassSetup::LABELS[] PROGMEM = {
  LABELS0,
  LABELS1,
  LABELS2,
  LABELS3,
  LABELS4
};

const uint8_t* const Menu::MassSetup::ICONS[] PROGMEM = {
  Bitmap::BACKARROW,
  Bitmap::BALL,
  Bitmap::BALL,
  Bitmap::BALL,
  Bitmap::GEAR
};

const char* const Menu::MassPreset::LABELS[] PROGMEM = {
  Menu::LABELS0
};

const char Menu::Settings::LABELS0[] PROGMEM = "Calibrate";

const char* const Menu::Settings::LABELS[] PROGMEM = {
  Menu::LABELS0,
  Menu::LABELS1,
  LABELS0
};

const uint8_t* const Menu::Settings::ICONS[] PROGMEM = {
  Bitmap::BACKARROW,
  Bitmap::GEAR,
  Bitmap::GEAR
};

const char Menu::Inspector::LABELS0[] PROGMEM = "Mass%9u.%.2dg";
const char Menu::Inspector::LABELS1[] PROGMEM = "Height%6u.%.2dcm";
const char Menu::Inspector::LABELS2[] PROGMEM = "Time%8u.%.2dms";
const char Menu::Inspector::LABELS3[] PROGMEM = "Vel.%7u.%.2um/s";
const char Menu::Inspector::LABELS4[] PROGMEM = "Kin.E.%6u.%.2umJ";
const char Menu::Inspector::LABELS5[] PROGMEM = "Pot.E.%6u.%.2umJ";
const char Menu::Inspector::LABELS6[] PROGMEM = "Loss%8d.%.2umJ";

Menu::NumberSelect Menu::numberSelect;
Menu::Menu Menu::menu;
Menu::MassSetup Menu::massSetup;
Menu::MassPreset Menu::massPreset;
Menu::Settings Menu::settings;
Menu::Credits Menu::credits;
Menu::Dashboard Menu::dashboard;
Menu::Measurement Menu::measurement;
Menu::Inspector Menu::inspector;
