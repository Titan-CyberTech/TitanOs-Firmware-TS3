#include <TFT_eSPI.h>    // Library for the TFT screen
#include <WiFi.h>         // Wi-Fi library for network management
#include <FS.h>           // Library for SPIFFS filesystem access
#include <SD.h>           // Library for SD card access
#include <esp_adc_cal.h>  // ADC calibration library

#include "titan.h"        // Inclusion du header contenant l'image felin

// GPIO and other constant definitions
#define BACKLIGHT_PIN 15
#define BUTTON_A 0
#define BUTTON_B 14
#define PIN_BAT_VOLT 4

// New firmware version
const char* firmware_version = "1.1.0";

// Main menu
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Info", "Settings", "Battery Info"};
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

// Fonction d'affichage de l'image de démarrage (splash screen)
void showSplashScreen() {
  tft.fillScreen(COLOR_BLACK);
  // Dimensions de l'image titan (à adapter si nécessaire)
  int imgWidth = 210;
  int imgHeight = 210;
  // Calcul pour centrer l'image
  int x = (tft.width() - imgWidth) / 2;
  int y = (tft.height() - imgHeight) / 2;
  // Affichage de l'image
  tft.pushImage(x, y, imgWidth, imgHeight, titan);
  delay(5000); // Affiche l'image pendant 5 secondes
}

void setup() {
  Serial.begin(115200);

  // GPIO configuration
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);  // S'assurer que le rétroéclairage est allumé
  analogWrite(BACKLIGHT_PIN, brightness);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  // Initialisation de l'écran TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  // Affichage du splash screen (image) pendant 5 secondes
  showSplashScreen();

  // Affichage du menu principal
  displayMenu();
}

void loop() {
  handleButtonPress();
  delay(50); // Anti-rebond et éviter des lectures trop rapides
}

// Gestion des boutons
void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
    enterCategory();
  } else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    nextCategory();
  }
}

// Détection d'appui de bouton avec anti-rebond
bool isButtonPressed(int pin, bool& buttonState) {
  if (digitalRead(pin) == LOW && !buttonState) {
    buttonState = true;
    delay(200);  // Anti-rebond
    return true;
  }
  if (digitalRead(pin) == HIGH && buttonState) {
    buttonState = false;
  }
  return false;
}

// Gestion du menu principal
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

// Entrer dans une catégorie du menu
void enterCategory() {
  if (strcmp(menu_items[current_menu_index], "Info") == 0) {
    displayInfo();
  } else if (strcmp(menu_items[current_menu_index], "Settings") == 0) {
    displaySettings();
  } else if (strcmp(menu_items[current_menu_index], "Battery Info") == 0) {
    displayBatteryInfo();
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

// Passage à la catégorie suivante
void nextCategory() {
  current_menu_index = (current_menu_index + 1) % menu_size;
  displayMenu();
}

// Affichage des informations système
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
    "Battery: N/A" // À mettre à jour si gestion de la batterie disponible
  };
  for (int i = 0; i < 5; i++) {
    tft.drawString(infos[i], 10, 50 + i * 20);
  }

  waitForButtonPress(BUTTON_A);
  displayMenu();
}

// Affichage des informations sur la batterie
void displayBatteryInfo() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

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

// Affichage des paramètres
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

    // Modification de la luminosité avec le bouton A
    if (digitalRead(BUTTON_A) == LOW) {
      brightness += 20;
      if (brightness > 255) brightness = 20;
      analogWrite(BACKLIGHT_PIN, brightness);
      delay(200);  // Anti-rebond
    }

    // Changement de couleur ou sortie avec une pression longue sur le bouton B
    if (digitalRead(BUTTON_B) == LOW) {
      unsigned long press_time = millis();
      while (digitalRead(BUTTON_B) == LOW) {
        if (millis() - press_time > 1000) {
          displayMenu();
          return;
        }
      }
      // Changement de couleur
      color_index = (color_index + 1) % 5;
      main_color = color_values[color_index];
      delay(200); // Anti-rebond
    }

    delay(100); // Petite pause pour éviter les boucles trop rapides
  }
}

// Fonction d'attente d'appui sur un bouton
void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50); // Attente que le bouton soit appuyé
  }
  delay(200); // Anti-rebond
}
