#include <Arduino.h>

#include "./display.h"

#include "../include/ui.h"

float Ui::Lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

void Ui::Scrollbar(float progress) {
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

Ui::List::ListElement Ui::List::list[8];
float Ui::List::progress = 0.0f;

void Ui::List::create(const char** labels, const uint8_t** icons, uint8_t items) {
  for (uint8_t i = 0; i < items; ++i) {
    char* ptr = (char*) pgm_read_word_near(labels + i);
    for (uint8_t j = 0; j < 18; ++j) {
      list[i].label[j] = pgm_read_byte_near(ptr + j);
      if (!list[i].label[j]) break;
    }
    list[i].icon = (uint8_t*) pgm_read_word_near(icons + i);
  }
}

void Ui::List::render( uint8_t optionFirst, uint8_t optionEnd, uint8_t optionCurrent) {
  constexpr int ANCHOR_X = 4;
  int anchor_y = 4;
  constexpr int ITEM_SPACING = 2;
  constexpr int ICON_TEXT_GAP = 3; // This just cannot be 3, I... don't know why
  constexpr int PADDING_X = 4;
  constexpr int PADDING_Y = 5;
  constexpr int SIZE_X = 120;
  constexpr int SIZE_Y = FONT_HEIGHT + 10;
  
  // Smooth scroll
  float real_progress = float(optionCurrent) / float(optionEnd - 1);
  progress = Ui::Lerp(progress, real_progress, 0.2);
  int scroll_y = -int(progress * float(SIZE_Y * (optionEnd - 1) + ITEM_SPACING * (optionEnd - 2))) + SCREEN_HEIGHT / 4;

  // Render items
  for (int s = optionFirst; s < optionEnd; ++s) {
    display.drawBitmap(ANCHOR_X + PADDING_X, anchor_y + PADDING_Y + scroll_y, list[s].icon, 8, 8, WHITE);
    display.setCursor(ANCHOR_X + PADDING_X + 8 + ICON_TEXT_GAP, anchor_y + PADDING_Y + scroll_y);
    
    if (s == optionCurrent) {
      display.drawRoundRect(
        ANCHOR_X,
        anchor_y + scroll_y,
        SIZE_X,
        SIZE_Y,
        3,
        SSD1306_WHITE
      );
    }

    display.print(list[s].label);
    anchor_y += SIZE_Y + ITEM_SPACING;
  }

  Ui::Scrollbar(progress);
}
