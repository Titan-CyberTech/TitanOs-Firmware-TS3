#include "screens.h"
#include "ui.h"
#include "utils.h"

// ════════════════════════════════════════════════════════════
//  Écran — Informations système
// ════════════════════════════════════════════════════════════
void screens_sysInfo() {
  ui_begin();
  ui_topbar("SYSTEM INFO");
  ui_botbar("Back");

  // Zone utile : 320 x ~120 px
  // Layout : grille 2 colonnes x 4 lignes, chaque carte 26px de haut
  // Cle en gris petit (haut), valeur en accent (bas)
  struct InfoItem { const char* key; const char* val; uint16_t color; };
  char fwbuf[12]; sprintf(fwbuf, "v%s", FIRMWARE_VERSION);

  const InfoItem items[] = {
    { "CHIP",      "ESP32-S3",       0 },
    { "CPU",       "240 MHz",        0 },
    { "FLASH",     "16 MB",          0 },
    { "PSRAM",     "8 MB OPI",       0 },
    { "SCREEN",    "320x170 ST7789", 0 },
    { "WiFi",      "802.11 b/g/n",   0 },
    { "BLUETOOTH", "BLE 5.0",        0 },
    { "FIRMWARE",  fwbuf,      C_WHITE },
  };

  const int COUNT = 8;
  const int COLS  = 2;
  const int CW    = (SCREEN_W - 12) / COLS;   // ~154px
  const int CH    = 26;
  const int PAD_X = 4;
  const int PAD_Y = UI_CONTENT_Y + 4;

  for (int i = 0; i < COUNT; i++) {
    int col = i % COLS;
    int row = i / COLS;
    int cx  = PAD_X + col * (CW + 4);
    int cy  = PAD_Y + row * (CH + 3);
    uint16_t valCol = items[i].color ? items[i].color : accent();

    spr.fillRect(cx, cy, CW, CH, C_DARK);

    spr.setTextDatum(TL_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(C_GRAY, C_DARK);
    spr.drawString(items[i].key, cx + 4, cy + 3);

    spr.setTextDatum(BL_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(valCol, C_DARK);
    spr.drawString(items[i].val, cx + 4, cy + CH - 4);
  }

  ui_push();
  utils_waitBtn(PIN_BUTTON_A);
}

// ════════════════════════════════════════════════════════════
//  Écran — Batterie
//  FIX : utils_waitBtn purge maintenant les états boutons
//  après le relâchement → plus de crash au retour menu
// ════════════════════════════════════════════════════════════
void screens_batteryInfo() {
  uint32_t volt = utils_readBattVoltage();
  bool     conn = utils_isBattConnected(volt);
  int      pct  = conn ? constrain((int)map(volt, BAT_VOLT_MIN, BAT_VOLT_MAX, 0, 100), 0, 100) : 0;

  ui_begin();
  ui_topbar("BATTERY INFO");
  ui_botbar("Back");

  int cy = SCREEN_H / 2 - 10;

  if (!conn) {
    ui_bigText("NO BATTERY", cy - 10, C_GRAY, 2);
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(C_GRAY, C_BLACK);
    spr.drawString("Connect a LiPo 3.7V battery", SCREEN_W / 2, cy + 20);
  } else {
    uint16_t col = pct > 50 ? C_GREEN : pct > 20 ? C_YELLOW : C_RED;

    int bx = SCREEN_W / 2 - 45, by = cy - 22;
    spr.drawRect(bx, by, 80, 34, col);
    spr.fillRect(bx + 80, by + 11, 6, 12, col);
    int fill = 78 * pct / 100;
    if (fill > 0) spr.fillRect(bx + 1, by + 1, fill, 32, col);

    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(2);
    spr.setTextColor(pct > 40 ? C_BLACK : C_WHITE, col);
    char pbuf[8]; sprintf(pbuf, "%d%%", pct);
    spr.drawString(pbuf, bx + 40, by + 17);

    spr.setTextSize(1);
    spr.setTextColor(C_LGRAY, C_BLACK);
    char vbuf[16]; sprintf(vbuf, "%.2fV  (%lumV)", volt / 1000.0f, volt);
    spr.setTextDatum(MC_DATUM);
    spr.drawString(vbuf, SCREEN_W / 2, by + 46);

    ui_progressBar(20, by + 58, SCREEN_W - 40, 8, pct, col);
  }

  ui_push();
  utils_waitBtn(PIN_BUTTON_A);  // purge les états → retour propre au menu
}

// ════════════════════════════════════════════════════════════
//  Écran — Paramètres
//  FIX LUMINOSITÉ : logique boutons simplifiée
//  - B seul : action directe selon mode (thème ou luminosité)
//  - A court : switcher de section
//  - A long (>800ms) : sauvegarder et quitter
// ════════════════════════════════════════════════════════════
void screens_settings() {
  bool running = true;
  int  mode    = 0;   // 0 = thème, 1 = luminosité

  // Purge l'état du bouton A qui vient de sélectionner cet écran
  while (digitalRead(PIN_BUTTON_A) == LOW) delay(10);
  g_btnA_state = false;
  g_btnB_state = false;

  while (running) {
    // ── Dessin ──────────────────────────────────────────────
    ui_begin();
    ui_topbar("SETTINGS");

    if (mode == 0) ui_botbar("Section", "Next Theme");
    else           ui_botbar("Section", "Bright +");

    int cy = UI_CONTENT_Y + 8;

    // Section thème
    {
      bool active = (mode == 0);
      uint16_t secCol = active ? accent() : C_GRAY;
      spr.setTextDatum(TL_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(secCol, C_BLACK);
      spr.drawString("THEME", 10, cy);
      if (active) spr.fillRect(8, cy - 1, 3, 10, accent());

      for (int i = 0; i < THEME_COUNT; i++) {
        int px = 10 + i * 28, py = cy + 14;
        spr.fillRect(px, py, 22, 14, THEMES[i].accent);
        if (i == g_themeIndex) spr.drawRect(px - 1, py - 1, 24, 16, C_WHITE);
        spr.setTextDatum(MC_DATUM);
        spr.setTextSize(1);
        spr.setTextColor(C_BLACK, THEMES[i].accent);
        char buf[2] = { THEMES[i].name[0], 0 };
        spr.drawString(buf, px + 11, py + 7);
      }
      spr.setTextDatum(TR_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(accent(), C_BLACK);
      spr.drawString(THEMES[g_themeIndex].name, SCREEN_W - 10, cy + 14);
    }

    cy += 42;
    ui_hline(cy, C_DARK);
    cy += 8;

    // Section luminosité
    {
      bool active = (mode == 1);
      uint16_t secCol = active ? accent() : C_GRAY;
      spr.setTextDatum(TL_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(secCol, C_BLACK);
      spr.drawString("BRIGHTNESS", 10, cy);
      if (active) spr.fillRect(8, cy - 1, 3, 10, accent());

      char bval[8]; sprintf(bval, "%d%%", g_brightness * 100 / 255);
      spr.setTextDatum(TR_DATUM);
      spr.setTextColor(active ? accent() : C_GRAY, C_BLACK);
      spr.drawString(bval, SCREEN_W - 10, cy);
      ui_progressBar(10, cy + 14, SCREEN_W - 20, 12, g_brightness * 100 / 255, secCol);
    }

    // Hint appui long en bas du contenu
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(C_GRAY, C_BLACK);
    spr.drawString("Hold A  ->  Save & Quit", SCREEN_W / 2, SCREEN_H - UI_BOTBAR_H - 10);

    ui_push();

    // ── Lecture bouton A ────────────────────────────────────
    if (digitalRead(PIN_BUTTON_A) == LOW) {
      uint32_t t = millis();
      // Attendre relâchement ou timeout long press
      while (digitalRead(PIN_BUTTON_A) == LOW) {
        delay(10);
        if (millis() - t > 800) {
          // Appui LONG → sauvegarder et quitter
          utils_saveSettings();
          // Attendre le relâchement avant de partir
          while (digitalRead(PIN_BUTTON_A) == LOW) delay(10);
          g_btnA_state = false;
          g_btnB_state = false;
          ui_toast("Settings saved!", accent());
          running = false;
          break;
        }
      }
      if (running) {
        // Appui COURT → changer de section
        mode = (mode + 1) % 2;
        g_btnA_state = false;
        delay(BTN_DEBOUNCE_MS);
      }
    }

    // ── Lecture bouton B ────────────────────────────────────
    // FIX LUMINOSITÉ : on lit directement le pin, pas utils_btnB()
    // utils_btnB() avait un delay(200) qui faisait rater les changements
    if (digitalRead(PIN_BUTTON_B) == LOW && !g_btnB_state) {
      g_btnB_state = true;
      if (mode == 0) {
        // Changer de thème
        g_themeIndex = (g_themeIndex + 1) % THEME_COUNT;
      } else {
        // Augmenter la luminosité ET l'appliquer immédiatement
        int newBr = g_brightness + 30;
        if (newBr > 255) newBr = 20;
        utils_setBrightness(newBr);  // ledcWrite appelé ici directement
      }
      delay(150);  // debounce rapide pour la luminosité (pas 200ms)
    }
    if (digitalRead(PIN_BUTTON_B) == HIGH) g_btnB_state = false;

    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  Écran — Power Off
// ════════════════════════════════════════════════════════════
void screens_powerOff() {
  for (int fade = g_brightness; fade >= 0; fade -= 8) {
    ledcWrite(LEDC_CH, max(0, fade));
    delay(15);
  }
  ui_begin();
  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(2);
  spr.setTextColor(C_GRAY, C_BLACK);
  spr.drawString("Shutting down...", SCREEN_W / 2, SCREEN_H / 2);
  ui_push();
  delay(800);
  utils_deepSleep();
}
