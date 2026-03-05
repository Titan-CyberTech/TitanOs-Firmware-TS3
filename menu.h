#pragma once
#include "config.h"
#include "ui.h"

// ID des entrées menu principal
enum MenuID {
  MENU_WIFI_ATTACK = 0,
  MENU_BLE_ATTACK,
  MENU_GAMES,
  MENU_SETTINGS,
  MENU_INFO,
  MENU_BATTERY,
  MENU_TURN_OFF,
  MENU_COUNT
};

void menu_run();         // boucle principale du menu
void menu_enter(int id); // exécuter l'action d'un item
