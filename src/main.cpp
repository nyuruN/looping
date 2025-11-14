#include <Arduino.h>

#include <Encoder.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int FONT_WIDTH = 6;
constexpr int FONT_HEIGHT = 8;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Fuck you Adafruit
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Used bitmaps
namespace Bitmap {
  static const unsigned char PROGMEM SKULL[] = {
    B00000111, B11100000, 
    B00011000, B00011000, 
    B00100000, B00000100, 
    B00100000, B00000100, 
    B01000000, B00000010, 
    B01000000, B00000010, 
    B10011110, B01111001, 
    B10111110, B01111101, 
    B10111110, B01111101, 
    B10111110, B01111101, 
    B01011100, B00111010, 
    B00100001, B10000100, 
    B00010001, B10001000, 
    B00010000, B00001000, 
    B00010010, B01001000, 
    B00001111, B11110000,
  };
  static const unsigned char PROGMEM BACKARROW[] = {
    B00000000, 
    B00000000, 
    B00100010, 
    B01000010, 
    B11111110, 
    B01000000, 
    B00100000, 
    B00000000,
  };
  static const unsigned char PROGMEM BACKARROW16[] = {
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00001000, B00001100, 
    B00011000, B00001100, 
    B00111000, B00001100, 
    B01111000, B00001100, 
    B11111111, B11111100, 
    B11111111, B11111100, 
    B01111000, B00000000, 
    B00111000, B00000000, 
    B00011000, B00000000, 
    B00001000, B00000000, 
    B00000000, B00000000, 
    B00000000, B00000000,
  };
  static const unsigned char PROGMEM BALL[] = {
    B00111100, 
    B01000010, 
    B10000001, 
    B10000001, 
    B10000001, 
    B10000001, 
    B01000010, 
    B00111100,
  };
  static const unsigned char PROGMEM CHECK16[] = {
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00000000, B00000111, 
    B00000000, B00001110, 
    B00000000, B00011100, 
    B00000000, B00111000, 
    B00000000, B01110000, 
    B01110000, B11100000, 
    B00111001, B11000000, 
    B00011111, B10000000, 
    B00001111, B00000000, 
    B00000110, B00000000, 
    B00000000, B00000000,
    B00000000, B00000000, 
  };
  static const unsigned char PROGMEM UP16X8[] = {
    B00000001, B10000000, 
    B00000011, B11000000, 
    B00000111, B11100000, 
    B00001110, B01110000, 
    B00011100, B00111000, 
    B00111000, B00011100, 
    B00000000, B00000000, 
    B00000000, B00000000, 
  };
/*   static const unsigned char PROGMEM UP16X8[] = {
    B00000001, B10000000, 
    B00000001, B10000000, 
    B00000110, B01100000, 
    B00000110, B01100000, 
    B00011000, B00011000, 
    B00011000, B00011000, 
    B00000000, B00000000, 
    B00000000, B00000000, 
  }; */
  static const unsigned char PROGMEM DOWN16X8[] = {
    B00000000, B00000000, 
    B00000000, B00000000, 
    B00111000, B00011100, 
    B00011100, B00111000, 
    B00001110, B01110000, 
    B00000111, B11100000, 
    B00000011, B11000000, 
    B00000001, B10000000, 
  };
}

// NotGui
namespace Ui {
  float progress = 0.0f;

