#ifndef MENU_H
#define MENU_H

#include "config.h"

extern const char* menu_items[];
extern const int menu_size;
extern int current_menu_index;

void handleButtonPress();
void enterCategory();

#endif // MENU_H