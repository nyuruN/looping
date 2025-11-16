#pragma once

#include <Arduino.h>

#include "./app.h"
#include "./bitmap.h"
#include "./display.h"
#include "./settings.h"
#include "./ui.h"

namespace Menu {
  namespace Units {
    extern const char GRAM[];
    extern const char MILLIMETER[];
    extern const char CENTIMETER[];
  }

  class Main {
    static constexpr uint8_t MAX = 18;
    int8_t selection = 0;

    float offsetPrev = 0.0;
    float offset = 0.0;
    float offsetNext = 0.0;

    public:
      static const char STR[] PROGMEM;

      void up() {
        selection = (selection == 0) ? MAX - 1 : selection - 1; 
        offsetNext = offset;
        offset = offsetPrev;
        offsetPrev = 0.0;
      }

      void down() {
        selection = (selection == MAX - 1) ? 0 : selection + 1;
        offsetPrev = offset;
        offset = offsetNext;
        offsetNext = 0.0;
      }

      void press() {
        app.toNextState(App::State::Menu);
      }

      void enter(App::State prevState) {
      }

      void exit(App::State nextState) {
      }

      void render() {
        float t = float(millis()) / 250.0;

        // Move back and forth after a sine wave
        int8_t offsetX = int(sin(t)* 10.);
        t = sin(t);
        t = 1.0 - t * t;
        t *= 10.0;

        // Move up and down after a up side down parabula
        int8_t offsetY = int(t);

        display.drawBitmap(
          display.width() / 2 - 8 + offsetX,
          display.height() / 2 - (FONT_HEIGHT + 16 + 2) / 2 - offsetY,
          Bitmap::SKULL, 16, 16, WHITE
        );

        display.setCursor(
          SCREEN_WIDTH / 2 - (FONT_WIDTH * 17 / 2),
          SCREEN_HEIGHT / 2 - (FONT_HEIGHT + 16 + 2) / 2 + 16 + 2
        );

        int8_t anchorX = SCREEN_WIDTH / 2 - (FONT_WIDTH * 17 / 2);
        constexpr int8_t ANCHOR_Y = SCREEN_HEIGHT / 2 - (FONT_HEIGHT + 16 + 2) / 2 + 16 + 2;

        float real_offset = 3.0;

        offset = Ui::Lerp(offset, real_offset, 0.4);
        offsetNext = Ui::Lerp(offsetNext, 0.0, 0.4);
        offsetPrev = Ui::Lerp(offsetPrev, 0.0, 0.4);

        for (uint8_t i = 0; i < MAX; ++i) {
          float currentOffset = 0;

          if (selection == i) {
            currentOffset = offset;
          } else if (selection - 1 == i) {
            currentOffset = offsetPrev;
          } else if (selection + 1 == i) {
            currentOffset = offsetNext;
          }

          display.setCursor(anchorX, round(ANCHOR_Y - currentOffset));
          display.write(pgm_read_byte_near(STR + i));
          anchorX += FONT_WIDTH;
        }
      }
  } extern main; 

  class NumberSelect {
    enum class State {
      Back = 0,
      Number,
      Confirm,
      End
    } state = State::Back;

    App::State prevState;

    float scrollX = 0.0;
    uint8_t digits[4] = {0}; // 99.99 MAX / stored in reverse order
    uint8_t cursor;
    bool editing = false;
    uint8_t upAnim = 0;
    uint8_t downAnim = 0;

    public:
      uint16_t number = 1101; // 11.01
      const char* unit;

      static inline uint8_t get_digit_at(uint16_t n, uint8_t index) {
        for (; index > 0; --index) {
          n = n / 10;
        }
        return n % 10;
      }

      void up() {
        if (editing) {
          digits[sizeof(digits) - cursor - 1] = (digits[sizeof(digits) - cursor - 1] == 0) ? 9 : digits[sizeof(digits) - cursor - 1] - 1;
          downAnim = 0;
          return;
        }

        if (state == State::Confirm) {
          cursor = 0;
        } else if (state == State::Number) {
          if (cursor == sizeof(digits) - 1) {
            state = State::Back;
          } else {
            cursor += 1;
          }
          return;
        }

        state = (State) (((int8_t) state - 1 == -1) ? (int8_t) State::End - 1 : ((int8_t) state - 1));
      }

