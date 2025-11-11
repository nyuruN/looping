#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define FONT_HEIGHT 8
#define FONT_WIDTH 6

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Fuck you Adafruit
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Paste your generated bitmap data here
static const unsigned char PROGMEM skull_bitmap[] = {
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

static const unsigned char PROGMEM backarrow_bitmap[] = {
B00000000, 
B00000000, 
B00100010, 
B01000010, 
B11111110, 
B01000000, 
B00100000, 
B00000000,
};

static const unsigned char PROGMEM gear_bitmap[] = {
B00000111, B11100000, 
B00011000, B00011000, 
B00100100, B00000100, 
B01001000, B00000010, 
B01010100, B00000010, 
B10001000, B00000001, 
B10010000, B00000001, 
B10000000, B00000001, 
B10000000, B00000001, 
B10000000, B00000001, 
B10000000, B00000001, 
B01000000, B00000010, 
B01000000, B00000010, 
B00100000, B00000100, 
B00011000, B00011000, 
B00000111, B11100000,
};

void setup() {
  Serial.begin(9600);
  Serial.println(""); // Open Serial monitor on WOKWI

  display.setTextWrap(false);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show Adafruit default logo
  display.display();
  delay(1000);
}

float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

// We have the following primitives
// A box
// A box with no fill (4 boxes)
// Lines (A really really thin box)
// Text
// Bitmaps
// This is enough to construct an UI
namespace Gui {
  typedef struct{
    int x;
    int y;
  } Vec2;

  constexpr int LINE_SPACING = 1;

  Vec2 cursor = {0, 0};

  void SetCursor(int x, int y) {
    cursor.x = x;
    cursor.y = y;
  }
  Vec2 GetTextSize(const char * str) {
    return {FONT_WIDTH * strlen(str), FONT_HEIGHT};
  }
  void Text(const char * str) {
    display.setCursor(cursor.x, cursor.y);
    display.write(str);
    cursor.y += FONT_HEIGHT + LINE_SPACING;
  }
  void TextWrapped(const char * string, int max_width) {
    int max_chars = max_width / FONT_WIDTH;
    size_t length = strlen(string);

    // We temporarily terminate the string :(
    char temp;
    char * ptr = string;
    size_t l = 0;
    char * terminator;
        
    while (l < length) {
      char * terminator = ptr + max_chars;
      temp = *terminator;
      *terminator = NULL;

      // I don't know what you're on but why won't you work?? You are the fucking same
      //Text(ptr);
      display.setCursor(cursor.x, cursor.y);
      display.write(ptr);
      cursor.y += FONT_HEIGHT + LINE_SPACING;
 
      ptr += max_chars;
      l += max_chars;
      *ptr = temp;
    }
  }
}


// Final sizes are resolved at runtime
// Each element has a border size, padding, margin, fix height and width
// Border rendering is resolved at runtime
// 
// Masse * * * * * * (=====)
class Menu {
public:
  enum MenuState {
    Back = 0,
    OptionA,
    OptionB,
    OptionC,
    OptionD,
    End,
  };

  bool active = false;
  bool option_a = false;
  bool lock = false;
private:
  MenuState state = MenuState::OptionA;
public:
  void up() {
    if (lock) return;
    state = (state - 1 == -1) ? MenuState::End - 1 : (state - 1);
  }
  void down() {
    if (lock) return;
    state = (state + 1 == MenuState::End) ? 0 : (state + 1);
  }
  void press() {
    switch (state) {
    case MenuState::Back:
      active = false;
      break;
    case MenuState::OptionA:
      option_a = !option_a;
      lock = !lock;
      Serial.println("Option A pressed!");
      break;
    }
  }
  void render() {  
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);

    static const unsigned char * icons[] = {
      backarrow_bitmap,
      skull_bitmap,
      gear_bitmap,
      backarrow_bitmap,
      backarrow_bitmap
    };
    static const char * labels[] = {
      "Back",
      "Option A",
      "Option B",
      "Option C",
      "Option D",
    };

    const int anchor_x = 4;
    int anchor_y = 4;
    const int size_x = 120;
    const int size_y = FONT_HEIGHT + 10;
    
    // Smooth scroll
    static float progress = float(state) / float(MenuState::End - 1);
    float real_progress = float(state) / float(MenuState::End - 1);
    progress = lerp(progress, real_progress, 0.2);
    int scrollY = -int(progress * float(size_y * (MenuState::End - 1) + 2 * (MenuState::End - 2)))
     + SCREEN_HEIGHT / 4;

    display.setTextColor(SSD1306_WHITE);

    // Render items
    for (MenuState s = 0; s < MenuState::End; s = s + 1) {
      display.drawBitmap(anchor_x + 4, anchor_y + 5 + scrollY, icons[s], 8, 8, WHITE);
      Gui::SetCursor(anchor_x + 4 + 8 + 2, anchor_y + 5 + scrollY);
      
      if (s == state) {
        display.drawRoundRect(
          anchor_x,
          anchor_y + scrollY,
          size_x,
          size_y,
          3,
          SSD1306_WHITE
        );
      }

      Gui::Text(labels[s]);
      anchor_y += size_y + 2;
    }

    // Render Scrollbar
    const int scrollbar_x = SCREEN_WIDTH - 2;
    for (int i = 0; i < SCREEN_HEIGHT; i += 2) {
      display.drawPixel(scrollbar_x, i, SSD1306_WHITE);
    }
    int scrollbar_y = int(lerp(float(4), float(SCREEN_HEIGHT - 4), progress));
    display.fillRect(
      scrollbar_x - 1,
      scrollbar_y - 4,
      3,
      8,
      SSD1306_WHITE
    );



    if (option_a) {
      constexpr int dialog_width = 60;
      constexpr int dialog_height = 40;
      constexpr int anchor_x = SCREEN_WIDTH / 2 - dialog_width / 2;
      constexpr int anchor_y = SCREEN_HEIGHT / 2 - dialog_height / 2;

      display.fillRect(
        anchor_x,
        anchor_y,
        dialog_width,
        dialog_height,
        SSD1306_BLACK
      );
      
      display.drawRect(
        anchor_x,
        anchor_y,
        dialog_width,
        dialog_height,
        SSD1306_WHITE
      );

      Gui::SetCursor(anchor_x + 2, anchor_y + 2);

      Gui::Text("Hey babe");
    }
  }
};

Menu menu = Menu();

void render_logopage() {
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
    skull_bitmap, 16, 16, WHITE
  );

  display.setCursor(
    display.width() / 2 - (FONT_WIDTH * 17 / 2),
    display.height() / 2 - (FONT_HEIGHT + 16 + 2) / 2 + 16 + 2
  );
  
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);

  display.println("Imagine dying lol");
}

void loop() {
  // TEMPORARY SERIAL CONTROL
  if (Serial.available() > 0) {
    // read the incoming byte:
    int serialInput = Serial.read();

    if (menu.active) {
      if (serialInput == 'w') {
        menu.up();
      }
      if (serialInput == 's') {
        menu.down();
      }
      if (serialInput == 'x') {
        menu.press();
      }
    } else {
      if (serialInput == 'x') {
        menu.active = !menu.active;
      }
    }
  }

  // Menu render
  display.clearDisplay();

  if (menu.active) {
    menu.render();
  } else {
    render_logopage();
  }

  display.display();
}
