#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Keypad.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID setup
HardwareSerial RFID(2);

// Keypad setup
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

// Variables
String nilai = "";
bool isInput = false;
String api_key = "";
const int API_KEY_ADDRESS = 0; // EEPROM address to store API key
int currentMenu = 0;
unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  RFID.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize EEPROM
  EEPROM.begin(512);
  // Retrieve stored API key from EEPROM
  api_key = readAPIKey();

  displayMenu(currentMenu);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    currentMenu = (currentMenu + 1) % 2;
    displayMenu(currentMenu);
  }

  char key = keypad.getKey();
  if (key) {
    handleMenuSelection(key);
  }
}

void displayMenu(int menu) {
  lcd.clear();
  if (menu == 0) {
    lcd.setCursor(0, 0);
    lcd.print("1. Set WiFi");
    lcd.setCursor(0, 1);
    lcd.print("2. Reconnect API");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("3. Cek Saldo");
    lcd.setCursor(0, 1);
    lcd.print("4. Transaksi");
  }
}

void handleMenuSelection(char key) {
  switch (key) {
    case '1':
      if (currentMenu == 0) settingWiFi();
      break;
    case '2':
      if (currentMenu == 0) reconnectAPI();
      break;
    case '3':
      if (currentMenu == 1) cekSaldo();
      break;
    case '4':
      if (currentMenu == 1) transaksi();
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pilihan salah");
      delay(2000);
      displayMenu(currentMenu);
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
    displayMenu(currentMenu);
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
  while (WiFi.status() != WL_CONNECTED && timeout < 20) { 
    delay(500);
    lcd.print(".");
    timeout++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("Connected!");
    if (api_key == "") {
      generateAPIKey();
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Gagal");
  }
  delay(2000);
  displayMenu(currentMenu);
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
        lcd.print("                "); 
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

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://oncard.id/app/api/device-manager/connect-device");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "api_key=" + api_key;
    int httpResponseCode = http.POST(postData);

    lcd.clear();
    if (httpResponseCode > 0) {
      String response = http.getString();
      lcd.setCursor(0, 0);
      lcd.print("Success!");
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Failed!");
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Not connected");
  }
  delay(2000);
  displayMenu(currentMenu);
}

void cekSaldo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cek Saldo...");
  // isian saldo
  delay(2000);
  displayMenu(currentMenu);
}

void transaksi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Transaksi...");
  // isian transaksi
  delay(2000);
  displayMenu(currentMenu);
}

void generateAPIKey() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://oncard.id/app/api/device-manager/connect-device");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = ""; // Add necessary parameters if any
    int httpResponseCode = http.POST(postData);

    lcd.clear();
    if (httpResponseCode > 0) {
      api_key = http.getString();
      lcd.setCursor(0, 0);
      lcd.print("API Key generated");
      Serial.println(httpResponseCode);
      Serial.println(api_key);
      storeAPIKey(api_key); // Store the generated API key
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Failed to generate");
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Not connected");
  }
  delay(2000);
}

void storeAPIKey(String key) {
  for (int i = 0; i < key.length(); i++) {
    EEPROM.write(API_KEY_ADDRESS + i, key[i]);
  }
  EEPROM.write(API_KEY_ADDRESS + key.length(), '\0'); // Null terminator
  EEPROM.commit();
}

String readAPIKey() {
  char key[100];
  int i = 0;
  while (i < 100) {
    key[i] = EEPROM.read(API_KEY_ADDRESS + i);
    if (key[i] == '\0') break;
    i++;
  }
  return String(key);
}
