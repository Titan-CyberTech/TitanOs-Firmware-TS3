#include "ui.h"
#include "utils.h"

void ui_init() {
  spr.createSprite(SCREEN_W, SCREEN_H);
  spr.setSwapBytes(true);
}
void ui_begin() { spr.fillSprite(C_BLACK); }
void ui_push()  { spr.pushSprite(0, 0); }

// ─── Icône batterie compacte (26×12 px, % DANS le corps) ────
void ui_batteryIcon(int x, int y, int percent, bool connected) {
  const int bw = 26, bh = 12;
  uint16_t col = connected
    ? (percent > 50 ? C_GREEN : percent > 20 ? C_YELLOW : C_RED)
    : C_GRAY;

  spr.drawRect(x, y, bw, bh, col);
  spr.fillRect(x + bw, y + 4, 2, bh - 8, col);  // borne +

  if (connected) {
    int fill = max(1, (bw - 2) * percent / 100);
    spr.fillRect(x + 1, y + 1, fill, bh - 2, col);
    char buf[5]; sprintf(buf, "%d%%", percent);
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(percent > 35 ? C_BLACK : C_WHITE, col);
    spr.drawString(buf, x + bw / 2, y + bh / 2);
  } else {
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(C_GRAY, C_DARK);
    spr.drawString("--", x + bw / 2, y + bh / 2);
  }
}

// ─── Topbar ─────────────────────────────────────────────────
void ui_topbar(const char* title, const char* subtitle) {
  spr.fillRect(0, 0, SCREEN_W, UI_TOPBAR_H, C_DARK);
  spr.drawFastHLine(0, UI_TOPBAR_H - 1, SCREEN_W, accent());

  // "TitanOs" à gauche
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(accent(), C_DARK);
  spr.setTextSize(1);
  spr.drawString(FIRMWARE_NAME, 6, UI_TOPBAR_H / 2);

  // Titre centré
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(C_WHITE, C_DARK);
  spr.setTextSize(subtitle ? 1 : 2);
  spr.drawString(title, SCREEN_W / 2, subtitle ? 8 : UI_TOPBAR_H / 2);
  if (subtitle) {
    spr.setTextSize(1);
    spr.setTextColor(accent(), C_DARK);
    spr.drawString(subtitle, SCREEN_W / 2, 20);
  }

  // Batterie — x = SCREEN_W - 32 : icône 26px + borne 2px + marge 4px = ok
  uint32_t volt = utils_readBattVoltage();
  bool     conn = utils_isBattConnected(volt);
  int      pct  = conn ? constrain(map(volt, BAT_VOLT_MIN, BAT_VOLT_MAX, 0, 100), 0, 100) : 0;
  ui_batteryIcon(SCREEN_W - 32, (UI_TOPBAR_H - 12) / 2, pct, conn);
}

// ─── Botbar ─────────────────────────────────────────────────
void ui_botbar(const char* hintA, const char* hintB) {
  int y = SCREEN_H - UI_BOTBAR_H;
  spr.fillRect(0, y, SCREEN_W, UI_BOTBAR_H, C_DARK);
  spr.drawFastHLine(0, y, SCREEN_W, accent());
  spr.setTextSize(1);
  spr.setTextDatum(MC_DATUM);

  if (hintA && hintB) {
    spr.setTextColor(C_LGRAY, C_DARK);
    spr.drawString("[A]", 28, y + UI_BOTBAR_H / 2);
    spr.setTextColor(accent(), C_DARK);
    spr.drawString(hintA, 70, y + UI_BOTBAR_H / 2);
    spr.setTextColor(C_LGRAY, C_DARK);
    spr.drawString("[B]", SCREEN_W / 2 + 22, y + UI_BOTBAR_H / 2);
    spr.setTextColor(accent(), C_DARK);
    spr.drawString(hintB, SCREEN_W - 50, y + UI_BOTBAR_H / 2);
    spr.drawFastVLine(SCREEN_W / 2, y + 4, UI_BOTBAR_H - 8, C_GRAY);
  } else if (hintA) {
    spr.setTextColor(C_LGRAY, C_DARK);
    spr.drawString("[A]", SCREEN_W / 2 - 24, y + UI_BOTBAR_H / 2);
    spr.setTextColor(accent(), C_DARK);
    spr.drawString(hintA, SCREEN_W / 2 + 20, y + UI_BOTBAR_H / 2);
  }
}

void ui_card(int x, int y, int w, int h, uint16_t borderColor) {
  spr.fillRect(x, y, w, h, C_DARK);
  if (borderColor) spr.drawRect(x, y, w, h, borderColor);
}

void ui_bigText(const char* text, int y, uint16_t color, int size) {
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(color, C_BLACK);
  spr.setTextSize(size);
  spr.drawString(text, SCREEN_W / 2, y);
}

void ui_label(const char* text, int x, int y, uint16_t color, int size, uint8_t datum) {
  spr.setTextDatum(datum);
  spr.setTextColor(color, C_BLACK);
  spr.setTextSize(size);
  spr.drawString(text, x, y);
}

void ui_hline(int y, uint16_t color, int margin) {
  spr.drawFastHLine(margin, y, SCREEN_W - margin * 2, color);
}

