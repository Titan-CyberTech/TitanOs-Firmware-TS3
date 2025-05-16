#include "config.h"

// Version du firmware
const char* firmware_version = "1.1.6";

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