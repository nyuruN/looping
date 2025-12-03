
#include <Arduino.h>

namespace LightBarrier {
  volatile bool measuring = false;
  volatile uint16_t time = 0;
  volatile bool success = false; // Indicates that no overflow happened

  void setup() {
    // Setup pin change interrupts on D8 and A2.
    PCICR |= (1 << PCIE1) | (1 << PCIE0);
    PCMSK0 |= (1 << PCINT0);
    PCMSK1 |= (1 << PCINT10);

    // Setup Timer1 (16 bit) with prescaler 64 in free-running mode.
    TCCR1A = B00000000;
    //TCCR1B = B00000011;
    TCCR1B = B00000011; // TEMPORARY CHANGE TO 128 PRESCALAR FOR TESTING!!!
    TCCR1C = B00000000;
    TIMSK1 |= (1 << TOIE1);
    TCNT1 = 0;
  }

  ISR(PCINT1_vect) {
    if (!measuring) {
      TCNT1 = 0;
      measuring = true;
      success = false;
    }
  }

  ISR(PCINT0_vect) {
    if (measuring) {
      // Serial.print("Time: ");
      // Serial.print(TCNT1 * 64.0 / 16000000.0);
      // Serial.println("s");
      measuring = false;
      time = TCNT1;
      success = true;
      // Serial.println(TCNT1);
    }
  }

  ISR(TIMER1_OVF_vect) {
    measuring = false; // Abort measurement if timer1 overflows.
  }
}
