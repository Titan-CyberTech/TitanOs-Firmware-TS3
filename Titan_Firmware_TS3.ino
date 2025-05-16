#include "config.h"
#include "display.h"
#include "menu.h"
#include "attacks.h"
#include "games.h"
#include "utils.h"

// Variables de gestion des boutons (état)
bool buttonA_pressed = false;
bool buttonB_pressed = false;

void setup() {
  Serial.begin(115200);

  setupBacklight();

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  setupTFT();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_max_tx_power(78);

  setupBatteryADC();

  showSplashScreen();
  displayMenu();
}

void handleInactivity(unsigned long &lastActivityTime, const unsigned long screenOffDelay) {
  if (millis() - lastActivityTime >= screenOffDelay) {
    tft.fillScreen(COLOR_BLACK);
    while (digitalRead(BUTTON_A) != LOW && digitalRead(BUTTON_B) != LOW) {
      delay(50);
    }
    displayMenu();
    lastActivityTime = millis();
  }
}

void loop() {
  static unsigned long lastActivityTime = millis();
  constexpr unsigned long screenOffDelay = 15000;

  handleButtonPress();
  handleInactivity(lastActivityTime, screenOffDelay);

  delay(50);
}

void setupBacklight() {
  pinMode(BACKLIGHT_PIN, OUTPUT);
  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, LEDC_RESOLUTION);
  ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, brightness);
}

void setupTFT() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);
}