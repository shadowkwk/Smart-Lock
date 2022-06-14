#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include<SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Keypad.h>
#include <Password.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define RST_PIN      8
#define SS_PIN       9
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define GSM_RX  A8
#define GSM_TX  A9

int red_light_pin = A13;
int green_light_pin = A14;
int blue_light_pin = A15;

String number = "+85265712597";
String RS;

SoftwareSerial yourSerial(11, 10); //RX, TX
SoftwareSerial mySerial(13, 12); //RX, TX For ESP8266 
SoftwareSerial sim(GSM_RX, GSM_TX);

const int pingPin = 3; // Trigger Pin of Ultrasonic Sensor
const int echoPin = 2; // Echo Pin of Ultrasonic Sensor

const byte ROWS = 4; //four rows
const byte COLS = 4; //3 columns
Password password = Password( "123ABC" );
volatile boolean pressed = false;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};

byte rowPins[ROWS] = {40, 38, 36, 34}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {32, 30, 28, 26}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int rxpointer = 0;
char buf[100];
int temp;
int sec = 0;
int address = 0;
int cuser = 1000;
boolean check = false;

struct RFIDTag {
  byte uid[4];
  char *name;
};


struct RFIDTag tags[] = {
  {{0, 0, 0, 0}, "User 1"},
  {{0, 0, 0, 0}, "User 2"},
  {{0, 0, 0, 0}, "User 3"},
  {{0, 0, 0, 0}, "User 4"},
};

//writeTag function
void writeTag(int k, int addres) {
  if (mySerial.available()) {
    buf[rxpointer] = mySerial.read();
    if (buf[rxpointer] < 15) {
      temp = 0;
      tags[k].uid[0] = 0;
      tags[k].uid[1] = 0;
      tags[k].uid[2] = 0;
      tags[k].uid[3] = 0;
      for (int j = 0; j < rxpointer; j++) {
        if (buf[j] != ',') {
          tags[k].uid[temp] *= 10;
          tags[k].uid[temp] += (buf[j] - 0x30);
        }
        else temp++;
      }
      rxpointer = 0;
      check = true;
    }
    rxpointer++;
  }
  for (int i = 0; i < 4; i++) {
    EEPROM.write(addres++, tags[k].uid[i]);
    //Serial.print(tags[k].uid[i]);
    //if (i < 3) Serial.print(", ");
  };
}

byte totalTags = sizeof(tags) / sizeof(RFIDTag);  // 計算結構資料筆數，結果為3。

MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件

byte personid = 1000;
int states = 0;

int errorTimes = 0;

