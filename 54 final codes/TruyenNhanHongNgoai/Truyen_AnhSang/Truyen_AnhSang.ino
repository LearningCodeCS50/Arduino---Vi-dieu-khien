#define PIN_SEND 3
#include <IRremote.h>
int val = 0;
int LIGHT = 0;

void setup() { 
  Serial.begin(9600);
  IrSender.begin(PIN_SEND); 
} 
  
void loop() { 
  val = analogRead(LIGHT);
  Serial.println(val);
  if (val > 700) {
    IrSender.sendRC5(0x1, 8);  // Truyền mã 0x1 với độ dài 8 bit
    delay(200);
  } else {
    IrSender.sendRC5(0x2, 8);  // Truyền mã 0x2 với độ dài 8 bit
    delay(200);
  }
  delay(500);
}
