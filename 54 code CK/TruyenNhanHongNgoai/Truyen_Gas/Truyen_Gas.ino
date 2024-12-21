#define PIN_SEND 3
#include <IRremote.h>

int GasPin = 0;
int val;

void setup() { 
  Serial.begin(9600);
  IrSender.begin(PIN_SEND);
} 

void loop() { 
  delay(500);
  val = analogRead(GasPin);
  Serial.println(val, DEC);
  
  // Truyền mã IR với giao thức NEC
  if (val > 120) {
    IrSender.sendRC5(0x1, 8);  // Truyền mã 0x1 với độ dài 8 bit
    delay(200);
  } else {
    IrSender.sendRC5(0x2, 8);  // Truyền mã 0x2 với độ dài 8 bit
    delay(200);
  }
}
