/*
     _             _   _                 _     _
  __| | __ _ _   _| |_| |__   __ _ _ __ | |__ | |_ _ __ _   _ _ __   __ _
 / _` |/ _` | | | | __| '_ \ / _` | '_ \| '_ \| __| '__| | | | '_ \ / _` |
| (_| | (_| | |_| | |_| | | | (_| | | | | | | | |_| |  | |_| | | | | (_| |
 \__,_|\__,_|\__,_|\__|_| |_|\__,_|_| |_|_| |_|\__|_|   \__,_|_| |_|\__, |
                                                                    |___/
*/
//*****************************************************************//
// Code by Dau Thanh Trung
// Project: Dien thoai di dong cam ung co cam bien van tay
// Date:    20/03/2022
// Name:    Dau Thanh Trung
// Phone:   0868.24.25.09
// Mail:    dauthanhtrung.kma@gmail.com
// Site:    www.facebook.com/kjmtrun
//*****************************************************************//


HardwareSerial & SIM = Serial3;
HardwareSerial & myFinger = Serial2;
HardwareSerial & LCD = Serial1;

#define BUTTON_LOCK 41  //power button
#define BUZZER 12 // connect to buzzer
#define FLASH 13 // connect to flash
// #define TOUCH A2   // fingerprint sensor

#include <Adafruit_Fingerprint.h> //fingerprint sensor library
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&myFinger);

char msisdn[30], ATcomm[30];
String Month[]={"NULL","JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE","JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER"};
String data_lcd, pageNum, msg, data_sim;
String page_number="0";
byte calling=0;
byte state_Flash=0;
unsigned long every_second;
int lock_state=0; // device lock status

void bip(){
  digitalWrite(BUZZER,HIGH);
  delay(40);
  digitalWrite(BUZZER,LOW);
  delay(40);
}
void flash(){
  digitalWrite(FLASH,HIGH);
  delay(100);
  digitalWrite(FLASH,LOW);
  delay(100);
}
void setup(){
  Serial.begin(9600); //Serial init
  SIM.begin(9600);    //module sim init
  LCD.begin(9600);	   //LCD display init
  finger.begin(57600);  //fingerprint sensor init
  
  pinMode(BUZZER,OUTPUT); //setting I/O pin
  pinMode(FLASH,OUTPUT);
  pinMode(BUTTON_LOCK,INPUT_PULLUP);
  pinMode(A0,INPUT);

  digitalWrite(FLASH,LOW);
  digitalWrite(BUZZER,LOW);
  while(!SIM){;}
  bip();flash();flash();
  Khoi_dong(); // active device
  bip();bip();
  Serial.println("Done, active successfully!");
}

