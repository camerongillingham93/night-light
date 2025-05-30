/**
 * Night Light Firmware V1 - LED Controller
 * ---------------------------------------
 * Controls LED strip operations including:
 * - Basic state management (on/off, brightness, color)
 * - Special effects including:
 *   - Low battery warning (pulsing red)
 *   - Firefly effect (random soft yellow-green flashes)
 * - Effect state management
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.0
 */

#include "LEDController.h"
#include <Arduino.h>

LEDController::LEDController(uint8_t pin, uint16_t numPixels)
    : strip(numPixels, pin, NEO_GRB + NEO_KHZ800), ledsOn(false),
      brightness(220), red(160), green(255), blue(30), effectRunning(false),
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

bool LEDController::isEffectRunning() { return effectRunning; }

void LEDController::lowBattery() {
  // Save the current state to restore later
  bool previousState = ledsOn;
  uint8_t prevRed = red;
  uint8_t prevGreen = green;
  uint8_t prevBlue = blue;
  uint8_t prevBrightness = brightness;

  // R and G are backwards???
  red = 0;
  green = 255;
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


void LEDController::fireflyEffect() {
  // Save the current state to restore later
  bool previousState = ledsOn;
  uint8_t prevRed = red;
  uint8_t prevGreen = green;
  uint8_t prevBlue = blue;
  uint8_t prevBrightness = brightness;

  // Set effect running flag
  effectRunning = true;

  // Colors for fireflies - warmish green-yellow
  const uint8_t fireflyRed = 160;
  const uint8_t fireflyGreen = 255;
  const uint8_t fireflyBlue = 30;

  // Effect duration and parameters
  const unsigned long effectDuration = 5000; // Run for 5 seconds
  const unsigned long startTime = millis();

  // Each LED is a potential firefly with its own variables
  struct Firefly {
    bool active;
    unsigned long nextFlashTime;
    unsigned long flashDuration;
    uint8_t maxBrightness;
    int flashPhase; // 0=off, 1=fade in, 2=fade out
  };

  // Initialize array of firefly data
  Firefly fireflies[strip.numPixels()];
  for (int i = 0; i < strip.numPixels(); i++) {
    fireflies[i].active = false;
    fireflies[i].nextFlashTime = startTime + random(0, 1000);
    fireflies[i].flashDuration = random(300, 800);
    fireflies[i].maxBrightness = random(100, 255);
    fireflies[i].flashPhase = 0;
  }

  // Turn all LEDs off initially
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();

  // Run the effect for the specified duration
  while (millis() - startTime < effectDuration) {
    unsigned long currentTime = millis();

    // Update each firefly
    for (int i = 0; i < strip.numPixels(); i++) {
      // Check if it's time to activate a new firefly
      if (!fireflies[i].active && currentTime >= fireflies[i].nextFlashTime) {
        fireflies[i].active = true;
        fireflies[i].flashPhase = 1; // Start fade-in
      }

      // Handle active fireflies
      if (fireflies[i].active) {
        unsigned long elapsedTime = currentTime - fireflies[i].nextFlashTime;

        if (fireflies[i].flashPhase == 1) { // Fade in
          if (elapsedTime < fireflies[i].flashDuration / 2) {
            // Calculate brightness - gentle fade in
            float progress =
                (float)elapsedTime / (fireflies[i].flashDuration / 2);
            uint8_t brightness = progress * fireflies[i].maxBrightness;

            // Softened yellow-green with adjustable brightness
            uint8_t r = (fireflyRed * brightness) / 255;
            uint8_t g = (fireflyGreen * brightness) / 255;
            uint8_t b = (fireflyBlue * brightness) / 255;

            strip.setPixelColor(i, strip.Color(r, g, b));
          } else {
            // Transition to fade out
            fireflies[i].flashPhase = 2;
          }
        } else if (fireflies[i].flashPhase == 2) { // Fade out
          unsigned long fadeOutTime =
              elapsedTime - (fireflies[i].flashDuration / 2);

          if (fadeOutTime < fireflies[i].flashDuration / 2) {
            // Calculate brightness - gentle fade out
            float progress =
                1.0 - ((float)fadeOutTime / (fireflies[i].flashDuration / 2));
            uint8_t brightness = progress * fireflies[i].maxBrightness;

            // Softened yellow-green with adjustable brightness
            uint8_t r = (fireflyRed * brightness) / 255;
            uint8_t g = (fireflyGreen * brightness) / 255;
            uint8_t b = (fireflyBlue * brightness) / 255;

            strip.setPixelColor(i, strip.Color(r, g, b));
          } else {
            // Flash complete, turn off and schedule next flash
            strip.setPixelColor(i, strip.Color(0, 0, 0));
            fireflies[i].active = false;
            fireflies[i].nextFlashTime = currentTime + random(500, 3000);
            fireflies[i].flashDuration = random(300, 800);
            fireflies[i].maxBrightness = random(100, 255);
          }
        }
      }
    }

    // Update the strip
    strip.show();
    delay(10); // Small delay for stability
  }

  // Fade out all fireflies at the end
  for (int fade = 255; fade >= 0; fade -= 5) {
    for (int i = 0; i < strip.numPixels(); i++) {
      uint32_t color = strip.getPixelColor(i);
      uint8_t r = ((color >> 16) & 0xFF) * fade / 255;
      uint8_t g = ((color >> 8) & 0xFF) * fade / 255;
      uint8_t b = (color & 0xFF) * fade / 255;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
    delay(10);
  }

  // Restore previous color and brightness settings
  red = prevRed;
  green = prevGreen;
  blue = prevBlue;
  brightness = prevBrightness;
  ledsOn = previousState; // Restore previous on/off state

  // End effect
  effectRunning = false;

  // Update LEDs with restored settings
  updateStrip();
}

void LEDController::wakeEffect() {
  // Save the current state to restore later
  bool previousState = ledsOn;

  // Set effect running flag
  effectRunning = true;

  // Heartbeat pulse parameters
  const int fadeSteps = 30;
  const int fastPulseDelay = 8; // Quick fade for heartbeat effect
  const int holdDelay = 50;     // Brief hold at peak
  const int pauseDelay = 200;   // Pause between pulses

  // Turn all LEDs off initially
  ledsOn = false;
  updateStrip();

  // Pulse three times like a heartbeat
  for (int pulse = 0; pulse < 3; pulse++) {
    // Fade in
    for (int i = 0; i < fadeSteps; i++) {
      uint8_t pulseBrightness = (i * brightness) / fadeSteps;

      // Apply brightness to each color component
      uint8_t r = (red * pulseBrightness) / 255;
      uint8_t g = (green * pulseBrightness) / 255;
      uint8_t b = (blue * pulseBrightness) / 255;
      uint32_t color = strip.Color(r, g, b);

      for (int j = 0; j < strip.numPixels(); j++) {
        strip.setPixelColor(j, color);
      }
      strip.show();
      delay(fastPulseDelay);
    }

    // Hold at full brightness briefly
    delay(holdDelay);

    // Only fade out if it's not the last pulse
    if (pulse < 2) {
      // Fade out
      for (int i = fadeSteps - 1; i >= 0; i--) {
        uint8_t pulseBrightness = (i * brightness) / fadeSteps;

        // Apply brightness to each color component
        uint8_t r = (red * pulseBrightness) / 255;
        uint8_t g = (green * pulseBrightness) / 255;
        uint8_t b = (blue * pulseBrightness) / 255;
        uint32_t color = strip.Color(r, g, b);

        for (int j = 0; j < strip.numPixels(); j++) {
          strip.setPixelColor(j, color);
        }
        strip.show();
        delay(fastPulseDelay);
      }

      // Pause between pulses
      delay(pauseDelay);
    }
  }

  // Don't restore color/brightness since we used the original values
  ledsOn = true; // Turn LEDs on after wake effect

  // End effect
  effectRunning = false;

  // Update LEDs with current settings - all LEDs should now be on
  updateStrip();
}