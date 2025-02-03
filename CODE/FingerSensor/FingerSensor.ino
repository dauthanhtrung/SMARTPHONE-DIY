#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h> // thu vien van tay
#include <Wire.h>   //thu vien giao tiep i2c
#include <LiquidCrystal_I2C.h> //Thu vien lcd
#include <Keypad.h>
#include <EEPROM.h>


#define PIN_TX    A2
#define PIN_RX    A3
#define BAUDRATE  9600
void isSMSread();

#define MESSAGE_LENGTH 6
char message[MESSAGE_LENGTH];
//String message;
int messageIndex = 0;
char phone[16];
char datetime[24];
bool check  = 0;

#define F_TX 4
#define F_RX 5
//#define S_TX 10
//#define S_RX 11
#define buzz 3
//#define button 12
#define relay  2

//Define mode nhan van tay
#define MODE_FIRST_FINGER 1
#define MODE_SECOND_FINGER 2
#define MODE_ID_FINGER_ADD 3
#define MODE_ID_FINGER_CLEAR 4

char p = FINGERPRINT_PACKETRECIEVEERR; //Khai bao bien p = -1;
unsigned char MODE = MODE_ID_FINGER_ADD; //Mode = 3

unsigned char id = 0;
unsigned char index_t = 0;
unsigned char error_in = 0;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {10, 11, 12, 13}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9 }; //connect to the column pinouts of the keypad

int addr = 0;
int cnt = 0;
char password[6] = "22222";
char pass_def[6]  = "12345";
char mode_changePass[6] = "*#01#";
char mode_resetPass[6] = "*#02#";
char mode_hardReset[6] = "*#03#";
char mode_addFinger[6] = "*101#";
char mode_delFinger[6] = "*102#";
char mode_delAllFinger[6] = "*103#";
char mode_sms[6] = "*####";
char data_input[6];
char new_pass1[6];
char new_pass2[6];
char message_SMS[6] = "ON";
unsigned char in_num = 0, error_pass = 0, error_finger = 0, isMode = 0;
char buffer[6];

//char phoneNum[20] = "0908732586";
//char WarningMess[64] = "Canh bao co nguoi xam nhap";

//SoftwareSerial SimSerial(11, 10);
SoftwareSerial mySerial(4, 5);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void clear_data_input() //xoa gia tri nhap vao hien tai
{
  int i = 0;
  for (i = 0; i < 6; i++)
  {
    data_input[i] = '\0';
  }
}

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  lcd.init();
  lcd.backlight();
  lcd.print("   SYSTEM INIT   ");
  //  pinMode(button, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(buzz, OUTPUT);
  digitalWrite(buzz, LOW);
  digitalWrite(relay, HIGH);
  readEpprom();
  delay(100);
  lcd.clear();
}

