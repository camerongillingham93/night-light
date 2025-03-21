#include "controls.h"
#include <Arduino.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TOUCH CLASS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CapTouch::CapTouch(uint8_t sensePin, uint8_t referencePin, uint16_t threshold,
                   uint8_t debounceCount) {
  _sensePin = sensePin;
  _referencePin = referencePin;
  _threshold = threshold;
  _debounceLimit = debounceCount;
  _touchState = false;
  _debounceCounter = 0;
  _lastMeasurement = 0;
  _longPressDetected = false;
  _shortPressDetected = false;

  // Default brightness settings
  _currentBrightness = 255;
  _brightnessIncreasing = false;
  _minBrightness = 10;
  _maxBrightness = 255;
  _brightnessStep = 10;
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

// New brightness control methods
void CapTouch::configureBrightness(uint8_t minBrightness, uint8_t maxBrightness,
                                   uint8_t step) {
  _minBrightness = minBrightness;
  _maxBrightness = maxBrightness;
  _brightnessStep = step;
  _currentBrightness = maxBrightness; // Start at max brightness
}

uint8_t CapTouch::adjustBrightness() {
  // Adjust direction at min/max values
  if (_currentBrightness <= _minBrightness) {
    _brightnessIncreasing = true;
  } else if (_currentBrightness >= _maxBrightness) {
    _brightnessIncreasing = false;
  }

  // Adjust brightness based on direction
  if (_brightnessIncreasing) {
    // Ensure we don't exceed MAX_BRIGHTNESS
    if (_currentBrightness <= _maxBrightness - _brightnessStep) {
      _currentBrightness += _brightnessStep;
    } else {
      _currentBrightness = _maxBrightness;
    }
  } else {
    // Ensure we don't go below MIN_BRIGHTNESS
    if (_currentBrightness >= _minBrightness + _brightnessStep) {
      _currentBrightness -= _brightnessStep;
    } else {
      _currentBrightness = _minBrightness;
    }
  }

  return _currentBrightness;
}

uint8_t CapTouch::getCurrentBrightness() { return _currentBrightness; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SHAKE CLASS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Shake::Shake(uint8_t tiltSWpin) {
  _tiltSWPin = tiltSWpin;

  // Initialize state tracking
  _previousTiltState = false;
  _shakeCount = 0;

  // Initialize timing variables
  _shakeStartTime = 0;
  _lastChangeTime = 0;

  // Default configuration
  _minShakeCount = 2;
  _shakeTimeWindow = 1200;
}

void Shake::begin() { pinMode(_tiltSWPin, INPUT); }

bool Shake::detectShake() {
  bool tiltState = !digitalRead(_tiltSWPin); // NORMAL = 0, TILTED = 1
  unsigned long currentTime = millis();

  if (tiltState != _previousTiltState) {
    _lastChangeTime = currentTime;

    if (_shakeCount == 0) // First state change, start timer
    {
      _shakeStartTime = currentTime;
    }
    _shakeCount++;
    _previousTiltState = tiltState;
  }

  // Check if we've reached the shake threshold within the time window
  if (_shakeCount >= _minShakeCount) {
    // Shake detected - reset counters and return true
    _shakeCount = 0;
    return true;
  }

  // Check if we've exceeded the time window without enough changes
  if (_shakeCount > 0 && (currentTime - _shakeStartTime > _shakeTimeWindow)) {
    // Time window expired without enough changes - reset counters
    _shakeCount = 0;
  }

  // No shake detected yet
  return false;
}

void Shake::setShakeThreshold(uint8_t count) { _minShakeCount = count; }

void Shake::setShakeWindow(unsigned long milliseconds) {
  _shakeTimeWindow = milliseconds;
}