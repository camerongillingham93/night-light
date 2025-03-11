#ifndef CAP_TOUCH_H
#define CAP_TOUCH_H

#include <Arduino.h>

class CapTouch {
public:
  CapTouch(uint8_t sensePin, uint8_t referencePin, uint16_t threshold,
           uint8_t debounceCount = 3);
  void begin();
  uint16_t measure();    // Take measurement and update touch state
  uint16_t measureRaw(); // Just raw measurement without state change
  bool isTouched();
  uint16_t getRawValue();
  void setThreshold(uint16_t threshold);

  // Press detection methods
  bool isShortPress();
  bool isLongPress();
  void setLongPressTime(unsigned long ms);

private:
  uint8_t _sensePin;
  uint8_t _referencePin;
  uint16_t _threshold;
  uint8_t _debounceLimit;
  bool _touchState;
  uint8_t _debounceCounter;
  uint16_t _lastMeasurement;

  // Press detection variables
  unsigned long _touchStartTime = 0;
  unsigned long _longPressThreshold = 1000; // 1 second default
  bool _longPressDetected = false;
  bool _shortPressDetected = false;
};

#endif