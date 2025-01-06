#include <TFT_eSPI.h> // Bibliothèque pour l'écran TFT

// Définition des GPIO et autres constantes
#define BACKLIGHT_PIN 15
#define BUTTON_A 0
#define BUTTON_B 14

// Nouvelle version du firmware
const char* firmware_version = "1.0.6";

// Menu principal
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Infos", "Parametres"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

// Couleurs communes
const uint16_t COLOR_RED = TFT_eSPI().color565(255, 0, 0);
const uint16_t COLOR_WHITE = TFT_WHITE;
const uint16_t COLOR_ORANGE = TFT_eSPI().color565(255, 165, 0);
const uint16_t COLOR_VIOLET = TFT_eSPI().color565(138, 43, 226);
const uint16_t COLOR_BLUE = TFT_eSPI().color565(0, 0, 255);
const uint16_t COLOR_BLACK = TFT_BLACK;

uint16_t main_color = COLOR_VIOLET; // Couleur principale par défaut
int brightness = 255; // Luminosité par défaut (max)

// État des boutons
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// Instance de l'écran TFT
TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Configuration des GPIO
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, brightness);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  // Initialisation de l'écran
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  // Affichage initial
  showLoadingScreen();
  displayMenu();
}

void loop() {
  handleButtonPress();
}

void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
    enterCategory();
  } else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    nextCategory();
  }
}

bool isButtonPressed(int pin, bool& buttonState) {
  if (digitalRead(pin) == LOW && !buttonState) {
    buttonState = true;
    delay(200); // Anti-rebond
    return true;
  }
  if (digitalRead(pin) == HIGH && buttonState) {
    buttonState = false;
  }
  return false;
}

void showLoadingScreen() {
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Titan Firmware", tft.width() / 2, 50);

  int barWidth = tft.width() - 80;
  int barHeight = 20;
  int barX = (tft.width() - barWidth) / 2;
  int barY = tft.height() / 2 + 20;

  tft.fillRect(barX, barY, barWidth, barHeight, COLOR_BLACK);
  for (int i = 0; i <= barWidth; i++) {
    tft.fillRect(barX + i, barY, 1, barHeight, main_color);
    delay(10);
  }
  delay(500);
}

void displayMenu() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(menu_items[current_menu_index], tft.width() / 2, tft.height() / 2);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Titan Firmware", 5, 5);

  tft.setTextDatum(TR_DATUM);
  tft.drawString(firmware_version, tft.width() - 5, 5);

  String categoryText = "Categorie: " + String(current_menu_index + 1) + "/" + String(menu_size);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(categoryText, tft.width() - 5, tft.height() - 5);
}

void enterCategory() {
  if (strcmp(menu_items[current_menu_index], "Infos") == 0) {
    displaySettings();
  } else if (strcmp(menu_items[current_menu_index], "Parametres") == 0) {
    displayParameters();
  } else {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Entree dans: " + String(menu_items[current_menu_index]), tft.width() / 2, tft.height() / 2);
    delay(1000);
    displayMenu();
  }
}

void nextCategory() {
  current_menu_index = (current_menu_index + 1) % menu_size;
  displayMenu();
}

void displaySettings() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Informations T-Display", tft.width() / 2, 20);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  const char* infos[] = {
    "Puce: ESP32-S3",
    "Ecran: 170x320",
    "Flash: 16MB",
    "PSRAM: 8MB",
    "Etat de la batterie: N/A"
  };
  for (int i = 0; i < 5; i++) {
    tft.drawString(infos[i], 10, 50 + i * 20);
  }

  waitForButtonPress(BUTTON_A);
  displayMenu();
}

void displayParameters() {
  const char* colors[] = {"Rouge", "Blanc", "Orange", "Violet", "Bleu"};
  uint16_t color_values[] = {COLOR_RED, COLOR_WHITE, COLOR_ORANGE, COLOR_VIOLET, COLOR_BLUE};
  int color_index = 0;

  while (true) {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(main_color, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Parametres", tft.width() / 2, 20);

    tft.setTextSize(2);
    tft.drawString("Couleur principale:", tft.width() / 2, tft.height() / 2 - 30);
    tft.setTextSize(3);
    tft.drawString(colors[color_index], tft.width() / 2, tft.height() / 2);

    tft.setTextSize(2);
    tft.drawString("Luminosite: " + String(brightness), tft.width() / 2, tft.height() / 2 + 50);

    // Bouton haut : Augmenter la luminosité
    if (digitalRead(BUTTON_A) == LOW) {
      brightness += 20;
      if (brightness > 255) brightness = 20;
      analogWrite(BACKLIGHT_PIN, brightness);
      delay(200); // Anti-rebond
    }

    // Bouton bas : Changer de couleur ou quitter avec un appui long
    if (digitalRead(BUTTON_B) == LOW) {
      unsigned long press_time = millis();
      while (digitalRead(BUTTON_B) == LOW) {
        if (millis() - press_time > 1000) {
          displayMenu();
          return;
        }
      }

      // Changer de couleur
      color_index = (color_index + 1) % 5;
      main_color = color_values[color_index];
      delay(200); // Anti-rebond
    }

    delay(100);
  }
}


void waitForButtonPress(int pin) {
  while (true) {
    if (digitalRead(pin) == LOW) {
      delay(200);
      break;
    }
  }
}