const int tiltSwitchPin = 13; // Pin for the tilt switch
unsigned long lastShakeTime = 0; // Time of the last shake
const unsigned long shakeDebounceTime = 200; // Debounce time for shake detection

const int requiredShakes = 2; // Number of consecutive LOW readings required to detect a shake
int shakeCount = 0; // Counter for consecutive LOW readings
int shakeNumber = 0; // Counter for the total number of shakes detected

void setup() {
    Serial.begin(9600);
    pinMode(tiltSwitchPin, INPUT_PULLUP); // Set tilt switch pin as input with internal pull-up resistor
}

void loop() {
    if (digitalRead(tiltSwitchPin) == LOW) {
        shakeCount++;
        if (shakeCount >= requiredShakes) {
            if (millis() - lastShakeTime > shakeDebounceTime) {
                shakeNumber++; // Increment the shake number
                Serial.print("Shake Detected #");
                Serial.println(shakeNumber);
                lastShakeTime = millis(); // Update the last shake time
            }
            shakeCount = 0; // Reset the shake count
        }
    } else {
        shakeCount = 0; // Reset the shake count if no consecutive LOW readings
    }

    delay(10); // Short delay to avoid flooding the serial port
}
