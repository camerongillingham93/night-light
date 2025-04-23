/**
 * Night Light Firmware V1 - Power Control Module
 * ---------------------------------------------
 * Handles power management for battery-powered night light:
 * - Sleep mode entry and exit
 * - Wake-up detection using tilt sensor
 * - Low battery auto-shutdown control
 * - Inactivity timer management
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.0
 */


#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H

#include "config.h"
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "LEDController.h"

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

  //Acces to LEDs
  LEDController &_ledController;

  // Private methods
  void configureInterrupts(bool enable);
  static void pinChangeISR(); // Static because it's used in an interrupt

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