const int buttonPin = 2;
const int pwm_a = 3;
const int pwm_b = 4;
const int a_in1 = 0;
const int a_in2 = 1;
const int b_in1 = 6;
const int b_in2 = 7;
const int a_controller = 18;

String inString = "";  // string to hold input
float a_control = 0.0;
int a_control_serial = 0.0;

void setup() {
  //analogWriteResolution(8);
  //pinMode(pwm_a, OUTPUT);
  //pinMode(pwm_b, OUTPUT);
  pinMode(a_in1, OUTPUT);
  pinMode(a_in2, OUTPUT);
  pinMode(b_in1, OUTPUT);
  pinMode(b_in2, OUTPUT);
  digitalWrite(a_in1, LOW);
  digitalWrite(a_in2, HIGH);
  digitalWrite(b_in1, LOW);
  digitalWrite(b_in2, HIGH);

  
  pinMode(b_in2, INPUT);

  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // send an intro:
  Serial.println("\n\nString toInt():");
  Serial.println();
}

void loop() {
  // Read serial input:
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    // if you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      Serial.print("Value:");
      Serial.println(inString.toInt());
      a_control_serial = inString.toInt();
      // clear the string for new input:
      inString = "";
    }
  }
  a_control = (analogRead(a_controller) - 576) / 350.0; // Convert raw value to range 0.0 - 1.0
  a_control = min(max(0.0, a_control), 1.0); // Limit output to 0..1
  a_control = a_control * 255; // Scale to analogWrite resolution

  if (a_control_serial > 0) {
    a_control = a_control_serial;
  }
  analogWrite(pwm_a, a_control);
  analogWrite(pwm_b, a_control);

  Serial.println(a_control);
}
