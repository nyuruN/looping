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
    case State::Main:
      Menu::main.up();
      break;
    case State::Menu:
      Menu::menu.up();
      break;
    case State::MassSetup:
      Menu::massSetup.up();
      break;
    case State::NumberSelect:
      Menu::numberSelect.up();
      break;
    case State::Settings:
      Menu::settings.up();
      break;
    case State::Calibrate:
      Menu::calibrate.up();
      break;
    case State::MassPreset:
      Menu::massPreset.up();
      break;
    default:
      break;
  }
}

void App::down() {
  switch (state) {
    case State::Main:
      Menu::main.down();
      break;
    case State::Menu:
      Menu::menu.down();
      break;
    case State::MassSetup:
      Menu::massSetup.down();
      break;
    case State::NumberSelect:
      Menu::numberSelect.down();
      break;
    case State::Settings:
      Menu::settings.down();
      break;
    case State::Calibrate:
      Menu::calibrate.down();
      break;
    case State::MassPreset:
      Menu::massPreset.down();
      break;
    default:
      break;
  }
}

void App::press() {
  switch (state) {
    case State::Main:
      Menu::main.press();
      break;
    case State::Menu:
      Menu::menu.press();
      break;
    case State::MassSetup:
      Menu::massSetup.press();
      break;
    case State::NumberSelect:
      Menu::numberSelect.press();
      break;
    case State::Settings:
      Menu::settings.press();
      break;
    case State::Calibrate:
      Menu::calibrate.press();
      break;
    case State::MassPreset:
      Menu::massPreset.press();
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
    case State::Main:
      Menu::main.enter(prevState);
      break;
    case State::Menu:
      Menu::menu.enter(prevState);
      break;
    case State::MassSetup:
      Menu::massSetup.enter(prevState);
      break;
    case State::NumberSelect:
      Menu::numberSelect.enter(prevState);
      break;
    case State::Settings:
      Menu::settings.enter(prevState);
      break;
    case State::Calibrate:
      Menu::calibrate.enter(prevState);
      break;
    case State::MassPreset:
      Menu::massPreset.enter(prevState);
      break;
    default:
      break;
  }
}

void App::exit() {
  switch (state) {
    case State::Main:
      Menu::main.exit(nextState);
      break;
    case State::Menu:
      Menu::menu.exit(nextState);
      break;
    case State::MassSetup:
      Menu::massSetup.exit(nextState);
      break;
    case State::NumberSelect:
      Menu::numberSelect.exit(nextState);
      break;
    case State::Settings:
      Menu::settings.exit(nextState);
      break;
    case State::Calibrate:
      Menu::calibrate.exit(nextState);
      break;
    case State::MassPreset:
      Menu::massPreset.exit(nextState);
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
    case State::Main:
      Menu::main.render();
      break;
    case State::Menu:
      Menu::menu.render();
      break;
    case State::MassSetup:
      Menu::massSetup.render();
      break;
    case State::NumberSelect:
      Menu::numberSelect.render();
      break;
    case State::Settings:
      Menu::settings.render();
      break;
    case State::Calibrate:
      Menu::calibrate.render();
      break;
    case State::MassPreset:
      Menu::massPreset.render();
      break;
    default:
      break;
  }

  display.display();
}