void setup() {
  keypad.addEventListener(keypadEvent);
  pinMode(18, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(18), isr, FALLING );
  Serial.begin(115200);
  sim.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  while (!Serial) {
    ;
  }
  Serial.println("hardware serial!");
  yourSerial.begin(115200);
  mySerial.begin(115200);
  mySerial.println("software seria");
  //yourSerial.begin(115200);
  //yourSerial.println("Cam starting");
  Serial.print("size of RFIDTag:");
  Serial.println(sizeof(RFIDTag));
  Serial.print("size of tag:");
  Serial.println(sizeof(tags));
  Serial.println("RFID reader is ready!");
  SPI.begin();
  mfrc522.PCD_Init();       // 初始化MFRC522讀卡機模組
  for (int k = 0; k < 4; k++) {
    for (int j = 0; j < 4; j++)
    {
      tags[k].uid[j] = EEPROM.read(address++);
    }
  }
  address = 0;
  mainMenu();
}
/*-------------------------------------------------------- */
void loop() {
  //if mySerial find "edit",entry edit mode
  while (mySerial.find("edit")) {
    Serial.println("Please Enter ID");
    int readValue = 10;
    while (readValue < 47 || readValue > 58) { //Could only entry 0-3
      readValue = mySerial.read();
    }
    Serial.println(readValue);
    while (mySerial.read() >= 0);   //Clean mySerial cache(must)
    while (yourSerial.read() >= 0);
    Serial.println("ok");

    switch (readValue) {
      case 48:
        while (check == false) {
          writeTag(0, 0);
        }
        check = false;
        rxpointer = 0;
        break;
      case 49:
        while (check == false) {
          writeTag(1, 4);
        }
        check = false;
        rxpointer = 0;
        break;
      case 50:
        while (check == false) {
          writeTag(2, 8);
        }
        check = false;
        rxpointer = 0;
        break;
      case 51:
        while (check == false) {
          writeTag(3, 12);
        }
        check = false;
        rxpointer = 0;
        break;
    }
    for (int k = 0; k < 4; k++) {
      for (int i = 0; i < 4; i++) {
        Serial.print(tags[k].uid[i]);
        if (i < 3) Serial.print(", ");
      };
      Serial.println();
    }
    Serial.println("out");
  }

  if (states == 1)
  {
    RGB_color(0, 255, 0); // Green
    //delay(1000);        //open door
    //RGB_color(0, 0, 0);
    //    display.clearDisplay();
    //    display.setTextSize(1);          // text size
    //    display.setTextColor(WHITE);
    //    display.setCursor(30, 30);        // position to display
    //    display.println("RFID Correct");
    //    display.display();
    delay(2000);
    states = 2;
    mainMenu();
  }
  if (states == 2)
  {
    long duration, cm;
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pingPin, LOW);
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);
    cm = microsecondsToCentimeters(duration);
    Serial.print(cm);
    Serial.print("cm");
    Serial.println();
    delay(1000);
    sec = sec + 1;
    if (sec == 1 || sec == 2) {
      display.clearDisplay();
      display.setTextSize(1);          // text size
      display.setTextColor(WHITE);
      display.setCursor(0, 10);        // position to display
      display.println("*********************");
      display.setTextSize(4);
      display.setCursor(55, 30);        // position to display
      display.println(6-sec);
      display.display();
    }
    if (cm < 10)
    {
      RGB_color(0, 0, 0);           //close door
      states = 3;
      sec = 0;
      Serial.println("User has entered");
    }
    if (sec > 2 && sec < 6)
    {
      display.clearDisplay();
      display.setTextSize(1);          // text size
      display.setTextColor(WHITE);
      display.setCursor(0, 10);        // position to display
      display.println("*********************");
      display.setTextSize(4);
      display.setCursor(55, 30);        // position to display
      display.println(6-sec);
      display.display();
      RGB_color(255, 255, 0);
      delay(500);
      RGB_color(0, 0, 0);
      delay(50);
    }
    if (sec > 6) {
      mainMenu();
      RGB_color(0, 0, 0);
      sec = 0;
      states = 0;
      Serial.println("Out of time,User has not entered");
    }
  }
  if (states == 3)                         //display user
  {
    if (personid != 1000) {
      states = 0;
    }
    mainMenu();
  }

  // 確認是否有新卡片
  if (states == 0)
  {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
      byte idSize = mfrc522.uid.size;   // 取得UID的長度
      bool foundTag = false;    // 是否找到紀錄中的標籤，預設為「否」。
      for (byte i = 0; i < totalTags; i++) {
        if (memcmp(tags[i].uid, id, idSize) == 0) {  // 比對陣列資料值

          while (mySerial.read() >= 0);
          mySerial.write(49);

          Serial.println(tags[i].name);  // 顯示標籤的名稱
          personid = i;

          foundTag = true;  // 設定成「找到標籤了！」
          display.clearDisplay();
          display.setTextSize(1);          // text size
          display.setTextColor(WHITE);
          display.setCursor(0, 10);        // position to display
          display.println("*********************");
          display.setCursor(30, 30);        // position to display
          display.println("RFID Correct");
          errorTimes=0;
          display.display();
          states = 1;

          break;  // 退出for迴圈
        }
      }


      if (!foundTag) {  // 若掃描到紀錄之外的標籤，則顯示"Wrong card!"。
        while (mySerial.read() >= 0);
        mySerial.write(48);
        yourSerial.write(48);
        //yourSerial.write(48);
        Serial.println("Wrong card!");
        RGB_color(255, 0, 0); // RED
        delay(1000);
        RGB_color(0, 0, 0);
        display.clearDisplay();
        display.setTextSize(1);          // text size
        display.setTextColor(WHITE);
        display.setCursor(0, 10);        // position to display
        display.println("*********************");
        display.setCursor(30, 30);        // position to display
        display.println("RFID Wrong");
        display.display();
        delay(2000);
        errorTimes++;
        Serial.print("Error times now: ");
        Serial.println(errorTimes);
        mainMenu();

        if (errorTimes >= 5) {
           sms();
          Serial.println("SMS has been sent to the user since five errors were detected");
          errorTimes = 0;
        }
      }
    }

    mfrc522.PICC_HaltA();  // 讓卡片進入停止模式
  }

  if (pressed) {
    Serial.println("Please enter password to open the door! Press * after entering");
    Serial.print("Pressed: ");
    char key;
    display.clearDisplay();
    display.setTextSize(1);          // text size
    display.setTextColor(WHITE);
    display.setCursor(0, 10);        // position to display
    display.println("*********************");
    display.setCursor(0, 30);        // position to display
    display.println("Enter password!");
    display.setCursor(0, 50);
    display.display();
    do {
      key = keypad.waitForKey();
      if (key != '*') {
        display.print(key);
        display.display();
      }
    }
    while (key != '*');
    pressed = false;
  }
}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
{
  digitalWrite(red_light_pin, red_light_value);
  digitalWrite(green_light_pin, green_light_value);
  digitalWrite(blue_light_pin, blue_light_value);
}

