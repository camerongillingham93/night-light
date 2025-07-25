/**
 * Night Light Firmware V1 - Main Controller
 * -----------------------------------------
 * Main program flow controlling a touch-sensitive LED night light with:
 * - Touch sensor for on/off toggling and brightness adjustment (via long press)
 * - Shake detection for special "firefly" effects
 * - LED strip control for various effects
 * - Power saving with sleep mode after inactivity
 * - Hard power cutoff for battery protection (no warning system)
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.2
 */

#include "LEDController.h"
#include "config.h"
#include "controls.h"
#include "powerControl.h"
#include <Arduino.h>

// Create sensor: sense pin, reference pin, threshold, debounce count
TouchSensor touchSensor(sensePin, refPin, touchThreshold, 5);
LEDController strip(LED_PIN, NUM_PIXELS);
Shake shake(tiltSW);
PowerController powerControl(POWER_CONTROL_PIN, strip);

// Variables for touch detection
bool lastTouchState = false;
bool currentTouchState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10; // Debounce time in milliseconds

void setup() {
  Serial.begin(9600);
  delay(100); // Short delay for serial to initialize

  // Initialize power controller
  powerControl.begin();
  powerControl.setInactivityTimeout(SLEEP_TIMEOUT_MS);

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
  // Check if we're in sleep mode - skip normal processing if so
  if (powerControl.isInSleepMode()) {
    // In sleep mode, control is handed over to the power controller
    // and interrupts. Very little code here will execute until wake-up.
    powerControl.update();
    return;
  }

  // Read battery voltage and update power controller
  uint16_t rawADC = analogRead(battMeasure);
  float batteryVoltage = (rawADC / 1023.0) * 5 * 2;
  powerControl.updateBatteryStatus(batteryVoltage);

  // Skip all control processing if effect is running
  if (strip.isEffectRunning()) {
    delay(20);                       // Small delay for stability
    powerControl.registerActivity(); // Count effect runtime as activity
    return;
  }

  // Detect shake only when lights are ON and no effect is running
  if (strip.getState() && !strip.isEffectRunning() && shake.detectShake()) {
    strip.fireflyEffect();
    powerControl.registerActivity(); // Count effect trigger as activity
    return;
  }

  // Update touch sensor state for long press detection
  touchSensor.update();

  // Check if the electrode is touched
  bool isTouched = touchSensor.isTouched();

  // Any touch registers as activity
  if (isTouched) {
    powerControl.registerActivity();
  }

  // Check for long press and adjust brightness if active
  if (touchSensor.isLongPress()) {
    // Ensure lights are on when adjusting brightness
    if (!strip.getState()) {
      strip.setState(true);
    }

    // Get brightness value
    uint8_t wavebrightness = touchSensor.getTriangleWaveBrightness();

    // Update brightness
    strip.setBrightness(wavebrightness);
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
        powerControl.registerActivity();
      }
    }
  }

  // Update last touch state
  lastTouchState = isTouched;

  // Let power controller check for sleep conditions
  powerControl.update();

  delay(20); // Small delay for stability
}