void loop() {

  lcd.setCursor(1, 0);
  lcd.print("Enter Password");
  checkPass();
  checkFingerPrinter();
  while (index_t == 1)
  {
    changePass();
  }
  while (index_t == 2)
  {
    resetPass();
  }
  while (index_t == 3)
  {
    openDoor();
    error_pass = 0;
    error_finger = 0;
    if (check == 1) resetBoard();
  }
  while (index_t == 4)
  {
    error();
    error_pass = 0;
    error_finger = 0;
  }
  while (index_t == 5)
  {
    addFingerPrinter();
  }
  while (index_t == 6)
  {
    deleteFingerPrinter();
  }
  while (index_t == 7)
  {
    clear_database();
  }
  while (index_t == 8)
  {

    isSMSread();
    if (compareData(message, message_SMS)) {
      index_t = 3;
      //  Serial.println(message);
      message[0] = 'a';
      check = 1;
    }
    if (cnt == 5) {
     resetBoard();
    }
    cnt  ++;
  }

}
void isSMSread() {

  //lcd.setCursor(1, 0);
  // lcd.print("please send sms");
  GPRS gprs(PIN_TX, PIN_RX, BAUDRATE); //RX,TX,PWR,BaudRate
  gprs.checkPowerUp();
  while (!gprs.init()) {
    Serial.print("init error\r\n");
    delay(1000);
  }
  delay(1000);
  Serial.println("Init Success, please send SMS message to me!");
  messageIndex = gprs.isSMSunread();
  if (messageIndex > 0) { //At least, there is one UNREAD SMS
    gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);


    //In order not to full SIM Memory, is better to delete it
    gprs.deleteSMS(messageIndex);

    Serial.print("From number: ");
    Serial.println(phone);
    Serial.print("Datetime: ");
    Serial.println(datetime);
    Serial.print("Recieved Message: ");
    Serial.println(message);
  }
  // Serial.println(message);

}
void resetBoard()
{
  asm volatile ("jmp 0");
}
void checkFingerPrinter()
{
  p = finger.getImage();
  if (p == FINGERPRINT_OK)
  {
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK)
    {
      p = finger.fingerFastSearch();
      if (p == FINGERPRINT_OK)
      {
        //Serial.print(finger.fingerID);
        notification(1);
        index_t = 3;
      }
      else
      {
        lcd.setCursor(0, 0);
        lcd.print("Not found finger");
        // notification(2);
        if (error_finger == 2) {
          index_t = 4;
          notification(2);
        } else
        {
          error_finger++;
        }
        delay(2000);
        lcd.clear();
      }
    }
  }
}

unsigned char numberInput() {
  char number[5];
  char count_i = 0;
  while (count_i < 2) {
    char key = keypad.getKey();
    if (key && key != 'A' && key != 'B' && key != 'C' && key != 'D' && key != '*' && key != '#' ) {
      lcd.setCursor(10 + count_i, 1);
      lcd.print(key);
      number[count_i] = key;
      count_i++;
    }
  }
  return (number[0] - '0') * 10 + (number[1] - '0');
}

void addFingerPrinter()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ADD NEW FINGERPRINT");
  switch (MODE) {
    case (MODE_ID_FINGER_ADD):
      Serial.print("ADD_IN");
      lcd.setCursor(0, 1);
      lcd.print("Input Id: ");
      id = numberInput();
      if (id == 0) {// ID #0 not allowed, try again!
        lcd.setCursor(3, 1);
        lcd.print("ID ERROR");
        break;
      }
      delay(2000);
      MODE = MODE_FIRST_FINGER;
      break;
    case (MODE_FIRST_FINGER):
      lcd.setCursor(0, 1);
      lcd.print("   Put Finger    ");
      p = finger.getImage();
      if (p == FINGERPRINT_OK)
      {
        p = finger.image2Tz(1);
        if (p == FINGERPRINT_OK)
        {
          lcd.setCursor(0, 1);
          lcd.print("   Put Again    ");
          delay(500);
          MODE = MODE_SECOND_FINGER;
        }
      }
      if (p == FINGERPRINT_NOFINGER)
      {
        Serial.println("==========> No finger ");
        delay(100);
      }
      if (p == FINGERPRINT_PACKETRECIEVEERR)
      {
        lcd.setCursor(0, 1);
        lcd.print("Communication error");
        delay(1000);
      }
      break;
    case MODE_SECOND_FINGER:
      p = finger.getImage();
      if (p == FINGERPRINT_OK)
      {
        p = finger.image2Tz(2);
        if (p == FINGERPRINT_OK)
        {
          p = finger.createModel();
          if (p == FINGERPRINT_OK)
          {
            p = finger.storeModel(id);
            if (p == FINGERPRINT_OK)
            {
              Serial.print("OK");
              lcd.setCursor(0, 1);
              lcd.print("Add Finger Done");
              notification(1);
              delay(2000);
              index_t = 0;
              Serial.print("ADD_OUT");
              lcd.clear();
              MODE = MODE_ID_FINGER_ADD;
            }
          }
          else if (p == FINGERPRINT_ENROLLMISMATCH)
          {
            notification(2);
            lcd.setCursor(0, 1);
            lcd.print("   Mismatched   ");
            //
            if (error_in == 2)
            {
              index_t = 0;
              Serial.print("ADD_OUT");
              MODE = MODE_SECOND_FINGER;
            }
            error_in++;
            delay(2000);
            lcd.clear();
            MODE = MODE_SECOND_FINGER;
          }
          else
          {

          }
          id = 0;
        }
      }
      break;
  }
}

