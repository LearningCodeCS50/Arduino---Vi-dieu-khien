#include <IRremote.h>
#include "Stepper.h"
#define PIN_RECV 2

const int stepsPerRevolution = 2048;
Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11);

IRrecv receiver(PIN_RECV);
decode_results output;

int degreeToSteps(int degree, int STEPS = 2048){
  if (degree == 0) return 0;
  return STEPS / (360/degree);
}

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(PIN_RECV, ENABLE_LED_FEEDBACK);  // Bắt đầu nhận tín hiệu IR
  myStepper.setSpeed(15);
  Serial.println("Đang chờ nhận tín hiệu...");
}

void loop() {
  if (IrReceiver.decode(&output)) {
    Serial.print("Mã IR nhận được: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // In ra mã IR nhận được
    Serial.print("Giao thức: ");
    Serial.println(IrReceiver.decodedIRData.protocol, DEC);
    // So sánh mã nhận được với mã bạn đã truyền
    if (IrReceiver.decodedIRData.decodedRawData == 0x101) {
      Serial.println("decode 0x1");
      myStepper.step(degreeToSteps(90));  // Xoay motor 90 độ
    } else if (IrReceiver.decodedIRData.decodedRawData == 0x102) {
      Serial.println("decode 0x2");
      myStepper.step(degreeToSteps(-90));  // Xoay motor -90 độ
    }
    IrReceiver.resume();  // Tiếp tục nhận tín hiệu IR tiếp theo
  }
}
