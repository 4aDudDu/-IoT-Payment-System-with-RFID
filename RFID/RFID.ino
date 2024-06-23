#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Keypad.h"
#include <WiFi.h> 

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

byte rowPins[ROWS] = { 19, 18, 5, 17 };
byte colPins[COLS] = { 23, 32, 3, 33 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String nilai = "";
bool isInput = false;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  RFID.begin(9600, SERIAL_8N1, 16, 17);

  displayMenu();
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    handleMenuSelection(key);
  }
}

void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1. Set WiFi");
  lcd.setCursor(0, 1);
  lcd.print("2. Reconnect API");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("3. Cek Saldo");
  lcd.setCursor(0, 1);
  lcd.print("4. Transaksi");
}

void handleMenuSelection(char key) {
  switch (key) {
    case '1':
      settingWiFi();
      break;
    case '2':
      reconnectAPI();
      break;
    case '3':
      cekSaldo();
      break;
    case '4':
      transaksi();
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilihan salah");
      delay(2000);
      displayMenu();
      break;
  }
}

void settingWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning WiFi...");
  delay(2000);

  int n = WiFi.scanNetworks();
  lcd.clear();
  if (n == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Tidak ada WIFI");
    delay(2000);
    displayMenu();
    return;
  } else {
    for (int i = 0; i < n; ++i) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(String(i + 1) + ": " + WiFi.SSID(i));
      delay(2000);
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Masukan Pilihan Wifi");
  int selectedNetwork = getNetworkSelection(n);

  String ssid = WiFi.SSID(selectedNetwork);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Selected: " + ssid);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter password:");

  String password = getPasswordInput();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  WiFi.begin(ssid.c_str(), password.c_str());

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) { // Timeout setelah 10 detik
    delay(500);
    lcd.print(".");
    timeout++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("Connected!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Gagal");
  }
  delay(2000);
  displayMenu();
}

int getNetworkSelection(int numNetworks) {
  String input = "";
  char key;
  while (true) {
    key = keypad.getKey();
    if (key && key != '#' && key != '*') {
      input += key;
      lcd.setCursor(0, 1);
      lcd.print(input);
    }
    if (key == '#') {
      int selected = input.toInt() - 1;
      if (selected >= 0 && selected < numNetworks) {
        return selected;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Invalid selection");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Masukan Pilihan Wifi");
        input = "";
      }
    }
  }
}

String getPasswordInput() {
  String input = "";
  char key;
  while (true) {
    key = keypad.getKey();
    if (key && key != '#' && key != '*') {
      input += key;
      lcd.setCursor(0, 1);
      lcd.print(input);
    }
    if (key == '#') {
      return input;
    }
    if (key == '*') {
      if (input.length() > 0) {
        input.remove(input.length() - 1);
        lcd.setCursor(0, 1);
        lcd.print("                "); // Clear previous input
        lcd.setCursor(0, 1);
        lcd.print(input);
      }
    }
  }
}

void reconnectAPI() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reconnect API...");
  // isian API
  delay(2000);
  displayMenu();
}

void cekSaldo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cek Saldo...");
  // isian saldo
  delay(2000);
  displayMenu();
}

void transaksi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Transaksi...");
  // isian transaksi
  delay(2000);
  displayMenu();
}
