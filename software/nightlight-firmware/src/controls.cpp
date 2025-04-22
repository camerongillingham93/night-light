
/**
 * Night Light Firmware V1 - Controls Module
 * -----------------------------------------
 * Implements sensor controls for the night light:
 * - TouchSensor: Capacitive touch sensing with debounce, long-press detection,
 *   and brightness adjustment via triangle wave pattern
 * - Shake: Tilt switch detection with configurable sensitivity for triggering
 *   special effects
 *
 * @authors Cameron Gillingham, Claude AI
 * @version 1.0
 */

#include "controls.h"
#include <Arduino.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TOUCH CLASS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TouchSensor::TouchSensor(uint8_t pinA, uint8_t pinB, uint16_t threshold,
                         uint8_t samples) {
  _pinA = pinA;
  _pinB = pinB;
  _threshold = threshold;
  _samples = samples;
  _isCalibrated = false;
  _baseline = 0;
  _lastReading = 0;

  // Initialize long press detection variables
  _touchStartTime = 0;
  _isLongPressActive = false;
  _wasLongPress = false;
  _touchActive = false;
  _longPressDuration = 2000; // Default to 2 seconds for long press

  // Initialize brightness variables
  _currentBrightness = 255; // Start at full brightness
  _isBrightnessIncreasing = true;
  _lastBrightnessUpdateTime = 0;
}

void TouchSensor::begin() {
  // Initialize pins but don't set modes yet as they will change during sensing
  pinMode(_pinA, INPUT); // Initially set as input (high impedance)
  pinMode(_pinB, INPUT); // Initially set as input (high impedance)
}

void TouchSensor::calibrate(uint8_t numSamples) {
  uint32_t sum = 0;

  // Take multiple readings and average them for the baseline
  for (uint8_t i = 0; i < numSamples; i++) {
    sum += getRawReading();
    delay(20); // Short delay between samples
  }

  _baseline = sum / numSamples;
  _isCalibrated = true;
}

uint16_t TouchSensor::getRawReading() {
  uint32_t sum = 0;

  // Take multiple readings and average them
  for (uint8_t i = 0; i < _samples; i++) {
    // Step 1: Discharge the capacitor
    pinMode(_pinA, OUTPUT);
    pinMode(_pinB, OUTPUT);
    digitalWrite(_pinA, LOW);
    digitalWrite(_pinB, LOW);
    delayMicroseconds(10);

    // Step 2: Charge through resistor and measure time
    pinMode(_pinB, INPUT);     // Set sensing pin to input (high impedance)
    digitalWrite(_pinA, HIGH); // Begin charging through resistor

    // Count how long it takes for the input pin to go high
    uint16_t count = 0;
    while (digitalRead(_pinB) == LOW && count < 500) {
      count++;
    }

    sum += count;
    delayMicroseconds(50); // Small delay between samples
  }

  _lastReading = sum / _samples;
  return _lastReading;
}

bool TouchSensor::isTouched() {
  if (!_isCalibrated) {
    calibrate(); // Auto-calibrate if not done yet
  }

  uint16_t reading = getRawReading();

  // Touch will typically increase capacitance, which means it takes longer to
  // charge So touched reading is usually higher than baseline
  return (reading > (_baseline + _threshold));
}

uint16_t TouchSensor::getLastReading() const { return _lastReading; }

uint16_t TouchSensor::getBaseline() const { return _baseline; }

void TouchSensor::setThreshold(uint16_t threshold) { _threshold = threshold; }

void TouchSensor::update() {
  bool currentlyTouched = isTouched();

  // If touch just started, record the time
  if (currentlyTouched && !_touchActive) {
    _touchStartTime = millis();
    _touchActive = true;
    _isLongPressActive = false;
    _wasLongPress = false;
  }

  // Check for long press if touch is active
  if (_touchActive && currentlyTouched) {
    if (!_isLongPressActive &&
        (millis() - _touchStartTime >= _longPressDuration)) {
      _isLongPressActive = true;
      // When entering long press mode, keep the last brightness
      _lastBrightnessUpdateTime = millis();
    }
  }

  // If touch ended
  if (!currentlyTouched && _touchActive) {
    _touchActive = false;
    _wasLongPress = _isLongPressActive;
    _isLongPressActive = false;
  }
}

bool TouchSensor::isLongPress() { return _isLongPressActive; }

bool TouchSensor::wasLongPress() const { return _wasLongPress; }

void TouchSensor::setLongPressDuration(unsigned long duration) {
  _longPressDuration = duration;
}

uint8_t TouchSensor::getTriangleWaveBrightness() {
  unsigned long currentTime = millis();

  // If this is the first time in long press mode, use the current brightness
  // and start moving from there. No need to reset to minimum brightness.
  if (!_isLongPressActive) {

    return _currentBrightness;
  }

  // Update brightness every 25ms for smooth transitions
  const unsigned long updateInterval = 35;
  if (currentTime - _lastBrightnessUpdateTime >= updateInterval) {
    _lastBrightnessUpdateTime = currentTime;

    // Save old brightness for comparison
    uint8_t oldBrightness = _currentBrightness;

    // Change brightness by steps at a time
    const uint8_t brightnessStep =
        5; // Adjust step size for faster/slower transition

    // Update brightness based on direction
    if (_isBrightnessIncreasing) {
      if (_currentBrightness < 220 - brightnessStep) {
        _currentBrightness += brightnessStep;
      } else {
        _currentBrightness = 220;
        _isBrightnessIncreasing = false; // Reverse direction at maximum
      }
    } else {
      if (_currentBrightness > 30 + brightnessStep) {
        _currentBrightness -= brightnessStep;
      } else {
        _currentBrightness = 30;        // Minimum brightness
        _isBrightnessIncreasing = true; // Reverse direction at minimum
      }
    }
  }

  return _currentBrightness;
}
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
  pinMode(_tiltSWPin, INPUT_PULLUP);             // Use internal pullup resistor
  _previousTiltState = !digitalRead(_tiltSWPin); // Initialize current state
}

bool Shake::detectShake() {
  unsigned long currentTime = millis();

  // Read the current tilt state
  bool currentTiltState = !digitalRead(_tiltSWPin); // NORMAL = 0, TILTED = 1

  // Debounce: Only consider a state change if it's been stable for some time
  const unsigned long debounceTime = 65; // 50ms debounce period

  // Check if the state is different than the previous state
  // AND enough time has passed since the last change
  if (currentTiltState != _previousTiltState &&
      (currentTime - _lastChangeTime) > debounceTime) {

    _lastChangeTime = currentTime;

    if (_shakeCount == 0) {
      // First state change, start timer
      _shakeStartTime = currentTime;
    }

    _shakeCount++;
    _previousTiltState = currentTiltState;
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