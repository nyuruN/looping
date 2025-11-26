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
    uint8_t upAnim = 255;
    uint8_t downAnim = 255;

    public:
      uint16_t number; // 11.01
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


        // Pretend like we're privileged enough to have delta time
        constexpr uint8_t delta_time = 30;

        const int8_t inverse = editing ? 1 : -1;
        
        //const int8_t wave = (1.0 + sin(millis() / 150.0)) * 2.0;

        const int8_t wave = max(min(abs(int8_t((millis() >> 3) % 128) - 64), 48), 16) >> 3;

        const int8_t offsetUp = offset_anim(float(upAnim) / 1000.0);
        const int8_t offsetDown = offset_anim(float(downAnim) / 1000.0);

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
            app.toNextState(App::State::Dashboard);
            break;
          case State::MassSetup:
            app.toNextState(App::State::MassSetup);
            break;
          case State::HeightSetup:
            numberSelect.unit = Units::CENTIMETER;
            numberSelect.number = EEPROMSettings::timestamps[0].height;
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
        if (prevState == App::State::NumberSelect) {
          EEPROMSettings::timestamps[0].height = numberSelect.number;
          EEPROMSettings::save();
        }

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
            EEPROMSettings::timestamps[0].mass = EEPROMSettings::massPresets[(uint8_t) state - 1];
            EEPROMSettings::save();
            app.toNextState(App::State::Menu);
            break;
          case State::Custom:
            numberSelect.number = EEPROMSettings::timestamps[0].mass;
            numberSelect.unit = Units::GRAM;
            app.toNextState(App::State::NumberSelect);
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        if (prevState == App::State::NumberSelect) {
          EEPROMSettings::timestamps[0].mass = numberSelect.number;
          EEPROMSettings::save();
        }

        Ui::List::create((const char**) LABELS, (const uint8_t**) ICONS, (uint8_t) State::End);

        sprintf_P((Ui::List::list[0].label + 7), FMT, EEPROMSettings::timestamps[0].mass / 100, EEPROMSettings::timestamps[0].mass % 100);

        for (uint8_t i = 0; i < 3; ++i) {
          sprintf_P((Ui::List::list[i + 1].label + 7), FMT, EEPROMSettings::massPresets[i] / 100, EEPROMSettings::massPresets[i] % 100);
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
            numberSelect.number = EEPROMSettings::massPresets[(uint8_t) state - 1];
            numberSelect.unit = Units::GRAM;
            app.toNextState(App::State::NumberSelect);
            break;
          default:
            break;
        }
      }

      void enter(App::State prevState) {
        if (prevState == App::State::NumberSelect) {
          EEPROMSettings::massPresets[(uint8_t) state - 1] = numberSelect.number;
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

#define clamp(a, b, x) (max(a, min(b, x)))

  class Dashboard {
    uint16_t maxKey = 0;
    uint16_t minKey = -1;

    // EPPROM data
    static constexpr uint8_t NODES = 10;
    static constexpr uint8_t MAX_KEYS = 5;

    uint8_t keysOffset = 0;
    int8_t selected = 0;
    EEPROMSettings::Timestamp* selectedTimestamp = nullptr;

    void refit() {
      keysOffset = clamp(0, NODES - MAX_KEYS, selected - 2);

      maxKey = 0;
      minKey = -1;
      for (uint8_t i = 0; i < NODES; ++i) {
        maxKey = max(maxKey, EEPROMSettings::timestamps[i].height);
        minKey = min(minKey, EEPROMSettings::timestamps[i].height);
      }

      selectedTimestamp = &EEPROMSettings::timestamps[selected];
    }

    public:
      void up() {
        if (++selected == NODES)
          selected = 0;
        
        refit();
      }

      void down() {
        if (--selected == -1)
          selected = NODES - 1;

        refit();
      }

      void press() {
        if (selected == 0)
          app.toNextState(App::State::Menu);
      }

      void enter(App::State prevState) {
        selected = 0;

        refit();
      }

      void exit(App::State nextState) {
      }

      void render() {
        constexpr float GRAPH_Y = 7.0f;
        constexpr float GRAPH_HEIGHT = 37.0f;

        static float scrollX = 0;
        scrollX = Ui::Lerp(scrollX, keysOffset, 0.2);

        // Render points
        int16_t lastAnchorX = 0;
        uint8_t lastAnchorY = 0;
        
        for (int8_t i = NODES - 1; i >= 0; --i) {
          constexpr int16_t POINT_PADDING = 0;

          const float ratio = (float) (EEPROMSettings::timestamps[i].height - minKey) / (float) (maxKey - minKey);
          uint8_t anchorY = (1. - ratio) * GRAPH_HEIGHT + GRAPH_Y;
          int16_t anchorX = 64 - (i - 2 - scrollX) * 20.;

          // Draw line
          if (i < NODES - 1)
            display.drawLine(lastAnchorX + POINT_PADDING, lastAnchorY, anchorX - POINT_PADDING, anchorY, SSD1306_WHITE);

          // Draw menu node
          if (i == 0) {
            constexpr uint8_t ASCII_MENU_ICON = 239;
            display.fillRoundRect(
              anchorX - 4,
              anchorY - 4,
              9,
              9,
              1,
              SSD1306_BLACK
            );
            display.setCursor(anchorX - 2, anchorY - 3);
            display.write(ASCII_MENU_ICON);
          } else {  
            // Draw point
            display.fillCircle(anchorX, anchorY, 2, SSD1306_WHITE);          
          }

          // Draw selection indicator
          if (i == selected) {
            display.drawRoundRect(
              anchorX - 4,
              anchorY - 4,
              9,
              9,
              1,
              SSD1306_WHITE
            );
          }

          lastAnchorX = anchorX;
          lastAnchorY = anchorY;
        }
        
        const char FMT1[] = "%d.%.2dcm";
        const char FMT2[] = "%d.%.2dg";

        sprintf(Ui::List::list[0].label, FMT1, selectedTimestamp->height / 100, selectedTimestamp->height % 100);
        display.drawBitmap(0, SCREEN_HEIGHT - FONT_HEIGHT, Bitmap::HEIGHT8X8, 8, 8, SSD1306_WHITE);
        display.setCursor(8 + 4, SCREEN_HEIGHT - FONT_HEIGHT);
        display.write(Ui::List::list[0].label);

        const uint8_t len = sprintf(Ui::List::list[1].label, FMT2, selectedTimestamp->mass / 100, selectedTimestamp->mass % 100);
        display.drawBitmap(SCREEN_WIDTH - FONT_WIDTH * len - 4 - 8, SCREEN_HEIGHT - FONT_HEIGHT, Bitmap::MASS8X8, 8, 8, SSD1306_WHITE);
        display.setCursor(SCREEN_WIDTH - FONT_WIDTH * len, SCREEN_HEIGHT - FONT_HEIGHT);
        display.write(Ui::List::list[1].label);
      }

  } extern dashboard;
};