  float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
  }
  void Scrollbar(float progress) {
    constexpr int SCROLLBAR_TRACK_X = SCREEN_WIDTH - 2;
    constexpr int SCROLLBAR_THUMB_WIDTH = 3;
    constexpr int SCROLLBAR_THUMB_HEIGHT = 8;
    constexpr int SCROLLBAR_THUMB_X = SCREEN_WIDTH - SCROLLBAR_THUMB_WIDTH;
    constexpr float SCROLLBAR_MAX = float(SCREEN_HEIGHT - SCROLLBAR_THUMB_HEIGHT);

    int scrollbar_thumb_y = int(Ui::Lerp(0.0f, SCROLLBAR_MAX, progress));
    for (int i = 0; i < SCREEN_HEIGHT; i += 2) {
      display.drawPixel(SCROLLBAR_TRACK_X, i, SSD1306_WHITE);
    }
    display.fillRect(
      SCROLLBAR_THUMB_X,
      scrollbar_thumb_y,
      SCROLLBAR_THUMB_WIDTH,
      SCROLLBAR_THUMB_HEIGHT,
      SSD1306_WHITE
    );
  }
  void Menuuu(
    int option_first,
    int option_end, // This is not a value! This is last option + 1
    int* selected,
    const char ** labels, // This can display a maximum of 17 characters!
    const unsigned char ** icons
  ) {
    constexpr int ANCHOR_X = 4;
    int anchor_y = 4;
    constexpr int ITEM_SPACING = 2;
    constexpr int ICON_TEXT_GAP = 3; // This just cannot be 3, I... don't know why
    constexpr int PADDING_X = 4;
    constexpr int PADDING_Y = 5;
    constexpr int SIZE_X = 120;
    constexpr int SIZE_Y = FONT_HEIGHT + 10;
    
    // Smooth scroll
    float real_progress = float(*selected) / float(option_end - 1);
    progress = Ui::Lerp(progress, real_progress, 0.2);
    int scroll_y = -int(progress * float(SIZE_Y * (option_end - 1) + ITEM_SPACING * (option_end - 2)))
    + SCREEN_HEIGHT / 4;

    // Render items
    for (int s = option_first; s < option_end; s++) {
      display.drawBitmap(ANCHOR_X + PADDING_X, anchor_y + PADDING_Y + scroll_y, icons[s], 8, 8, WHITE);
      display.setCursor(ANCHOR_X + PADDING_X + 8 + ICON_TEXT_GAP, anchor_y + PADDING_Y + scroll_y);
      
      if (s == *selected) {
        display.drawRoundRect(
          ANCHOR_X,
          anchor_y + scroll_y,
          SIZE_X,
          SIZE_Y,
          3,
          SSD1306_WHITE
        );
      }

      display.print(labels[s]);
      anchor_y += SIZE_Y + ITEM_SPACING;
    }

    // Render Scrollbar
    Ui::Scrollbar(progress);
  }
}

// Different UI sections that are independent of each other
enum AppState {
  Dashboard = 0,
  Measurement,
  Menu,
  MassSetup,
  HeightSetup,
  NumberSelect,
  Settings,
  Credits,
};
AppState app_state = AppState::Dashboard;
AppState next_state = -1;

// Each element should be confined by anchor & size
// Every size and position should be known at compile time
// and no size resolver is required
namespace Ui {
  class Menu {
    public:
    enum MenuState {
      Back = 0,
      MassSetup,
      HeightSetup,
      Settings,
      Credits,
      End,
    };
    static constexpr int ITEMS = MenuState::End;
    // I have no words...
    static inline constexpr const unsigned char * ICONS[ITEMS] = {
      Bitmap::BACKARROW,
      Bitmap::BACKARROW,
      Bitmap::BACKARROW,
      Bitmap::BACKARROW,
      Bitmap::BACKARROW
    };
    static inline constexpr char * LABELS[ITEMS] = {
      "Back",
      "Mass",
      "Height",
      "Settings",
      "Credits",
    };

    private:
    MenuState state = MenuState::MassSetup;

