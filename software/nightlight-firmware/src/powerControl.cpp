/**
 * Night Light Firmware V1 - Power Control Module Implementation
 * -----------------------------------------------------------
 * Implements power management features for battery-powered night light
 * with periodic battery monitoring during sleep
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.2 - Fixed tilt wake-up conflicts with RTC
 */

#include "powerControl.h"
#include "LEDController.h"
#include "config.h"
#include <Arduino.h>

// Flags used by interrupt handlers to communicate with main code
volatile bool wakeFlag = false;
volatile bool rtcWakeFlag = false;

// ISR for pin change - FIXED: Now uses PORTB for tiltSW (PB0)
ISR(PORTB_PORT_vect) {
  // Just set the flag - don't do any processing in ISR
  wakeFlag = true;

  // Clear the interrupt flag for PB0
  PORTB.INTFLAGS = PORT_INT0_bm; // Clear PB0 interrupt flag
}

// ISR for RTC periodic interrupt
ISR(RTC_PIT_vect) {
  // Set flag for battery check
  rtcWakeFlag = true;

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
      _rtcWakeupCounter(0) {}

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

  Serial.println("Entering sleep mode...");
  Serial.flush(); // Ensure message is sent before sleep

  _isInSleepMode = true;
  _rtcWakeupCounter = 0; // Reset counter when entering sleep

  // Configure wake-up interrupt and RTC
  configureInterrupts(true);
  configureRTCWakeup(true);

  // Prepare for sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Sleep loop - stay in sleep until tilt wake-up or critical battery
  while (_isInSleepMode) {
    // Clear flags before sleep
    wakeFlag = false;
    rtcWakeFlag = false;

    Serial.println("Going to sleep...");
    Serial.flush();

    // Enter sleep mode
    sleep_enable();
    sei();       // Ensure interrupts are enabled
    sleep_cpu(); // Enter sleep mode
    sleep_disable();

    Serial.print("Woke up - wakeFlag: ");
    Serial.print(wakeFlag);
    Serial.print(", rtcWakeFlag: ");
    Serial.println(rtcWakeFlag);

    // Check which interrupt woke us up
    if (wakeFlag) {
      Serial.println("Tilt wake-up detected, checking validity...");
      // Tilt sensor wake-up - check if it's a valid wake-up tilt
      if (checkForWakeUpTilt()) {
        Serial.println("Valid tilt wake-up confirmed!");
        wakeUp();
        break;
      }
      Serial.println("Invalid tilt wake-up, continuing sleep...");
      // If not a valid wake-up tilt, continue sleeping
    }

    if (rtcWakeFlag) {
      Serial.println("RTC wake-up for battery check...");
      // RTC wake-up - check battery
      checkBatteryDuringSleep();
      // If battery is critical, we won't return from checkBatteryDuringSleep()
      // Continue sleeping if battery is okay
    }
  }

  // Disable RTC when exiting sleep
  configureRTCWakeup(false);
}

void PowerController::wakeUp() {
  Serial.println("Waking up from sleep mode...");

  // Exit sleep mode
  _isInSleepMode = false;

  // Disable wake-up interrupt and RTC
  configureInterrupts(false);
  configureRTCWakeup(false);

  // Register activity
  registerActivity();

  // Trigger wake-up lighting effect
  _ledController.wakeEffect();
}