void loop(){
  // if ( (unsigned long) (millis() - every_second) > 60000) // update time every 20s
  //   {
  //   if(clock_state==1)
  //   {
  //     GetTimeDate(); 
  //   }
  //     every_second = millis();// update timer millis
  // }
//  getFingerprintID();
  if(page_number=="19"){
    getFingerprintID();
    if(finger.fingerID==1||finger.fingerID==2||finger.fingerID==3||finger.fingerID==4||finger.fingerID==5||finger.fingerID==15||finger.fingerID==11){
      Serial.println("Mở khóa thiết bị");
      bip();
      String lcd_text="page page0"; LCD_print(lcd_text);
//      Serial.print("Trang số: ");Serial.println(page_number);
      page_number="0";
      finger.fingerID=0;
    }
    else{
//      bip();
//      String lcd_text="page20.t4.txt=\"no match finger\""; LCD_print(lcd_text);
    }
  }
  while(LCD.available())
  {
    data_lcd.concat(char(LCD.read()));
  }
    delay(5);
  if(!LCD.available())
  {										
    if(data_lcd.length())
    {
      Serial.println("String from LCD send:");
      pageNum = data_lcd[data_lcd.length()-4];Serial.println(pageNum);		//get page number on LCD
      Serial.print("Số trang: ");Serial.print(pageNum);
  	  msg = data_lcd.substring(1, data_lcd.length()-4);						//get string on LCD
      Serial.print(" - Nội dung: ");Serial.println(msg);
  	  if((pageNum == "0") && (msg.length() != 0)){querySMS(msg);}		//from page 0, get SMS and strings
      if((pageNum == "1") && (msg.length() != 0)){connectCall(msg);}	//from page7 : calling
      if((pageNum == "2") && (msg.length() != 0)){endCall(msg);}	//from page8 : end call
      if((pageNum == "3") && (msg.length() != 0)){sendSMS(msg);}		//get contents from page 9 and send messages
  	  if((pageNum == "5") && (msg.length() != 0)){answerCall(msg);}		//page 15 : answer incoming call
  	  if((pageNum == "6") && (msg.length() != 0)){delReadSMS(msg);}	  	//delete read messages
  	  if((pageNum == "7") && (msg.length() != 0)){delSMS(msg);}	  		//delete all messages
      if((pageNum == "8") && (msg.length() != 0)){updateSMS();}	  		//update messages
      if((pageNum == "9") && (msg.length() != 0)) // turn on/off FLASH light
      {
        if(msg=="on") digitalWrite(FLASH,HIGH);
        if(msg=="off") digitalWrite(FLASH,LOW);
      }  
      if((pageNum == "A") && (msg.length() != 0)){delFinger(msg);}   // delete fingerprint
      if((pageNum == "B") && (msg.length() != 0)){addFinger(msg);}   // set fingerprint
      if((pageNum == "C") && (msg.length() != 0))
      {
        if(msg=="lock") page_number="19";
        if(msg=="unlock") page_number="0";
        Serial.print("Trang số: ");Serial.println(page_number);
      }  
      if((pageNum == "D") && (msg.length() != 0)){checkTK(msg);} //Check account *101#
      data_lcd="";pageNum="";msg="";
    }
  }
  //*******************************************************************************************
  while(SIM.available())// get strings from module sim
  {
    data_sim=SIM.readString();
    Serial.println(data_sim);
  }					
  //*******************************************************************************************
  if(!SIM.available() && data_sim.length()){
    if(data_sim.indexOf("NO CARRIER") != -1){								
      String lcd_text = "page page19";LCD_print(lcd_text);
	    sendATcommand("AT", "OK", 2000);	
      calling=0;  
    }
	if(data_sim.indexOf("BUSY") != -1){										// busy device
      String lcd_text = "page page19";LCD_print(lcd_text);
	    sendATcommand("AT", "OK", 2000);
      calling=0;
	}
	if(data_sim.indexOf("NO ANSWER") != -1) // no answer
	{									
      String lcd_text = "page page19";LCD_print(lcd_text);
	    sendATcommand("AT", "OK", 2000);
      calling=0;
	}		
  
    //********************************************
  if(data_sim.indexOf("+CLIP") != -1)// When a call comes in, pick up the call
  {			
    calling++;
  }
  if(calling==1)
  {
      LCD_print("sleep=0");
      int chuoi1 = data_sim.indexOf("\"");
      int chuoi2 = data_sim.indexOf("\"", chuoi1+1);
      String number_phone = data_sim.substring(chuoi1+1, chuoi2);
      Serial.print("Cuộc gọi đến");Serial.println(number_phone);
      String lcd_text = "page page15";LCD_print(lcd_text);
      lcd_text = "page15.number.txt=\""+number_phone+"\"";LCD_print(lcd_text);
      lcd_text = "page15.t0.txt=\"Calling...\"";LCD_print(lcd_text);
      sendATcommand("AT", "OK", 2000);
       calling++;
   }

	if(data_sim.indexOf("+CMTI") != -1)
  {				//When a message comes in
    Serial.println("Tin nhắn đến");
		smsComputation();
    LCD_print("sleep=0");//wake up machine
		LCD_print("page0.bMessage.picc=41");
    // LCD_print("page16.ting_ting.txt=\"1\"");
    LCD_print("page19.t1.txt=\"new message..\"");
	}
	data_sim="";
  }
  if(digitalRead(BUTTON_LOCK)==LOW) // power button function
  {
    delay(200);
    if(digitalRead(BUTTON_LOCK)!=LOW);
    {
      bip();
      lock_state=!lock_state;
      String lcd_text="sleep="+String(lock_state);LCD_print(lcd_text);
      LCD_print("page page19");
      Serial.println("Lock screen!!");
//      page_number="19";
      delay(20);
    }
//   if(calling!=0)
//    {
//      if ( (unsigned long) (millis() - every_second) > 100) //Nhấp nháy Flash khi có cuộc gọi đến
//     {
//       state_Flash=!state_Flash;
//       digitalWrite(FLASH,state_Flash);
//       every_second = millis();// cập nhật lại biến time
//     }
//    }
  }
}

