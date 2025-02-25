#ifndef TOUCH_H
#define TOUCH_H

#include <avr/io.h>
#include <util/delay.h>

class CapTouch {
public:
  CapTouch(PORT_t &port, uint8_t pin1, uint8_t pin2, ADC_MUXPOS_t adcChannel1,
           ADC_MUXPOS_t adcChannel2);
           
  void begin();
  uint16_t measure();
  bool isTouched(uint16_t threshold);

private:
  PORT_t &sensorPort;
  uint8_t sensorPin1, sensorPin2;
  ADC_MUXPOS_t adcMuxPos1, adcMuxPos2;
};

#endif

