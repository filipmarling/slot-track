#include <LedControl.h>

// Define LED control (assumes dataPin=12, clkPin=11, csPin=10)
LedControl lc = LedControl(12, 11, 10, 1);

const int threshold = 700;  // Threshold value
unsigned long lastTrigger = 0;  // Last time the threshold was triggered
unsigned long hysteresisStart = 0;
bool triggered = false;

void setup() {
  lc.shutdown(0, false);  // Wake up MAX7219
  lc.setIntensity(0, 8);  // Set brightness level (0-15)
  lc.clearDisplay(0);     // Clear display
  delay(1000);
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  int sensorValue = analogRead(14);  // Read the analog value
  //Serial.println(sensorValue);
  
  unsigned long currentMillis = millis();
  
  // Hysteresis check
  if (sensorValue > threshold && !triggered) {
    Serial.println("Triggered");
    triggered = true;
    hysteresisStart = currentMillis;
    unsigned long timeSinceLastTrigger = currentMillis - lastTrigger;
    lastTrigger = currentMillis;  // Update last trigger time
    
    // Convert to seconds and format
    float elapsedSeconds = timeSinceLastTrigger / 1000.0;
    displayTime(elapsedSeconds);
  } 
  else if (sensorValue <= threshold && currentMillis - hysteresisStart >= 1000) {
    triggered = false;  // Reset triggered state after hysteresis
  }

}

void displayTime(float time) {
  Serial.println(time);
  // Split the time into integer and decimal parts
  int intPart = (int)time;
  int decimalPart = (int)((time - intPart) * 100);

  // Extract digits for 7-segment display
  if ((intPart / 10) % 10) {
    lc.setDigit(0, 3, (intPart / 10) % 10, false);  // Tens of seconds
  } else {
    lc.setChar(0,3,' ',false);
  }
  
  lc.setDigit(0, 2, intPart % 10, true);         // Seconds
  lc.setDigit(0, 1, (decimalPart / 10) % 10, false);  // Tenths of seconds
  lc.setDigit(0, 0, decimalPart % 10, false);         // Hundredths of seconds
}
