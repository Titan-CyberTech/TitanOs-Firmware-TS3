#pragma once
#include "config.h"

// ─── Sous-menu WiFi ─────────────────────────────────────────
void attacks_wifiSubMenu();

// ─── Deauth ─────────────────────────────────────────────────
void attacks_deauthScan();
int  attacks_selectNetwork(int n);

// ─── Beacon Flood ───────────────────────────────────────────
void attacks_beaconFlood();

// ─── Evil Portal ────────────────────────────────────────────
void attacks_evilPortal();

// ─── BLE ────────────────────────────────────────────────────
void attacks_bleMenu();

// ─── Frame deauth (broadcast) ───────────────────────────────
extern const uint8_t DEAUTH_FRAME[26];
