#pragma once

#include <Arduino.h>

class App {
  public:
    enum class State {
      Main,
      Menu,
      MassSetup,
      MassPreset,
      NumberSelect,
      Dashboard,
      Settings,
      Credits,
      End
    };

  private:
    State state = State::Main;
    State nextState = state;

  public:
    void render();

    void up();

    void down();

    void press();

    void toNextState(State state);

    void enter();

    void exit();

    void setup();
} extern app;
