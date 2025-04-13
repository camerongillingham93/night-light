#ifndef CONTROLS_H
#define CONTROLS_H

#include <Arduino.h>

// CapTouch Class
class CapTouch {
private:
  // uint8_t _sensePin;  // Remove these member variables
  // uint8_t _referencePin;
  uint16_t _touchThresholdHigh; // Hysteresis: Touch ON threshold
  uint16_t _touchThresholdLow;  // Hysteresis: Touch OFF threshold
  uint8_t _debounceLimit;
  uint8_t _debounceCounter;
  bool _touchState;
  uint16_t _lastMeasurement;

  // Gesture detection
  unsigned long _touchStartTime;
  unsigned long _longPressThreshold = 1000; // Default 1 second
  bool _longPressDetected;
  bool _shortPressDetected;

  // Brightness control
  uint8_t _currentBrightness;
  bool _brightnessIncreasing;
  uint8_t _minBrightness;
  uint8_t _maxBrightness;
  uint8_t _brightnessStep;

  // Filtering and baseline
  uint16_t _baseline;
  bool _baselineInitialized;

public:
  CapTouch(uint8_t sensingPin, uint8_t referencePin, uint16_t threshold,
           uint8_t debounceCount);
  void begin(uint8_t sensePin, uint8_t referencePin); // Modified
  uint16_t measureChargeTime(uint8_t chargePin, uint8_t sensePin);
  uint16_t measure(uint8_t sensePin, uint8_t referencePin); // Modified
  bool isTouched();
  uint16_t getRawValue();
  void setThreshold(uint16_t threshold);
  bool isShortPress();
  bool isLongPress();
  void setLongPressTime(unsigned long ms);

  // New brightness control methods
  void configureBrightness(uint8_t minBrightness, uint8_t maxBrightness,
                           uint8_t step);
  uint8_t adjustBrightness();
  uint8_t getCurrentBrightness();

  void calibrateBaseline(uint8_t sensePin, uint8_t referencePin); // Modified
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