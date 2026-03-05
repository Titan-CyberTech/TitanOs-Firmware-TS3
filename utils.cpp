#include "utils.h"
#include <Preferences.h>

static Preferences prefs;
static esp_adc_cal_characteristics_t adc_chars;

// ─── Boutons ────────────────────────────────────────────────
void utils_initButtons() {
  pinMode(PIN_BUTTON_A, INPUT_PULLUP);
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);
}

bool utils_btnA() {
  if (digitalRead(PIN_BUTTON_A) == LOW && !g_btnA_state) {
    g_btnA_state = true;
    delay(BTN_DEBOUNCE_MS);
    return true;
  }
  if (digitalRead(PIN_BUTTON_A) == HIGH) g_btnA_state = false;
  return false;
}

bool utils_btnB() {
  if (digitalRead(PIN_BUTTON_B) == LOW && !g_btnB_state) {
    g_btnB_state = true;
    delay(BTN_DEBOUNCE_MS);
    return true;
  }
  if (digitalRead(PIN_BUTTON_B) == HIGH) g_btnB_state = false;
  return false;
}

bool utils_btnAHeld(uint32_t ms) {
  if (digitalRead(PIN_BUTTON_A) != LOW) return false;
  uint32_t t = millis();
  while (digitalRead(PIN_BUTTON_A) == LOW) {
    if (millis() - t >= ms) return true;
    delay(10);
  }
  return false;
}

// FIX BUG BATTERIE : après waitBtn, on vide l'état des boutons
// pour éviter que menu_run() détecte un faux appui au retour.
void utils_waitBtn(int pin) {
  while (digitalRead(pin) != LOW) delay(20);
  delay(BTN_DEBOUNCE_MS);
  // Purge des états boutons — évite le re-déclenchement immédiat
  g_btnA_state = true;   // marque "déjà pressé" → ne se re-déclenchera pas
  g_btnB_state = true;   // idem pour B
  // Attendre le relâchement complet
  while (digitalRead(pin) == LOW) delay(10);
  delay(BTN_DEBOUNCE_MS);
  g_btnA_state = false;
  g_btnB_state = false;
}

// ─── Batterie ───────────────────────────────────────────────
void utils_initADC() {
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11,
                           ADC_WIDTH_BIT_12, 1100, &adc_chars);
  pinMode(PIN_BAT_VOLT, INPUT);
}

uint32_t utils_readBattVoltage() {
  uint32_t sum = 0;
  for (int i = 0; i < 8; i++) {
    sum += analogRead(PIN_BAT_VOLT);
    delay(2);
  }
  uint32_t raw = sum / 8;
  return esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;
}

bool utils_isBattConnected(uint32_t mV) {
  return (mV > BAT_VOLT_ABSENT && mV < 5500);
}

int utils_battPercent() {
  uint32_t v = utils_readBattVoltage();
  if (!utils_isBattConnected(v)) return -1;
  return constrain((int)map(v, BAT_VOLT_MIN, BAT_VOLT_MAX, 0, 100), 0, 100);
}

// ─── Backlight ──────────────────────────────────────────────
void utils_initBacklight() {
  pinMode(PIN_BACKLIGHT, OUTPUT);
  ledcSetup(LEDC_CH, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(PIN_BACKLIGHT, LEDC_CH);
  ledcWrite(LEDC_CH, g_brightness);
}

void utils_setBrightness(int val) {
  g_brightness = constrain(val, 10, 255);
  ledcWrite(LEDC_CH, g_brightness);
}

// ─── Persistance NVS ────────────────────────────────────────
void utils_loadSettings() {
  prefs.begin("titan", true);
  g_themeIndex = prefs.getInt("theme",      0);
  g_brightness = prefs.getInt("brightness", 200);
  prefs.end();
}

void utils_saveSettings() {
  prefs.begin("titan", false);
  prefs.putInt("theme",      g_themeIndex);
  prefs.putInt("brightness", g_brightness);
  prefs.end();
}

// ─── Deep sleep ─────────────────────────────────────────────
void utils_deepSleep() {
  utils_saveSettings();
  ledcWrite(LEDC_CH, 0);
  tft.writecommand(0x10);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_A, 0);
  delay(200);
  esp_deep_sleep_start();
}
