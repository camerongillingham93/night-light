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
#define NUM_PIXELS 10

// Brightness control
#define BRIGHTNESS_STEP 7
#define MIN_BRIGHTNESS 30
#define MAX_BRIGHTNESS 255

// Power management settings
#define POWER_CONTROL_PIN PIN_PB1
#define SLEEP_TIMEOUT_MS 30000         // 30s of inactivity before sleep
#define CRITICAL_BATTERY_THRESHOLD 2.7 // Volts
#define WAKEUP_BATTERY_THRESHOLD 2.9   // Volts (with hysteresis)
#define WAKEUP_TILT_DURATION 3000      // 3 seconds upside down to wake up

#endif

// **IMPORTANT: Double-check these pin numbers against your ATtiny1614 board's
// pinout!**
// sensePin: PA6
// refPin:   PA5
// tiltSW:   PA7
// LED_PIN:  PB1