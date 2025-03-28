#ifndef UTILS_H
#define UTILS_H

#include "config.h"

extern bool buttonA_pressed;
extern bool buttonB_pressed;

void waitForButtonPress(int pin);
bool isButtonPressed(int pin, bool &buttonState);
void sendDeauthFrameToAll(int channel);
void continuousDeauthAttackToAll(int channel);
void displayAttackScreen();
void updatePacketCountDisplay(uint32_t packetCount);
void displayInfo();
void displayBatteryInfo();
void displaySettings();
void displaySettingsScreen(const char* colors[], int color_index);
void turnOffDisplay();

#endif // UTILS_H
