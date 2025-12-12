#include <Arduino.h>

#include "./display.h"
#include "./menu.h"

#include "../include/app.h"

void App::setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }

  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  App::enter();
}

void App::up() {
  switch (state) {
    case State::NumberSelect:
      Menu::numberSelect.up();
      break;
    case State::Menu:
      Menu::menu.up();
      break;
    case State::MassSetup:
      Menu::massSetup.up();
      break;
    case State::MassPreset:
      Menu::massPreset.up();
      break;
    case State::Settings:
      Menu::settings.up();
      break;
    case State::Credits:
      Menu::credits.up();
      break;
    case State::Dashboard:
      Menu::dashboard.up();
      break;
    case State::Measurement:
      Menu::measurement.up();
      break;
    case State::Inspector:
      Menu::inspector.up();
      break;
    default:
      break;
  }
}

void App::down() {
  switch (state) {
    case State::NumberSelect:
      Menu::numberSelect.down();
      break;
    case State::Menu:
      Menu::menu.down();
      break;
    case State::MassSetup:
      Menu::massSetup.down();
      break;
    case State::MassPreset:
      Menu::massPreset.down();
      break;
    case State::Settings:
      Menu::settings.down();
      break;
    case State::Credits:
      Menu::credits.down();
      break;
    case State::Dashboard:
      Menu::dashboard.down();
      break;
    case State::Measurement:
      Menu::measurement.down();
      break;
    case State::Inspector:
      Menu::inspector.down();
      break;
    default:
      break;
  }
}

void App::press() {
  switch (state) {
    case State::NumberSelect:
      Menu::numberSelect.press();
      break;
    case State::Menu:
      Menu::menu.press();
      break;
    case State::MassSetup:
      Menu::massSetup.press();
      break;
    case State::MassPreset:
      Menu::massPreset.press();
      break;
    case State::Settings:
      Menu::settings.press();
      break;
    case State::Credits:
      Menu::credits.press();
      break;
    case State::Dashboard:
      Menu::dashboard.press();
      break;
    case State::Measurement:
      Menu::measurement.press();
      break;
    case State::Inspector:
      Menu::inspector.press();
      break;
    default:
      break;
  }
}

void App::toNextState(App::State state) {
  nextState = state;
}

void App::enter() {
  State prevState = state;
  state = nextState;

  switch (state) {
    case State::NumberSelect:
      Menu::numberSelect.enter(prevState);
      break;
    case State::Menu:
      Menu::menu.enter(prevState);
      break;
    case State::MassSetup:
      Menu::massSetup.enter(prevState);
      break;
    case State::MassPreset:
      Menu::massPreset.enter(prevState);
      break;
    case State::Settings:
      Menu::settings.enter(prevState);
      break;
    case State::Credits:
      Menu::credits.enter(prevState);
      break;
    case State::Dashboard:
      Menu::dashboard.enter(prevState);
      break;
    case State::Measurement:
      Menu::measurement.enter(prevState);
      break;
    case State::Inspector:
      Menu::inspector.enter(prevState);
      break;
    default:
      break;
  }
}

void App::exit() {
  switch (state) {
    case State::NumberSelect:
      Menu::numberSelect.exit(nextState);
      break;
    case State::Menu:
      Menu::menu.exit(nextState);
      break;
    case State::MassSetup:
      Menu::massSetup.exit(nextState);
      break;
    case State::MassPreset:
      Menu::massPreset.exit(nextState);
      break;
    case State::Settings:
      Menu::settings.exit(nextState);
      break;
    case State::Credits:
      Menu::credits.exit(nextState);
      break;
    case State::Dashboard:
      Menu::dashboard.exit(nextState);
      break;
    case State::Measurement:
      Menu::measurement.exit(nextState);
      break;
    case State::Inspector:
      Menu::inspector.exit(nextState);
      break;
    default:
      break;
  }
}

void App::render() {
  if (state != nextState) {
    exit();
    enter();
  }

  display.clearDisplay();

  switch (state) {
    case State::NumberSelect:
      Menu::numberSelect.render();
      break;
    case State::Menu:
      Menu::menu.render();
      break;
    case State::MassSetup:
      Menu::massSetup.render();
      break;
    case State::MassPreset:
      Menu::massPreset.render();
      break;
    case State::Settings:
      Menu::settings.render();
      break;
    case State::Credits:
      Menu::credits.render();
      break;
    case State::Dashboard:
      Menu::dashboard.render();
      break;
    case State::Measurement:
      Menu::measurement.render();
      break;
    case State::Inspector:
      Menu::inspector.render();
      break;
    default:
      break;
  }

  display.display();
}
