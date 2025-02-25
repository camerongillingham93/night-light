#include "LEDController.h"
#include "config.h"
#include "touch.h"
#include <Arduino.h>

CapTouch touchSensor(PORTA, sensePin, refPin, ADC_MUXPOS_AIN6_gc,
                     ADC_MUXPOS_AIN7_gc); // PA6 as electrode, PA7 as reference
LEDController ledController(LED_PIN, NUM_PIXELS);

void setup() {
  Serial.begin(9600);
  delay(3000);
  Serial.write("HEY CAM I WORK");
  delay(3000);
  touchSensor.begin();
  ledController.begin();
  ledController.setBrightness(255);
}

void loop() {
  // if (touchSensor.isTouched(touchThreshold)) {
  //   ledController.toggle();
  //   delay(75); // Simple debounce
  // }
}
