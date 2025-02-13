#include <CapacitiveSensor.h>

// Capacitive sensor setup
CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2); // 1M resistor between pins 4 & 2, pin 2 is sensor pin

const long pressThreshold = 300; // Threshold for considering a press
const unsigned long longPressDuration = 1000; // Duration threshold for a long press (in milliseconds)
const unsigned long debounceTime = 50; // Debounce time (in milliseconds)
const unsigned long incrementInterval = 100; // Interval for incrementing longPressValue (in milliseconds)

bool buttonState = false; // Current state of the button
bool lastButtonState = false; // Previous state of the button
unsigned long buttonPressStartTime = 0; // Time when the button press started
bool longPressDetected = false; // Flag to indicate if a long press has been detected
unsigned long lastIncrementTime = 0; // Time of the last increment

// Tilt switch setup
const int tiltSwitchPin = 13; // Pin for the tilt switch
unsigned long lastShakeTime = 0; // Time of the last shake
const unsigned long shakeDebounceTime = 200; // Debounce time for shake detection

const int requiredShakes = 2; // Number of consecutive LOW readings required to detect a shake
int shakeCount = 0; // Counter for consecutive LOW readings
int shakeNumber = 0; // Counter for the total number of shakes detected

// Variables for button state and long press mode
bool isOn = false; // ON/OFF state for the short press
int longPressValue = 0; // Variable to increase from 0 to 255
const int incrementAmount = 10; // Amount to increment the long press value

void setup() {
    Serial.begin(9600);
    pinMode(tiltSwitchPin, INPUT_PULLUP); // Set tilt switch pin as input with internal pull-up resistor
    cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF); // Turn off auto-calibrate for capacitive sensor
}

void loop() {
    // Capacitive sensor reading
    long sensorValue = cs_4_2.capacitiveSensor(10);

    // Check for button press and long press detection
    if (sensorValue > pressThreshold) {
        if (!buttonState && (millis() - buttonPressStartTime > debounceTime)) {
            buttonState = true;
            buttonPressStartTime = millis();
            longPressDetected = false; // Reset the long press flag when a new press starts
            Serial.println("Button Press Detected");
        }

        if (buttonState && !longPressDetected && (millis() - buttonPressStartTime >= longPressDuration)) {
            Serial.print("Long Press Detected, Sensor Value: ");
            Serial.println(sensorValue);
            longPressDetected = true; // Set the flag to indicate that a long press has been detected
            lastIncrementTime = millis(); // Initialize the last increment time
        }

        // Increment long press value while in long press mode
        if (longPressDetected) {
            unsigned long currentTime = millis();
            if (currentTime - lastIncrementTime >= incrementInterval) {
                longPressValue += incrementAmount; // Increase the value by incrementAmount
                if (longPressValue > 255) {
                    longPressValue = 0; // Reset the value if it surpasses 255
                }
                Serial.print("Long Press Value: ");
                Serial.println(longPressValue); // Print the incremented long press value
                lastIncrementTime = currentTime; // Update the time of the last increment
            }
        }
    } else {
        if (buttonState) {
            buttonState = false;
            unsigned long pressDuration = millis() - buttonPressStartTime;
            if (pressDuration < longPressDuration) {
                isOn = !isOn; // Toggle the ON/OFF state
                Serial.print("ON/OFF State: ");
                Serial.println(isOn ? "ON" : "OFF");
            }
            buttonPressStartTime = millis(); // Reset the start time for the next press
        }
        longPressDetected = false; // Reset the long press detection flag if button is released
    }

    // Tilt switch reading and shake detection
    if (digitalRead(tiltSwitchPin) == LOW) {
        shakeCount++;
        if (shakeCount >= requiredShakes) {
            if (millis() - lastShakeTime > shakeDebounceTime) {
                shakeNumber++; // Increment the shake number
                Serial.print("Shake Detected #");
                Serial.println(shakeNumber);
                lastShakeTime = millis(); // Update the last shake time
                
                // Trigger event if short press or long press detected with a shake
                if (longPressDetected || (sensorValue > pressThreshold && !longPressDetected)) {
                    Serial.println("Event Triggered");
                }
            }
            shakeCount = 0; // Reset the shake count after detecting a shake
        }
    } else {
        shakeCount = 0; // Reset the shake count if no consecutive LOW readings
    }

    delay(10); // Short delay to avoid flooding the serial port
}

