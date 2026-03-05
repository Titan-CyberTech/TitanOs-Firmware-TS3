#include "config.h"

TFT_eSPI    tft     = TFT_eSPI();
TFT_eSprite spr     = TFT_eSprite(&tft);   // sprite 320×170 pour double-buffer

WebServer   httpServer(80);
DNSServer   dnsServer;

int  g_themeIndex = 0;
int  g_brightness = 200;
bool g_btnA_state = false;
bool g_btnB_state = false;