void Khoi_dong(){	// start the device
//Initialize sim module, update location, check network operator
  Serial.println("My phone: Đậu Thành Trung");
  Serial.println("Starting phone!");
  // LCD_print("rest");  //reset LCD display
  String lcd_text = "page page18";LCD_print(lcd_text);
  uint8_t answer = 0;
  Serial.print("Conecting.");
  while(answer == 0)
  {
    answer = sendATcommand("AT", "OK", 2000);
    Serial.print(".");
  }
  Serial.println("OK");
  while((sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0);
  LCD_print("page18.t0.txt=\"starting phone of Trun\"");delay(1000);
  Serial.println("Searching...");
  LCD_print("page18.t0.txt=\"searching network...\"");delay(1000);
  searchNetwork();
  LCD_print("page18.t0.txt=\"get time & date...\"");delay(1000);
  // GetTimeDate(); 
  LCD_print("page18.t0.txt=\"wait a minute...\"");delay(10);
  SIM.println("AT+CMGF=1"); delay(300);
  SIM.println("AT+CLIP=1"); delay(300);
  smsComputation(); 
  LCD_print("page18.t0.txt=\"done!\"");delay(1000);
  LCD_print("page page19"); //go to lock screen
  page_number="19";
  Serial.print("Trang số: ");Serial.println(page_number);
  LCD_print("thsp=100");delay(100); // screen sleep after 30s of inactivity
  LCD_print("thup=1");delay(100);    // touch screen to wake up
  Serial.println("Khởi động thành công!");
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){
// Send AT command and receive response from sim module
  uint8_t x=0, answer=0; 
  char GSMreponse[100]; 
  unsigned long previous;
  memset(GSMreponse, '\0', 100); delay(100);
  while(SIM.available()>0) SIM.read();
  SIM.println(ATcommand); previous = millis();
  do
  {
    if(SIM.available() != 0){GSMreponse[x] = SIM.read();
    x++;
    if(strstr(GSMreponse, expected_answer) != NULL){answer = 1;}
  }
  }while((answer == 0) &&((millis() - previous) < timeout));
  return answer;
}

void searchNetwork(){
	String data_sim, network_name, network_text;
	int startCOPS=-1, copsFirstcomma, copsSecondcomma, copsNL;
	while(SIM.available())(SIM.read());
	String sim_text="AT+COPS?"; SIM.println(sim_text); // send command to get carrier value
	while(!SIM.available()); 
	while(SIM.available())
	{
	  data_sim=SIM.readString();
	}
	if(data_sim.indexOf("+COPS:") != -1) // get carrier information
	{
		copsFirstcomma=data_sim.indexOf(",", startCOPS+1);
		copsSecondcomma=data_sim.indexOf(",", copsFirstcomma+2);
		copsNL=data_sim.indexOf("\n", copsSecondcomma+1);// 
		network_name=data_sim.substring(copsSecondcomma+2, copsNL-2);

		network_text="page19.net.txt=\""+network_name+"\"";LCD_print(network_text);
    network_text="page19.t2.txt=\""+network_name+"\"";LCD_print(network_text);
    network_text="page0.t0.txt=\""+network_name+"\"";LCD_print(network_text);
    Serial.print("Nhà mạng:");Serial.println(network_name);
	}
	sim_text=data_sim=network_name="";
	copsFirstcomma=copsNL=0;
}

void querySMS(String querySMSContent){
//Nhận dữ liệu SMS
	SIM.println(querySMSContent);
}

void checkTK(String msg){
  String data_sim,checkTK_text;
  int startCOPS=-1, copsFirstcomma, copsSecondcomma;
  String lcd_text="page7.t1.txt=\"waiting...\"";LCD_print(lcd_text);
  while(SIM.available())(SIM.read());
  String sim_text="ATD*101#;"; SIM.println(sim_text); // send command to get account information
  while(!SIM.available()); 
  while(SIM.available())
  {
    data_sim=SIM.readString();
  }
  if(data_sim.indexOf("+CUSD:") != -1) // get information 
  {
    copsFirstcomma=data_sim.indexOf("\"", startCOPS);
    copsSecondcomma=data_sim.indexOf("\"", copsFirstcomma);
    checkTK_text=data_sim.substring(copsFirstcomma, copsSecondcomma);

    lcd_text="page7.t1.txt=\""+checkTK_text+"\"";LCD_print(lcd_text);
    Serial.print("Nội dung:");Serial.println(checkTK_text);
  }
  lcd_text=data_sim=checkTK_text="";
  copsFirstcomma=copsSecondcomma=0;
}

void connectCall(String conCallContent){
//Hàm kết nối với cuộc gọi đến
  Serial.println("Đang goi dien cho!"); 
  String lcd_text = "page8.t0.txt=\"Connecting...\"";LCD_print(lcd_text);
  while((sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 	//0,1:	User registered, home network.
  sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0);	//0,5:	User registered, roaming.
  conCallContent.toCharArray(msisdn, data_lcd.length());
  sprintf(ATcomm, "ATD%s;", msisdn);
  sendATcommand(ATcomm, "OK", 10000);
  lcd_text = "page8.t0.txt=\"Connected!!\"";LCD_print(lcd_text);
  memset(msisdn, '\0', 30);memset(ATcomm, '\0', 30);
  calling=0;
}

void endCall(String relCallContent){
  calling=0;
  SIM.println(relCallContent); 
}

void sendSMS(String sms_text){
//Function to send message content to phone number
   Serial.println("Đang gửi tin nhắn sms"); 
  int firstDelim = sms_text.indexOf(byte(189));
  int secondDelim = sms_text.indexOf(byte(189), firstDelim+1);
  String smsContent = sms_text.substring(0, firstDelim);
  String phoneNumber = sms_text.substring(firstDelim+1, secondDelim);
  Serial.println(smsContent); 
  Serial.println(phoneNumber); 
  while((sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0);
  if(sendATcommand("AT+CMGF=1", "OK", 2500)) //Initialize text mode
  {
    phoneNumber.toCharArray(msisdn, msg.length());
    sprintf(ATcomm, "AT+CMGS=\"%s\"", msisdn);
    if(sendATcommand(ATcomm, ">", 2000))
    {
      SIM.println(smsContent);SIM.write(0x1A);
      if(sendATcommand("", "OK", 20000))
      {
        Serial.println("Đã gửi tin nhắn"); // message sent
      }
	    else {
	      Serial.println("Gửi lỗi!"); // error
	    }
	  }
  	else {
  	  Serial.println("Không tìm thấy >");//not found
  	}
  }
  smsContent=""; phoneNumber="";
  memset(msisdn, '\0', 30);
  memset(ATcomm, '\0', 30);
}

void answerCall(String ansCallContent){ //answer incoming call

	SIM.println(ansCallContent);
  Serial.println("Tra loi dien thoai!"); 
  LCD_print("sleep=0");
	String lcd_text = "page8.t0.txt=\"Connected!!\"";LCD_print(lcd_text);
  calling=0;
}

void smsComputation(){

  Serial.println("Khoi tao SMS..");
	int startCPMS=-1, CPMS1comma, CPMS2comma, msgCount=0;
	int startCMGR=-1, cmgrIndex, cmgrNLindex, smsNLindex;
	String sendCPMScommand, readCPMScommand, CPMSMsgCount;
	String sendCMGRcommand, readCMGRcommand, inCMGR, inSMS, dspCount;
	sendCPMScommand = "AT+CPMS?"; SIM.println(sendCPMScommand);
	while(!SIM.available());
	while(SIM.available())
	{
	  readCPMScommand=SIM.readString();
	}
	if(readCPMScommand.indexOf("+CPMS:") != -1)
  {
		CPMS1comma = readCPMScommand.indexOf(",", startCPMS+1);
		CPMS2comma = readCPMScommand.indexOf(",", CPMS1comma+1);
		CPMSMsgCount = readCPMScommand.substring(CPMS1comma+1, CPMS2comma);
		msgCount = CPMSMsgCount.toInt();//number of messages
		// dspCount="page6.sms.txt=\""+CPMSMsgCount+"\"";LCD_print(dspCount);
    Serial.print("Số tin nhắn: "); Serial.println(msgCount);
		if((msgCount > 0) && (msgCount <= 5))
		{
		  LCD_print("page16.b2.picc=34");
		}
		if(msgCount > 5)
		{
		  LCD_print("page16.b2.picc=71"); // when the trash can is full
		}
		int STT = 0;
		while(SIM.available())(SIM.read());
		if(msgCount != 0)
		{
			for(int iSMS=6; iSMS>0; iSMS--) //iSMS=20
			{
				sendCMGRcommand="AT+CMGR="+String(iSMS, DEC); SIM.println(sendCMGRcommand); //read incoming messages
				while(!SIM.available()); while(SIM.available())
				{
				  readCMGRcommand=SIM.readString();
				}
        Serial.print("Nội dung tin nhắn: "); Serial.println(readCMGRcommand);
				if((readCMGRcommand.indexOf("+CMGR:")!=-1) && (STT<(30)))
				{
					cmgrIndex = readCMGRcommand.indexOf("+CMGR:", startCMGR+1);
					cmgrNLindex = readCMGRcommand.indexOf("\n", cmgrIndex+1);
					inCMGR = readCMGRcommand.substring(cmgrIndex, cmgrNLindex-1);
					smsNLindex = readCMGRcommand.indexOf("\n", cmgrNLindex+1);
					inSMS = readCMGRcommand.substring(cmgrNLindex+1, smsNLindex-1);
					readSMS(inCMGR, inSMS, iSMS, STT);
          //  Serial.print("Tin nhắn: "); Serial.println(STT);
					STT=STT+5;
				}
				startCMGR=-1;
				cmgrIndex=cmgrNLindex=smsNLindex=0;
				inCMGR=inSMS=readCMGRcommand="";
			}			
		}
	}
	readCPMScommand=CPMSMsgCount="";
	startCPMS=-1;
	CPMS1comma=CPMS2comma=msgCount=0;
}

void readSMS(String readinCMGR, String readinSMS, int readiSMS ,int STT){
	int startComma=-1, firstColon, firstComma, secondComma, thirdComma;
	String smsSeqNum, smsStatus, smsBNumber, smsDateTime, actualSMS;
	String lcd_text;
	String startOfPage="page16.t", middleOfPage=".txt=\"";

	firstColon = readinCMGR.indexOf(":", startComma+1);
	firstComma = readinCMGR.indexOf(",", startComma+1);
	secondComma = readinCMGR.indexOf(",", firstComma+1);
	thirdComma = readinCMGR.indexOf(",", secondComma+1);
	smsSeqNum = String(readiSMS);
	smsStatus = readinCMGR.substring(firstColon+7, firstComma-1);
	smsBNumber = readinCMGR.substring(firstComma+2, secondComma-1);
	smsDateTime = readinCMGR.substring(thirdComma+2, readinCMGR.length()-4);//-1
  Serial.println(smsDateTime);
	actualSMS = readinSMS.substring(0, readinSMS.length());

	lcd_text = startOfPage + STT + middleOfPage + smsStatus  +"\"";LCD_print(lcd_text);STT++;//t0 - trạng thái đã đọc tin nhắn
	lcd_text = startOfPage + STT + middleOfPage + smsBNumber +"\"";LCD_print(lcd_text);STT++;//t1 - lấy số điện thoại nhắn đến
	lcd_text = startOfPage + STT + middleOfPage + smsSeqNum  +"\"";LCD_print(lcd_text);STT++;//t2 -  số thứ tự
	lcd_text = startOfPage + STT + middleOfPage + smsDateTime+"\"";LCD_print(lcd_text);STT++;//t3 - ngày tháng gửi tin
	lcd_text = startOfPage + STT + middleOfPage + actualSMS  +"\"";LCD_print(lcd_text); //     t4 - nội dung tin nhắn
	smsSeqNum=smsStatus=smsBNumber=smsDateTime=actualSMS=lcd_text="";
	firstColon=firstComma=secondComma=thirdComma=STT=0;
}

void delReadSMS(String inDelReadSMS){
	SIM.println(inDelReadSMS); //delete read message.delay(500);
  delay(500);
  smsComputation();
}

void delSMS(String indelSMS){ // delete all read messages
	SIM.println(indelSMS);
  delay(500);
  smsComputation();
}

void updateSMS(){ //update message
  smsComputation();
}
//***************************************************************************************//
void addFinger(String str){
 if(str=="add1"){
  //add fingerprint 1
  getFingerprintEnroll(1);
 }
 if(str=="add2"){
  //add fingerprint 2
  getFingerprintEnroll(2);
 }
 if(str=="add3"){
  //add fingerprint 3
  getFingerprintEnroll(3);
 }
 if(str=="add4"){
  //add fingerprint 4
  getFingerprintEnroll(4);
 }
 if(str=="add5"){
  //add fingerprint 5
  getFingerprintEnroll(5);
 }
}
void delFinger(String str){
 if(str=="del1"){
  //delete fingerprint 1
  deleteFingerprint(1);
 }
 if(str=="del2"){
  //delete fingerprint 2
  deleteFingerprint(2);
 }
 if(str=="del3"){
  //delete fingerprint 3
  deleteFingerprint(3);
 }
 if(str=="del4"){
  //delete fingerprint 4
  deleteFingerprint(4);
 }
 if(str=="del5"){
  //delete fingerprint 5
  deleteFingerprint(5);
 }
 if(str=="delall"){
  //delete all fingerprint
  for(int i=1;i<=5;i++){
    deleteFingerprint(i);
    delay(50);
  }
 }
}
// Subroutines for fingerprint sensor
uint8_t deleteFingerprint(uint8_t id) { // delete fingerprint by id
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Đã xóa!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Giao tiếp cảm biến lỗi");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Không thể xóa tại vị trí này");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Lỗi khi đọc ghi");
  } else {
    Serial.print("Lỗi không xác định: 0x"); Serial.println(p, HEX);
  }
  return p;
}

//***********************************************************************************************
// Program to add fingerprint
uint8_t getFingerprintEnroll(uint8_t id) { 
  int p = -1;
  String lcd_text="page26.t0.txt=\"Start!!\""; LCD_print(lcd_text);
//  lcd_text="page page26"; SIM.println(lcd_text); // send command to screen
  Serial.print("Hãy đặt ngón tay vào cảm biến #"); Serial.println(id);
  while (p != FINGERPRINT_OK) { //Enter fingerprint 1st time
    p = finger.getImage(); // add fingerprint to sensor
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Đã lưu vân tay");
      lcd_text="page26.t0.txt=\"Save Finger!!\""; LCD_print(lcd_text);
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi giao tiếp cảm biến");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Vân tay bị lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text); // send command to screen
      break;
    default:
      Serial.println("Lỗi không xác định");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      break;
    }
  }

  //Save fingerprint results for the first time
  p = finger.image2Tz(1); // Re-confirm fingerprint 1st time
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Valid fingerprint");
      bip();
      lcd_text="page26.t0.txt=\"Finger Success!!\""; LCD_print(lcd_text);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Messy fingerprints:))");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Fingerprints not visible");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Fingerprints not visible");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    default:
      Serial.println("Unknown error");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
  }


  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("Fingerprint ID Verification "); Serial.println(id);
