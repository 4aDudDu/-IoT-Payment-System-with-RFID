#include <HardwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Keypad.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
HardwareSerial RFID(2); 

const byte ROWS = 4; 
const byte COLS = 4; 

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 19, 18, 5, 17 };  // Tentukan pin baris untuk ESP32
byte colPins[COLS] = { 23, 32, 3, 33 };  // Tentukan pin kolom untuk ESP32

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String nilai = "";
bool isInput = false; 

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // Inisialisasi RFID
  RFID.begin(9600, SERIAL_8N1, 16, 17);  // RX, TX

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID Tag");
}

void loop() {
  readRFID();
  readKeypad();
}

void readRFID() {
  if (RFID.available()) {
    String tag = RFID.readStringUntil('\n');
    tag.trim();
    if (tag.length() > 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tag ID:");
      lcd.setCursor(0, 1);
      lcd.print(tag);
      delay(2000); 

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Input : ");
      nilai = "";
      isInput = true;
    }
  }
}

void readKeypad() {
  char key = keypad.getKey();

  if (key && isInput) {
    if (key == '#') {
      if (nilai != "") {
        LCDSend();
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tidak ada input!");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Input : ");
      }
    } else if (key == '*' && nilai != "") {
      nilai.remove(nilai.length() - 1);
      LCDKey();
    } else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
      nilai += key;
      LCDKey();
    }
  }
}

void LCDKey() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(nilai);
}

void LCDSend() {
  lcd.clear();
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 0);
    lcd.print(".");
    delay(150);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Terkirim");
  nilai = "";
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID Tag");
  isInput = false;  
}