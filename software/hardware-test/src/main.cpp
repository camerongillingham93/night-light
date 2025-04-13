#include <Arduino.h>
// /*
// February 7 2025

// Sketch to test the hardware components of the nightLight PCB.

// Uses a modded USB-UART device to flash the AT-tiny 1614, Select Serial UPDI
// -SLOW as prgogrammer

// Pins must be labelled as "PIN_PA2"
// */

// #include <tinyNeoPixel.h>

// #define LED_DATA_PIN PIN_PA4 // Data pin for RGB strand
// #define CONTROL_PIN PIN_PA6
// #define SENSE_PIN PIN_PA5

// #define NUMPIXELS 15

// tinyNeoPixel pixels =
//     tinyNeoPixel(NUMPIXELS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

// int sensorThreshold = 700;
// bool toggle = 0;
// bool lastState = 0;

// uint16_t senseTouch();

// void setup() {
//   pixels.begin(); // This initializes the NeoPixel library.
//   Serial.begin(4800);
// }

// void loop() {
//   Serial.println("Hello CAM");
//   uint16_t sensorValue = senseTouch();
//   Serial.println(sensorValue);

//   bool currentState = (sensorValue < sensorThreshold);

//   if (currentState && !lastState) {
//     toggle = !toggle;
//     if (toggle) {
//       pixels.setPixelColor(
//           1, pixels.Color(0, 150, 0)); // Moderately bright green color.
//     } else {
//       pixels.clear();
//     }
//     pixels.show();
//   }

//   lastState = currentState;
// }

// uint16_t senseTouch() {
//   pinMode(CONTROL_PIN, OUTPUT);
//   digitalWrite(CONTROL_PIN, LOW); // Discharge through CONTROL_PIN

//   pinMode(SENSE_PIN, INPUT);
//   uint16_t count = 0;

//   // Charge the pad through the 1MΩ resistor
//   pinMode(CONTROL_PIN, INPUT); // Let it float
//   while (digitalRead(SENSE_PIN) == LOW) {
//     count++;
//     if (count > 1000)
//       break; // Prevent infinite loop
//   }

//   return count; // Higher count = more capacitance (finger detected)
// }
#define pinBatteryVoltage PIN_PA4 // ADC input pin (connected to divider midpoint)

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for Serial if necessary
}

void loop() {
  uint16_t rawADC = analogRead(pinBatteryVoltage);

  // ADC reference = 5V (regulated VDD)
  // Battery is divided in half, so multiply by 2 to recover real voltage
  float batteryVoltage = (rawADC / 1023.0) * 5.0 * 2.0;

  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.println(" V");

  delay(1000); // Read once per second
}
