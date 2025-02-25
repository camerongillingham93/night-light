#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <tinyNeoPixel.h>

class LEDController {
public:
  LEDController(uint8_t pin, uint16_t numPixels);

  void begin();
  void setState(bool state);
  void toggle();
  void setBrightness(uint8_t newBrightness);

private:
  tinyNeoPixel strip;
  bool ledsOn;
  uint8_t brightness;

  void updateStrip();
};

#endif
