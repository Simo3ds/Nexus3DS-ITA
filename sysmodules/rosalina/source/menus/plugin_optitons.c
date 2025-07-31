#include "menus/plugin_options.h"
#include "menu.h"
#include "menus.h"
#include "plugin.h"
#include "draw.h"
#include "luma_config.h"

#include <stdio.h>
#include <string.h>

extern bool PluginChecker_isEnabled;
extern bool PluginWatcher_isEnabled;
extern bool PluginWatcher_isRunning;
extern bool PluginConverter_UseCache;
extern u32 PluginWatcher_WatchLevel;

Menu pluginOptionsMenu = {
    "Plugin settings menu",
    {
        { "", METHOD, .method = &PluginLoaderOptions__MenuCallback },
        { "", METHOD, .method = &PluginChecker__MenuCallback },
        { "", METHOD, .method = &PluginWatcher__MenuCallback },
        { "Set watch level", METHOD, .method = &PluginWatcher_SetWatchLevel },
        { "", METHOD, .method = &PluginConverter__ToggleUseCacheFlag },
        {},
    }
};

void PluginLoaderOptions__MenuCallback(void)
{
    PluginLoaderCtx.isEnabled = !PluginLoaderCtx.isEnabled;
    LumaConfig_RequestSaveSettings();
    PluginLoader__UpdateMenu();
    PluginLoaderOptions__UpdateMenu();
}

void PluginLoaderOptions__UpdateMenu(void)
{
    static const char *status[2] =
    {
        "Plugin Loader: [Disabled]",
        "Plugin Loader: [Enabled]"
    };

    rosalinaMenu.items[3].menu->items[0].title = status[PluginLoaderCtx.isEnabled];
}

void PluginChecker__MenuCallback(void)
{
    PluginChecker_isEnabled = !PluginChecker_isEnabled;
    LumaConfig_RequestSaveSettings();
    PluginChecker__UpdateMenu();
}

void PluginChecker__UpdateMenu(void)
{
    static const char *status[2] =
    {
        "Plugin Checker: [Disabled]",
        "Plugin Checker: [Enabled]"
    };

    rosalinaMenu.items[3].menu->items[1].title = status[PluginChecker_isEnabled];
}

void PluginWatcher__MenuCallback(void)
{
    PluginWatcher_isEnabled = !PluginWatcher_isEnabled;
    LumaConfig_RequestSaveSettings();
    PluginWatcher__UpdateMenu();

    if(PluginWatcher_isEnabled)
    {
        PluginLoaderContext *ctx = &PluginLoaderCtx;

        if(ctx->target != 0 && !ctx->pluginIsHome && !ctx->pluginIsSwapped)
            PluginWatcher_isRunning = true;
    }
    else
        PluginWatcher_isRunning = false;
}

void PluginWatcher__UpdateMenu(void)
{
    static const char *status[2] =
    {
        "Plugin Watcher: [Disabled]",
        "Plugin Watcher: [Enabled]"
    };
    rosalinaMenu.items[3].menu->items[2].title = status[PluginWatcher_isEnabled];
}

void PluginConverter__ToggleUseCacheFlag(void)
{
    PluginConverter_UseCache = !PluginConverter_UseCache;
    LumaConfig_RequestSaveSettings();
    PluginConverter__UpdateMenu();
}

void PluginConverter__UpdateMenu(void)
{
    static const char *status[2] =
    {
        "Use cache in plugin converter: [OFF]",
        "Use cache in plugin converter: [ON]"
    };
    rosalinaMenu.items[3].menu->items[4].title = status[PluginConverter_UseCache];
}

void PluginWatcher_SetWatchLevel(void)
{
    static const char *watchOptions[] = {
        "Detects file deletion",
        "Detects directory deletion",
        "Detects internet connections",
        "Detects camera access",
    };

    s32 selected = 0;
    s32 nbOptions = sizeof(watchOptions) / sizeof(const char *);
    u32 *watchLv = &PluginWatcher_WatchLevel;

    do
    {
        Draw_Lock();
        Draw_DrawMenuFrame("Set watch level");

        for(s32 i = 0; i < nbOptions; i++)
        {
            char buf[256];
            const char *checkbox = (*watchLv & (1 << i)) ? "(x)" : "( )";
            sprintf(buf, "%s %s", checkbox, watchOptions[i]);
            u32 yPos = 40 + i * SPACING_Y;
            Draw_DrawMenuCursor(yPos, (i == selected), buf);
        }

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed;
        do
        {
            pressed = waitInputWithTimeout(50);
        } while(pressed == 0 && !menuShouldExit);

        if(pressed & KEY_B)
            break;
        else if(pressed & KEY_A)
        {
            *watchLv ^= (1 << selected);
            LumaConfig_RequestSaveSettings();
        }
        else if(pressed & KEY_DOWN)
        {
            selected++;
            if(selected >= nbOptions)
                selected = 0;
        }
        else if(pressed & KEY_UP)
        {
            selected--;
            if(selected < 0)
                selected = nbOptions - 1;
        }

    } while(!menuShouldExit);
}