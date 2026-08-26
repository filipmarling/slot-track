
#include <LedControl.h> // Include the LedControl library

// Debug verbosity level (0: none, 1: basic, 2: detailed)
const int VERBOSITY = 1;

// Define the CarControl class
class CarControl {
public:
    CarControl(int pwmA, int pwmB, int in1A, int in2A, int in1B, int in2B, int controllerA, int controllerB);
    void setSpeed(int car, int speed, bool forward);
    void setControllerMode(bool enabled);
    void update();

private:
    int pwmA, pwmB;
    int in1A, in2A;
    int in1B, in2B;
    int controllerA, controllerB;
    bool controllerMode;
    int speedA, speedB;
    bool forwardA, forwardB;
};

// Constructor
CarControl::CarControl(int pwmA, int pwmB, int in1A, int in2A, int in1B, int in2B, int controllerA, int controllerB)
    : pwmA(pwmA), pwmB(pwmB), in1A(in1A), in2A(in2A), in1B(in1B), in2B(in2B),
      controllerA(controllerA), controllerB(controllerB), controllerMode(false),
      speedA(0), speedB(0), forwardA(true), forwardB(true) {
    pinMode(pwmA, OUTPUT);
    pinMode(pwmB, OUTPUT);
    pinMode(in1A, OUTPUT);
    pinMode(in2A, OUTPUT);
    pinMode(in1B, OUTPUT);
    pinMode(in2B, OUTPUT);
    pinMode(controllerA, INPUT);
    pinMode(controllerB, INPUT);
}

// Set the speed of the car
void CarControl::setSpeed(int car, int speed, bool forward) {
    if (car == 0) {
        speedA = map(speed, 0, 100, 0, 255);
        forwardA = forward;
    } else {
        speedB = map(speed, 0, 100, 0, 255);
        forwardB = forward;
    }
}

// Enable or disable controller mode
void CarControl::setControllerMode(bool enabled) {
    controllerMode = enabled;
}

// Update the car control logic
void CarControl::update() {
    if (controllerMode) {
        int rawA = analogRead(controllerA);
        int rawB = analogRead(controllerB);
        
        if (VERBOSITY >= 2) {
            // Print raw analog readings
            Serial.print("Raw A: ");
            Serial.print(rawA);
            Serial.print(" | Raw B: ");
            Serial.print(rawB);
            
            // Print mapping process
            Serial.print(" | Mapped A: ");
            Serial.print(map(rawA, 500, 900, 0, 255));
            Serial.print(" | Mapped B: ");
            Serial.print(map(rawB, 500, 900, 0, 255));
        }
        
        speedA = constrain(map(rawA, 500, 900, 0, 255), 0, 255);
        speedB = constrain(map(rawB, 500, 900, 0, 255), 0, 255);
    }
    digitalWrite(in1A, forwardA ? HIGH : LOW);
    digitalWrite(in2A, forwardA ? LOW : HIGH);
    analogWrite(pwmA, speedA);

    digitalWrite(in1B, forwardB ? LOW : HIGH);
    digitalWrite(in2B, forwardB ? HIGH : LOW);
    analogWrite(pwmB, speedB);

    if (VERBOSITY >= 2) {
        // Print final car control information
        Serial.print(" | Final A: ");
        Serial.print(speedA);
        Serial.print(" (");
        Serial.print(forwardA ? "Forward" : "Reverse");
        Serial.print(") | Final B: ");
        Serial.print(speedB);
        Serial.print(" (");
        Serial.print(forwardB ? "Forward" : "Reverse");
        Serial.println(")");
    }
}

// Define the Display class
class Display {
public:
    Display(int dataPin, int clkPin, int csPin);
    void displayTime(float time, int car);

private:
    LedControl lc; // LedControl object
};

// Constructor
Display::Display(int dataPin, int clkPin, int csPin) : lc(dataPin, clkPin, csPin, 1) {
    lc.shutdown(0, false);  // Wake up MAX7219
    lc.setIntensity(0, 8);  // Set brightness level (0-15)
    lc.clearDisplay(0);     // Clear display
}

// Display time on the 7-segment display
void Display::displayTime(float time, int car) {
    // Serial.println(time);

    // Split the time into integer and decimal parts
    int intPart = (int)time;
    int decimalPart = (int)((time - intPart) * 100);

    // Extract digits for 7-segment display
    int offset = car * 4;
    if ((intPart / 10) % 10) {
        lc.setDigit(0, offset + 3, (intPart / 10) % 10, false);  // Tens of seconds
    } else {
        lc.setChar(0, offset + 3, ' ', false);
    }

    lc.setDigit(0, offset + 2, intPart % 10, true);             // Seconds
    lc.setDigit(0, offset + 1, (decimalPart / 10) % 10, false); // Tenths of seconds
    lc.setDigit(0, offset + 0, decimalPart % 10, false);        // Hundredths of seconds
}

