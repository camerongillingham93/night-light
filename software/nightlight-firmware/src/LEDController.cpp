#include "LEDController.h"

LEDController::LEDController(uint8_t pin, uint16_t numPixels)
    : strip(numPixels, pin, NEO_GRB + NEO_KHZ800), ledsOn(false),
      brightness(255) {}

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

void LEDController::updateStrip() {
  uint32_t color = ledsOn ? strip.Color(brightness, brightness, brightness)
                          : strip.Color(0, 0, 0);

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }

  strip.show(); // This function may introduce slight delay
}
