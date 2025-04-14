#include "LEDController.h"
#include <Arduino.h>

LEDController::LEDController(uint8_t pin, uint16_t numPixels)
    : strip(numPixels, pin, NEO_GRB + NEO_KHZ800), ledsOn(false),
      brightness(255), red(255), green(180), blue(70), effectRunning(false),
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
  //Serial.println("-- updateStrip called --");

  // if (effectRunning) {
  //   Serial.println("Effect running, not updating strip");
  //   return; // Don't interfere with running effects
  // }

  uint32_t color;
  if (ledsOn) {
    // Apply brightness to each color component
    uint8_t r = (red * brightness) / 255;
    uint8_t g = (green * brightness) / 255;
    uint8_t b = (blue * brightness) / 255;
    color = strip.Color(r, g, b);

    // Serial.print("LEDs ON, color components: R=");
    // Serial.print(r);
    // Serial.print(" G=");
    // Serial.print(g);
    // Serial.print(" B=");
    // Serial.println(b);
  } else {
    color = strip.Color(0, 0, 0);
   // Serial.println("LEDs OFF, setting color to black");
  }

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }

  strip.show();
  // Serial.println("-- strip.show() called --");
  // Serial.print("Current Brightness is:");
  // Serial.println(brightness);
}

void LEDController::startSpecialEffect() {
  Serial.println("LED Controller: Starting special effect");
  effectStartTime = millis();
  effectRunning = true;
}

void LEDController::stopSpecialEffect() {
  //Serial.println("LED Controller: Stopping special effect");
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

void LEDController::lowBattery(){
  // Save the current state to restore later
  bool previousState = ledsOn;
  uint8_t prevRed = red;
  uint8_t prevGreen = green;
  uint8_t prevBlue = blue;
  uint8_t prevBrightness = brightness;

  // Set color to red
  red = 255;
  green = 0;
  blue = 0;

  // Number of steps for fade in/out
  const int fadeSteps = 25;
  // Longer duration for each pulse
  const int pulseDelay = 15; // milliseconds per fade step

  // Pulse three times
  for (int pulse = 0; pulse < 3; pulse++) {
    // Fade in
    for (int i = 0; i < fadeSteps; i++) {
      brightness = (i * 255) / fadeSteps; // Calculate brightness from 0 to 255
      ledsOn = true;
      updateStrip();
      delay(pulseDelay);
    }

    // Hold at full brightness briefly
    delay(100);

    // Fade out
    for (int i = fadeSteps - 1; i >= 0; i--) {
      brightness = (i * 255) / fadeSteps; // Calculate brightness from 255 to 0
      updateStrip();
      delay(pulseDelay);
    }

    // Pause between pulses if not the last one
    if (pulse < 2) {
      ledsOn = false;
      updateStrip();
      delay(300); // Longer pause between pulses
    }
  }

  // Ensure LEDs are off at the end
  ledsOn = false;
  updateStrip();

  // Restore previous color and brightness settings
  red = prevRed;
  green = prevGreen;
  blue = prevBlue;
  brightness = prevBrightness;

  // Don't restore previous on/off state - in low battery mode we want to keep
  // lights off ledsOn = previousState;  // We're not restoring this
  updateStrip();
}
