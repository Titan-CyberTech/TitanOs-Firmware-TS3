#include "display.h"
#include "menu.h"
#include "utils.h"

const int SPLASH_SCREEN_DURATION = 5000;
const int TEXT_SIZE_LARGE = 6;
const int TEXT_SIZE_MEDIUM = 3;
const int TEXT_SIZE_SMALL = 1;

const int BATTERY_VOLTAGE_MIN = 3000;
const int BATTERY_VOLTAGE_MAX = 3700;

esp_adc_cal_characteristics_t adc_chars;

void setupBatteryADC() {
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
}

void setTextAttributes(int size, int color, int bgColor, int datum) {
  tft.setTextSize(size);
  tft.setTextColor(color, bgColor);
  tft.setTextDatum(datum);
}

void showSplashScreen() {
  tft.fillScreen(COLOR_BLACK);

  setTextAttributes(TEXT_SIZE_LARGE, COLOR_WHITE, COLOR_BLACK, MC_DATUM);
  tft.drawString("Titan", tft.width() / 2, tft.height() / 2 - 25);

  setTextAttributes(TEXT_SIZE_LARGE, COLOR_VIOLET, COLOR_BLACK, MC_DATUM);
  tft.drawString("Os", tft.width() / 2, tft.height() / 2 + 35);

  delay(SPLASH_SCREEN_DURATION);
}

void displayMenu() {
  tft.fillScreen(COLOR_BLACK);

  setTextAttributes(TEXT_SIZE_MEDIUM, main_color, COLOR_BLACK, MC_DATUM);
  tft.drawString(menu_items[current_menu_index], tft.width() / 2, tft.height() / 2);

  setTextAttributes(TEXT_SIZE_SMALL, main_color, COLOR_BLACK, TL_DATUM);
  tft.drawString("Titan Firmware", 5, 5);

  tft.drawFastHLine(5, 20, tft.width() - 10, main_color);

  setTextAttributes(TEXT_SIZE_SMALL, main_color, COLOR_BLACK, BL_DATUM);
  tft.drawString(firmware_version, 5, tft.height() - 5);

  String categoryText = "Category: " + String(current_menu_index + 1) + "/" + String(menu_size);
  setTextAttributes(TEXT_SIZE_SMALL, main_color, COLOR_BLACK, BR_DATUM);
  tft.drawString(categoryText, tft.width() - 5, tft.height() - 5);

  tft.drawFastHLine(5, tft.height() - 25, tft.width() - 10, main_color);

  displayBatteryPercentage();
}

void displayBatteryPercentage() {
  uint32_t voltage = readBatteryVoltage();
  if (!isBatteryConnected(voltage)) {
    setTextAttributes(TEXT_SIZE_SMALL, COLOR_RED, COLOR_BLACK, TR_DATUM);
    tft.drawString("No Bat", tft.width() - 5, 5);
    return;
  }
  int percentage = map(voltage, BATTERY_VOLTAGE_MIN, BATTERY_VOLTAGE_MAX, 0, 100);
  percentage = constrain(percentage, 0, 100);
  setTextAttributes(TEXT_SIZE_SMALL, main_color, COLOR_BLACK, TR_DATUM);
  tft.drawString(String(percentage) + "%", tft.width() - 5, 5);
}