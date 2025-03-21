#include "LEDController.h"
#include <Arduino.h>

LEDController::LEDController(uint8_t pin, uint16_t numPixels)
    : strip(numPixels, pin, NEO_GRB + NEO_KHZ800), ledsOn(false),
      brightness(255), red(255), green(255), blue(255), effectRunning(false),
      effectStartTime(0) {}

void LEDController::begin() {
  strip.begin();
  strip.show(); // Ensure all LEDs are off at startup
}

void LEDController::setState(bool state) {
  if (ledsOn != state) { // Prevent redundant updates
    ledsOn = state;
    updateStrip();
  }
}

void LEDController::toggle() {
  ledsOn = !ledsOn;
  updateStrip();
}

void LEDController::setBrightness(uint8_t newBrightness) {
  if (brightness != newBrightness) { // Avoid unnecessary updates
    brightness = newBrightness;
    updateStrip();
  }
}

void LEDController::setColor(uint8_t r, uint8_t g, uint8_t b) {
  red = r;
  green = g;
  blue = b;
  updateStrip();
}

void LEDController::updateStrip() {
  if (effectRunning)
    return; // Don't interfere with running effects

  uint32_t color;
  if (ledsOn) {
    // Apply brightness to each color component
    uint8_t r = (red * brightness) / 255;
    uint8_t g = (green * brightness) / 255;
    uint8_t b = (blue * brightness) / 255;
    color = strip.Color(r, g, b);
  } else {
    color = strip.Color(0, 0, 0);
  }

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }

  strip.show();
}

void LEDController::startSpecialEffect() {
  Serial.println("LED Controller: Starting special effect");
  effectStartTime = millis();
  effectRunning = true;
}

void LEDController::stopSpecialEffect() {
  Serial.println("LED Controller: Stopping special effect");
  effectRunning = false;
  updateStrip(); // Restore normal LED state
}

void LEDController::updateSpecialEffect() {
  if (!effectRunning)
    return;

  // Simple green blinking effect
  unsigned long progress = millis() - effectStartTime;
  bool isOn = ((progress / 250) % 2) == 0; // Blink every 250ms

  if (isOn) {
    // Set all pixels to green
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
  } else {
    // Turn all pixels off
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }

  strip.show();
}

bool LEDController::isEffectRunning() { return effectRunning; }