    public:
    void up() {
      state = (state - 1 == -1) ? MenuState::End - 1 : (state - 1);
    }
    void down() {
      state = (state + 1 == MenuState::End) ? 0 : (state + 1);
    }
    void press() {
      switch (state) {
      case MenuState::Back:
        next_state = AppState::Dashboard;
        break;
      case MenuState::MassSetup:
        next_state = AppState::MassSetup;
        break;
      }
    }
    void render() {
      static const unsigned char * icons[ITEMS] = {
        Bitmap::BACKARROW,
        Bitmap::BACKARROW,
        Bitmap::BACKARROW,
        Bitmap::BACKARROW,
        Bitmap::BACKARROW
      };
      static const char * labels[ITEMS] = {
        "Back",
        "Mass",
        "Height",
        "Settings",
        "Credits",
      };
      Ui::Menuuu(
        MenuState::Back,
        MenuState::End,
        (int*)&state,
        LABELS,
        ICONS
      );
    }
  };
  class Dashboard {
    public:
    void render() {
      float t = float(millis()) / 250.;
      // Move back and forth after a sine wave
      int offset_x = int(sin(t)* 10.);
      t = sin(t);
      t = 1. - t * t;
      t *= 10.;
      // Move up and down after a up side down parabula
      int offset_y = int(t);

      display.drawBitmap(
        display.width() / 2 - 8 + offset_x,
        display.height() / 2 - (FONT_HEIGHT + 16 + 2) / 2 - offset_y,
        Bitmap::SKULL, 16, 16, WHITE
      );

      display.setCursor(
        display.width() / 2 - (FONT_WIDTH * 17 / 2),
        display.height() / 2 - (FONT_HEIGHT + 16 + 2) / 2 + 16 + 2
      );

      display.println("Imagine dying lol");
    }
  }; 
  class MassSetup {
    public:
    enum MassOption {
      Current = 0,
      Light,
      Medium,
      Heavy,
      Custom,
      End,
    };
    static constexpr int ITEMS = MassOption::End;
    static inline constexpr const unsigned char * ICONS[ITEMS] = {
      Bitmap::BACKARROW,
      Bitmap::BALL,
      Bitmap::BALL,
      Bitmap::BALL,
      Bitmap::BACKARROW
    };
    static inline constexpr const char * PRESET_LABELS[ITEMS] = {
      "Light       25.0g",
      "Medium      50.0g",
      "Heavy      100.0g",
      "Custom",
    };
    static inline constexpr int PRESETS[3] = { // 1 ^= 10 mg
      2500, // light: 25.00 g
      5000, // medium: 50.00 g
      10000, // heavy: 100.00 g
    };

    MassOption option = MassOption::Current;

    void up() {
      option = (option - 1 == -1) ? MassOption::End - 1 : (option - 1);
    }
    void down() {
      option = (option + 1 == MassOption::End) ? 0 : (option + 1);
    }
    void press() {
      switch (option) {
      case MassOption::Current:
        next_state = AppState::Menu;
        break;
      case MassOption::Light:
        next_state = AppState::Menu;
        break;
      case MassOption::Medium:
        next_state = AppState::Menu;
        break;
      case MassOption::Heavy:
        next_state = AppState::Menu;
        break;
      case MassOption::Custom:
        next_state = AppState::NumberSelect;
        break;
      }
    }
    void render() {
      static char current_buf[18];
      static const char *labels[5] = {
        current_buf,
        PRESET_LABELS[0],
        PRESET_LABELS[1],
        PRESET_LABELS[2],
        PRESET_LABELS[3],
      };

      constexpr int mass = 0;
      sprintf(current_buf, "Current%9.1dg", mass);

      Ui::Menuuu(MassOption::Current, MassOption::End, (int*)&option, labels, ICONS);
    }
  };
  class NumberSelect {
    public:
    enum Option {
      Number = 0,
      Back,
      Confirm,
      End
    };

    Option option = Option::Back;
    float scroll_x = 0.0f;
    int cursor = 0;
    int number = 11101; // 111.01 g
    int array[6] = {0}; // 9999.99 g MAX / stored in reverse order
    int digits = 5;
    char unit = 'g';
    int prev_menu;
    bool is_editing = false;
    u32 offset_up_anim_start = 0;
    u32 offset_down_anim_start = 0;
    
    // _ _ _ _ 1 . 1 0 mg n y

