#pragma once

#include <Arduino.h>

namespace Ui {
  float Lerp(float a, float b, float t);

  void Scrollbar(float progress);

  namespace List {
    struct ListElement {
      char label[18];
      const uint8_t* icon;
    };

    extern ListElement list[8];
    extern float progress;

    void create(const char** labels, const uint8_t** icons, uint8_t items);

    void render(uint8_t optionFirst, uint8_t optionEnd, uint8_t optionCurrent);
  };
}
