#include <IRremote.h>
#define PIN_RECV 2

IRrecv receiver(PIN_RECV);
decode_results output;

// Kết nối động cơ A
int enableA = 3; // Không cần cắm
int input1 = 5; // Cắm vào chân xung D5 (đúng tên biến input1 vì trong class DC dùng)
int input2 = 6; // Cắm vào chân xung D6


/*
  Bên dưới là code cho lớp chạy động cơ DC
*/
struct DC{
  int speed;
  int curspeed = 0;
  int v1,v2;
  void tov(int speed){
    if (speed>=0){
      v2 = 0;
      v1 = speed;
    }
    else{
      v1 = 0;
      v2 = -speed;
    }
  }
  void apply_speed(){
    tov(curspeed);
    if (v1==0){
      digitalWrite(input1,LOW);
    }
    else{
      analogWrite(input1,v1);
    }
    if (v2==0){
      digitalWrite(input2,LOW);
    }
    else{
      analogWrite(input2,v2);
    }
    
  }
  //nhanh dần : 0~255 ->0 ~255
  // chậm dần : 255 ~ 0 -> ...
  // Thuận chiều:  
  // hàm gọi sẽ gán giá trị speed 
  
  //Gọi hàm trong vòng loop, 
  void thuan_chieu_nhanh_dan(){
  
    if (curspeed<0) curspeed=0;
    curspeed+=25;
    if (curspeed>255){
      curspeed = 0;
    }
    apply_speed();
  }
  void thuan_chieu_cham_dan(){
    if (curspeed>255) curspeed = 255;
    curspeed -= 25;
    if (curspeed<0) curspeed = 255;
    apply_speed();
  }
  void nguoc_chieu_nhanh_dan(){
    if (curspeed>0) curspeed = 0;
    if (curspeed< -255) curspeed = 0;
    curspeed -=25;
    apply_speed();
  }
  void nguoc_chieu_cham_dan(){
    if (curspeed>0) curspeed = -255;
    if (curspeed<-255) curspeed = -255;
    curspeed +=25;
    apply_speed();
  }
  void thuan_chieu_quay_deu(){
    curspeed = 255;
    apply_speed();
  }
  void nguoc_chieu_quay_deu(){
    curspeed = -255;
    apply_speed();
  } 
  void loop(int type) {
    switch (type)
    {
      case 1: // thuan_chieu_nhanh_dan
        thuan_chieu_nhanh_dan();
      break;
      case 2: // thuan_chieu_cham_dan
        thuan_chieu_cham_dan();
      break;
      case 3: // nguoc_chieu_nhanh_dan
        nguoc_chieu_nhanh_dan();
      break;
      case 4: // nguoc_chieu_cham_dan
        nguoc_chieu_cham_dan();
      break;
      case 5: // thuan_chieu_quay_deu
        thuan_chieu_quay_deu();
      break;
      case 6: // nguoc_chieu_quay_deu
        nguoc_chieu_quay_deu();
      break;
    default:
      break;
    }
    Serial.print(v1);
    Serial.print("-");
    Serial.println(v2);
    delay(255);
  }
} DCTask;

void setup() {
  // pinMode(enableA, OUTPUT);
  // Cài đặt cho động cơ 
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  Serial.begin(9600);
  // cài đặt cho IR
  IrReceiver.begin(PIN_RECV, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.print("Mã IR nhận được: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // In ra mã IR nhận được
    Serial.print("Giao thức: ");
    Serial.println(IrReceiver.decodedIRData.protocol, DEC);
    if (IrReceiver.decodedIRData.decodedRawData == 0x101) {
      Serial.println("decode");
      DCTask.thuan_chieu_nhanh_dan();// mỗi lần gọi sẽ thay đổi vận tốc
    } else if (IrReceiver.decodedIRData.decodedRawData == 0x102) {
      Serial.println("decoode2");
      DCTask.nguoc_chieu_cham_dan();// trong hàm này không có loop
    }
    delay(255);// nhớ có delay ở trong cái hàm loop này
    IrReceiver.resume();
  }
}
