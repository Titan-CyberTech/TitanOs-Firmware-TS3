#pragma once
#include "config.h"

// ════════════════════════════════════════════════════════════
//  UI — Composants graphiques modernes (double-buffer sprite)
//  Résolution : 320×170 (rotation=1 sur TFT 170×320)
// ════════════════════════════════════════════════════════════

// ─── Constantes layout ──────────────────────────────────────
#define UI_TOPBAR_H    28   // hauteur barre du haut
#define UI_BOTBAR_H    22   // hauteur barre du bas
#define UI_CONTENT_Y   (UI_TOPBAR_H + 2)
#define UI_CONTENT_H   (SCREEN_H - UI_TOPBAR_H - UI_BOTBAR_H - 4)

void ui_init();

// Outils de base sprite
void ui_begin();           // efface le sprite
void ui_push();            // pousse le sprite vers l'écran

// Barres
void ui_topbar(const char* title, const char* subtitle = nullptr);
void ui_botbar(const char* hintA, const char* hintB = nullptr);

// Composants
void ui_card(int x, int y, int w, int h, uint16_t borderColor = 0);
void ui_bigText(const char* text, int y, uint16_t color, int size = 4);
void ui_label(const char* text, int x, int y, uint16_t color, int size = 1, uint8_t datum = TL_DATUM);
void ui_hline(int y, uint16_t color, int margin = 8);
void ui_progressBar(int x, int y, int w, int h, int percent, uint16_t fillColor);
void ui_icon(int x, int y, uint8_t icon);  // icônes 8×8 internes

// Écran spéciaux
void ui_splash();
void ui_statusBar();       // met à jour uniquement la barre batterie/wifi

// Menu liste
struct MenuItem {
  const char* label;
  const char* icon;   // emoji-like string (2 chars max, ASCII)
};

int  ui_menu(const MenuItem* items, int count, int selected);
void ui_drawMenu(const MenuItem* items, int count, int selected);

// Popups / overlays
void ui_toast(const char* msg, uint16_t color = 0);
void ui_confirm(const char* msg);  // juste affiche, pas de logique

// Batterie
void ui_batteryIcon(int x, int y, int percent, bool connected);
