#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions and constants

// Touch Sensor Pins and Threshold
#define sensePin PIN_PA5
#define refPin PIN_PA6
#define tiltSW PIN_PB0
#define touchThreshold 5

#define battMeasure PIN_PA4

// NeoPixel LED Strip Setup
#define LED_PIN PIN_PB1
#define NUM_PIXELS 10

// Brightness control
#define BRIGHTNESS_STEP 7
#define MIN_BRIGHTNESS 30
#define MAX_BRIGHTNESS 255

// Power management settings
#define POWER_CONTROL_PIN PIN_PA7
#define SLEEP_TIMEOUT_MS 180000        // 3 minutes of inactivity before sleep
#define CRITICAL_BATTERY_THRESHOLD 2.8 // Volts - hard shutdown
#define WAKEUP_BATTERY_THRESHOLD 2.9   // Volts - can wake up again (hysteresis)
#define WAKEUP_TILT_DURATION 3000      // 3 seconds upside down to wake up

#endif

// **IMPORTANT: Double-check these pin numbers against your ATtiny1614 board's
// pinout!**
// sensePin: PA5
// refPin:   PA6
// tiltSW:   PB0
// battMeasure: PA4
// LED_PIN:  PB1
// POWER_CONTROL_PIN: PA7