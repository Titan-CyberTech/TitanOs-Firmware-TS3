#pragma once
#include "config.h"

// ─── Boutons ────────────────────────────────────────────────
void     utils_initButtons();
bool     utils_btnA();           // true si appui détecté (avec debounce)
bool     utils_btnB();
bool     utils_btnAHeld(uint32_t ms = 1000);  // appui long
void     utils_waitBtn(int pin);

// ─── Batterie ───────────────────────────────────────────────
void     utils_initADC();
uint32_t utils_readBattVoltage();   // retourne mV
bool     utils_isBattConnected(uint32_t mV);
int      utils_battPercent();       // 0-100

// ─── Backlight ──────────────────────────────────────────────
void     utils_initBacklight();
void     utils_setBrightness(int val);  // 0-255

// ─── Persistance settings (NVS) ─────────────────────────────
void     utils_loadSettings();
void     utils_saveSettings();

// ─── Sleep ──────────────────────────────────────────────────
void     utils_deepSleep();