    void enter(int prev) {
      prev_menu = prev;
      //digits = get_digits(number);
      cursor = digits;
      for (int i = 0; i <= digits; i++) {
        array[i] = get_digit_at(number, i);
      }
    }
    static inline int get_digits(int n) {
      return int(log10(n));
    }
    static inline int get_digit_at(int n, int index) {
      return ((n % int(pow(10, index + 1))) - (n % int(pow(10, index)))) / int(pow(10, index));
    }
    void up() {
      if (is_editing) {
        array[digits - cursor] = (array[digits - cursor] == 9) ? 0 : array[digits - cursor] + 1;
        offset_up_anim_start = millis();
        return;
      }
      if (option == Option::Number) {
        if (cursor == digits) {
          option = Option::Back;
        } else {
          cursor += 1;
        }
        return;
      }
      if (option == Option::Back)
        cursor = 0;
      option = (option + 1 == Option::End) ? 0 : (option + 1);
    }
    void down() {
      if (is_editing) {
        array[digits - cursor] = (array[digits - cursor] == 0) ? 9 : array[digits - cursor] - 1;
        offset_down_anim_start = millis();
        return;
      }
      if (option == Option::Number) {
        if (cursor - 1 < 0) {
          option = Option::Confirm;
        } else {
          cursor -= 1;
        }
        return;
      }
      if (option == Option::Confirm)
        cursor = digits;
      option = (option - 1 == -1) ? Option::End - 1 : (option - 1);
    }
    void press() {
      switch (option) {
      case Option::Confirm:
      case Option::Back:
        next_state = prev_menu;
        progress = 1.5f;
        break;
      case Option::Number:
        is_editing = !is_editing;
      }
    }
    float offset_anim(float t) {
      // Quatratic function (soft falloff, but we already lerp between values)
      //return 10. * max(0., 1. - (8.0 * t - 1.0) * (8.0 * t - 1.0));
      // Sign function
      constexpr float amplitude = 8.;
      constexpr float duration = 0.2;
      return amplitude * max(0., -(t - duration)/abs(t - duration));
    }
    void render() {
      constexpr int NS_FONT_HEIGHT = FONT_HEIGHT * 2;
      constexpr int NS_FONT_WIDTH = FONT_WIDTH * 2;
      constexpr int ANCHOR_Y = SCREEN_HEIGHT / 2 - NS_FONT_HEIGHT / 2;
      constexpr int SELECT_ANCHOR_X = SCREEN_WIDTH / 2 - NS_FONT_WIDTH / 2;
      constexpr int SELECT_ANCHOR_Y = SCREEN_HEIGHT / 2;
      constexpr int NUMBER_SPACING = 1;
      constexpr int ITEM_SPACING = 7;
      const int decimal_point = 2;

      //int scroll_x = option * (NS_FONT_WIDTH * 2);
      int anchor_x = 0.0f;
      float real_scroll_x = 0.0f;

      display.setTextSize(2);

      // Render Back Icon
      if (option == Option::Back)
        real_scroll_x = float(anchor_x + 5);
      display.drawBitmap(
        anchor_x + scroll_x,
        ANCHOR_Y,
        Bitmap::BACKARROW16,
        16, 16,
        SSD1306_WHITE
      );
      anchor_x += 16 + ITEM_SPACING;
      
      // Render numbers
      for (int i = 0; i <= digits; i++) {
        if (option == Option::Number && cursor == (digits - i))
          real_scroll_x = float(anchor_x + 3);
        display.setCursor(anchor_x + round(scroll_x), ANCHOR_Y);
        display.write(int(array[i]) + 48);
        anchor_x += NS_FONT_WIDTH + NUMBER_SPACING;

        if ((digits - i) == decimal_point) {
          display.setCursor(anchor_x + scroll_x - 3, ANCHOR_Y);
          display.write('.');
          anchor_x += NS_FONT_WIDTH + NUMBER_SPACING - 6;
        }
      }

      // Render unit
      display.setCursor(anchor_x + scroll_x, ANCHOR_Y);
      display.write(unit);
      anchor_x += NS_FONT_WIDTH + ITEM_SPACING;

      // Debug
      //display.setCursor(60, ANCHOR_Y + 20);
      //display.print(log);

      // Render Confirm Icon
      if (option == Option::Confirm)
        real_scroll_x = float(anchor_x + 6);
      display.drawBitmap(
        anchor_x + scroll_x,
        ANCHOR_Y,
        Bitmap::CHECK16,
        16, 16,
        SSD1306_WHITE
      );

      //if (option == Option::Number) {
      const int inverse = is_editing ? 1 : -1;
      //int offset = 1.0 + sin(millis() / 150.) * 2;
      static int offset_up = 0;
      static int offset_down = 0;
      const float wave = (1.0 + sin(millis() / 150.)) * 2.0;
      offset_up = Lerp(offset_up, offset_anim((millis() - offset_up_anim_start) / 1000.), 0.5);
      offset_down = Lerp(offset_down, offset_anim((millis() - offset_down_anim_start) / 1000.), 0.5);

      display.drawBitmap(
        SELECT_ANCHOR_X,
        SELECT_ANCHOR_Y - 4 - (NS_FONT_HEIGHT + offset_up + (is_editing ? 0 : wave)) * inverse,
        Bitmap::UP16X8,
        16, 8,
        SSD1306_WHITE
      );
      if (is_editing)
        display.drawBitmap(
          SELECT_ANCHOR_X,
          SELECT_ANCHOR_Y - 4 + (NS_FONT_HEIGHT + offset_down) * inverse,
          Bitmap::DOWN16X8,
          16, 8,
          SSD1306_WHITE
        );
      //}
      
/*       if (option != Option::Number) {
        display.drawRect(
          SELECT_ANCHOR_X - 1 - 2,
          SELECT_ANCHOR_Y - 8 - 1,
          20 + 1,
          NS_FONT_HEIGHT + 2,
          SSD1306_INVERSE
        );
      } */

      // Smooth scroll
      scroll_x = Ui::Lerp(scroll_x, -real_scroll_x + SCREEN_WIDTH / 2, 0.2);

      display.setTextSize(1);
    }
  };
};

