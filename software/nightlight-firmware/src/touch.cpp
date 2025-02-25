#include "touch.h"

CapTouch::CapTouch(PORT_t &port, uint8_t pin1, uint8_t pin2,
                   ADC_MUXPOS_t adcChannel1, ADC_MUXPOS_t adcChannel2)
    : sensorPort(port), sensorPin1(pin1), sensorPin2(pin2),
      adcMuxPos1(adcChannel1), adcMuxPos2(adcChannel2) {}

void CapTouch::begin() {
  // Set ADC reference voltage
  VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc; // Use 1.1V reference

  // Configure ADC
  ADC0.CTRLA = ADC_RESSEL_10BIT_gc; // 10-bit resolution
  ADC0.CTRLB = 0;                   // Normal mode
  ADC0.CTRLC = ADC_PRESC_DIV4_gc;   // Set prescaler

  ADC0.MUXPOS = adcMuxPos1;
  ADC0.CTRLA |= ADC_ENABLE_bm; // Enable ADC
}

  uint16_t CapTouch::measure() {
    // Charge the electrode via sense pin
    sensorPort.DIRSET = (1 << sensorPin1); // Set pin1 (electrode) as output
    sensorPort.OUTSET = (1 << sensorPin1); // Drive it HIGH
    _delay_us(5);                          // Allow charge to build

    // Set both pins to input (high impedance)
    sensorPort.DIRCLR = (1 << sensorPin1); // Set pin1 as input
    sensorPort.DIRCLR = (1 << sensorPin2); // Set pin2 as input (reference)

    // Perform dummy read to allow ADC mux to settle
    ADC0.MUXPOS = adcMuxPos1;
    ADC0.COMMAND = ADC_STCONV_bm;
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
      ;
    (void)ADC0.RES; // Discard dummy read
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Measure electrode charge (sense pin)
    ADC0.COMMAND = ADC_STCONV_bm;
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
      ;
    uint16_t result1 = ADC0.RES;
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Measure reference pin
    ADC0.MUXPOS = adcMuxPos2;
    ADC0.COMMAND = ADC_STCONV_bm;
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
      ;
    uint16_t result2 = ADC0.RES;
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Return differential measurement (sense - reference)
    return result1 - result2;
  }

bool CapTouch::isTouched(uint16_t threshold) { return measure() > threshold; }
