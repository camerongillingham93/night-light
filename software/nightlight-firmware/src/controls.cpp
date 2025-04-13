#include "controls.h"
#include <Arduino.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TOUCH CLASS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CapTouch::CapTouch(uint8_t sensingPin, uint8_t referencePin, uint16_t threshold,
                   uint8_t debounceCount) {
  //_sensePin = sensingPin;  // Remove these assignments
  //_referencePin = referencePin;
  _touchThresholdHigh = threshold;
  _touchThresholdLow = threshold - 10;
  _debounceLimit = debounceCount;
  _touchState = false;
  _debounceCounter = 0;
  _lastMeasurement = 0;
  _longPressDetected = false;
  _shortPressDetected = false;

  _currentBrightness = 255;
  _brightnessIncreasing = false;
  _minBrightness = 10;
  _maxBrightness = 255;
  _brightnessStep = 10;

  _baseline = 0;
  _baselineInitialized = false;
}

void CapTouch::begin(uint8_t sensePin, uint8_t referencePin) { // Modified
  // Initial pin setup
  pinMode(sensePin, INPUT); // Use passed parameters
  pinMode(referencePin, INPUT);
  calibrateBaseline(sensePin, referencePin); // Use passed parameters
}

uint16_t CapTouch::measureChargeTime(uint8_t chargePin, uint8_t sensePin) {
  // Discharge
  pinMode(sensePin, OUTPUT);
  pinMode(chargePin, OUTPUT);
  digitalWrite(sensePin, LOW);
  digitalWrite(chargePin, LOW);
  delayMicroseconds(5);

  // Charge
  pinMode(chargePin, OUTPUT);
  digitalWrite(chargePin, HIGH);
  pinMode(sensePin, INPUT);

  uint16_t count = 0;
  const uint16_t MAX_COUNT = 2000;
  while (digitalRead(sensePin) == LOW && count < MAX_COUNT) {
    count++;
  }

  // Restore pin states
  digitalWrite(chargePin, LOW);
  pinMode(chargePin, INPUT);
  pinMode(sensePin, INPUT);
  pinMode(chargePin, INPUT);

  return count;
}

uint16_t CapTouch::measure(uint8_t sensePin, uint8_t referencePin) { // Modified
  uint16_t measurement1 = measureChargeTime(sensePin, referencePin); // Modified
  uint16_t measurement2 = measureChargeTime(referencePin, sensePin); // Modified
  uint16_t measurement = (measurement1 + measurement2) / 2;

  // ---[ Filtering ]---
  // Simple moving average (can be tuned)
  const uint8_t FILTER_SAMPLES = 4;
  static uint16_t history[FILTER_SAMPLES];
  static uint8_t historyIndex = 0;
  uint32_t sum = 0;

  history[historyIndex] = measurement;
  historyIndex = (historyIndex + 1) % FILTER_SAMPLES;

  for (uint8_t i = 0; i < FILTER_SAMPLES; i++) {
    sum += history[i];
  }
  measurement = sum / FILTER_SAMPLES;
  _lastMeasurement = measurement;

  // ---[ Dynamic Baseline Adjustment ]---
  if (!_baselineInitialized) {
    return measurement; // Or handle this differently
  }

  // Update baseline slowly (moving average)
  _baseline = (_baseline * 999 + measurement) / 1000;

  bool previousTouchState = _touchState;

  // ---[ Hysteresis and Debouncing ]---
  if (_touchState) {
    if (measurement < _baseline + _touchThresholdLow) {
      if (_debounceCounter > 0) {
        _debounceCounter--;
      } else {
        _touchState = false;
        // Touch ended
        if (!_longPressDetected &&
            (millis() - _touchStartTime < _longPressThreshold)) {
          _shortPressDetected = true;
        }
      }
    }
  } else {
    if (measurement > _baseline + _touchThresholdHigh) {
      if (_debounceCounter < _debounceLimit) {
        _debounceCounter++;
      } else {
        _touchState = true;
        _touchStartTime = millis();
        _longPressDetected = false;
        _shortPressDetected = false;
      }
    }
  }

  return measurement;
}

void CapTouch::calibrateBaseline(uint8_t sensePin,
                                 uint8_t referencePin) { // Modified
  const uint16_t BASELINE_SAMPLES = 100;
  uint32_t sum = 0;
  for (int i = 0; i < BASELINE_SAMPLES; i++) {
    sum += measure(sensePin, referencePin); // Modified
    delay(2);
  }
  _baseline = sum / BASELINE_SAMPLES;
  _baselineInitialized = true;
}

bool CapTouch::isTouched() { return _touchState; }

uint16_t CapTouch::getRawValue() { return _lastMeasurement; }

void CapTouch::setThreshold(uint16_t threshold) {
  _touchThresholdHigh = threshold;
  _touchThresholdLow = threshold - 10;
}

bool CapTouch::isShortPress() {
  if (_shortPressDetected) {
    _shortPressDetected = false;
    return true;
  }
  return false;
}

bool CapTouch::isLongPress() {
  if (_longPressDetected) {
    _longPressDetected = false;
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
  _currentBrightness = maxBrightness;
}

uint8_t CapTouch::adjustBrightness() {
  if (_currentBrightness <= _minBrightness) {
    _brightnessIncreasing = true;
  } else if (_currentBrightness >= _maxBrightness) {
    _brightnessIncreasing = false;
  }

  if (_brightnessIncreasing) {
    if (_currentBrightness <= _maxBrightness - _brightnessStep) {
      _currentBrightness += _brightnessStep;
    } else {
      _currentBrightness = _maxBrightness;
    }
  } else {
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

void Shake::begin() {
  pinMode(_tiltSWPin, INPUT_PULLUP);
} // Use internal pullup

bool Shake::detectShake() {
  bool tiltState = !digitalRead(_tiltSWPin); // NORMAL = 0, TILTED = 1
  unsigned long currentTime = millis();

  if (tiltState != _previousTiltState) {
    if (currentTime - _lastChangeTime > 50) { // Debounce
      _lastChangeTime = currentTime;

      if (_shakeCount == 0) // First state change, start timer
      {
        _shakeStartTime = currentTime;
      }
      _shakeCount++;
      _previousTiltState = tiltState;
    }
  }

  // Check if we've reached the shake threshold within the time window
  if (_shakeCount >= _minShakeCount) {
    _shakeCount = 0;
    return true;
  }

  // Check if we've exceeded the time window without enough changes
  if (_shakeCount > 0 && (currentTime - _shakeStartTime > _shakeTimeWindow)) {
    _shakeCount = 0;
  }

  return false;
}

void Shake::setShakeThreshold(uint8_t count) { _minShakeCount = count; }

void Shake::setShakeWindow(unsigned long milliseconds) {
  _shakeTimeWindow = milliseconds;
}