#pragma once
#include "data/constants.h"

void renderMenu(MenuState menuType);
void renderMenu(MenuState menuType, bool animate_menu);
void teardownAllMenus();
MenuState getCurrentMenu();