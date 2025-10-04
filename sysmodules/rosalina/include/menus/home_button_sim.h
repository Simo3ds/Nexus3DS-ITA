#pragma once

#include <3ds/types.h>
#include "menu.h"

extern Menu homeButtonSimMenu;
extern bool hideReturnToHomeMenu;
extern bool enableHomeButtonCombo;
extern u32 homeButtonCombo;

void HomeButtonSimMenu_ToggleReturnToHomeMenu(void);
bool HomeButtonSimMenu_ShouldShowReturnToHomeMenu(void);
void HomeButtonSimMenu_ToggleEnableCombo(void);
void HomeButtonSimMenu_LoadConfig(void);
void HomeButtonSimMenu_ChangeCombo(void);
