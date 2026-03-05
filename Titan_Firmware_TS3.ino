// ═══════════════════════════════════════════════════════════
//  TITAN FIRMWARE v2.0
//  Board  : Lilygo T-Display-S3 (ESP32-S3)
//  Screen : ST7789V 170×320 (rotation 1 → 320×170)
//  Author : Titan.cybertech
//  License: MIT
// ═══════════════════════════════════════════════════════════
//  ⚠  WiFi/BLE features are for educational purposes only.
//     Never use on networks you don't own or without consent.
// ═══════════════════════════════════════════════════════════

#include "config.h"
#include "utils.h"
#include "ui.h"
#include "menu.h"

// ─── Variables ──────────────────────────────────────────────
static unsigned long s_lastActivity = 0;

// ─── Prototypes locaux ──────────────────────────────────────
void handleInactivity();

// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // Power enable (T-Display-S3 : GPIO15 = PWR_EN)
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  // Périphériques
  utils_initButtons();
  utils_initBacklight();
  utils_initADC();

  // Charger settings depuis NVS
  utils_loadSettings();
  utils_setBrightness(g_brightness);

  // Init TFT + sprite double-buffer
  tft.init();
  tft.setRotation(1);          // 320×170
  tft.fillScreen(C_BLACK);
  tft.setSwapBytes(true);
  ui_init();

  // WiFi en mode station au démarrage
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_max_tx_power(78);

  // Splash screen
  ui_splash();

  s_lastActivity = millis();
}

// ════════════════════════════════════════════════════════════
void loop() {
  // Réinitialiser le timer si un bouton est pressé
  if (digitalRead(PIN_BUTTON_A) == LOW || digitalRead(PIN_BUTTON_B) == LOW) {
    s_lastActivity = millis();
  }

  handleInactivity();

  // Le menu gère sa propre boucle bloquante
  // On appelle menu_run() une fois, il ne revient jamais
  menu_run();
}

// ─── Auto-sleep après inactivité ────────────────────────────
void handleInactivity() {
  if (millis() - s_lastActivity >= SLEEP_TIMEOUT_MS) {
    // Éteindre l'écran
    ledcWrite(LEDC_CH, 0);
    tft.writecommand(0x28);   // display off

    // Attendre un appui bouton pour se réveiller
    while (digitalRead(PIN_BUTTON_A) == HIGH && digitalRead(PIN_BUTTON_B) == HIGH) {
      delay(50);
    }
    delay(BTN_DEBOUNCE_MS);

    // Rallumer
    tft.writecommand(0x29);   // display on
    utils_setBrightness(g_brightness);
    s_lastActivity = millis();

    // Redessiner le menu
    // (menu_run() sera rappelé via loop)
  }
}
