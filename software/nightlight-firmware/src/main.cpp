#include "CapTouch.h"
#include "LEDController.h"
#include "config.h"
#include <Arduino.h>

// Create sensor: sense pin, reference pin, threshold, debounce count
CapTouch touchSensor(sensePin, refPin, touchThreshold, 3);
LEDController strip(LED_PIN, NUM_PIXELS);

uint8_t brightness = 128; // Medium brightness to start
uint8_t currentBrightness = MAX_BRIGHTNESS;
bool brightnessIncreasing = false; // Direction flag for triangle wave

void setup() {
  Serial.begin(9600);
  delay(100); // Short delay for serial to initialize

  // Initialize the touch sensor
  touchSensor.begin();
  // Set the long press detection time (milliseconds)
  touchSensor.setLongPressTime(800); // 0.8 seconds for long press
  // Initialize the LED strip
  strip.begin();
}

void loop() {
  // Take measurement and update all touch states
  uint16_t rawValue = touchSensor.measure();

  // Process different types of touches
  if (touchSensor.isShortPress()) {
    Serial.println("Short press - LED toggled");
    strip.toggle();
  }

  // Handle long press (adjust brightness in triangle wave pattern)
  if (touchSensor.isLongPress()) {
    Serial.println("Long press detected - changing brightness");

    // Adjust direction at min/max values
    if (currentBrightness <= MIN_BRIGHTNESS) {
      brightnessIncreasing = true;
    } else if (currentBrightness >= MAX_BRIGHTNESS) {
      brightnessIncreasing = false;
    }

    // Adjust brightness based on direction
    if (brightnessIncreasing) {
      // Ensure we don't exceed MAX_BRIGHTNESS
      if (currentBrightness <= MAX_BRIGHTNESS - BRIGHTNESS_STEP) {
        currentBrightness += BRIGHTNESS_STEP;
      } else {
        currentBrightness = MAX_BRIGHTNESS;
      }
    } else {
      // Ensure we don't go below MIN_BRIGHTNESS
      if (currentBrightness >= MIN_BRIGHTNESS + BRIGHTNESS_STEP) {
        currentBrightness -= BRIGHTNESS_STEP;
      } else {
        currentBrightness = MIN_BRIGHTNESS;
      }
    }

    Serial.print("Setting brightness to: ");
    Serial.println(currentBrightness);

    // Update LED brightness
    strip.setBrightness(currentBrightness);

    delay(50); // 20Hz update rate - good balance of responsiveness and power
  }
}