/**
 * Night Light Firmware V1 - Power Control Module Implementation
 * -----------------------------------------------------------
 * Implements power management features for battery-powered night light
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.0
 */

#include "powerControl.h"
#include "LEDController.h"
#include "config.h"
#include <Arduino.h>

// Flag used by interrupt handler to communicate with main code
volatile bool wakeFlag = false;

// ISR for pin change - keep it minimal
ISR(PORTA_PORT_vect) {
  // Just set the flag - don't do any processing in ISR
  wakeFlag = true;

  // Clear the interrupt flag
  PORTA.INTFLAGS = PORT_INT7_bm; // Assuming tiltSW is on PA7
}

PowerController::PowerController(uint8_t powerControlPin,
                                 LEDController &ledController)
    : _powerControlPin(powerControlPin), _ledController(ledController),
      _lastActivityTime(0),
      _inactivityTimeout(SLEEP_TIMEOUT_MS), // Default 5 minutes
      _sleepModeEnabled(true), _isInSleepMode(false), _lastBatteryVoltage(0.0),
      _criticalBatteryThreshold(
          CRITICAL_BATTERY_THRESHOLD), // V - shutdown threshold
      _wakeupBatteryThreshold(
          WAKEUP_BATTERY_THRESHOLD), // V - safe to wake up threshold
      _batteryLow(false), _upsideDownDetected(false), _upsideDownStartTime(0) {}

void PowerController::begin() {
  // Initialize power control pin to active HIGH (keeping power on)
  pinMode(_powerControlPin, OUTPUT);
  digitalWrite(_powerControlPin, HIGH);

  // Initialize the last activity time
  _lastActivityTime = millis();

  // Prepare the tilt sensor interrupt
  pinMode(tiltSW, INPUT_PULLUP);
}

void PowerController::update() {
  // Skip processing if sleep mode is disabled
  if (!_sleepModeEnabled)
    return;

  // Get current time
  unsigned long currentTime = millis();

  // Check if we should enter sleep mode due to inactivity
  if (!_isInSleepMode &&
      (currentTime - _lastActivityTime) > _inactivityTimeout &&
      !_ledController.ledsOn) {
    enterSleepMode();
  }
}

void PowerController::enterSleepMode() {
  // Don't enter sleep if we're already in sleep mode
  if (_isInSleepMode)
    return;

  _isInSleepMode = true;

  // Configure wake-up interrupt
  configureInterrupts(true);

  // Prepare for sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Enter sleep mode
  sleep_enable();
  sei();       // Ensure interrupts are enabled
  sleep_cpu(); // Enter sleep mode

  // The following code runs after wake-up
  sleep_disable();

  // Manually check wake-up condition and process if necessary
  if (wakeFlag) {
    wakeUp();
    wakeFlag = false;
  }
}

void PowerController::wakeUp() {
  // Exit sleep mode
  _isInSleepMode = false;

  // Disable wake-up interrupt
  configureInterrupts(false);

  // Register activity
  registerActivity();

  // Trigger wake-up lighting effect
  // triggerWakeUpEffect();
  _ledController.wakeEffect();
}

bool PowerController::isInSleepMode() const { return _isInSleepMode; }

void PowerController::registerActivity() { _lastActivityTime = millis(); }

void PowerController::setInactivityTimeout(unsigned long milliseconds) {
  _inactivityTimeout = milliseconds;
}

unsigned long PowerController::getTimeSinceLastActivity() const {
  return millis() - _lastActivityTime;
}

void PowerController::updateBatteryStatus(float voltage) {
  _lastBatteryVoltage = voltage;

  // Check for critical battery level
  if (voltage <= _criticalBatteryThreshold && !_batteryLow) {
    _batteryLow = true;
    shutdownPower();
  }
  // Add hysteresis - we need significantly more voltage to wake back up
  else if (voltage >= _wakeupBatteryThreshold && _batteryLow) {
    _batteryLow = false;
  }
}

bool PowerController::isBatteryCritical() const { return _batteryLow; }

void PowerController::shutdownPower() {
  // Pull power control pin LOW to turn off MOSFET
  digitalWrite(_powerControlPin, LOW);

  // Note: After this, the device should be powered off
  // It will only turn back on when external power is applied
}

bool PowerController::checkForWakeUpTilt() {
  // Check if device is upside down (tilt switch open)
  bool isUpsideDown = (digitalRead(tiltSW) == HIGH);

  // Start or continue timing upside down period
  if (isUpsideDown) {
    if (!_upsideDownDetected) {
      // First detection of upside down
      _upsideDownDetected = true;
      _upsideDownStartTime = millis();
    } else {
      // Check if we've been upside down long enough to trigger wake-up
      if ((millis() - _upsideDownStartTime) >= 3000) { // 3 seconds
        _upsideDownDetected = false;

        // Wake up if we're in sleep mode
        if (_isInSleepMode) {
          wakeUp();
          return true;
        }
      }
    }
  } else {
    // Reset upside down detection if right side up
    _upsideDownDetected = false;
  }

  return false;
}

void PowerController::enableSleepMode(bool enable) {
  _sleepModeEnabled = enable;

  // If disabling while in sleep mode, wake up
  if (!enable && _isInSleepMode) {
    wakeUp();
  }
}

void PowerController::configureInterrupts(bool enable) {
  if (enable) {
    // Configure pin change interrupt for tilt sensor
    // We're assuming tiltSW is on PA7

    // Enable pin change interrupt on tiltSW
    PORTA.PIN7CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;

    // Clear any pending interrupts
    PORTA.INTFLAGS = PORT_INT7_bm;
  } else {
    // Disable pin change interrupt
    PORTA.PIN7CTRL = PORT_ISC_INTDISABLE_gc | PORT_PULLUPEN_bm;
  }
}