void ui_progressBar(int x, int y, int w, int h, int percent, uint16_t fillColor) {
  spr.drawRect(x, y, w, h, C_GRAY);
  int fill = (w - 2) * percent / 100;
  if (fill > 0) spr.fillRect(x + 1, y + 1, fill, h - 2, fillColor);
}

// ─── Splash ─────────────────────────────────────────────────
void ui_splash() {
  for (int i = 0; i < 4; i++) {
    ui_begin();
    spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2, 60 + i * 5, accent());
    spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2, 40 + i * 5, accent2());
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(5);
    spr.setTextColor(C_WHITE, C_BLACK);
    spr.drawString("TITAN", SCREEN_W / 2, SCREEN_H / 2 - 15);
    spr.setTextSize(3);
    spr.setTextColor(accent(), C_BLACK);
    spr.drawString("OS", SCREEN_W / 2, SCREEN_H / 2 + 25);
    spr.setTextSize(1);
    spr.setTextColor(C_GRAY, C_BLACK);
    spr.setTextDatum(BC_DATUM);
    spr.drawString("github.com/Titan-CyberTech/TitanOs-Firmware-TS3", SCREEN_W / 2, SCREEN_H - 4);
    ui_push();
    delay(200);
  }
  for (int p = 0; p <= 100; p += 5) {
    ui_begin();
    spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2, 60, accent());
    spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2, 40, accent2());
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(5);
    spr.setTextColor(C_WHITE, C_BLACK);
    spr.drawString("TITAN", SCREEN_W / 2, SCREEN_H / 2 - 15);
    spr.setTextSize(3);
    spr.setTextColor(accent(), C_BLACK);
    spr.drawString("OS", SCREEN_W / 2, SCREEN_H / 2 + 25);
    ui_progressBar(40, SCREEN_H - 30, SCREEN_W - 80, 8, p, accent());
    spr.setTextSize(1);
    spr.setTextColor(C_GRAY, C_BLACK);
    spr.setTextDatum(MC_DATUM);
    spr.drawString("Initializing...", SCREEN_W / 2, SCREEN_H - 40);
    ui_push();
    delay(25);
  }
  delay(400);
}

// ─── Toast ──────────────────────────────────────────────────
void ui_toast(const char* msg, uint16_t color) {
  if (color == 0) color = accent();
  int tw = strlen(msg) * 6 + 20;
  int tx = (SCREEN_W - tw) / 2;
  int ty = SCREEN_H - UI_BOTBAR_H - 26;
  spr.fillRect(tx, ty, tw, 20, C_DARK);
  spr.drawRect(tx, ty, tw, 20, color);
  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(color, C_DARK);
  spr.drawString(msg, SCREEN_W / 2, ty + 10);
  ui_push();
  delay(1500);
}

// ─── Menu liste ─────────────────────────────────────────────
void ui_drawMenu(const MenuItem* items, int count, int selected) {
  ui_begin();
  ui_topbar("TITAN OS", "v" FIRMWARE_VERSION);
  ui_botbar("Select", "Navigate");

  const int ITEM_H  = 26;
  const int VISIBLE = 4;
  const int START_Y = UI_TOPBAR_H + 4;

  int pageStart = selected - (VISIBLE / 2);
  if (pageStart < 0)               pageStart = 0;
  if (pageStart + VISIBLE > count) pageStart = max(0, count - VISIBLE);

  for (int i = pageStart; i < pageStart + VISIBLE && i < count; i++) {
    int  y   = START_Y + (i - pageStart) * ITEM_H;
    bool sel = (i == selected);

    if (sel) {
      spr.fillRect(4, y, SCREEN_W - 8, ITEM_H - 2, accent());
      spr.fillRect(0, y, 3,            ITEM_H - 2, accent2());
    } else {
      spr.fillRect(4, y, SCREEN_W - 8, ITEM_H - 2, C_DARK);
    }

    spr.setTextDatum(ML_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(sel ? C_BLACK : accent(), sel ? accent() : C_DARK);
    spr.drawString(items[i].icon, 12, y + ITEM_H / 2);

    spr.setTextSize(sel ? 2 : 1);
    spr.setTextColor(sel ? C_BLACK : C_WHITE, sel ? accent() : C_DARK);
    spr.drawString(items[i].label, 30, y + ITEM_H / 2);

    if (sel) {
      spr.setTextColor(C_BLACK, accent());
      spr.setTextDatum(MR_DATUM);
      spr.setTextSize(2);
      spr.drawString(">", SCREEN_W - 10, y + ITEM_H / 2);
    }
  }

  if (count > VISIBLE) {
    int sbH   = SCREEN_H - UI_TOPBAR_H - UI_BOTBAR_H - 8;
    int sbY   = UI_TOPBAR_H + 4 + (sbH * pageStart / count);
    int sbLen = sbH * VISIBLE / count;
    spr.fillRect(SCREEN_W - 3, UI_TOPBAR_H + 4, 3, sbH,   C_DARK);
    spr.fillRect(SCREEN_W - 3, sbY,              3, sbLen, accent());
  }

  ui_push();
}
