#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

//#include <Arduino.h>
#include <tinyNeoPixel.h>

class LEDController {
public:
  LEDController(uint8_t pin, uint16_t numPixels);
  void begin();
  void setState(bool state);
  void toggle();
  void setBrightness(uint8_t newBrightness);
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void updateStrip();
  void lowBattery();
  void fireflyEffect();
  bool isEffectRunning(); // New method to check effect status
  // Add this to LEDController.h in the public section:
  bool getState() const { return ledsOn; }

private:
  tinyNeoPixel strip;
  bool ledsOn;
  uint8_t brightness;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  bool effectRunning;
  unsigned long effectStartTime;
};

#endif