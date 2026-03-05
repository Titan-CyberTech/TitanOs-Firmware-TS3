#include "menu.h"
#include "utils.h"
#include "attacks.h"
#include "games.h"
#include "screens.h"

static const MenuItem MAIN_MENU[MENU_COUNT] = {
  { "WiFi Attack",  "W>" },
  { "BLE Attack",   "B>" },
  { "Games",        ":)" },
  { "Settings",     "**" },
  { "System Info",  "i " },
  { "Battery",      "[] " },
  { "Power Off",    "X " },
};

static int s_selected = 0;

void menu_run() {
  ui_drawMenu(MAIN_MENU, MENU_COUNT, s_selected);

  while (true) {
    if (utils_btnA()) {
      menu_enter(s_selected);
      // Après retour d'une action, on redessine le menu
      ui_drawMenu(MAIN_MENU, MENU_COUNT, s_selected);
    }
    if (utils_btnB()) {
      s_selected = (s_selected + 1) % MENU_COUNT;
      ui_drawMenu(MAIN_MENU, MENU_COUNT, s_selected);
    }
    delay(20);
  }
}

void menu_enter(int id) {
  switch (id) {
    case MENU_WIFI_ATTACK:  attacks_wifiSubMenu();    break;
    case MENU_BLE_ATTACK:   attacks_bleMenu();        break;
    case MENU_GAMES:        games_snakeRun();         break;
    case MENU_SETTINGS:     screens_settings();       break;
    case MENU_INFO:         screens_sysInfo();        break;
    case MENU_BATTERY:      screens_batteryInfo();    break;
    case MENU_TURN_OFF:     screens_powerOff();       break;
    default: break;
  }
}
