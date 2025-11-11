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

const float masse = 20; // Gramm
const float strecke = 5; // cm

uint16_t timer;

// Interurupt auf Port 2, Pin 4
ISR(PCINT2_vect) {
  if (digitalRead(4) == HIGH) {
    TCNT1 = 0;
  } else {
    timer = TCNT1;
  }
}

void setup() {
  Serial.begin(1200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show Adafruit default logo
  display.display();

  // Set pin modes
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  
  // Setup Change interrupts
  PCICR = (1 << PCIE2); // Port 2 anschalten
  PCMSK2 = (1 << PCINT20); // Pin 4 anschalten

  // Setup Timer 1
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000101; // Setup Prescalar 1024
}

void text_wrap(int ofst_x, int ofst_y, int space, const char * string) {
  int max_chars = space / FONT_WIDTH;
  size_t length = strlen(string);

  // We temporarily terminate the string :(
  char temp = 0;
  char * ptr = string;
  size_t l = 0;
  
  //char * terminator = ptr + max_chars;
  //temp = *terminator;
  //*terminator = NULL;

  //display.setCursor(ofst_x, ofst_y + FONT_HEIGHT * (l / max_chars));
  //display.println(ptr);
      
  while (l < length) {
    /*if (l + max_chars < length) {
      display.setCursor(ofst_x, ofst_y + FONT_HEIGHT * (l / max_chars));
      display.println(ptr);
      break;
    }*/

    char * terminator = ptr + max_chars;
    temp = *terminator;
    *terminator = NULL;

    display.setCursor(ofst_x, ofst_y + FONT_HEIGHT * (l / max_chars));
    display.println(ptr);

    ptr += max_chars + 1;
    l += max_chars;
    //*ptr = temp;
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
    OptionA = 0,
    OptionB,
    OptionC,
    OptionD,
    End,
  };
private:
  MenuState state = MenuState::OptionA;
public:
  void up() {
    state = (state - 1 == -1) ? MenuState::End - 1 : (state - 1);
  }
  void down() {
    state = (state + 1 == MenuState::End) ? 0 : (state + 1);
  }
  void render() {  
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1, 1);
    display.drawRect(0, 0, display.width(), display.height(), SSD1306_WHITE);

    if (state == MenuState::OptionA)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Back and foreground color
    else display.setTextColor(SSD1306_WHITE);
    display.println("OptionA");

    display.setCursor(1, 1 + FONT_HEIGHT * 1);

    if (state == MenuState::OptionB)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Back and foreground color
    else display.setTextColor(SSD1306_WHITE);
    display.println("OptionB");
    
    display.setCursor(1, 1 + FONT_HEIGHT * 2);

    if (state == MenuState::OptionC)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Back and foreground color
    else display.setTextColor(SSD1306_WHITE);
    display.println("OptionC");
    
    display.setCursor(1, 1 + FONT_HEIGHT * 3);
 
    if (state == MenuState::OptionD)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Back and foreground color
    else display.setTextColor(SSD1306_WHITE);
    display.println("OptionD");
    
    display.drawLine(
      1 + FONT_WIDTH * 7 + 1, 1 + FONT_HEIGHT * 0,
      1 + FONT_WIDTH * 7 + 1, display.height() - 1, SSD1306_WHITE);

     

    display.setCursor(1 + FONT_WIDTH * 7 + 1, 1 + FONT_HEIGHT * 0);
    display.setTextColor(SSD1306_WHITE);
    //display.println("This is option a");
    text_wrap(1 + FONT_WIDTH * 7 + 1 + 1, 1, display.width() - (1 + FONT_WIDTH * 7 + 1) - 1,
    "REALLYREALLYREALLYREALLYREALLYREALLYLONGTEXT");
    //display.println("La la la la la la la la la this text is wayyyyyy to long");

  }
};

class Button {
  bool prev = false;
  bool curr = false;
  int pin;

public:
  Button(int digitalPin) : pin(digitalPin) {};
  void poll() {
    prev = curr;
    curr = digitalRead(pin) == HIGH;
  }
  bool is_just_pressed() { return !prev && curr; }
};

Button myButton(2);
Menu menu = Menu();

int serialInput = 0; // for incoming serial data


void loop() {
  float delta = 1024.*float(timer)/16000000.;

  myButton.poll();
  if (myButton.is_just_pressed()) {
    menu.down();
  }

  // TEMPORARY SERIAL CONTROL
  if (false && Serial.available() > 0) {
    // read the incoming byte:
    serialInput = Serial.read();

    if (serialInput == '0') {
      Serial.println("hit zero!");
    }
  }

  // Menu render
  display.clearDisplay();

  menu.render();

  display.display();
  
  //Serial.println(digitalRead(2) == HIGH);
}
