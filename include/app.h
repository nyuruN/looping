#pragma once

#include <Arduino.h>

class App {
  public:
    enum class State {
      NumberSelect,
      Menu,
      MassSetup,
      MassPreset,
      Settings,
      Credits,
      Dashboard,
      Measurement,
      Inspector
    } state = State::Dashboard;

  private:
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
