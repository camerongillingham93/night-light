#include <CapacitiveSensor.h>

CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2); // 1M resistor between pins 4 & 2, pin 2 is sensor pin

const long pressThreshold = 300; // Threshold for considering a press
const unsigned long longPressDuration = 1000; // Duration threshold for a long press (in milliseconds)
const unsigned long debounceTime = 50; // Debounce time (in milliseconds)

bool buttonState = false; // Current state of the button
bool lastButtonState = false; // Previous state of the button
unsigned long buttonPressStartTime = 0; // Time when the button press started
bool longPressDetected = false; // Flag to indicate if a long press has been detected

void setup() {
    cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off auto-calibrate
    Serial.begin(9600);
}

void loop() {
    long sensorValue = cs_4_2.capacitiveSensor(10);

    if (sensorValue > pressThreshold) {
        if (!buttonState && (millis() - buttonPressStartTime > debounceTime)) {
            buttonState = true;
            buttonPressStartTime = millis();
            longPressDetected = false; // Reset the long press flag when a new press starts
        }

        if (buttonState && !longPressDetected && (millis() - buttonPressStartTime >= longPressDuration)) {
            Serial.print("Long Press Detected, Sensor Value: ");
            Serial.println(sensorValue);
            longPressDetected = true; // Set the flag to indicate that a long press has been detected
        }
    } else {
        if (buttonState) {
            buttonState = false;
            unsigned long pressDuration = millis() - buttonPressStartTime;
            if (pressDuration < longPressDuration) {
                Serial.print("Short Press Detected, Sensor Value: ");
                Serial.println(sensorValue);
            }
            buttonPressStartTime = millis(); // Reset the start time for the next press
        }
    }

    lastButtonState = buttonState;
    delay(10); // Short delay to avoid flooding the serial port
}
