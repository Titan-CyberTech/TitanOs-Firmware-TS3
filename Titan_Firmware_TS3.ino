#include "config.h"
#include "display.h"
#include "menu.h"
#include "attacks.h"
#include "games.h"
#include "utils.h"

// Variables de gestion des boutons
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// Version du firmware
const char* firmware_version = "1.1.5";

// Couleurs utilisées
const uint16_t COLOR_RED    = TFT_RED;
const uint16_t COLOR_WHITE  = TFT_WHITE;
const uint16_t COLOR_ORANGE = TFT_ORANGE;
const uint16_t COLOR_VIOLET = TFT_VIOLET;
const uint16_t COLOR_BLUE   = TFT_BLUE;
const uint16_t COLOR_BLACK  = TFT_BLACK;
const uint16_t COLOR_GREEN  = TFT_GREEN;

uint16_t main_color = COLOR_VIOLET;
int brightness = 255;

// Instance de l'affichage TFT
TFT_eSPI tft = TFT_eSPI();

// Backlight via LEDC (ESP32)
const int LEDC_CHANNEL    = 0;
const int LEDC_FREQUENCY  = 5000;
const int LEDC_RESOLUTION = 8;

// Serveur web et DNS pour le portail malveillant
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

void setup() {
  Serial.begin(115200);

  // Initialisation du rétroéclairage
  setupBacklight();

  // Initialisation des boutons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  // Initialisation de l'écran TFT
  setupTFT();

  // Initialisation de la WiFi en mode station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Régler la puissance d'émission maximale
  esp_wifi_set_max_tx_power(78);  // 78 * 0.25 = 19.5 dBm

  // Initialisation de l'ADC pour la batterie
  setupBatteryADC();

  // Affichage de l'écran de démarrage
  showSplashScreen();
  displayMenu();
}

void loop() {
  static unsigned long lastActivityTime = millis();
  const unsigned long screenOffDelay = 15000; // 15 secondes d'inactivité avant extinction de l'écran

  handleButtonPress();

  // Gérer l'inactivité
  if (millis() - lastActivityTime >= screenOffDelay) {
    tft.fillScreen(COLOR_BLACK);
    bool screenOff = true;
    while (screenOff) {
      if (digitalRead(BUTTON_A) == LOW || digitalRead(BUTTON_B) == LOW) {
        screenOff = false;
        displayMenu();
        lastActivityTime = millis();  // Réinitialiser le timer d'inactivité
      }
      delay(50);  // Petite pause pour vérifier les entrées
    }
  }
  delay(50);  // Petite pause pour éviter un trop grand appel à la boucle
}

// Fonction pour configurer le rétroéclairage
void setupBacklight() {
  pinMode(BACKLIGHT_PIN, OUTPUT);
  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, LEDC_RESOLUTION);
  ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, brightness);
}

// Fonction pour configurer l'écran TFT
void setupTFT() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);
}
