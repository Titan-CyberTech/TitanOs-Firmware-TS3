#include "games.h"
#include "ui.h"
#include "utils.h"

// ════════════════════════════════════════════════════════════
//  Snake — Version améliorée
//  Zone de jeu : 320×118 px (après topbar 28px + botbar 22px)
//  Cellule : 8px → grille 40×14
// ════════════════════════════════════════════════════════════

static const int CELL    = 8;
static const int GRID_W  = SCREEN_W / CELL;           // 40
static const int GRID_H  = (SCREEN_H - UI_TOPBAR_H - UI_BOTBAR_H) / CELL;  // 14
static const int OFFSET_Y = UI_TOPBAR_H;
static const int MAX_LEN = 200;

static int  sx[MAX_LEN], sy[MAX_LEN];
static int  slen;
static int  dir;     // 0=up 1=right 2=down 3=left
static int  fx, fy;  // food
static int  score;
static bool running;

static void _placeFood() {
  bool ok = false;
  while (!ok) {
    fx = random(0, GRID_W);
    fy = random(0, GRID_H);
    ok = true;
    for (int i = 0; i < slen; i++) {
      if (sx[i] == fx && sy[i] == fy) { ok = false; break; }
    }
  }
}

static void _drawGame() {
  // On dessine uniquement la zone de jeu dans le sprite
  // Fond zone de jeu
  spr.fillRect(0, OFFSET_Y, SCREEN_W, GRID_H * CELL, C_BLACK);

  // Grille légère
  for (int x = 0; x < SCREEN_W; x += CELL)
    spr.drawFastVLine(x, OFFSET_Y, GRID_H * CELL, 0x0841);  // très sombre
  for (int y = 0; y < GRID_H * CELL; y += CELL)
    spr.drawFastHLine(0, OFFSET_Y + y, SCREEN_W, 0x0841);

  // Food — pastille rouge clignotante
  static bool fp = false; fp = !fp;
  uint16_t fc = fp ? C_RED : 0xD000;
  spr.fillRect(fx * CELL + 1, OFFSET_Y + fy * CELL + 1, CELL - 2, CELL - 2, fc);
  spr.drawRect(fx * CELL,     OFFSET_Y + fy * CELL,     CELL,     CELL,     C_RED);

  // Snake — tête plus lumineuse
  for (int i = slen - 1; i >= 0; i--) {
    uint16_t col = (i == 0) ? accent2() : accent();
    // Corps avec dégradé simulé : les derniers segments plus sombres
    if (i > slen / 2) col = accent();
    spr.fillRect(sx[i] * CELL + 1, OFFSET_Y + sy[i] * CELL + 1,
                 CELL - 2, CELL - 2, col);
  }
  // Yeux sur la tête
  int hx = sx[0] * CELL, hy = OFFSET_Y + sy[0] * CELL;
  spr.fillRect(hx + 2, hy + 2, 2, 2, C_BLACK);
  spr.fillRect(hx + 4, hy + 2, 2, 2, C_BLACK);
}

static void _drawHUD() {
  ui_topbar("SNAKE", nullptr);
  // Score dans la topbar à droite
  char sc[12]; sprintf(sc, "Score: %d", score);
  spr.setTextDatum(MR_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(accent(), C_DARK);
  spr.drawString(sc, SCREEN_W - 32, UI_TOPBAR_H / 2);

  ui_botbar("Turn L", "Turn R");
}

void games_snakeRun() {
  // ── Init ──────────────────────────────────────────────────
  slen  = 3;
  dir   = 1;   // droite
  score = 0;
  sx[0] = GRID_W / 2;     sy[0] = GRID_H / 2;
  sx[1] = GRID_W / 2 - 1; sy[1] = GRID_H / 2;
  sx[2] = GRID_W / 2 - 2; sy[2] = GRID_H / 2;
  _placeFood();
  running = true;

  uint32_t lastMove = millis();
  int speed = 180;  // ms par tick (diminue avec le score)

  // Countdown
  for (int c = 3; c > 0; c--) {
    ui_begin();
    _drawHUD();
    _drawGame();
    char buf[4]; sprintf(buf, "%d", c);
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(6);
    spr.setTextColor(C_WHITE, C_BLACK);
    spr.drawString(buf, SCREEN_W / 2, SCREEN_H / 2);
    ui_push();
    delay(700);
  }

  // ── Boucle jeu ────────────────────────────────────────────
  while (running) {
    // Bouton A → tourne à gauche (sens antihoraire)
    if (digitalRead(PIN_BUTTON_A) == LOW) {
      dir = (dir + 3) % 4;
      delay(120);
    }
    // Bouton B → tourne à droite (sens horaire)
    if (digitalRead(PIN_BUTTON_B) == LOW) {
      dir = (dir + 1) % 4;
      delay(120);
    }

    if (millis() - lastMove >= (uint32_t)speed) {
      lastMove = millis();

      // Déplacer corps
      for (int i = slen - 1; i > 0; i--) {
        sx[i] = sx[i - 1];
        sy[i] = sy[i - 1];
      }
      // Déplacer tête
      switch (dir) {
        case 0: sy[0]--; break;
        case 1: sx[0]++; break;
        case 2: sy[0]++; break;
        case 3: sx[0]--; break;
      }

      // Collision murs
      if (sx[0] < 0 || sx[0] >= GRID_W || sy[0] < 0 || sy[0] >= GRID_H) {
        running = false; break;
      }
      // Collision corps
      for (int i = 1; i < slen; i++) {
        if (sx[0] == sx[i] && sy[0] == sy[i]) { running = false; break; }
      }
      if (!running) break;

      // Manger
      if (sx[0] == fx && sy[0] == fy) {
        score++;
        if (slen < MAX_LEN) slen++;
        // Accélérer progressivement
        speed = max(80, 180 - score * 5);
        _placeFood();
      }

      // Rendu
      ui_begin();
      _drawHUD();
      _drawGame();
      ui_push();
    }
    delay(5);
  }

  // ── Game Over ─────────────────────────────────────────────
  for (int f = 0; f < 5; f++) {
    ui_begin();
    _drawGame();
    spr.fillRect(40, SCREEN_H / 2 - 24, SCREEN_W - 80, 50, C_DARK);
    spr.drawRect(40, SCREEN_H / 2 - 24, SCREEN_W - 80, 50, danger());
    spr.setTextDatum(MC_DATUM);
    spr.setTextSize(2);
    spr.setTextColor(danger(), C_DARK);
    spr.drawString("GAME OVER", SCREEN_W / 2, SCREEN_H / 2 - 10);
    char sc[16]; sprintf(sc, "Score: %d", score);
    spr.setTextSize(1);
    spr.setTextColor(C_WHITE, C_DARK);
    spr.drawString(sc, SCREEN_W / 2, SCREEN_H / 2 + 10);
    ui_push();
    delay(300);

    ui_begin();
    _drawGame();
    ui_push();
    delay(150);
  }
  delay(1000);
  utils_waitBtn(PIN_BUTTON_A);
}