      void down() {
        if (editing) {
          digits[sizeof(digits) - cursor - 1] = (digits[sizeof(digits) - cursor - 1] == 9) ? 0 : digits[sizeof(digits) - cursor - 1] + 1;
          upAnim = 0;
          return;
        }

        if (state == State::Back) {
          cursor = sizeof(digits) - 1;
        } else if (state == State::Number) {
          if (cursor == 0) {
            state = State::Confirm;
          } else {
            cursor -= 1;
          }
          return;
        }

        state = (State) (((int8_t) state + 1 == (int8_t) State::End) ? 0 : ((int8_t) state + 1));
      }
      void press() {
        switch (state) {
          case State::Confirm:
            number = 0;
            for (uint8_t i = 0; i < sizeof(digits); ++i) {
              number = number * 10 + digits[i];
            }
          case State::Back:
            app.toNextState(prevState);
            Ui::List::progress = 1.5;
            break;
          case State::Number:
            editing = !editing;
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        this->prevState = prevState;
        for (uint8_t i = 0; i < sizeof(digits); ++i) {
          digits[i] = get_digit_at(number, sizeof(digits) - i - 1);
        }
      }

      void exit(App::State nextState) {
      }

      float offset_anim(float t) {
        constexpr float AMPLITUDE = 8.0;
        constexpr float DURATION = 0.2;
        constexpr float MULTIPLIER = 1.0 / (DURATION / 2.0);
        // Quatratic function with natural rise & fall
        return AMPLITUDE * max(0.0, 1.0 - (MULTIPLIER * t - 1.0) * (MULTIPLIER * t - 1.0));
      }

      void render() {
        constexpr uint8_t NS_FONT_HEIGHT = FONT_HEIGHT * 2;
        constexpr uint8_t NS_FONT_WIDTH = FONT_WIDTH * 2;
        constexpr uint8_t ANCHOR_Y = SCREEN_HEIGHT / 2 - NS_FONT_HEIGHT / 2;
        constexpr uint8_t SELECT_ANCHOR_X = SCREEN_WIDTH / 2 - NS_FONT_WIDTH / 2;
        constexpr uint8_t SELECT_ANCHOR_Y = SCREEN_HEIGHT / 2;
        constexpr uint8_t NUMBER_SPACING = 1;
        constexpr uint8_t ITEM_SPACING = 7;
        constexpr uint8_t DECIMAL_POINT = 2;

        int8_t anchorX = 0;
        float realScrollX = 0.0;

        display.setTextSize(2);

        // Render Back Icon
        if (state == State::Back)
          realScrollX = float(anchorX + 5);

        display.drawBitmap(
          anchorX + round(scrollX),
          ANCHOR_Y,
          Bitmap::BACKARROW16,
          16, 16,
          SSD1306_WHITE
        );

        anchorX += 16 + ITEM_SPACING;
        
        // Render numbers
        for (uint8_t i = 0; i < sizeof(digits); ++i) {
          if (state == State::Number && cursor == (sizeof(digits) - i - 1))
            realScrollX = float(anchorX + 3);

          display.setCursor(anchorX + round(scrollX), ANCHOR_Y);
          display.write(int(digits[i]) + 48);
          anchorX += NS_FONT_WIDTH + NUMBER_SPACING;

          if ((sizeof(digits) - i - 1) == DECIMAL_POINT) {
            display.setCursor(anchorX + round(scrollX) - 3, ANCHOR_Y);
            display.write('.');
            anchorX += NS_FONT_WIDTH + NUMBER_SPACING - 6;
          }
        }

        // Render unit
        display.setCursor(anchorX + round(scrollX), ANCHOR_Y);
        display.write(unit);
        anchorX = display.getCursorX() + ITEM_SPACING - round(scrollX);

        // Render Confirm Icon
        if (state == State::Confirm)
          realScrollX = float(anchorX + 6);

        display.drawBitmap(
          anchorX + round(scrollX),
          ANCHOR_Y,
          Bitmap::CHECK16,
          16, 16,
          SSD1306_WHITE
        );

        const int8_t inverse = editing ? 1 : -1;
        const int8_t wave = (1.0 + sin(millis() / 150.0)) * 2.0;
        const int8_t offsetUp = offset_anim(float(upAnim) / 1000.0);
        const int8_t offsetDown = offset_anim(float(downAnim) / 1000.0);

        // Pretend like we're privileged enough to have delta time
        constexpr uint8_t delta_time = 30;
        if (upAnim < 255 - delta_time)
          upAnim += delta_time;
        if (downAnim < 255 - delta_time)
          downAnim += delta_time;

        display.drawBitmap(
          SELECT_ANCHOR_X,
          SELECT_ANCHOR_Y - 4 - (NS_FONT_HEIGHT + offsetUp + (editing ? 0 : wave)) * inverse,
          Bitmap::UP16X8,
          16, 8,
          SSD1306_WHITE
        );

        if (editing)
          display.drawBitmap(
            SELECT_ANCHOR_X,
            SELECT_ANCHOR_Y - 4 + (NS_FONT_HEIGHT + offsetDown) * inverse,
            Bitmap::DOWN16X8,
            16, 8,
            SSD1306_WHITE
          );

        // Smooth scroll
        scrollX = Ui::Lerp(scrollX, - realScrollX + SCREEN_WIDTH / 2.0, 0.4);

        display.setTextSize(1);
      }
  } extern numberSelect;

