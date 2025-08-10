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

extern LEDController strip;

// Flag used by interrupt handler to communicate with main code
volatile bool wakeFlag = false;

volatile uint16_t batteryCheckCounter = 0;
volatile bool rtcWakeFlag = false;
const uint16_t BATTERY_CHECK_INTERVAL =
    10; // Start with 10 seconds for testing, change to 600 later

ISR(PORTB_PORT_vect) { // Changed from PORTA_PORT_vect
  // Just set the flag - don't do any processing in ISR
  wakeFlag = true;

  // Clear the interrupt flag
  PORTB.INTFLAGS = PORT_INT0_bm; // Changed from PORTA and PORT_INT7_bm
}

ISR(RTC_PIT_vect) {
  batteryCheckCounter++;

  if (batteryCheckCounter >= BATTERY_CHECK_INTERVAL) {
    rtcWakeFlag = true;
    batteryCheckCounter = 0;
  }

  // Clear the interrupt flag
  RTC.PITINTFLAGS = RTC_PI_bm;
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
      _batteryLow(false), _upsideDownDetected(false), _upsideDownStartTime(0),
      _batteryCheckWakeUp(false) {}

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
  configureInterrupts(true);  // tilt sensor
  //configureBatteryCheckTimer(); // RTC timer

  // DEBUG: Flash yellow before going to sleep
  _ledController.setState(true);
  _ledController.setBrightness(255);
  _ledController.setColor(255, 255, 0); // Yellow - "Going to sleep"
  delay(500);
  _ledController.setState(false);
  delay(100);

  // Prepare for sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Enter sleep mode
  sleep_enable();
  sei();       // Ensure interrupts are enabled
  sleep_cpu(); // Enter sleep mode

  // The following code runs after wake-up
  sleep_disable();

  // Check what woke us up
  if (rtcWakeFlag) {
    rtcWakeFlag = false;
    handleBatteryCheckWakeUp();
    // Stay in sleep mode after battery check (continue while loop)
  }

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
  strip.wakeEffect();
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

void PowerController::handleBatteryCheckWakeUp() {
  // DEBUG: Flash blue LEDs to show battery check wake-up
  for (int i = 0; i < 2; i++) {
    _ledController.setColor(0, 0, 255); // Blue
    _ledController.setBrightness(100);
    _ledController.setState(true);
    delay(200);
    _ledController.setState(false);
    delay(200);
  }

  // Quick battery check (using the same code from your main loop)
  uint16_t rawADC = analogRead(battMeasure);
  float batteryVoltage = (rawADC / 1023.0) * 5 * 2;

  if (batteryVoltage <= _criticalBatteryThreshold) {
    // DEBUG: Flash red to show critical battery before shutdown
    for (int i = 0; i < 5; i++) {
      _ledController.setColor(0, 255, 0); // Red
      _ledController.setBrightness(255);
      _ledController.setState(true);
      delay(100);
      _ledController.setState(false);
      delay(100);
    }

    // Battery critical - shutdown immediately
    shutdownPower();
    return; // Device will be off after this
  }

  // Battery OK - the device will go back to sleep automatically
  // since _isInSleepMode is still true
}

void PowerController::configureInterrupts(bool enable) {
  if (enable) {
    // Configure pin change interrupt for tilt sensor on PB0
    PORTB.PIN0CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;

    // Clear any pending interrupts
    PORTB.INTFLAGS = PORT_INT0_bm;
  } else {
    // Disable pin change interrupt
    PORTB.PIN0CTRL = PORT_ISC_INTDISABLE_gc | PORT_PULLUPEN_bm;
  }
  }

  void PowerController::configureBatteryCheckTimer() {
    // Configure RTC to use internal 32kHz oscillator
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

    // Configure PIT (Periodic Interrupt Timer) for 1-second intervals
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;

    // Enable PIT interrupt
    RTC.PITINTCTRL = RTC_PI_bm;

    // Reset counter
    batteryCheckCounter = 0;
  }
