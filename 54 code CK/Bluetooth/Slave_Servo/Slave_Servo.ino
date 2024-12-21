// Connect the HC-05 and communicate using the serial monitor
// When first powered on. you need to manually enter AT mode
// The default baud rate for AT mode is 38400
#include <SoftwareSerial.h>
#include <Servo.h> 
#define Servo_PWM 6 
SoftwareSerial BTSerial(10, 11); // RX | TX
// Connect the HC-05 TX to Arduino pin 10
// Connect the HC-05 RX to Arduino pin 11
Servo MG995_Servo; 

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino is ready:");
  Serial.println("Remember to select Both BL & CR in the serial monitor");
  BTSerial.begin(38400); // HC-05 default speed in AT command

  MG995_Servo.attach(Servo_PWM);
  MG995_Servo.write(0);
}

void loop() {
  while (BTSerial.available())
  {
    char c = BTSerial.read();
    Serial.println(c);
    if (c== '1')
    {
      Serial.println("decode");
      MG995_Servo.write(90);
      delay(1000);
      MG995_Servo.write(0);
      delay(1000);
    }
    else {
      Serial.println("decode2");
      MG995_Servo.write(45);
      delay(1000);
      MG995_Servo.write(0);
      delay(1000);
    }
  }
  if (Serial.available())
  {
    BTSerial.write(Serial.read());
  }
}
