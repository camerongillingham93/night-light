#include <CapacitiveSensor.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define numPixels 15
#define FADE_DELAY 4 // Time delay for each step in the fade effect
#define FADE_STEPS 25 // Number of steps to reach full brightness

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, PIN, NEO_GRB + NEO_KHZ800);

// Capacitive sensor setup
CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2); // 1M resistor between pins 4 & 2, pin 2 is sensor pin

const long pressThreshold = 300; // Threshold for considering a press
const unsigned long longPressDuration = 1200; // Duration threshold for a long press (in milliseconds)
const unsigned long debounceTime = 50; // Debounce time (in milliseconds)
const unsigned long incrementInterval = 8; // Interval for incrementing longPressValue (in milliseconds)
const unsigned long fireflyDuration = 5000; // Duration for the firefly effect (in milliseconds)

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
int longPressValue = 150; // Variable to increase from 0 to 255
int incrementAmount = 2; // Amount to increment the long press value

// Function prototypes
void handleButtonPress();
void handleLongPress();
void handleTiltSwitch();
void updateNeoPixelBrightness();
void turnOffNeoPixel();
void fireflyLightShow();

void setup() {
    Serial.begin(9600);
    pinMode(tiltSwitchPin, INPUT_PULLUP); // Set tilt switch pin as input with internal pull-up resistor
    cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF); // Turn off auto-calibrate for capacitive sensor
    
    strip.begin(); // Initialize the NeoPixel strip
    strip.show(); // Initialize all pixels to 'off'
}

void loop() {
    handleButtonPress();
    handleLongPress();
    handleTiltSwitch();
    delay(10); // Short delay to avoid flooding the serial port
}

void handleButtonPress() {
    long sensorValue = cs_4_2.capacitiveSensor(10);

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
    } else {
        if (buttonState) {
            buttonState = false;
            unsigned long pressDuration = millis() - buttonPressStartTime;
            if (pressDuration < longPressDuration) {
                isOn = !isOn; // Toggle the ON/OFF state
                Serial.print("ON/OFF State: ");
                Serial.println(isOn ? "ON" : "OFF");
                
                // Toggle NeoPixel strip on/off
                if (isOn) {
                    // Turn on NeoPixel strip with current brightness
                    updateNeoPixelBrightness();
                } else {
                    // Turn off NeoPixel strip
                    turnOffNeoPixel();
                }
            }
            buttonPressStartTime = millis(); // Reset the start time for the next press
        }
        longPressDetected = false; // Reset the long press detection flag if button is released
    }
}

void handleLongPress() {
    if (longPressDetected) {
        unsigned long currentTime = millis();
        if (currentTime - lastIncrementTime >= incrementInterval) {
            longPressValue += incrementAmount; // Increase the value by incrementAmount

            if (longPressValue >= 255 || longPressValue <= 30) {
                incrementAmount = -incrementAmount; // Reverse the direction when reaching the limits
                longPressValue += incrementAmount; // Correct the value after reversing direction
            }

            Serial.print("Long Press Value: ");
            Serial.println(longPressValue); // Print the incremented long press value
            
            // Update NeoPixel brightness
            updateNeoPixelBrightness();
            lastIncrementTime = currentTime; // Update the time of the last increment
        }
    }
}

void handleTiltSwitch() {
    if (digitalRead(tiltSwitchPin) == LOW) {
        shakeCount++;
        if (shakeCount >= requiredShakes) {
            if (millis() - lastShakeTime > shakeDebounceTime) {
                shakeNumber++; // Increment the shake number
                Serial.print("Shake Detected #");
                Serial.println(shakeNumber);
                lastShakeTime = millis(); // Update the last shake time
                
                // Trigger event if short press or long press detected with a shake
                if (longPressDetected || (cs_4_2.capacitiveSensor(10) > pressThreshold && !longPressDetected)) {
                    Serial.println("Event Triggered");
                    
                    // Firefly light show
                    fireflyLightShow();
                }
            }
            shakeCount = 0; // Reset the shake count after detecting a shake
        }
    } else {
        shakeCount = 0; // Reset the shake count if no consecutive LOW readings
    }
}

void updateNeoPixelBrightness() {
    int brightness = map(longPressValue, 0, 255, 0, 255); // Map longPressValue to 0-255 range
    for (int i = 0; i < numPixels; i++) {
        // Set all pixels to a more refined golden color with varying brightness using HSV
        strip.setPixelColor(i, strip.ColorHSV(18000, 248, brightness)); // Warm yellow color
    }
    strip.show(); // Apply the brightness change
}

void turnOffNeoPixel() {
    for (int i = 0; i < numPixels; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show(); // Apply the change
}

void fireflyLightShow() {
    unsigned long eventStartTime = millis();
    while (millis() - eventStartTime < fireflyDuration) {
// Randomly select four different LEDs to light up
  int pixelIndices[4];
  for (int i = 0; i < 4; ++i) {
    pixelIndices[i] = random(numPixels);
    // Ensure all indices are unique
    for (int j = 0; j < i; ++j) {
      if (pixelIndices[i] == pixelIndices[j]) {
        i--; // Retry to ensure uniqueness
        break;
      }
    }
  }

  // Warm yellow hue value (approximately 18000), saturation of 250
  uint16_t hue = 18000;
  uint8_t saturation = 250;

  // Fade the LEDs on and off
  for (int step = 0; step <= FADE_STEPS; ++step) {
    int brightness = map(step, 0, FADE_STEPS, 0, 255);
    for (int i = 0; i < 4; ++i) {
      uint32_t color = strip.ColorHSV(hue, saturation, brightness);
      strip.setPixelColor(pixelIndices[i], color);
    }
    strip.show();
    delay(FADE_DELAY);
  }
  
  for (int step = FADE_STEPS; step >= 0; --step) {
    int brightness = map(step, 0, FADE_STEPS, 0, 255);
    for (int i = 0; i < 4; ++i) {
      uint32_t color = strip.ColorHSV(hue, saturation, brightness);
      strip.setPixelColor(pixelIndices[i], color);
    }
    strip.show();
    delay(FADE_DELAY);
  }

  // Wait for a random time before repeating the fade
  delay(random(100, 800));
    }
}
