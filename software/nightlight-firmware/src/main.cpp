#include "LEDController.h"
#include "config.h"
#include "controls.h"
#include <Arduino.h>

// Create sensor: sense pin, reference pin, threshold, debounce count
CapTouch touchSensor(sensePin, refPin, touchThreshold, 3);
LEDController strip(LED_PIN, NUM_PIXELS);
Shake shake(tiltSW);

// State variables
bool longPressActive = false;
unsigned long effectStartTime = 0;
const unsigned long EFFECT_DURATION = 3000; // Effect runs for 3 seconds

void setup() {
  Serial.begin(9600);
  delay(100); // Short delay for serial to initialize

  // Initialize the touch sensor
  touchSensor.begin(sensePin, refPin); // Corrected: Pass both pins
  // Set the long press detection time (milliseconds)
  touchSensor.setLongPressTime(800); // 0.8 seconds for long press

  // Configure brightness control
  touchSensor.configureBrightness(MIN_BRIGHTNESS, MAX_BRIGHTNESS,
                                  BRIGHTNESS_STEP);

  // Initialize the LED strip
  strip.begin();

  // Initialize shake sensor
  shake.begin();
}

void loop() {
  // First, check if a special effect is already running
  if (strip.isEffectRunning()) {
    // Let the LED controller handle the effect animation
    strip.updateSpecialEffect();

    // Check if the effect should end
    if (millis() - effectStartTime > EFFECT_DURATION) {
      strip.stopSpecialEffect();
      Serial.println("Special effect complete");
    }

    // Skip other controls while effect is running
    return;
  }

  // Normal control flow when no special effect is running
  uint16_t rawValue =
      touchSensor.measure(sensePin, refPin); // Corrected: Pass both pins
  bool currentlyTouched = touchSensor.isTouched();

  // Process different types of touches
  if (touchSensor.isShortPress()) {
    Serial.println("Short press - LED toggled");
    strip.toggle();
  }

  // Handle long press (adjust brightness)
  if (touchSensor.isLongPress()) {
    Serial.println("Long press detected - changing brightness");
    uint8_t newBrightness = touchSensor.adjustBrightness();
    Serial.print("Setting brightness to: ");
    Serial.println(newBrightness);
    strip.setBrightness(newBrightness);
    longPressActive = true;
  }

  // Update longPressActive state based on touch state
  if (!currentlyTouched) {
    longPressActive = false;
  }

  // Check for shake
  if (shake.detectShake()) {
    Serial.println("Shake detected!");
    Serial.print("longPressActive status: ");
    Serial.println(longPressActive ? "true" : "false");

    // Check for combined long-press + shake gesture
    if (longPressActive) {
      Serial.println("Special effect triggered! (Long press + shake)");
      // Start the special effect
      strip.startSpecialEffect();
      effectStartTime = millis();
    }
  }

  // Short delay for stability
  delay(10);
}