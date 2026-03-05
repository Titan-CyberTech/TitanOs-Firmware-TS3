#pragma once

// ═══════════════════════════════════════════════════════════
//  TITAN FIRMWARE v2.0 — Lilygo T-Display-S3
//  Board: ESP32-S3 | Screen: ST7789V 170×320 | 8-bit parallel
// ═══════════════════════════════════════════════════════════

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_adc_cal.h>
#include <esp_sleep.h>
#include <Preferences.h>

// ─── Firmware ───────────────────────────────────────────────
#define FIRMWARE_VERSION   "2.0.0"
#define FIRMWARE_NAME      "Titan OS"

// ─── Pins T-Display-S3 ──────────────────────────────────────
#define PIN_BACKLIGHT      38   // GPIO38 = LCD BL (PWM)
#define PIN_BUTTON_A        0   // Boot button
#define PIN_BUTTON_B       14   // Side button
#define PIN_BAT_VOLT        4   // Battery ADC
#define PIN_POWER_ON       15   // Power enable (HIGH = ON)

// ─── Écran ──────────────────────────────────────────────────
#define SCREEN_W          320
#define SCREEN_H          170

// ─── LEDC Backlight ─────────────────────────────────────────
#define LEDC_CH             0
#define LEDC_FREQ        5000
#define LEDC_RES            8   // 8-bit → 0-255

// ─── Timings ────────────────────────────────────────────────
#define SLEEP_TIMEOUT_MS  20000  // 20s inactivité
#define SPLASH_DURATION    3000
#define BTN_DEBOUNCE_MS     200

// ─── Batterie ───────────────────────────────────────────────
#define BAT_VOLT_MIN       3000  // mV (0%)
#define BAT_VOLT_MAX       4200  // mV (100%)
#define BAT_VOLT_ABSENT    1000  // en dessous = pas de batterie

// ─── DNS/Web ────────────────────────────────────────────────
#define DNS_PORT            53

// ════════════════════════════════════════════════════════════
//  Palette UI — Thème sombre moderne
// ════════════════════════════════════════════════════════════
// Couleurs fixes
#define C_BLACK    0x0000
#define C_WHITE    0xFFFF
#define C_DARK     0x1082   // Gris très sombre (fond secondaire)
#define C_GRAY     0x7BEF   // Gris moyen
#define C_LGRAY    0xBDF7   // Gris clair
#define C_RED      0xF800
#define C_ORANGE   0xFD20
#define C_YELLOW   0xFFE0
#define C_GREEN    0x07E0
#define C_CYAN     0x07FF
#define C_BLUE     0x001F
#define C_PURPLE   0x781F
#define C_MAGENTA  0xF81F

// ─── Thèmes disponibles ─────────────────────────────────────
struct ColorTheme {
  const char* name;
  uint16_t    accent;    // couleur principale
  uint16_t    accent2;   // couleur secondaire / highlight
  uint16_t    danger;    // rouge/alerte
};

static const ColorTheme THEMES[] = {
  { "Violet",  0x9818, 0xC81F, C_RED    },  // violet
  { "Cyan",    0x07FF, 0x0599, C_RED    },  // cyan
  { "Orange",  0xFD20, 0xFB00, C_RED    },  // orange
  { "Green",   0x07E0, 0x03E0, C_YELLOW },  // vert
  { "Red",     0xF800, 0xD000, C_YELLOW },  // rouge
};
static const int THEME_COUNT = 5;

// ─── Variables globales ─────────────────────────────────────
extern TFT_eSPI    tft;
extern TFT_eSprite spr;        // sprite double-buffer

extern WebServer   httpServer;
extern DNSServer   dnsServer;

extern int         g_themeIndex;
extern int         g_brightness;
extern bool        g_btnA_state;
extern bool        g_btnB_state;

// Helpers rapides accents
inline uint16_t accent()  { return THEMES[g_themeIndex].accent;  }
inline uint16_t accent2() { return THEMES[g_themeIndex].accent2; }
inline uint16_t danger()  { return THEMES[g_themeIndex].danger;  }
