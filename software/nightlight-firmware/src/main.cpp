/**
 * Night Light Firmware V1 - Main Controller
 * -----------------------------------------
 * Main program flow controlling a touch-sensitive LED night light with:
 * - Touch sensor for on/off toggling and brightness adjustment (via long press)
 * - Shake detection for special "firefly" effects
 * - Low battery detection and warning
 * - LED strip control for various effects
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.0
 */

#include "LEDController.h"
#include "config.h"
#include "controls.h"
#include <Arduino.h>

// Create sensor: sense pin, reference pin, threshold, debounce count
TouchSensor touchSensor(sensePin, refPin, touchThreshold, 5);
LEDController strip(LED_PIN, NUM_PIXELS);
Shake shake(tiltSW);

// Variables for touch detection
bool lastTouchState = false;
bool currentTouchState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10; // Debounce time in milliseconds

// Flag to track if we're in low battery mode
bool inLowBatteryMode = false;

void setup() {
  Serial.begin(9600);
  delay(100); // Short delay for serial to initialize

  // Initialize touch sensor
  touchSensor.begin();

  // Initial calibration - hold for 1 second to get baseline
  delay(1000);
  touchSensor.calibrate(10);

  // Initialize the LED strip
  strip.begin();

  // Initialize shake sensor
  shake.begin();
  shake.setShakeThreshold(5);
  shake.setShakeWindow(900);
}

void loop() {
  uint16_t rawADC = analogRead(battMeasure);
  float batteryVoltage = (rawADC / 1023.0) * 5 * 2;

  // Skip all control processing if effect is running
  if (strip.isEffectRunning()) {
    delay(20); // Small delay for stability
    return;
  }

  // Detect shake only when lights are ON and no effect is running
  if (strip.getState() && !strip.isEffectRunning() && shake.detectShake()) {
    strip.fireflyEffect();
    // Return after effect is done to avoid processing other controls
    // immediately
    return;
  }

  // Update touch sensor state for long press detection
  touchSensor.update();

  // Check if the electrode is touched - declare outside if/else for scope
  bool isTouched = touchSensor.isTouched();

  // Check for long press and adjust brightness if active
  if (touchSensor.isLongPress() && batteryVoltage > 2.8) {

    // Ensure lights are on when adjusting brightness
    if (!strip.getState()) {
      // Serial.println("Turning LEDs ON for brightness adjustment");
      strip.setState(true);
    } else {
      // Serial.println("LEDs already ON");
    }

    // Get brightness value
    uint8_t wavebrightness = touchSensor.getTriangleWaveBrightness();

    // Update brightness
    strip.setBrightness(wavebrightness);
  }

  // Check battery voltage and set low battery mode flag
  if (batteryVoltage <= 2.8) {
    // If we just entered low battery mode, turn off the lights
    if (!inLowBatteryMode) {
      inLowBatteryMode = true;
      strip.setState(
          false); // Make sure lights are off when entering low battery mode
      strip.lowBattery();
    }

    // Debounce the touch reading
    if (isTouched != lastTouchState) {
      lastDebounceTime = millis();
    }

    // Only flash low battery warning if debounce time has passed
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (isTouched != currentTouchState) {
        currentTouchState = isTouched;
        if (isTouched == true) {
          strip.lowBattery(); // Just show low battery warning
          // Don't turn lights back on afterward
        }
      }
    }
  } else { // Normal operation (battery > 2.8V)
    // Clear low battery mode flag if we're returning from low battery state
    if (inLowBatteryMode) {
      inLowBatteryMode = false;
      // Keep lights off when coming out of low battery mode, until user turns
      // them on
    }

    // Debounce the touch reading
    if (isTouched != lastTouchState) {
      lastDebounceTime = millis();
    }

    // Only change LED state if debounce time has passed
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (isTouched != currentTouchState) {
        currentTouchState = isTouched;

        // Only toggle if touch ended (released) AND it was NOT a long press
        if (isTouched == false && touchSensor.wasLongPress() == false) {
          strip.toggle();
        }
      }
    }
  }

  // Update last touch state
  lastTouchState = isTouched;

  delay(20); // Small delay for stability
}
