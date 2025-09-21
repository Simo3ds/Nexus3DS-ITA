#include <3ds.h>
#include "fmt.h"
#include "draw.h"
#include "ifile.h"
#include "menu.h"
#include "menus.h"
#include "menus/config_extra.h"
#include "luma_config.h"

config_extra configExtra;

static int scrollOffset = 0;
static u32 lastSelectedHash = 0;
static int scrollDir = 1;
static int scrollWait = 0;
static const int scrollSpeed = 2;
static const int scrollWaitFrames = 40;
static const int scrollInitialWaitFrames = 15;

typedef struct {
    const char *displayText;
    bool *configValue;
    bool (*visibility)(void);
} ConfigMenuEntry;

static inline const char* ConfigExtra_GetCheckboxDisplay(bool value)
{
    return value ? "(x)" : "( )";
}

static bool CheckNotO2DS(void)
{
    return mcuInfoTableRead && mcuInfoTable[9] != 3;
}

static void ConfigExtra_DrawScrollableText(u32 xPos, u32 yPos, const char *text, bool selected)
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

void ConfigExtra_DrawDetailedMenu(void)
{
    static const ConfigMenuEntry configMenuEntries[] = {
        {"Automatically suppress LEDs", &configExtra.suppressLeds, NULL},
        {"Cut power to TWL Flashcards", &configExtra.cutSlotPower, NULL},
        {"Cut 3DS Wifi in sleep mode", &configExtra.cutSleepWifi, NULL},
        {"Include title ID in screenshot filename", &configExtra.includeScreenshotTitleId, NULL},
        {"Save screenshots in date folders", &configExtra.screenshotDateFolders, NULL},
        {"Combine top/bottom screenshots", &configExtra.screenshotCombined, NULL},
        {"Toggle bottom LCD backlight (start+select)", &configExtra.toggleLcdCombo, &CheckNotO2DS},
        {"Use Fahrenheit for temperature display", &configExtra.temperatureUnit, NULL}
    };

    s32 selected = 0;
    s32 nbOptions = sizeof(configMenuEntries) / sizeof(ConfigMenuEntry);

    s32 visibleCount = 0;
    s32 visibleMapping[sizeof(configMenuEntries) / sizeof(ConfigMenuEntry)];
    
    for (s32 i = 0; i < nbOptions; i++) {
        if (configMenuEntries[i].visibility == NULL || configMenuEntries[i].visibility()) {
            visibleMapping[visibleCount] = i;
            visibleCount++;
        }
    }

    if (visibleCount == 0) return;

    do {
        Draw_Lock();
        Draw_DrawMenuFrame("Extra config menu");

        s32 displayIndex = 0;
        for (s32 i = 0; i < nbOptions; i++) {
            const ConfigMenuEntry *entry = &configMenuEntries[i];

            if (entry->visibility != NULL && !entry->visibility()) {
                continue;
            }

            u32 yPos = 40 + displayIndex * SPACING_Y;
            const char *checkbox = ConfigExtra_GetCheckboxDisplay(*(entry->configValue));

            if (displayIndex == selected) {
                Draw_DrawString(15, yPos, COLOR_ORANGE, ">>");
                Draw_DrawString(250, yPos, COLOR_ORANGE, "<<        ");
                Draw_DrawString(35, yPos, COLOR_CYAN, checkbox);
                ConfigExtra_DrawScrollableText(59, yPos, entry->displayText, true);
            } else {
                Draw_DrawString(15, yPos, COLOR_GRAY, " *");
                Draw_DrawString(250, yPos, COLOR_WHITE, "  ");
                Draw_DrawString(35, yPos, COLOR_WHITE, checkbox);
                Draw_DrawString(59, yPos, COLOR_WHITE, entry->displayText);
            }

            displayIndex++;
        }
        
        Draw_FlushFramebuffer();
        Draw_Unlock();
        
        u32 pressed = waitInputWithTimeout(30);
        
        if(pressed & KEY_B)
            break;
        else if(pressed & KEY_A)
        {
            s32 actualIndex = visibleMapping[selected];
            bool *selectedValue = configMenuEntries[actualIndex].configValue;
            *selectedValue = !(*selectedValue);
            LumaConfig_RequestSaveSettings();
        }
        else if(pressed & KEY_DOWN)
            selected++;
        else if(pressed & KEY_UP)
            selected--;
            
        if(selected < 0)
            selected = visibleCount - 1;
        else if(selected >= visibleCount)
            selected = 0;
            
    } while(!menuShouldExit);
}

void ConfigExtra_ReadConfigExtra(void)
{
    s64 extraConfigFlags = 0;
    Result res = svcGetSystemInfo(&extraConfigFlags, 0x10000, 0x183);
    
    if (R_SUCCEEDED(res)) {
        configExtra.suppressLeds = (extraConfigFlags >> 0) & 1;
        configExtra.cutSlotPower = (extraConfigFlags >> 1) & 1;
        configExtra.cutSleepWifi = (extraConfigFlags >> 2) & 1;
        configExtra.includeScreenshotTitleId = (extraConfigFlags >> 3) & 1;
        configExtra.screenshotDateFolders = (extraConfigFlags >> 4) & 1;
        configExtra.screenshotCombined = (extraConfigFlags >> 5) & 1;
        configExtra.toggleLcdCombo = (extraConfigFlags >> 6) & 1;
        configExtra.temperatureUnit = (extraConfigFlags >> 7) & 1;
    } else {
        configExtra.suppressLeds = false;
        configExtra.cutSlotPower = false;
        configExtra.cutSleepWifi = false;
        configExtra.includeScreenshotTitleId = true;
        configExtra.screenshotDateFolders = true;
        configExtra.screenshotCombined = true;
        configExtra.toggleLcdCombo = false;
        configExtra.temperatureUnit = false;
    }
}