//  lcd_text="vis b0,1"; LCD_print(lcd_text);
  p = -1;
  Serial.println("Put the same finger back");
  lcd_text="page26.t0.txt=\"Xac nhan van tay!!\""; LCD_print(lcd_text);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Đã lưu");
      lcd_text="page26.t0.txt=\"save!!\""; LCD_print(lcd_text);
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Fingerprint error");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      break;
    default:
      Serial.println("unknow error");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Chuyển đổi vân tay");
      lcd_text="page26.t0.txt=\"Ok!!\""; LCD_print(lcd_text);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Vân tay quá lộn xộn");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Giao tiếp lỗi");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Không thể tìm thấy");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Không thể tìm thấy");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
    default:
      Serial.println("Lỗi không xác định");
      lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
      return p;
  }

  // OK converted!
  Serial.print("Tạo vân tay cho #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Vân tay hợp lệ!");
    lcd_text="page26.t0.txt=\"Van tay hop le!!\""; LCD_print(lcd_text);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Giao tiếp lỗi");
    lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
    failMode();
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Vân tay không trùng khớp, hãy làm lại");
    lcd_text="page26.t0.txt=\"Van tay khong trung khop!!\""; LCD_print(lcd_text);
    failMode();
    return p;
  } else {
    Serial.println("Lỗi không xác định");
    lcd_text="page26.t0.txt=\"Finger Error!!\""; LCD_print(lcd_text);
    failMode();
    return p;
  }
  lcd_text="page26.t0.txt=\"SAVE!!\""; LCD_print(lcd_text);
  
  Serial.print("Lưu vân tay "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Đã lưu trữ vào cảm biến!"); // thêm vân tay thành công
    bip();bip();
    lcd_text="page26.t0.pco=8986"; LCD_print(lcd_text);
    lcd_text="page26.t0.txt=\"done!!\""; LCD_print(lcd_text);
    delay(1000);
    lcd_text="page page25"; LCD_print(lcd_text);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) 
  {
    Serial.println("Giao tiếp lỗi");
    failMode();
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) 
  {
    Serial.println("Không thể lưu vào vị trí này");
    failMode();
    return p;
  } else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Có lỗi trong khi ghi");
    failMode();
    return p;
  } else 
  {
    Serial.println("Lỗi không xác định");
    failMode();
    return p;
  }

  return true;
}
void failMode(){
  String lcd_text="page26.t0.txt=\"fail!!\""; LCD_print(lcd_text);
  lcd_text="vis b0,1"; LCD_print(lcd_text);
  lcd_text="vis bBack,1"; LCD_print(lcd_text);
}

