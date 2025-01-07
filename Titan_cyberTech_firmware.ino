#include <TFT_eSPI.h>    // Library for the TFT screen
#include <WiFi.h>         // Wi-Fi library for network management
#include <FS.h>           // Library for SPIFFS filesystem access
#include <SD.h>           // Library for SD card access

// GPIO and other constant definitions
#define BACKLIGHT_PIN 15
#define BUTTON_A 0
#define BUTTON_B 14

// New firmware version
const char* firmware_version = "1.0.7";

// Main menu
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Info", "Settings"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

// Common colors
const uint16_t COLOR_RED = TFT_eSPI().color565(255, 0, 0);
const uint16_t COLOR_WHITE = TFT_WHITE;
const uint16_t COLOR_ORANGE = TFT_eSPI().color565(255, 165, 0);
const uint16_t COLOR_VIOLET = TFT_eSPI().color565(138, 43, 226);
const uint16_t COLOR_BLUE = TFT_eSPI().color565(0, 0, 255);
const uint16_t COLOR_BLACK = TFT_BLACK;

uint16_t main_color = COLOR_VIOLET; // Default main color
int brightness = 255; // Default brightness (max)

// Button states
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// TFT screen instance
TFT_eSPI tft = TFT_eSPI();

// Initialization function
void setup() {
  Serial.begin(115200);

  // GPIO configuration
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);  // Ensure backlight is on
  analogWrite(BACKLIGHT_PIN, brightness);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  // TFT screen initialization
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  // Initial display
  showLoadingScreen();
  displayMenu();
}

// Main loop
void loop() {
  handleButtonPress();
  delay(50); // Delay to prevent too rapid reads
}

// Button press handling function
void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
    enterCategory();
  } else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    nextCategory();
  }
}

// Button press detection with debounce
bool isButtonPressed(int pin, bool& buttonState) {
  if (digitalRead(pin) == LOW && !buttonState) {
    buttonState = true;
    delay(200);  // Debounce
    return true;
  }
  if (digitalRead(pin) == HIGH && buttonState) {
    buttonState = false;
  }
  return false;
}

// Loading screen function
void showLoadingScreen() {
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Titan Firmware", tft.width() / 2, 50);

  int barWidth = tft.width() - 80;
  int barHeight = 20;
  int barX = (tft.width() - barWidth) / 2;
  int barY = tft.height() / 2 + 20;

  tft.fillRect(barX, barY, barWidth, barHeight, COLOR_BLACK);
  for (int i = 0; i <= barWidth; i++) {
    tft.fillRect(barX + i, barY, 1, barHeight, main_color);
    delay(10);
  }
  delay(500);
}

// Menu management function
void displayMenu() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(menu_items[current_menu_index], tft.width() / 2, tft.height() / 2);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Titan Firmware", 5, 5);

  tft.setTextDatum(TR_DATUM);
  tft.drawString(firmware_version, tft.width() - 5, 5);

  String categoryText = "Category: " + String(current_menu_index + 1) + "/" + String(menu_size);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(categoryText, tft.width() - 5, tft.height() - 5);
}

// Enter a menu category
void enterCategory() {
  if (strcmp(menu_items[current_menu_index], "Info") == 0) {
    displayInfo();
  } else if (strcmp(menu_items[current_menu_index], "Settings") == 0) {
    displaySettings();
  } else {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Entering: " + String(menu_items[current_menu_index]), tft.width() / 2, tft.height() / 2);
    delay(1000);
    displayMenu();
  }
}

// Function to go to the next category
void nextCategory() {
  current_menu_index = (current_menu_index + 1) % menu_size;
  displayMenu();
}

// Display system info
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
    "Battery: N/A" // Add battery management if available
  };
  for (int i = 0; i < 5; i++) {
    tft.drawString(infos[i], 10, 50 + i * 20);
  }

  waitForButtonPress(BUTTON_A);
  displayMenu();
}

// Display settings
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

    // Change brightness with button A
    if (digitalRead(BUTTON_A) == LOW) {
      brightness += 20;
      if (brightness > 255) brightness = 20;
      analogWrite(BACKLIGHT_PIN, brightness);
      delay(200);  // Debounce
    }

    // Change color or quit with a long press on button B
    if (digitalRead(BUTTON_B) == LOW) {
      unsigned long press_time = millis();
      while (digitalRead(BUTTON_B) == LOW) {
        if (millis() - press_time > 1000) {
          displayMenu();
          return;
        }
      }

      // Change color
      color_index = (color_index + 1) % 5;
      main_color = color_values[color_index];
      delay(200); // Debounce
    }

    delay(100); // Pause to prevent too rapid loops
  }
}

// Wait for button press
void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50); // Wait for the button to be pressed
  }
  delay(200); // Debounce
}