  class Menu {
    enum class State {
      Back = 0,
      MassSetup,
      HeightSetup,
      Settings,
      Credits,
      End,
    } state;

    static const char* const LABELS[] PROGMEM;
    static const uint8_t* const ICONS[] PROGMEM;

    public:
      static const char LABELS0[] PROGMEM;
      static const char LABELS1[] PROGMEM;
      static const char LABELS2[] PROGMEM;
      static const char LABELS3[] PROGMEM;
      static const char LABELS4[] PROGMEM;

      void up() {
        state = (State) (((int8_t) state - 1 == -1) ? (int8_t) State::End - 1 : ((int8_t) state - 1));
      }

      void down() {
        state = (State) (((int8_t) state + 1 == (int8_t) State::End) ? 0 : ((int8_t) state + 1));
      }

      void press() {
        switch (state) {
          case State::Back:
          case State::Credits:
            app.toNextState(App::State::Main);
            break;
          case State::MassSetup:
            app.toNextState(App::State::MassSetup);
            break;
          case State::HeightSetup:
            numberSelect.unit = Units::CENTIMETER;
            app.toNextState(App::State::NumberSelect);
            break;
          case State::Settings:
            app.toNextState(App::State::Settings);
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        Ui::List::create((const char**) LABELS, (const uint8_t**) ICONS, (uint8_t) State::End);
      }

      void exit(App::State nextState) {
      }

      void render() {
        Ui::List::render((uint8_t) State::Back, (uint8_t) State::End, (uint8_t) state);
      }
  } extern menu;

  class MassSetup {
    enum class State {
      Current = 0,
      Light,
      Medium,
      Heavy,
      Custom,
      End,
    } state;


    public:
      static const char* const LABELS[] PROGMEM;
      static const uint8_t* const ICONS[] PROGMEM;

      static const char FMT[] PROGMEM;

      static const char LABELS0[] PROGMEM;
      static const char LABELS1[] PROGMEM;
      static const char LABELS2[] PROGMEM;
      static const char LABELS3[] PROGMEM;
      static const char LABELS4[] PROGMEM;

      void up() {
      state = (State) (((int8_t) state - 1 == -1) ? (int8_t) State::End - 1 : ((int8_t) state - 1));
      }

      void down() {
        state = (State) (((int8_t) state + 1 == (int8_t) State::End) ? 0 : ((int8_t) state + 1));
      }

