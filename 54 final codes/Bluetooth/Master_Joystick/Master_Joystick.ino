#include <SoftwareSerial.h>
SoftwareSerial BTSerial(10, 11); // RX | TX
//START JOYSTICK VARIABLE
int JoyStickX = 1; //A1
int JoyStickY = 2; // A2
int JoyStickZ = 3;

bool joystick(int thresholdX = 550, int thresholdY = 550, int thresholdZ= LOW) {
  int x, y, z;
  x = analogRead(JoyStickX);
  y = analogRead(JoyStickY);
  z = digitalRead(JoyStickZ);
  Serial.println("JOYSTICK " + String(x) + ", " + String(y) + ", " + z );
  int valSent = 0;
  if (x >= thresholdX  || y>=thresholdY || z == LOW) {
    return true;
  } else {
    return false;
  }
}
void setup() { 
  Serial.begin(9600);
  BTSerial.begin(38400); // HC-05 default speed in AT command
  pinMode(JoyStickZ, INPUT_PULLUP);
} 
void loop(){
  Serial.print("Sent ");
  if(joystick()){
    Serial.println("YES");
    BTSerial.write("1");
  } else {
    Serial.println("NO");
    BTSerial.write("0");
  }
  delay(2000);
}