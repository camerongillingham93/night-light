/**
 * Night Light Firmware V1 - Power Control Module
 * ---------------------------------------------
 * Handles power management for battery-powered night light:
 * - Sleep mode entry and exit
 * - Wake-up detection using tilt sensor
 * - Low battery auto-shutdown control
 * - Inactivity timer management
 * - Periodic battery monitoring during sleep via RTC
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.1
 */

#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H

#include "LEDController.h"
#include "config.h"
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

// Global flags for interrupt communication
extern volatile bool wakeFlag;
extern volatile bool rtcWakeFlag;

class PowerController {
private:
  // Power control pin for MOSFET that cuts power
  uint8_t _powerControlPin;

  // Sleep mode variables
  unsigned long _lastActivityTime;
  unsigned long _inactivityTimeout;
  bool _sleepModeEnabled;
  bool _isInSleepMode;

  // Battery management
  float _lastBatteryVoltage;
  float _criticalBatteryThreshold;
  float _wakeupBatteryThreshold;
  bool _batteryLow;

  // Wake-up tilt detection
  bool _upsideDownDetected;
  unsigned long _upsideDownStartTime;

  // RTC battery monitoring
  uint8_t _rtcWakeupCounter;
  static const uint8_t RTC_WAKEUPS_PER_BATTERY_CHECK =
      60; // 60 * 10sec = 10 minutes

  // Access to LEDs
  LEDController &_ledController;

  // Private methods
  void configureInterrupts(bool enable);
  void configureRTCWakeup(bool enable);
  void checkBatteryDuringSleep();

public:
  // Constructor
  PowerController(uint8_t powerControlPin, LEDController &ledController);

  // Initialize the power controller
  void begin();

  // Main functions
  void update();
  void enterSleepMode();
  void wakeUp();
  bool isInSleepMode() const;

  // Activity tracking
  void registerActivity();
  void setInactivityTimeout(unsigned long milliseconds);
  unsigned long getTimeSinceLastActivity() const;

  // Battery management
  void updateBatteryStatus(float voltage);
  bool isBatteryCritical() const;
  void shutdownPower();

  // Wake-up detection
  bool checkForWakeUpTilt();

  // Enable/disable sleep functionality
  void enableSleepMode(bool enable);
};

#endif