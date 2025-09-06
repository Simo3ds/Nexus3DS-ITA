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

static int scrollOffset = 0;
static u32 lastSelectedHash = 0;
static int scrollDir = 1;
static int scrollWait = 0;
static const int scrollSpeed = 2;
static const int scrollWaitFrames = 40;
static const int scrollInitialWaitFrames = 15;

static void PluginOptions_DrawScrollableText(u32 xPos, u32 yPos, const char *text, bool selected)
{
    if (!selected) {
        Draw_DrawString(xPos, yPos, COLOR_WHITE, text);
        return;
    }
    
    u32 currentHash = yPos ^ ((u32)text);

    if (lastSelectedHash != currentHash) {
        scrollOffset = 0;
        lastSelectedHash = currentHash;
        scrollDir = 1;
        scrollWait = scrollInitialWaitFrames;
    }
    
    int textLen = strlen(text);
    int maxTextChars = 32;
    
    if (textLen > maxTextChars) {
        int maxOffset = (textLen - maxTextChars) * 8;
        
        if (scrollWait > 0) {
            scrollWait--;
        } else {
            scrollOffset += scrollSpeed * scrollDir;
            if (scrollDir == 1 && scrollOffset >= maxOffset) {
                scrollOffset = maxOffset;
                scrollWait = scrollWaitFrames;
                scrollDir = -1;
            } else if (scrollDir == -1 && scrollOffset <= 0) {
                scrollOffset = 0;
                scrollWait = scrollWaitFrames;
                scrollDir = 1;
            }
        }
        
        char buf[33];
        int startChar = scrollOffset / 8;
        strncpy(buf, text + startChar, maxTextChars);
        buf[maxTextChars] = '\0';
        Draw_DrawString(xPos, yPos, COLOR_CYAN, buf);
    } else {
        Draw_DrawString(xPos, yPos, COLOR_CYAN, text);
    }
}

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
        "Detects internet connections in 3gx",
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
            u32 yPos = 40 + i * SPACING_Y;
            const char *checkbox = (*watchLv & (1 << i)) ? "(x)" : "( )";
            
            if (i == selected) {
                Draw_DrawString(15, yPos, COLOR_ORANGE, ">>");
                Draw_DrawString(250, yPos, COLOR_ORANGE, "<<        ");
                Draw_DrawString(35, yPos, COLOR_CYAN, checkbox);
                PluginOptions_DrawScrollableText(59, yPos, watchOptions[i], true);
            } else {
                Draw_DrawString(15, yPos, COLOR_GRAY, " *");
                Draw_DrawString(250, yPos, COLOR_WHITE, "  ");
                Draw_DrawString(35, yPos, COLOR_WHITE, checkbox);
                Draw_DrawString(59, yPos, COLOR_WHITE, watchOptions[i]);
            }
        }

        Draw_FlushFramebuffer();
        Draw_Unlock();

        u32 pressed = waitInputWithTimeout(30);

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