#include <SoftwareSerial.h>
#include "Stepper.h"

SoftwareSerial BTSerial(10, 11); // RX | TX
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 4,6,5,7);

int degreeToSteps(int degree, int STEPS = 2048) {
  if (degree == 0) return 0;
  return STEPS / (360 / degree);
}

void setup() {
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  Serial.begin(9600);
  Serial.println("Slave is ready:");
  BTSerial.begin(38400); // HC-05 baud rate
  myStepper.setSpeed(10);
}

void loop() {
  if (BTSerial.available()) {
    char c = BTSerial.read();
    Serial.println("Received: " + String(c)); // Hiển thị ký tự nhận được trên Serial Monitor
     
    int degree = degreeToSteps(90);
    if (c == '1') {
      Serial.println("Rotating CW");
      myStepper.step(degree);
    } else if (c == '0') {
      Serial.println("Rotating CCW");
      myStepper.step(-degree);
    }
    yield();
  }
}
