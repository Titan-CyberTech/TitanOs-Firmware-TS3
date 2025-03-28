#ifndef ATTACKS_H
#define ATTACKS_H

#include "config.h"

// Déclaration avec une taille fixe
extern const uint8_t deauthFrame[26];
extern const char* evilPortalHtml;

void displayWifiAttackSubMenu();
void displayWifiDeauthAttack();
void displayWifiEvilPortal();
void displayBLEAttack();
void displayWifiAttackOptions(int attackOption);
int selectWifiNetwork(int n);
void displayWifiNetworks(int targetIndex, int &pageStart, int pageSize, int listY, int listH, int n);
void handleCapturedCredentials();
void displayEvilPortalRunningScreen();

#endif // ATTACKS_H
