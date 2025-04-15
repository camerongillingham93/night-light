#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions and constants

// Touch Sensor Pins and Threshold
#define sensePin PIN_PA5
#define refPin PIN_PA6
#define tiltSW PIN_PA7
#define touchThreshold 5

#define rstButton PIN_PB0
#define battMeasure PIN_PA4

// NeoPixel LED Strip Setup
#define LED_PIN PIN_PB1
#define NUM_PIXELS 12

// Brightness control
#define BRIGHTNESS_STEP 7
#define MIN_BRIGHTNESS 30
#define MAX_BRIGHTNESS 255

#endif

// **IMPORTANT: Double-check these pin numbers against your ATtiny1614 board's
// pinout!**
// sensePin: PA6
// refPin:   PA5
// tiltSW:   PA7
// LED_PIN:  PB1