void deleteFingerPrinter()
{
  char buffDisp[20];
  lcd.setCursor(0, 0);
  lcd.print("  DELETE FINGER   ");
  Serial.print("DEL_IN");
  lcd.setCursor(0, 1);
  lcd.print("Input ID: ");
  id = numberInput();
  delay(2000);
  if (id == 0) {// ID #0 not allowed, try again!
    lcd.setCursor(3, 1);
    lcd.print("ID ERROR");
    return;
  }
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK)
  {
    sprintf(buffDisp, "Clear id:%d Done", id);
    lcd.setCursor(0, 1);
    lcd.print(buffDisp);
    //   Serial.print("DEL_OUT");
    notification(1);
    delay(2000);
    lcd.clear();
    index_t = 0;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    sprintf(buffDisp, "Invalid id: %d   ", id);
    lcd.setCursor(0, 1);
    lcd.print(buffDisp);
    //Serial.print("DEL_OUT");
    delay(2000);
    lcd.clear();
    index_t = 0;
  }
  else
  {
    ///Serial.print("DEL_OUT");
  }
}

void clear_database()
{
  char key = keypad.getKey();
  lcd.setCursor(3 , 0);
  lcd.print("CLEAR DATA ?");
  if (key == '*')
  {
    isMode = 0;
  }
  if (key == '#') {
    isMode = 1;
  }
  if (isMode == 0)
  {
    lcd.setCursor(0, 1);
    lcd.print("> Yes      No  ");
  }
  if (isMode == 1)
  {
    lcd.setCursor(0, 1);
    lcd.print("  Yes    > No  ");
  }
  if (key == '0' && isMode == 0)
  {
    finger.emptyDatabase();
    lcd.setCursor(0, 1);
    lcd.print("  Clear done  ");
    delay(2000);
    index_t = 0;
    lcd.clear();
  }
  if (key == '0' && isMode == 1)
  {
    lcd.clear();
    index_t = 0;
  }
}
/*********************************************************************************/
/*
    Password with keypad
  /
  /*********************************************************************************/

unsigned char isBufferdata(char data[]) //Kiem tra buffer da co gia tri chua
{
  unsigned char i = 0;
  for (i = 0; i < 5 ; i++)
  {
    if (data[i] == '\0')
    {
      return 0;
    }
  }
  return 1;
}

bool compareData(char data1[], char data2[]) //Kiem tra 2 cai buffer co giong nhau hay khong
{
  unsigned char i = 0;
  for (i = 0; i < 5; i++)
  {
    if (data1[i] != data2[i])
    {
      return false;
    }
  }
  return true;
}

void insertData(char data1[], char data2[]) //Gan buffer 2 cho buffer 1
{
  unsigned char i = 0;
  for (i = 0; i < 5; i++)
  {
    data1[i] = data2[i];
  }
}

void getData() //Nhan buffer tu ban phim
{
  char key = keypad.getKey();  //Doc gia tri ban phim
  if (key)
  {
    //Serial.println("key != 0");
    if (in_num == 0)
    {
      data_input[0] = key;
      lcd.setCursor(5, 1);
      lcd.print(data_input[0]);
      delay(200);
      lcd.setCursor(5, 1);
      lcd.print("*");
    }
    if (in_num == 1) {
      data_input[1] = key;
      lcd.setCursor(6, 1);
      lcd.print(data_input[1]);
      delay(200);
      lcd.setCursor(6, 1);
      lcd.print("*");
    }
    if (in_num == 2) {
      data_input[2] = key;
      lcd.setCursor(7, 1);
      lcd.print(data_input[2]);
      delay(200);
      lcd.setCursor(7, 1);
      lcd.print("*");
    }
    if (in_num == 3) {
      data_input[3] = key;
      lcd.setCursor(8, 1);
      lcd.print(data_input[3]);
      delay(200);
      lcd.setCursor(8, 1);
      lcd.print("*");
    }
    if (in_num == 4) {
      data_input[4] = key;
      lcd.setCursor(9, 1);
      lcd.print(data_input[4]);
      delay(200);
      lcd.setCursor(9, 1);
      lcd.print("*");
    }
    if (in_num == 4)
    {
      //Serial.println(data_input);
      in_num = 0;
    }
    else {
      in_num ++;
    }
  }
}