class App {
  public:
  Ui::Menu menu = Ui::Menu();
  Ui::Dashboard dashboard = Ui::Dashboard();
  Ui::MassSetup masssetup = Ui::MassSetup();
  Ui::NumberSelect numberselect = Ui::NumberSelect();

  public:
  void up() {
    switch (app_state) {
    case AppState::Menu:
      menu.up();
      break;
    case AppState::MassSetup:
      masssetup.up();
      break;
    case AppState::NumberSelect:
      numberselect.up();
      break;
    }
  }
  void down() {
    switch (app_state) {
    case AppState::Menu:
      menu.down();
      break;
    case AppState::MassSetup:
      masssetup.down();
      break;
    case AppState::NumberSelect:
      numberselect.down();
      break;
    }
  }
  void press() {
    switch (app_state) {
    case AppState::Menu:
      menu.press();
      break;
    case AppState::MassSetup:
      masssetup.press();
      break;
    case AppState::Dashboard:
      next_state = AppState::Menu;
      break;
    case AppState::NumberSelect:
      numberselect.press();
      break;
    }
  }
  void enter(AppState prev_state) {
    switch(app_state) {
    case AppState::Menu:
      break;
    case AppState::MassSetup:
      break;
    case AppState::NumberSelect:
      numberselect.enter(prev_state);
      break;
    }
  }
  void exit(AppState next_state) {

  }
  void setup() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

    display.setTextWrap(false);
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
  }
  void render() {
    // State change logic
    if (next_state != -1) {
      exit(next_state);
      auto prev_state = app_state;
      app_state = next_state;
      next_state = -1;
      enter(prev_state);
    }

    // Menu render
    display.clearDisplay();

    switch (app_state) {
    case AppState::Menu:
      menu.render();
      break;
    case AppState::Dashboard:
      dashboard.render();
      break;
    case AppState::MassSetup:
      masssetup.render();
      break;
    case AppState::NumberSelect:
      numberselect.render();
    }

    display.display();
  }
};
App app = App();

namespace RotaryEncoder {
  // Arduino and KY-040 module
  int encoderBtn = 3; // SW pin
  volatile unsigned long lastPress = millis();

  void button() {
    app.press();
  }
  void buttonISR(){
    if (millis() - lastPress < 100)
      return;
    if (!digitalRead(3))
      button();
    lastPress = millis();
  }

  inline void setup() {
    pinMode(encoderBtn, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderBtn), buttonISR, CHANGE);
  }
}

Encoder encoder(2, 4);

// Arduino Code
void setup() {
  Serial.begin(9600);
  Serial.println(""); // TEMPORARY: Open Serial monitor on WOKWI

  app.setup();

  RotaryEncoder::setup();
}
void loop() {
  // TEMPORARY SERIAL CONTROL
  if (Serial.available() > 0) {
    // read the incoming byte:
    int serialInput = Serial.read();

    if (serialInput == 'w') {
      app.up();
    }
    if (serialInput == 's') {
      app.down();
    }
    if (serialInput == 'x') {
      app.press();
    }
  }

  int8_t c = encoder.read();
  if (abs(c) > 3) {
    Serial.println(c);
    if (c < 0) {
      app.up();
      encoder.write(c + 4);
    } else {
      app.down();
      encoder.write(c - 4);
    }
  }

  app.render();
}