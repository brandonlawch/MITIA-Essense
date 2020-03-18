/*
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

//lcd______________________________________________________________________
#define I2C_ADDR 0x27  // Add your address here.
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
//RFID_____________________________________________________________________
constexpr uint8_t RST_PIN = 9;
constexpr uint8_t SS_PIN = 10;
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte nuidPICC[4];

byte tag[6][4] = {{64, 217, 112, 139}, {188, 222, 78, 131}, {131, 171, 29, 46}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
String tagname[6] = {"keys", "phone", "wallet", "custom1", "custom2", "custom3"};
int tagfirst[6] = {1, 1, 1, 1, 1, 1};
int counter = 0;
int pointer = 0;
int total = 3;
int all = false;
int addvalid = 0;
int activate = 2;
int addbutton = 3;
int buzz = 4;
int led = 8;

void setup() {
  Serial.begin(9600);
  pinMode(activate, INPUT);
  pinMode(addbutton, INPUT);
  digitalWrite(addbutton, HIGH);
  pinMode(led, OUTPUT);
  pinMode(buzz, OUTPUT);
  //lcd________________________________________________
  lcd.begin (20, 4);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home();
  lcd.clear();
  //RFID_______________________________________________
  SPI.begin();
  rfid.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if (digitalRead(activate) == LOW) {
    check();
  }
  if (digitalRead(addbutton) == LOW) {
    delay(400);
    add();
  }
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("scanning...");
  Serial.println("scanning...");
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    return;
  }
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
  for (int i = 0; i < 6; i++) {
    if (nuidPICC[0] == tag[i][0] && nuidPICC[1] == tag[i][1] && nuidPICC[2] == tag[i][2] && nuidPICC[3] == tag[i][3] && tagfirst[i] == 1) {
      digitalWrite(led, HIGH);
      delay(200);
      digitalWrite(led, LOW);
      tagfirst[i] = 0;
      counter = counter + 1;
      break;
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  if (counter == total) {
    all = true;
  }
}

void check() {
  if (all == true) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("All good to go!");
    Serial.println("All good to go!");
    for (int i = 0; i < 6; i++) {
      tagfirst[i] = 1;
    }
    delay(3000);
    lcd.clear();
    counter = 0;
    all = false;
    return;
  }
  if (all == false) {
    digitalWrite(led, HIGH);
    digitalWrite(buzz, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You left something:");
    Serial.println("You left something:");
    for (int i = 0, u = 1, j = 1; i < 6; i++) {
      if (tagfirst[i] == 1 && tag[i][0] != 0) {
        if ((u % 2) == 1) {
          lcd.setCursor(1, j);
          lcd.print(tagname[i]);
          Serial.println(tagname[i]);
          u = u + 1;
        } else if ((u % 2) == 0) {
          lcd.setCursor(10, j);
          lcd.print(tagname[i]);
          Serial.println(tagname[i]);
          u = u + 1;
          j = j + 1;
        }
      }
    }
    for (int i = 0; i < 6; i++) {
      tagfirst[i] = 1;
    }
    delay(5000);
    digitalWrite(led, LOW);
    digitalWrite(buzz, LOW);
    lcd.clear();
    counter = 0;
    all = false;
    return;
  }
}

void add() {
  lcd.clear();
  lcd.setCursor(0, 1);
  if (total == 6) {
    lcd.print("tag limit reached");
    Serial.println("tag limit reached");
    digitalWrite(led, HIGH);
    delay(300);
    digitalWrite(led, LOW);
    delay(300);
    digitalWrite(led, HIGH);
    delay(300);
    digitalWrite(led, LOW);
    delay(3000);
    return;
  }
  lcd.print("tap tag on reader");
  Serial.println("tap tag on reader");
  while ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) {
    if (digitalRead(addbutton) == LOW) {
      delay(200);
      return;
    }
  }
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    return;
  }
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
  for (int i = 0; i < 6; i++) {
    if (nuidPICC[0] == tag[i][0] && nuidPICC[1] == tag[i][1] && nuidPICC[2] == tag[i][2] && nuidPICC[3] == tag[i][3]) {
      addvalid = 0;
      break;
    }
    addvalid = 1;
  }
  Serial.println(total);
  if (addvalid == 1) {
    for (int i = 0; i < 4; i++) {
      tag[total][i] = nuidPICC[i];
    }
    total = total + 1;
    addvalid = 0;
    digitalWrite(buzz, HIGH);
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(buzz, LOW);
    digitalWrite(led, LOW);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("OK!");
    Serial.println("OK!");
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("tag already added");
    Serial.println("tag already added");
    digitalWrite(led, HIGH);
    delay(300);
    digitalWrite(led, LOW);
    delay(300);
    digitalWrite(led, HIGH);
    delay(300);
    digitalWrite(led, LOW);
    delay(2000);
  }
}
