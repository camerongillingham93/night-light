#include "touch.h"

void touch_init() {
    // Enable PTC
    PTC.CTRLA = PTC_ENABLE_bm;

    // Set resolution (8-bit or 16-bit)
    PTC.CTRLB = PTC_RESSEL_8BIT_gc;

    // Configure PTC channel (select pin to measure)
    PTC.CHANNEL = TOUCH_PIN_INDEX;

    // Adjust settings (gain, prescaler, etc.)
    PTC.GAIN = 3;  // Adjust based on sensitivity needs
}

uint16_t touch_read() {
    PTC.COMMAND = PTC_START_bm;  // Start touch measurement
    while (!(PTC.INTFLAGS & PTC_TRIG_bm));  // Wait for measurement to complete
    return PTC.DATA;  // Read the result
}
