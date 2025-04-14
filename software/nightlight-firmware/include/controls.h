#ifndef CONTROLS_H
#define CONTROLS_H

#include <Arduino.h>
#include "config.h"

class TouchSensor {
private:
  uint8_t _pinA;         // Output pin (charging pin)
  uint8_t _pinB;         // Input pin (sensing pin)
  uint16_t _threshold;   // Touch detection threshold
  uint16_t _baseline;    // Baseline reading for no-touch state
  bool _isCalibrated;    // Flag to indicate if sensor is calibrated
  uint8_t _samples;      // Number of samples to average for reading
  uint16_t _lastReading; // Last raw reading

  // Long press detection variables
  unsigned long _touchStartTime;    // When touch began
  bool _isLongPressActive;          // Currently in a long press
  bool _wasLongPress;               // Previous touch was a long press
  bool _touchActive;                // Touch is currently active
  unsigned long _longPressDuration; // How long to consider a long press (ms)
  uint8_t _currentBrightness;
  bool _isBrightnessIncreasing;
  unsigned long _lastBrightnessUpdateTime;

public:
  // Constructor
  TouchSensor(uint8_t pinA, uint8_t pinB, uint16_t threshold = 50,
              uint8_t samples = 5);

  // Initialize the sensor
  void begin();

  // Calibrate the sensor (establish baseline)
  void calibrate(uint8_t numSamples = 10);

  // Get raw touch reading
  uint16_t getRawReading();

  // Check if touch is detected
  bool isTouched();

  // Get the last reading value
  uint16_t getLastReading() const;

  // Get the current baseline value
  uint16_t getBaseline() const;

  // Set a new threshold value
  void setThreshold(uint16_t threshold);

  // New methods for long press detection
  bool isLongPress();        // Returns true if current touch is a long press
  bool wasLongPress() const; // Returns true if previous touch was a long press
  void update(); // Update touch state, should be called in every loop iteration

  // Method to adjust brightness in a triangle wave pattern
  uint8_t getTriangleWaveBrightness();

  // Set long press duration
  void setLongPressDuration(unsigned long duration);
};

// Shake Class
class Shake {
private:
  uint8_t _tiltSWPin;

  // State tracking
  bool _previousTiltState;
  uint8_t _shakeCount;

  // Timing
  unsigned long _shakeStartTime;
  unsigned long _lastChangeTime;

  // Configuration
  uint8_t _minShakeCount;
  unsigned long _shakeTimeWindow;

public:
  Shake(uint8_t tiltSWpin);
  void begin();
  bool detectShake();
  void setShakeThreshold(uint8_t count);
  void setShakeWindow(unsigned long milliseconds);
};

#endif