#include <Adafruit_Fingerprint.h>
#define mySerial Serial2

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(9600);
  Serial.println("\n\nCode phát hiện dấu vân tay");
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Đã tìm thấy cảm biến!");
  } else {
    Serial.println("Không thấy cảm biến nào cả, chán vl :(");
    while (1) { delay(1); }
  }
}

void loop()                     // run over and over again
{
  getFingerprintID();

  delay(50);            //don't ned to run this at full speed.
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Tìm thấy vân tay");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("Lỗi");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Lỗi");
      return p;
    default:
      Serial.println("Lỗi không xác định");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("So sánh hình ảnh");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Lỗi");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Lỗi");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Lỗi");
      return p;
    default:
      Serial.println("Lỗi");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Vân tay trùng khớp!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Lỗi");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Lỗi");
    return p;
  } else {
    Serial.println("Lỗi");
    return p;
  }

  Serial.print("Vân tay là ID #"); Serial.print(finger.fingerID);
  if(page_number=="19"){
    if(finger.fingerID==1||finger.fingerID==2||finger.fingerID==3||finger.fingerID==4||finger.fingerID==5||finger.fingerID==25){
      Serial.println("Mở khóa thiết bị");
      String lcd_text="page page0"; SIM.println(lcd_text);
    }
  }
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
//int getFingerprintIDez() {
//  uint8_t p = finger.getImage();
//  if (p != FINGERPRINT_OK)  return -1;
//
//  p = finger.image2Tz();
//  if (p != FINGERPRINT_OK)  return -1;
//
//  p = finger.fingerFastSearch();
//  if (p != FINGERPRINT_OK)  return -1;
//
//  // found a match!
//  Serial.print("Found ID #"); Serial.print(finger.fingerID);
//  Serial.print(" with confidence of "); Serial.println(finger.confidence);
//  return finger.fingerID;
//}