void PowerController::checkBatteryDuringSleep() {
  _rtcWakeupCounter++;

  // Only check battery every 10 minutes (600 wake-ups at 1-second intervals)
  if (_rtcWakeupCounter >= 600) { // Check every 10 minutes
    _rtcWakeupCounter = 0;

    Serial.println("Performing battery check during sleep...");

    // Read battery voltage
    uint16_t rawADC = analogRead(battMeasure);
    float batteryVoltage = (rawADC / 1023.0) * 5 * 2;

    Serial.print("Sleep battery voltage: ");
    Serial.println(batteryVoltage);

    // Update battery status
    updateBatteryStatus(batteryVoltage);

    // If battery is critical, shutdown immediately
    if (isBatteryCritical()) {
      Serial.println("Critical battery during sleep, shutting down...");

      // Disable interrupts and RTC before shutdown
      configureInterrupts(false);
      configureRTCWakeup(false);

      // Shutdown - this function doesn't return
      shutdownPower();
    }
  } else {
    // Reduced frequency logging to avoid spam
    if (_rtcWakeupCounter % 60 == 0) { // Log every minute
      Serial.print("RTC wake-up ");
      Serial.print(_rtcWakeupCounter);
      Serial.println("/600");
    }
  }
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
  Serial.println("Shutting down power...");
  Serial.flush();

  // Pull power control pin LOW to turn off MOSFET
  digitalWrite(_powerControlPin, LOW);

  // Note: After this, the device should be powered off
  // It will only turn back on when external power is applied
}

bool PowerController::checkForWakeUpTilt() {
  // Only handle tilt wake-up in sleep mode
  if (!_isInSleepMode) {
    // Normal operation mode logic (unchanged)
    bool isUpsideDown = (digitalRead(tiltSW) == HIGH);

    if (isUpsideDown) {
      if (!_upsideDownDetected) {
        _upsideDownDetected = true;
        _upsideDownStartTime = millis();
      } else {
        if ((millis() - _upsideDownStartTime) >= WAKEUP_TILT_DURATION) {
          _upsideDownDetected = false;
          return true;
        }
      }
    } else {
      _upsideDownDetected = false;
    }
    return false;
  }

  // Sleep mode logic - simplified approach to avoid RTC conflicts
  // Check if we're upside down when we wake up
  if (digitalRead(tiltSW) != HIGH) {
    Serial.println("Not upside down, ignoring wake-up");
    return false; // Not upside down, ignore wake-up
  }

  Serial.println("Upside down detected, starting timing...");

  // Wait for the required duration while checking tilt state
  unsigned long startTime = millis();

  while (millis() - startTime < WAKEUP_TILT_DURATION) {
    // If tilt changes during timing, abort
    if (digitalRead(tiltSW) != HIGH) {
      Serial.println("Tilt changed during timing, aborting...");
      return false;
    }
    delay(50); // Small delay to prevent busy waiting
  }

  // Final check - still upside down?
  if (digitalRead(tiltSW) == HIGH) {
    Serial.println("Tilt held for required duration!");
    return true;
  }

  Serial.println("Final tilt check failed, aborting...");
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
    Serial.println("Enabling tilt interrupt...");

    // FIXED: Configure pin change interrupt for tilt sensor on PB0
    // Enable pin change interrupt on tiltSW (PB0)
    PORTB.PIN0CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;

    // Clear any pending interrupts
    PORTB.INTFLAGS = PORT_INT0_bm;

    Serial.print("Tilt switch initial state: ");
    Serial.println(digitalRead(tiltSW) ? "HIGH (upside down)" : "LOW (normal)");
  } else {
    Serial.println("Disabling tilt interrupt...");

    // Disable pin change interrupt
    PORTB.PIN0CTRL = PORT_ISC_INTDISABLE_gc | PORT_PULLUPEN_bm;
  }
}

void PowerController::configureRTCWakeup(bool enable) {
  if (enable) {
    Serial.println("Configuring RTC for 1-second wake-ups...");

    // Use internal 32kHz oscillator
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

    // Enable periodic interrupt
    RTC.PITINTCTRL = RTC_PI_bm;

    // Set period to 1 second (we'll count multiple wake-ups for battery check)
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;

    // Clear any pending interrupts
    RTC.PITINTFLAGS = RTC_PI_bm;
  } else {
    Serial.println("Disabling RTC wake-up...");

    // Disable RTC periodic interrupt
    RTC.PITCTRLA = 0;
    RTC.PITINTCTRL = 0;
  }
}