      void press() {
        switch (state) {
          case State::Current:
            app.toNextState(App::State::Menu);
            break;
          case State::Light:
          case State::Medium:
          case State::Heavy:
            EEPROMSettings::masses[0] = EEPROMSettings::masses[(uint8_t) state];
            EEPROMSettings::save();
            app.toNextState(App::State::Menu);
            break;
          case State::Custom:
            numberSelect.number = EEPROMSettings::masses[0];
            numberSelect.unit = Units::GRAM;
            app.toNextState(App::State::NumberSelect);
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        if (prevState == App::State::NumberSelect) {
          EEPROMSettings::masses[0] = numberSelect.number;
          EEPROMSettings::save();
        }

        Ui::List::create((const char**) LABELS, (const uint8_t**) ICONS, (uint8_t) State::End);

        for (uint8_t i = 0; i < 4; ++i) {
          sprintf_P((Ui::List::list[i].label + 7), FMT, EEPROMSettings::masses[i] / 100, EEPROMSettings::masses[i] % 100);
        }
      }

      void exit(App::State nextState) {
      }

      void render() {
        Ui::List::render((uint8_t) State::Current, (uint8_t) State::End, (uint8_t) state);
      }
  } extern massSetup;

  class MassPreset {
    enum class State {
      Back = 0,
      Light,
      Medium,
      Heavy,
      End
    } state;

    static const char* const LABELS[] PROGMEM;

    public:
      void up() {
      state = (State) (((int8_t) state - 1 == -1) ? (int8_t) State::End - 1 : ((int8_t) state - 1));
      }

      void down() {
        state = (State) (((int8_t) state + 1 == (int8_t) State::End) ? 0 : ((int8_t) state + 1));
      }

      void press() {
        switch (state) {
          case State::Back:
            app.toNextState(App::State::Settings);
            break;
          case State::Light:
          case State::Medium:
          case State::Heavy:
            numberSelect.number = EEPROMSettings::masses[(uint8_t) state];
            numberSelect.unit = Units::GRAM;
            app.toNextState(App::State::NumberSelect);
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        if (prevState == App::State::NumberSelect) {
          EEPROMSettings::masses[(uint8_t) state] = numberSelect.number;
          EEPROMSettings::save();
        }

        massSetup.enter(App::State::MassSetup);
        Ui::List::create((const char**) LABELS, (const uint8_t**) MassSetup::ICONS, (uint8_t) State::Back + 1);
      }

      void exit(App::State nextState) {
      }

      void render() {
        Ui::List::render((uint8_t) State::Back, (uint8_t) State::End, (uint8_t) state);
      }
  } extern massPreset;

  class Settings {
    enum class State {
      Back = 0,
      MassPreset,
      Calibrate,
      End,
    } state;

    static const char* const LABELS[] PROGMEM;
    static const uint8_t* const ICONS[] PROGMEM;

    public:
      static const char LABELS0[] PROGMEM;

      void up() {
        state = (State) (((int8_t) state - 1 == -1) ? (int8_t) State::End - 1 : ((int8_t) state - 1));
      }

      void down() {
        state = (State) (((int8_t) state + 1 == (int8_t) State::End) ? 0 : ((int8_t) state + 1));
      }

      void press() {
        switch (state) {
          case State::Back:
            app.toNextState(App::State::Menu);
            break;
          case State::MassPreset:
            app.toNextState(App::State::MassPreset);
            break;
          case State::Calibrate:
            app.toNextState(App::State::Calibrate);
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        Ui::List::create((const char**) LABELS, (const uint8_t**) ICONS, (uint8_t) State::End);
      }

      void exit(App::State nextState) {
      }

      void render() {
        Ui::List::render((uint8_t) State::Back, (uint8_t) State::End, (uint8_t) state);
      }
  } extern settings;

  class Calibrate {
    public:
      void up() {
      }

      void down() {
      }

      void press() {
        app.toNextState(App::State::Settings);
      }

      void enter(App::State prevState) {
      }

      void exit(App::State nextState) {
      }

