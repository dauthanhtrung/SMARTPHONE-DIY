//HardwareSerial & SIM800 = Serial3;
#include <SoftwareSerial.h>
//
////Create software serial object to communicate with SIM800L
//SoftwareSerial SIM800(2, 3); //SIM800L Tx & Rx is connected to Arduino #3 & #2
SoftwareSerial SIM800(3, 2);

const String PHONE = "+84868242509"; // Số điện thoại chủ xe
String mapGoogle[]={"20.980087,105.795609","20.979934,105.796416","20.981786,105.795889","20.980643,105.795901","20.980998,105.796911","20.978651,105.796657"};
String locate="";
void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(115200);
  // 
  //Begin serial communication with Arduino and SIM800L
  SIM800.begin(9600);

  Serial.println("Initializing...");
  delay(1000);

  SIM800.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  SIM800.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  SIM800.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  SIM800.println("AT+CUSD=1"); //Check whether it has registered in the network
  updateSerial();delay(2000);
  
    int ran = random(0,3);
    Serial.print("Random: ");Serial.println(ran);
    Serial.print("http://maps.google.com/maps?q=");Serial.println(mapGoogle[ran]);
    SIM800.println("AT+CMGF=1\r");
    delay(1000);
    SIM800.println("AT+CMGS=\""+PHONE+"\"\r");
    delay(1000);
    SIM800.print("http://maps.google.com/maps?q=");SIM800.println(mapGoogle[ran]);Serial.println("Ủa");
    delay(100);
    SIM800.write(0x1A); 
    delay(1000);
    Serial.println("OK! Đã gửi tin nhắn");
}

void loop()
{
  updateSerial();
}
void sendSms(String message) // gửi nội dung tin nhắn
{
    SIM800.println("AT+CMGF=1\r");
    delay(1000);
    SIM800.println("AT+CMGS=\""+PHONE+"\"\r");
    delay(1000);
    SIM800.println(message);
    delay(100);
    SIM800.write(0x1A); 
    delay(1000);
    Serial.println("OK! Đã gửi tin nhán");
}
void updateSerial()
{
  delay(10);
  while (Serial.available()) 
  {
    SIM800.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(SIM800.available()) 
  {
    Serial.write(SIM800.read());//Forward what Software Serial received to Serial Port
  }
}
//void checkTK(String msg){
//  String data_sim,checkTK_text;
//  int startCOPS=-1, copsFirstcomma, copsSecondcomma,copsNL;
//  Serial.println("waiting...");
//  while(SIM800.available())(SIM800.read());
//  SIM800.println("ATD*101#;"); // gửi lệnh để lấy thông tin tài khoản
//  delay(5000);
//  while(!SIM800.available()); 
//  while(SIM800.available())
//  {
//    data_sim=SIM800.readString();
//  }
//   Serial.print("Dữ liệu từ sim:");Serial.println(data_sim);
//  if(data_sim.indexOf("+CUSD: 1") != -1) // lấy thông tin 
//  {
//    copsFirstcomma=data_sim.indexOf(",", startCOPS+1);
//    copsSecondcomma=data_sim.indexOf(",", copsFirstcomma+2);
//    copsNL=data_sim.indexOf("\n", copsSecondcomma+1);// cắt chữ nhà mạng
//    checkTK_text=data_sim.substring(copsFirstcomma+2, copsNL-2);
//
////    lcd_text="page7.t1.txt=\""+checkTK_text+"\"";LCD_print(lcd_text);
//    Serial.print("Nội dung:");Serial.println(checkTK_text);
//  }
////  Serial.print("Dữ liệu 2:");Serial.println(data_sim);
//  Serial.print("done!!");
//  data_sim=checkTK_text="";
//  copsFirstcomma=copsSecondcomma=copsNL=0;
//}
//
//void searchNetwork(){
//  String data_sim, network_name, network_text;
//  int startCOPS=-1, copsFirstcomma, copsSecondcomma, copsNL;
//  while(SIM800.available())(SIM800.read());
//  String sim_text="AT+COPS?"; SIM800.println(sim_text); // gửi lệnh để nhận giá trị nhà mạng
//  while(!SIM800.available()); 
//  while(SIM800.available())
//  {
//    data_sim=SIM800.readString();
//  }
//  Serial.print("Dữ liệu từ sim:");Serial.println(data_sim);
//  if(data_sim.indexOf("+COPS:") != -1) // lấy thông tin nhà mạng
//  {
//    copsFirstcomma=data_sim.indexOf(",", startCOPS+1);
//    copsSecondcomma=data_sim.indexOf(",", copsFirstcomma+2);
//    copsNL=data_sim.indexOf("\n", copsSecondcomma+1);// cắt chữ nhà mạng
//    network_name=data_sim.substring(copsSecondcomma+2, copsNL-2);
//
//
//    Serial.print("Nhà mạng:");Serial.println(network_name);
//  }
//  sim_text=data_sim=network_name="";
//  copsFirstcomma=copsNL=0;
//}
