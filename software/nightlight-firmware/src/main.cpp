#include "CapTouch.h"
#include "LEDController.h"
#include "config.h"
#include <Arduino.h>

// Create sensor: sense pin, reference pin, threshold, debounce count
CapTouch touchSensor(sensePin, refPin, 100, 3);
LEDController strip(LED_PIN, NUM_PIXELS);

// Variables to track LED states
bool ledState = false;
uint8_t brightness = 128; // Medium brightness to start

void setup() {
  Serial.begin(9600);
  delay(100); // Short delay for serial to initialize

  // Initialize the touch sensor
  touchSensor.begin();
  strip.begin();

  // Set the long press detection time (milliseconds)
  touchSensor.setLongPressTime(800); // 0.8 seconds for long press

  // Set up LED pin if you're controlling one
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Capacitive Touch Test with Short/Long Press");
  Serial.println("-------------------------------------------");
  Serial.println("Short press: Toggle LED");
  Serial.println("Long press: Change brightness");
}

void loop() {
  // Take measurement and update all touch states
  uint16_t rawValue = touchSensor.measure();

  // Process different types of touches
  if (touchSensor.isShortPress()) {
    // Short press detected - toggle the LED
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);

    Serial.println("Short press - LED toggled");
    strip.toggle();
  }

  if (touchSensor.isLongPress()) {
    // Long press detected - change brightness
    brightness = (brightness + 64) % 256;

    // If using PWM pin for LED:
    // analogWrite(PWM_LED_PIN, brightness);

    Serial.print("Long press - Brightness: ");
    Serial.println(brightness);
  }

  // Display the current state periodically (not on every loop to avoid serial
  // flooding)
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > 500) {
    Serial.print("Raw: ");
    Serial.print(rawValue);
    Serial.print(" | Touched: ");
    Serial.print(touchSensor.isTouched() ? "YES" : "NO");
    Serial.print(" | LED: ");
    Serial.println(ledState ? "ON" : "OFF");

    lastDisplay = millis();
  }

  // Optional visual indicator of touch
  if (touchSensor.isTouched()) {
    // You could add a visual indication of being touched
  }

  delay(50); // 20Hz update rate - good balance of responsiveness and power
}