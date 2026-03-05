#pragma once
#include "config.h"

// ─── Sous-menu WiFi ─────────────────────────────────────────
void attacks_wifiSubMenu();

// ─── Deauth ─────────────────────────────────────────────────
void attacks_deauthScan();           // scan + sélection + attaque
int  attacks_selectNetwork(int n);   // UI sélection réseau

// ─── Evil Portal ────────────────────────────────────────────
void attacks_evilPortal();

// ─── BLE ────────────────────────────────────────────────────
void attacks_bleMenu();

// ─── Frame deauth (broadcast) ───────────────────────────────
extern const uint8_t DEAUTH_FRAME[26];
