#ifndef __FDD41143_87A7_41D5_910D_43C3FD88F1E7__
#define __FDD41143_87A7_41D5_910D_43C3FD88F1E7__

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define FEATURE_MATRIX

#include <ReactESP.h>
#include <event_loop.h>
#include <events.h>
using namespace reactesp;

#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>

static const uint8_t PROGMEM
    smile_bmp[] = {0b00111100, 0b01000010, 0b10100101, 0b10000001,
                   0b10100101, 0b10011001, 0b01000010, 0b00111100},
    neutral_bmp[] = {0b00111100, 0b01000010, 0b10100101, 0b10000001,
                     0b10111101, 0b10000001, 0b01000010, 0b00111100},
    frown_bmp[] = {0b00111100, 0b01000010, 0b10100101, 0b10000001,
                   0b10011001, 0b10100101, 0b01000010, 0b00111100};

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();

struct MarqueeEvent {
  const char *text;
  uint32_t delay_ms;
};

void __marquee_task(void *args) {
  MarqueeEvent *event = (MarqueeEvent *)args;
  const char *text = event->text;
  char *buf = new char[strlen(text) + 1];
  strcpy(buf, text);

  uint16_t textLength = strlen(event->text) * 7;
  uint32_t delay_ms = event->delay_ms;

  for (uint16_t x = 0; x < textLength; x++) {
    matrix.clear();
    matrix.setCursor(-x, 1);
    matrix.print(buf);
    delay(delay_ms);
  }
  Serial.println("Marquee done");
  delete[] buf;
  delete event;
  vTaskDelete(NULL);
}

void marquee_text(const char *text, uint16_t brightness = 255,
                  uint32_t delay_ms = 50) {
  uint16_t textLength = strlen(text) * 7;
  matrix.setTextSize(1);
  matrix.setTextWrap(false);
  matrix.setTextColor(brightness);

  MarqueeEvent *event = new MarqueeEvent{text, delay_ms};
  xTaskCreatePinnedToCore(__marquee_task, "Task Matrix", 8192, event, 1, NULL,
                          ARDUINO_RUNNING_CORE);
}

void init_matrix() {
  // Initialize LED Matrix
  matrix.begin();
  matrix.clear();
  matrix.setRotation(0);
  marquee_text("Hello World!");
}

void draw_face(uint8_t face) {
  matrix.clear();
  switch (face) {
  case 0:
    matrix.drawBitmap(0, 0, smile_bmp, 8, 8, 255);
    break;
  case 1:
    matrix.drawBitmap(0, 0, neutral_bmp, 8, 8, 255);
    break;
  case 2:
    matrix.drawBitmap(0, 0, frown_bmp, 8, 8, 255);
    break;
  }
}

#endif /* __FDD41143_87A7_41D5_910D_43C3FD88F1E7__ */