//****************************************************************************************************
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Tìm thấy vân tay");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("Lỗi#00");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi#1");
      return p;
    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Lỗi#2");
      return p;
    default:
      Serial.println("Lỗi#3 không xác định");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("So sánh hình ảnh");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Lỗi#4");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Lỗi#5");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Lỗi#6");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Lỗi#7");
      return p;
    default:
      Serial.println("Lỗi#8");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Vân tay trùng khớp!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Lỗi#9");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Lỗi#10");
    return p;
  } else {
    Serial.println("Lỗi#11");
    return p;
  }

  Serial.print("Vân tay là ID #"); Serial.println(finger.fingerID);
  
  return finger.fingerID;
}


// void GetTimeDate(){ //nhận ngày giờ từ sim
// 	String getDay, getMonth, getYear, getHour, getMin, data_time, sendLCD, timeLCD, dateLCD;
// 	while(SIM.available())(SIM.read());
// 	String getTimeCommand="AT+CCLK?"; SIM.println(getTimeCommand);
//   Serial.println("Lenh lay ngay gio:");
// 	while(!SIM.available()); 
// 	while(SIM.available())+
// 	{
// 	  data_time=SIM.readString();
// 	}
// 	if(data_time.indexOf("+CCLK:") != -1) // lấy chuỗi ngày tháng và xử lý tách chuỗi
// 	{
//     getHour=data_time.substring(28, 30);
//     getMin=data_time.substring(31, 33);

//     getYear=data_time.substring(19, 21);
//     getMonth=data_time.substring(22, 24);
//     getDay=data_time.substring(25, 27);
//     int month_int = getMonth.toInt();

//     timeLCD=getHour+":"+getMin;
//     dateLCD=getDay+" "+Month[month_int]+", 20"+getYear;
//     sendLCD="page19.t0.txt=\""+timeLCD+"\"";LCD_print(sendLCD);
//     sendLCD="page19.t1.txt=\""+dateLCD+"\"";LCD_print(sendLCD);
//     sendLCD="page19.timeLCD.txt=\""+timeLCD+"\"";LCD_print(sendLCD);
//     sendLCD="page19.dateLCD.txt=\""+dateLCD+"\"";LCD_print(sendLCD);
//     Serial.println(timeLCD);
//     Serial.println(dateLCD);
// 	}
// 	data_time=getTimeCommand=timeLCD=dateLCD="";
// }
void LCD_print(String lcd_data){
//Send script to communicate with display
  for(int i=0; i < lcd_data.length(); i++)
  {
    LCD.write(lcd_data[i]);
  }
  LCD.write(0xff);
  LCD.write(0xff);
  LCD.write(0xff);
}
