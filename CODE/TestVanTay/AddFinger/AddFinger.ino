

#include <Adafruit_Fingerprint.h>

#define mySerial Serial2


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//uint8_t id;

void setup()
{
  Serial.begin(9600);
  Serial.println("\n\nVân tay nè!!!!!!!!!!!!!!!!!");
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Đã tìm thấy cảm biến vân tay!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

//  Serial.println(F("Reading sensor parameters"));
//  finger.getParameters();
//  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
//  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
//  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
//  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
//  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
//  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
//  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop()                     // run over and over again
{
  Serial.println("Bắt đầu nhập!");
  Serial.println("Hãy nhập ID # (1 -127)");
  int number = readnumber();
  Serial.print("Đang ghi vân tay vào ID #");
  Serial.println(number);
  while (!  getFingerprintEnroll(number) );
}

uint8_t getFingerprintEnroll(uint8_t id) { // thêm dấu vân tay vào id
  int p = -1;
  String lcd_text="page26.t0.txt=\"Start!!\""; SIM.println(lcd_text);
//  lcd_text="page page26"; SIM.println(lcd_text); // gửi lệnh đđến màn hình
  Serial.print("Hãy đặt ngón tay vào cảm biến #"); Serial.println(id);
  while (p != FINGERPRINT_OK) { //Nhập vân tay lần 1
    p = finger.getImage(); // thêm dấu vân tay vào cảm biến
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Đã lưu vân tay");
      lcd_text="page26.t0.txt=\"Save Finger!!\""; SIM.println(lcd_text);
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi giao tiếp cảm biến");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Vân tay bị lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text); // gửi lệnh đđến màn hình
      break;
    default:
      Serial.println("Lỗi không xác định");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      break;
    }
  }

  //Lưu kết quả vân tay lần thứ 1
  p = finger.image2Tz(1); // xác nhận lại dấu vân tay lần 1
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Vân tay hợp lệ");
      lcd_text="page26.t0.txt=\"Finger Success!!\""; SIM.println(lcd_text);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Vân tay lộn xộn:))");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Giao tiếp lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Không thể thấy vân tay");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Không thể thấy vân tay");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    default:
      Serial.println("Lỗi không xác định");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
  }


  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("Xác nhận vân tay ID "); Serial.println(id);
//  lcd_text="vis b0,1"; SIM.println(lcd_text);
  p = -1;
  Serial.println("Đặt lại cùng một ngón tay");
  lcd_text="page26.t0.txt=\"Xac nhan van tay!!\""; SIM.println(lcd_text);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Đã lưu");
      lcd_text="page26.t0.txt=\"save!!\""; SIM.println(lcd_text);
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Giao tiếp lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Vân tay lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      break;
    default:
      Serial.println("Lỗi không xác định");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Chuyển đổi vân tay");
      lcd_text="page26.t0.txt=\"Ok!!\""; SIM.println(lcd_text);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Vân tay quá lộn xộn");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Giao tiếp lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Không thể tìm thấy");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Không thể tìm thấy");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
    default:
      Serial.println("Lỗi không xác định");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
      return p;
  }

  // OK converted!
  Serial.print("Tạo vân tay cho #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Vân tay hợp lệ!");
    lcd_text="page26.t0.txt=\"Van tay hop le!!\""; SIM.println(lcd_text);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Giao tiếp lỗi");
    lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Vân tay không trùng khớp, hãy làm lại");
    lcd_text="page26.t0.txt=\"Van tay khong trung khop!!\""; SIM.println(lcd_text);
    return p;
  } else {
    Serial.println("Lỗi không xác định");
    lcd_text="page26.t0.txt=\"Finger Error!!\""; SIM.println(lcd_text);
    return p;
  }
  lcd_text="page26.t0.txt=\"SAVE!!\""; SIM.println(lcd_text);
  Serial.print("Lưu vân tay "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Đã lưu trữ vào cảm biến!"); // thêm vân tay thành công
    lcd_text="page26.t0.pco=8986"; SIM.println(lcd_text);
    lcd_text="page26.t0.txt=\"done!!\""; SIM.println(lcd_text);
    delay(1000);
    lcd_text="page page25"; SIM.println(lcd_text);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) 
  {
    Serial.println("Giao tiếp lỗi");
    lcd_text="page26.t0.txt=\"fail!!\""; SIM.println(lcd_text);
    lcd_text="vis b0,1"; SIM.println(lcd_text);
    lcd_text="vis bBack,1"; SIM.println(lcd_text);
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) 
  {
    Serial.println("Không thể lưu vào vị trí này");
    lcd_text="page26.t0.txt=\"fail!!\""; SIM.println(lcd_text);
    lcd_text="vis b0,1"; SIM.println(lcd_text);
    lcd_text="vis bBack,1"; SIM.println(lcd_text);
    return p;
  } else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Có lỗi trong khi ghi");
    lcd_text="page26.t0.txt=\"fail!!\""; SIM.println(lcd_text);
    lcd_text="vis b0,1"; SIM.println(lcd_text);
    lcd_text="vis bBack,1"; SIM.println(lcd_text);
    return p;
  } else 
  {
    Serial.println("Lỗi không xác định");
    lcd_text="page26.t0.txt=\"fail!!\""; SIM.println(lcd_text);
    lcd_text="vis b0,1"; SIM.println(lcd_text);
    lcd_text="vis bBack,1"; SIM.println(lcd_text);
    return p;
  }

  return true;
}
