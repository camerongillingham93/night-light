#include "CapTouch.h"
#include <Arduino.h>

CapTouch::CapTouch(uint8_t sensePin, uint8_t referencePin, uint16_t threshold,
                   uint8_t debounceCount) {
  _sensePin = sensePin;
  _referencePin = referencePin;
  _threshold = threshold;
  _debounceLimit = debounceCount;
  _touchState = false;
  _debounceCounter = 0;
  _lastMeasurement = 0;
}

void CapTouch::begin() {
  // Initial pin setup
  pinMode(_sensePin, INPUT);
  pinMode(_referencePin, INPUT); // Start in high-impedance state
}

uint16_t CapTouch::measureRaw() {
  // Discharge capacitance
  pinMode(_referencePin, OUTPUT);
  pinMode(_sensePin, OUTPUT);
  digitalWrite(_referencePin, LOW);
  digitalWrite(_sensePin, LOW);
  delayMicroseconds(5); // Short discharge time

  // Set up for measurement
  pinMode(_sensePin, INPUT);

  // Charge and measure
  digitalWrite(_referencePin, HIGH);

  // Optimized counting loop
  uint16_t count = 0;
  const uint16_t MAX_COUNT = 1000; // Cap the maximum to save time

  // Time how long it takes to reach HIGH
  while (digitalRead(_sensePin) == LOW && count < MAX_COUNT) {
    count++;
  }

  // Return pins to input state when not in use (saves power)
  digitalWrite(_referencePin, LOW);
  pinMode(_referencePin, INPUT);

  return count;
}

uint16_t CapTouch::measure() {
  uint16_t measurement = measureRaw();
  _lastMeasurement = measurement;

  bool previousTouchState = _touchState;

  // Update touch state with debouncing
  if (measurement > _threshold) {
    if (_debounceCounter < _debounceLimit) {
      _debounceCounter++;
    } else {
      if (!previousTouchState) {
        // Touch just started
        _touchStartTime = millis();
        _longPressDetected = false;
        _shortPressDetected = false;
      } else {
        // Continuing touch - check for long press
        if (!_longPressDetected &&
            (millis() - _touchStartTime > _longPressThreshold)) {
          _longPressDetected = true;
        }
      }
      _touchState = true;
    }
  } else {
    if (_debounceCounter > 0) {
      _debounceCounter--;
    } else {
      if (previousTouchState) {
        // Touch just ended - check for short press
        if (!_longPressDetected &&
            (millis() - _touchStartTime < _longPressThreshold)) {
          _shortPressDetected = true;
        }
      }
      _touchState = false;
    }
  }

  return measurement;
}

bool CapTouch::isTouched() { return _touchState; }

uint16_t CapTouch::getRawValue() { return _lastMeasurement; }

void CapTouch::setThreshold(uint16_t threshold) { _threshold = threshold; }

bool CapTouch::isShortPress() {
  if (_shortPressDetected) {
    _shortPressDetected = false; // Clear the flag once read
    return true;
  }
  return false;
}

bool CapTouch::isLongPress() {
  if (_longPressDetected) {
    _longPressDetected = false; // Clear the flag once read
    return true;
  }
  return false;
}

void CapTouch::setLongPressTime(unsigned long ms) { _longPressThreshold = ms; }