// Define the Timing class
class Timing {
public:
    Timing(int sensorPinA, int sensorPinB, int threshold);
    void update();
    bool isNewTrigger(int car);
    float getTime(int car);

private:
    int sensorPinA, sensorPinB, threshold;
    unsigned long lastTriggerA, lastTriggerB;
    unsigned long hysteresisStartA, hysteresisStartB;
    bool triggeredA, triggeredB;
    float elapsedTimeA, elapsedTimeB;
};

// Constructor
Timing::Timing(int sensorPinA, int sensorPinB, int threshold)
    : sensorPinA(sensorPinA), sensorPinB(sensorPinB), threshold(threshold),
      lastTriggerA(0), lastTriggerB(0), hysteresisStartA(0), hysteresisStartB(0),
      triggeredA(false), triggeredB(false), elapsedTimeA(0), elapsedTimeB(0) {
    pinMode(sensorPinA, INPUT);
    pinMode(sensorPinB, INPUT);
}

// Update method to handle sensor readings and calculate elapsed time
void Timing::update() {
    unsigned long currentMillis = millis();
    int sensorValueA = analogRead(sensorPinA);
    int sensorValueB = analogRead(sensorPinB);

    
    // Car A timing logic
    if (sensorValueA > threshold && !triggeredA) {
        triggeredA = true;
        hysteresisStartA = currentMillis;
        unsigned long timeSinceLastTrigger = currentMillis - lastTriggerA;
        lastTriggerA = currentMillis;
        elapsedTimeA = timeSinceLastTrigger / 1000.0;
    } else if (sensorValueA <= threshold && currentMillis - hysteresisStartA >= 500) {
        triggeredA = false;
    }

    // Car B timing logic
    if (sensorValueB > threshold && !triggeredB) {
        triggeredB = true;
        hysteresisStartB = currentMillis;
        unsigned long timeSinceLastTrigger = currentMillis - lastTriggerB;
        lastTriggerB = currentMillis;
        elapsedTimeB = timeSinceLastTrigger / 1000.0;
    } else if (sensorValueB <= threshold && currentMillis - hysteresisStartB >= 500) {
        triggeredB = false;
    }
        
    if (VERBOSITY >= 1) {
        // Print sensor values and trigger status
        Serial.print("Sensor A: ");
        Serial.print(sensorValueA);
        Serial.print(" (");
        Serial.print(triggeredA ? "TRIGGERED" : "not triggered");
        Serial.println(")");
        // Serial.print(" | Sensor B: ");
        // Serial.print(sensorValueB);
        // Serial.print(" (");
        // Serial.print(triggeredB ? "TRIGGERED" : "not triggered");
        // Serial.println(")");
    }
}

// Check if a new trigger event occurred
bool Timing::isNewTrigger(int car) {
    if (car == 0) {
        return triggeredA;
    } else {
        return triggeredB;
    }
}

// Get the elapsed time for a specific car
float Timing::getTime(int car) {
    if (car == 0) {
        return elapsedTimeA;
    } else {
        return elapsedTimeB;
    }
}

// CarControl carControl(3, 4, 0, 1, 6, 7, 18, 19);
CarControl carControl(3, 4, 0, 1, 6, 7, 18, 19);
Display display(12, 11, 10);
Timing timing(14, 15, 800);
int i = 0;

// LED blink variables
unsigned long previousLedMillis = 0;
const long ledInterval = 500;  // interval at which to blink (milliseconds)
bool ledState = false;

void setup() {
    Serial.begin(9600);
    // Wait up to 2.5 seconds for Serial Monitor to connect, then proceed anyway
    unsigned long startWait = millis();
    while (!Serial && (millis() - startWait < 2500)) {
        ; // Non-blocking startup timeout
    }
    carControl.setControllerMode(true); // Enable controller mode
    pinMode(LED_BUILTIN, OUTPUT); // Initialize the built-in LED pin
    Serial.println("Setup done!");
}

void loop() {
    // Non-blocking LED blink
    unsigned long currentMillis = millis();
    if (currentMillis - previousLedMillis >= ledInterval) {
        previousLedMillis = currentMillis;
        ledState = !ledState;  // Toggle the LED state
        digitalWrite(LED_BUILTIN, ledState);
    }

    carControl.update(); // Update car control
    timing.update(); // Update timing

    if (timing.isNewTrigger(0)) {
        float timeA = timing.getTime(0);
        // Serial.println("Trigger A");
        display.displayTime(timeA, 1); // Display time for car A
    }

    if (timing.isNewTrigger(1)) {
        float timeB = timing.getTime(1);
        // Serial.println("Trigger B");
        display.displayTime(timeB, 0); // Display time for car B
    }
    delay(100);
}