void checkPass() //kiem tra password
{
  getData();
  if (isBufferdata(data_input))
  {
    if (compareData(data_input, password)) //Dung pass
    {
      //Serial.print("Dung pass");
      notification(1);
      lcd.clear();
      clear_data_input();
      index_t = 3;
    }
    else if (compareData(data_input, mode_changePass))
    {
      //Serial.print("mode_changePass");
      lcd.clear();
      clear_data_input();
      index_t = 1;
    }
    else if (compareData(data_input, mode_resetPass))
    {
      //Serial.print("mode_resetPass");
      lcd.clear();
      clear_data_input();
      index_t = 2;
    }
    else if (compareData(data_input, mode_hardReset))
    {
      lcd.setCursor(0, 0);
      lcd.print("---HardReset---");
      writeEpprom(pass_def);
      insertData(password, pass_def);
      clear_data_input();
      delay(2000);
      lcd.clear();
      index_t = 0;
    }
    else if (compareData(data_input, mode_addFinger)) {
      lcd.clear();
      clear_data_input();
      index_t = 5;
    } else if (compareData(data_input, mode_delFinger)) {
      lcd.clear();
      clear_data_input();
      index_t = 6;
    }
    else if (compareData(data_input, mode_delAllFinger)) {
      lcd.clear();
      clear_data_input();
      index_t = 7;
    }
    else if (compareData(data_input, mode_sms)) {
      lcd.clear();
      clear_data_input();
      index_t = 8;
    }
    else
    {
      if (error_pass == 2)
      {
        notification(3);
        clear_data_input();
        lcd.clear();
        index_t = 4;
      }
      //Serial.print("Error");
      notification(2);
      lcd.setCursor(1, 1);
      lcd.print("               ");
      clear_data_input();
      error_pass++;
    }
  }
}
void changePass() //Thay doi pass
{
  lcd.setCursor(0, 0);
  lcd.print("-- Change Pass --");
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("--- New Pass ---");
  while (1)
  {
    getData();
    if (isBufferdata(data_input))
    {
      insertData(new_pass1, data_input);
      //Serial.println(new_pass1);
      clear_data_input();
      break;
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("---- AGAIN ----");
  while (1)
  {
    getData();
    if (isBufferdata(data_input))
    {
      insertData(new_pass2, data_input);
      //Serial.println(new_pass2);
      clear_data_input();
      break;
    }
  }
  delay(1000);
  if (compareData(new_pass1, new_pass2))
  {
    lcd.clear();
    //Serial.println("Success");
    lcd.setCursor(0, 0);
    lcd.print("--- Success ---");
    notification(1);
    delay(1000);
    writeEpprom(new_pass2);
    insertData(password, new_pass2);
    lcd.clear();
    index_t = 0;
  }
  else
  {
    lcd.clear();
    //Serial.println("miss");
    lcd.setCursor(0, 0);
    lcd.print("-- Mismatched --");
    notification(3);
    delay(1000);
    lcd.clear();
    index_t = 0;
  }
}

void resetPass()
{
  unsigned char choise = 0;
  //Serial.println("Pass reset");
  lcd.setCursor(0, 0);
  lcd.print("---Reset Pass---");
  getData();
  if (isBufferdata(data_input))
  {
    if (compareData(data_input, password))
    {
      lcd.clear();
      clear_data_input();
      while (1)
      {
        lcd.setCursor(0, 0);
        lcd.print("---Reset Pass---");
        char key = keypad.getKey();
        if (choise == 0)
        {
          lcd.setCursor(0, 1);
          lcd.print(">");
          lcd.setCursor(2, 1);
          lcd.print("YES");
          lcd.setCursor(9, 1);
          lcd.print(" ");
          lcd.setCursor(11, 1);
          lcd.print("NO");
        }
        if (choise == 1)
        {
          lcd.setCursor(0, 1);
          lcd.print(" ");
          lcd.setCursor(2, 1);
          lcd.print("YES");
          lcd.setCursor(9, 1);
          lcd.print(">");
          lcd.setCursor(11, 1);
          lcd.print("NO")   ;
        }
        if (key == '*')
        {
          if (choise == 1)
          {
            choise = 0;
          }
          else
          {
            choise ++;
          }
        }
        if (key == '#' && choise == 0)
        {
          lcd.clear();
          delay(1000);
          writeEpprom(pass_def);
          insertData(password, pass_def);
          lcd.setCursor(0, 0);
          lcd.print("---Reset ok---");
          notification(1);
          delay(1000);
          lcd.clear();
          break;
        }
        if (key == '#' && choise == 1)
        {
          lcd.clear();
          break;
        }
      }
      index_t = 0;
    }
    else {
      notification(1);
      index_t = 0;
      lcd.clear();
    }


  }

}
void notification(char select)
{
  unsigned char i = 0;
  if (select == 1)
  {
    for (i = 0; i < 2; i++)
    {
      digitalWrite(buzz, HIGH);
      delay(50);
      digitalWrite(buzz, LOW);
      delay(50);
    }
  }
  if (select == 2)
  {
    for (i = 0; i < 3; i++)
    {
      digitalWrite(buzz, HIGH);
      delay(50);
      digitalWrite(buzz, LOW);
      delay(50);
    }
  }
  if (select == 3)
  {
    for (i = 0; i < 4; i++)
    {
      digitalWrite(buzz, HIGH);
      delay(50);
      digitalWrite(buzz, LOW);
      delay(50);
    }
  }
}

void writeEpprom(char data[])
{
  unsigned char i = 0;
  for (i = 0; i < 5; i++)
  {
    EEPROM.write(i, data[i]);
  }
}
void readEpprom()
{
  unsigned char i = 0;
  for (i = 0; i < 5; i++)
  {
    password[i] = EEPROM.read(i);
  }
}

void openDoor()
{
  //Serial.println("Open The Door");
  unsigned char pos;
  lcd.setCursor(0, 0);
  lcd.print("-OPEN THE DOOR-");
  delay(1000);
  digitalWrite(relay, LOW);
  delay(5000);
  digitalWrite(relay, HIGH);
  Serial.print("C");
  lcd.clear();
  index_t = 0;
}

void error()
{
  unsigned char minute = 1;
  unsigned char i = 30;
  char buff[3];
  // Serial.println("Send SMS");
  //SendSMS(phoneNum, WarningMess);
  lcd.setCursor(1, 0);
  lcd.print("Try again for");
  while (i > 0)
  {
    if (i == 1 && minute > 0)
    {
      minute --;
      i = 59;
    }
    if (i == 1 && minute == 0)
    {
      break;
    }
    sprintf(buff, "%.2d", i);
    lcd.setCursor(5, 1);
    lcd.print(minute);
    lcd.setCursor(6, 1);
    lcd.print(":");
    lcd.setCursor(7, 1);
    lcd.print(buff);
    i--;
    digitalWrite(buzz, HIGH);
    delay(300);
    digitalWrite(buzz, LOW);
    delay(200);
    digitalWrite(buzz, HIGH);
    delay(300);
    digitalWrite(buzz, LOW);
    delay(100);
  }
  Serial.print("I");
  index_t = 0;
  lcd.clear();
}
