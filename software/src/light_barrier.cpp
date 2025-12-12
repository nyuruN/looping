#include <Arduino.h>

#include "app.h"
#include "menu.h"

#include "light_barrier.h"

namespace LightBarrier {
  volatile uint16_t time = 0;
  volatile State state = State::Idle;

  void setup() {
    // Setup pin change interrupts on D8 and A2.
    PCICR |= (1 << PCIE1) | (1 << PCIE0);
    PCMSK0 |= (1 << PCINT0);
    PCMSK1 |= (1 << PCINT10);

    // Setup Timer1 (16 bit) with prescaler 64 in free-running mode.
    TCCR1A = B00000000;
    TCCR1B = B00000011;
    TCCR1C = B00000000;
    TIMSK1 |= (1 << TOIE1);
    TCNT1 = 0;
  }

  ISR(PCINT1_vect) {
    if (digitalRead(A2)) {
      if (app.state != App::State::Inspector || Menu::inspector.prevState != App::State::Measurement) {
        state = State::Measuring;
        app.toNextState(App::State::Measurement);
        TCNT1 = 0;
      }
    } else {
    }
  }

  ISR(PCINT0_vect) {
    if (digitalRead(8)) {
      if (state == State::Measuring) {
        time = TCNT1;
        state = State::Success;
      }
    } else {
    }
  }

  ISR(TIMER1_OVF_vect) {
    if (state == State::Measuring)
      state = State::Failed;
  }
}
