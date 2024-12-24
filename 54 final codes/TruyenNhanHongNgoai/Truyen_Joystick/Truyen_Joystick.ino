#include <IRremote.h>
//START JOYSTICK VARIABLE
int JoyStickX = 1; //A1
int JoyStickY = 2; // A2
int JoyStickZ = 4;

bool joystick(int thresholdX = 550, int thresholdY = 550, int thresholdZ = 0) {
  int x, y, z;
  x = analogRead(JoyStickX);
  y = analogRead(JoyStickY);
  z = digitalRead(JoyStickZ);
  Serial.println("JOYSTICK " + String(x) + ", " + String(y) + ", " + String(z) );
  int valSent = 0;
  if (x >= thresholdX  || y>=thresholdY || z == thresholdZ) {
    return true;
  } else {
    return false;
  }
}
void setup() { 
  Serial.begin(9600);
  IrSender.begin(3); 
  pinMode(JoyStickZ, INPUT_PULLUP);
} 
void loop(){
  Serial.print("Sent ");
  if(joystick()){
  Serial.println("1");
    IrSender.sendRC5(0x1, 8);  // Truyền mã 0x1 với độ dài 8 bit
    delay(200);
  } else {
    IrSender.sendRC5(0x2, 8);  // Truyền mã 0x2 với độ dài 8 bit
    delay(200);
  }    
  delay(500);
}