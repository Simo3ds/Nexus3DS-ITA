#pragma once

#include "menu.h"

extern Menu pluginOptionsMenu;

void PluginChecker__MenuCallback(void);
void PluginChecker__UpdateMenu(void);
void PluginWatcher__MenuCallback(void);
void PluginWatcher__UpdateMenu(void);

void PluginWatcher_SetWatchLevel(void);