#define PIN_SEND 3
#include <IRremote.h> 
#define LINE 2

int val = 0;


void setup() { 
  Serial.begin(9600);
  IrSender.begin(PIN_SEND); 
} 
  
void loop() { 
  delay(500);
  val = digitalRead(LINE);
   val ? Serial.println("Black"):Serial.println("White");
  if (val == 0) {
    IrSender.sendRC5(0x1, 8);  // Truyền mã 0x1 với độ dài 8 bit
    delay(200);
  } else {
    IrSender.sendRC5(0x2, 8);  // Truyền mã 0x2 với độ dài 8 bit
    delay(200);
  }   
}
