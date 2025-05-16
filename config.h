#ifndef CONFIG_H
#define CONFIG_H

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <esp_adc_cal.h>
#include "esp_wifi.h"
#include <WebServer.h>
#include <DNSServer.h>

// GPIO and definitions
#define BACKLIGHT_PIN 15
#define BUTTON_A      0
#define BUTTON_B      14
#define PIN_BAT_VOLT  4
#define PIN_POWER_ON  21
#define PIN_LCD_BL    22
#define PIN_BUTTON_2  23

extern const char* firmware_version;

// Colors used
extern const uint16_t COLOR_RED;
extern const uint16_t COLOR_WHITE;
extern const uint16_t COLOR_ORANGE;
extern const uint16_t COLOR_VIOLET;
extern const uint16_t COLOR_BLUE;
extern const uint16_t COLOR_BLACK;
extern const uint16_t COLOR_GREEN;

extern uint16_t main_color;
extern int brightness;

// TFT display instance
extern TFT_eSPI tft;

// Backlight via LEDC (ESP32)
extern const int LEDC_CHANNEL;
extern const int LEDC_FREQUENCY;
extern const int LEDC_RESOLUTION;

// Web server and DNS for the Evil Portal
extern WebServer server;
extern DNSServer dnsServer;
extern const byte DNS_PORT;

#endif // CONFIG_H