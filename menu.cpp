#include "menu.h"
#include "display.h"
#include "attacks.h"
#include "games.h"
#include "utils.h"

const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Games", "Settings", "Info", "Battery Info", "Turn Off"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed))
    enterCategory();
  else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    current_menu_index = (current_menu_index + 1) % menu_size;
    displayMenu();
  }
}

void enterCategory() {
  String currentItem = menu_items[current_menu_index];
  if (currentItem == "Wifi Attack") {
    displayWifiAttackSubMenu();
  } else if (currentItem == "Info") {
    displayInfo();
  } else if (currentItem == "Settings") {
    displaySettings();
  } else if (currentItem == "Battery Info") {
    displayBatteryInfo();
  } else if (currentItem == "BLE Attack") {
    displayBLEAttack();
  } else if (currentItem == "Games") {
    displaySnakeGame();
  } else if (currentItem == "Turn Off") {
    turnOffDisplay();
  } else {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Unknown Menu", tft.width()/2, tft.height()/2);
    delay(1000);
    displayMenu();
  }
}