      void drawIndicator(int16_t value, int8_t offsetX) {
        constexpr int8_t SPACING_Y = FONT_HEIGHT + 2;

        static char buf[5];

        int16_t roundValue = round(value / 10.0) * 10;
        int8_t offset = (value - roundValue) / 10.0 * SPACING_Y;

        for (int8_t i = 0; i < 5; ++i) {
          sprintf(buf, "%4.1d", roundValue + (2 - i) * 10);
          display.setCursor(
            SCREEN_WIDTH / 2 - FONT_HEIGHT * 4 / 2 - offsetX,
            SCREEN_HEIGHT / 2.0 - FONT_HEIGHT / 2.0 - (2 - i) * SPACING_Y + offset
          );
          display.write(buf);
        }
      }

      void render() {
        // constexpr uint8_t RECT_WIDTH = 120;
        // constexpr uint8_t RECT_HEIGHT = 6;
        //
        // constexpr uint8_t RANGE = 10;
        //
        // constexpr int16_t TARGET_VALUE = 400; 
        //
        // static float l1OffsetX = 0.0;
        // static float l2OffsetX = 0.0;
        //
        // l1OffsetX = Ui::Lerp(l1OffsetX, (analogRead(A0) - TARGET_VALUE) / (1024.0 / RECT_WIDTH), 0.4);
        // l2OffsetX = Ui::Lerp(l2OffsetX, (analogRead(A1) - TARGET_VALUE) / (1024.0 / RECT_WIDTH), 0.4);
        //
        // //
        // // int16_t value = 0;
        // // int16_t anchorY = SCREEN_HEIGHT / 2 - NS_FONT_HEIGHT
        // //
        // display.drawBitmap(
        //   SCREEN_WIDTH / 2 + round(l1OffsetX) - 8,
        //   SCREEN_HEIGHT / 2 - 8 - (RECT_HEIGHT / 2) - 2,
        //   Bitmap::DOWN16X8,
        //   16, 8,
        //   SSD1306_WHITE
        // );
        //
        // display.drawBitmap(
        //   SCREEN_WIDTH / 2 + round(l2OffsetX) - 8,
        //   SCREEN_HEIGHT / 2 + (RECT_HEIGHT / 2) + 2,
        //   Bitmap::UP16X8,
        //   16, 8,
        //   SSD1306_WHITE
        // );
        //
        // display.drawRect(
        //   SCREEN_WIDTH / 2 - RECT_WIDTH / 2,
        //   SCREEN_HEIGHT / 2 - RECT_HEIGHT / 2,
        //   RECT_WIDTH,
        //   RECT_HEIGHT,
        //   SSD1306_WHITE
        // );
        //
        // display.fillRect(
        //   SCREEN_WIDTH / 2 - RANGE / 2,
        //   SCREEN_HEIGHT / 2 - RECT_HEIGHT / 2,
        //   RANGE,
        //   RECT_HEIGHT,
        //   SSD1306_WHITE
        // );

        static float valueA0 = 0;
        static float valueA1 = 0;
        static int16_t readA0 = -400;
        static int16_t readA1 = -400;

        int16_t newVal = analogRead(A0);
        if (abs(newVal - readA0) > 4)
          readA0 = newVal;

        newVal = analogRead(A1);
        if (abs(newVal - readA1) > 4)
          readA1 = newVal;

        valueA0 = Ui::Lerp(valueA0, readA0 - 400, 0.2);
        valueA1 = Ui::Lerp(valueA1, readA1 - 400, 0.2);

        drawIndicator(valueA0, -16);
        drawIndicator(valueA1, 16);

        display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 4, SSD1306_BLACK);
        display.fillRect(0, SCREEN_HEIGHT - SCREEN_HEIGHT / 4, SCREEN_WIDTH, SCREEN_HEIGHT / 4, SSD1306_BLACK);
        display.drawRoundRect(
          SCREEN_WIDTH / 2 - FONT_WIDTH * 4 / 2 - 4 - 16,
          SCREEN_HEIGHT / 2 - SCREEN_HEIGHT / 4 - 4,
          FONT_WIDTH * 4 + 8 + 32,
          SCREEN_HEIGHT / 2 + 8,
          3,
          SSD1306_WHITE
        );
      }
  } extern calibrate;
};
