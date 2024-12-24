#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11);
void setup() {
  digitalWrite(9, HIGH);
  pinMode(9, OUTPUT);
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  BTSerial.begin(38400);
}

void loop() {
  if (BTSerial.available()) {
    Serial.write(BTSerial.read());
  }
  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }
}
