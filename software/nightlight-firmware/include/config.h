// Pin definitions and constants

#ifndef CONFIG_H
#define CONFIG_H

// Touch Sensor Pins and Threshold
#define sensePin PIN_PA6
#define refPin PIN_PA5
#define tiltSW PIN_PA7
#define touchThreshold 80

// NeoPixel LED Strip Setup
#define LED_PIN PIN_PB1
#define NUM_PIXELS 8

// Brightness control
#define BRIGHTNESS_STEP 7  // Amount to change brightness on each long press
#define MIN_BRIGHTNESS 30  // Minimum brightness (completely off is 0)
#define MAX_BRIGHTNESS 255 // Maximum brightness

#endif
