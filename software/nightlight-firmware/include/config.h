#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions and constants

// Touch Sensor Pins and Threshold
#define sensePin 6
#define refPin 5
#define tiltSW 7
#define touchThreshold 80

// NeoPixel LED Strip Setup
<<<<<<< HEAD
<<<<<<< HEAD
#define LED_PIN 1
#define NUM_PIXELS 8

// Brightness control
#define BRIGHTNESS_STEP 7
#define MIN_BRIGHTNESS 30
#define MAX_BRIGHTNESS 255
=======
#define LED_PIN PIN_PA4
#define NUM_PIXELS 15

// Brightness control
=======
#define LED_PIN PIN_PA4
#define NUM_PIXELS 15

// Brightness control
>>>>>>> parent of ad9992a (messing around during testing)
#define BRIGHTNESS_STEP 7 // Amount to change brightness on each long press
#define MIN_BRIGHTNESS 30  // Minimum brightness (completely off is 0)
#define MAX_BRIGHTNESS 255 // Maximum brightness
>>>>>>> parent of ad9992a (messing around during testing)

#endif

// **IMPORTANT: Double-check these pin numbers against your ATtiny1614 board's
// pinout!**
// sensePin: PA6
// refPin:   PA5
// tiltSW:   PA7
// LED_PIN:  PB1