long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}
long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void mainMenu()
{
  display.clearDisplay();
  display.setTextSize(1);          // text size
  display.setTextColor(WHITE);
  display.setCursor(0, 10);        // position to display
  display.println("*********************");
  display.setCursor(0, 25);        // position to display
  display.println("Tag Your RFID Card /");
  display.setCursor(0, 35);        // position to display
  display.println("Enter a password");
  display.setCursor(0, 50);        // position to display
  display.print("Error count: ");
  display.println(errorTimes);
  display.display();
}

void sms()
{
  Serial.println ("Sending Message");
  //Serial.println("AT+CMGF=1");
  sim.println("AT+CMGF=1");
  RS = sim.readString();
  Serial.println(RS);

  delay(1000);
  Serial.println ("Set SMS Number");
  //Serial.println("AT+CMGS=\"" + number + "\"\r");
  sim.println("AT+CMGS=\"" + number + "\"\r");
  RS = sim.readString();
  Serial.println(RS);


  delay(1000);
  String SMS = "Warning. 5 times attempt reached \x1A";

  //Serial.println(SMS);
  sim.println(SMS);

  delay(100);
  sim.print((char)26);// ASCII code of CTRL+Z

  RS = sim.readString();
  Serial.println(RS);
  //sim.print(sim);
  delay(1000);

}

void call() {
  sim.print (F("ATD"));
  sim.print (number);
  sim.print (F(";\r\n"));
}

void isr() {
  pressed = true;
}

void keypadEvent(KeypadEvent eKey) {
  switch (keypad.getState()) {
    case PRESSED:
      switch (eKey) {
        case '*':
          checkPassword();
          break;
        default:
          password.append(eKey);
          Serial.print(eKey);
      }
  }
}

void checkPassword() {
  Serial.println();
  boolean correct = false;
  if (password.evaluate()) {
    correct = true;
    mySerial.write(50);
    Serial.println("Correct password!");
    errorTimes=0;
    password.reset();
    display.clearDisplay();
    display.setTextSize(1);          // text size
    display.setTextColor(WHITE);
    display.setCursor(0, 10);        // position to display
    display.println("*********************");
    display.setCursor(0, 30);        // position to display
    display.println("Password Correct");
    display.display();
    //Add code to run if it works
  } else {
    mySerial.write(51);
    yourSerial.write(51);
    Serial.println("Wrong password!");
    password.reset();
    display.clearDisplay();
    display.setTextSize(1);          // text size
    display.setTextColor(WHITE);
    display.setCursor(0, 10);        // position to display
    display.println("*********************");
    display.setCursor(0, 30);        // position to display  delay(4000);

    display.println("Password Incorrect");
    display.display();
    RGB_color(255, 0, 0); // RED
    errorTimes++;
    Serial.print("Error times now: ");
    Serial.println(errorTimes);
    if (errorTimes >= 5) {
       sms();
      Serial.println("SMS has been sent to the use since five errors were detected");
      errorTimes = 0;
    }

    //add code to run if it did not work
  }
  RGB_color(0, 0, 0);
  mainMenu();
  if (correct)
  {
    correct = false;
    states = 1;
  }
}
