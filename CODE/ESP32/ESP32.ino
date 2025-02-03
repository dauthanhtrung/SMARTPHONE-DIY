//code gửi tin nhắn chuẩn cho esp32
const String PHONE = "+84868242509";

//GSM Module RX pin to ESP32 Pin 2
//GSM Module TX pin to ESP32 Pin 4
#define rxPin 4
#define txPin 2
#define BAUD_RATE 9600
HardwareSerial SIM800L(1);



String smsStatus, senderNumber, receivedDate, msg, data_sim;
boolean isReply = false;
String inputString;
void setup() {

  Serial.begin(9600);
  Serial.println("ESP32 serial initialize");

  SIM800L.begin(BAUD_RATE, SERIAL_8N1, rxPin, txPin);
  Serial.println("Khởi tạo module sim!");

  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";

  SIM800L.println("AT+CMGF=1");delay(500); // khởi động chức năng SMS
  SIM800L.println("AT+CNMI=1,2,0,0,0");delay(500);
//  SIM800L.println("AT+CMGL=\"REC UNREAD\"");delay(500); // đọc tin nhắn tới
  SIM800L.println("AT+CMGDA=\"DEL ALL\"");delay(500);
  delay(1000);
  Serial.println("SETUP DONE!!!");
//  Reply("done!");
}
void loop() {
  SIM800L_Event();
  while(SIM800L.available())// nhận chuỗi từ module sim
  {
    data_sim=SIM800L.readString();
    Serial.println(data_sim);
  }          
  if(!SIM800L.available() && data_sim.length()){
    if(data_sim.indexOf("+CMT") != -1)
      {  
        Serial.println("Tin nhắn đến");
        parseData(data_sim);
      }
  }
}



//***************************************************
void parseData(String buff) {
  Serial.print("Giải mã bắt đầu:"); Serial.println(buff);
  unsigned int len, index;
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();
  Serial.println("ERROR#1");
  if (buff != "OK") {
    Serial.print("ERROR#2:");Serial.println(buff);
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);
    Serial.print("Mã nhận được: ");
    Serial.println(cmd);
    if (cmd == "+CMTI") {
      Serial.println("ERROR#5");
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      SIM800L.println(temp);
    }
//    else if (cmd == "+CMGR") {
    else if (cmd == "+CMT") {
      extractSms(buff);
      Serial.print("Số điện thoại người gửi: ");
      Serial.print(senderNumber); Serial.print(" = ");
      Serial.println(PHONE);

      if (senderNumber == PHONE) {
        Serial.println("So sánh số điện thoại!");
        doAction();
      }
    }
  }
  else {
    Serial.println("ERROR#3");
  }
  Serial.println("done!");
  data_sim="";
}

//************************************************************
void extractSms(String buff) {
  unsigned int index;
  Serial.println("Bắt đầu giải mã:");Serial.println(buff);
  index = buff.indexOf(",");
  senderNumber = buff.substring(1, 13);
  Serial.print("Số điện thoại:");
  Serial.println(senderNumber);
  smsStatus = buff.substring(1, index - 1);
  buff.remove(0, index + 2);
  buff.remove(0, 19);
  receivedDate = buff.substring(0, 20);
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();

  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();
  Serial.print("Giải nén xong:");Serial.println(msg);
}

void doAction() {
  Serial.println("Giải mã tin nhắn!");
  if (msg == "status") {
    Serial.println("Gửi dữ liệu cảm biến qua số điện thoại!");
    Reply("Hi Hoang Tien Dat!");
  }
  else {
    Serial.println("Không gửi được!");
  }

  
  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";
}

void Reply(String text)
{
  SIM800L.print("AT+CMGF=1\r");
  delay(1000);
  SIM800L.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  SIM800L.print(text);
  delay(100);
  SIM800L.write(0x1A); //ascii code for ctrl-26 //SIM800L.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}
void SIM800L_Event() {
  while (Serial.available())  {
    SIM800L.println(Serial.readString());
  }
}
