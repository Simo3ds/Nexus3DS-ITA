#include <3ds.h>
#include "fmt.h"
#include "draw.h"
#include "ifile.h"
#include "menu.h"
#include "menus.h"
#include "menus/config_extra.h"
#include "luma_config.h"

#define EXTRACONFIG_GET(flag) ((extraConfigFlags >> (flag)) & 1)
#define EXTRACONFIG_SET(flag) (extraConfigFlags |= (1 << (flag)))
#define EXTRACONFIG_CLEAR(flag) (extraConfigFlags &= ~(1 << (flag)))

enum {
    EXTRA_FLAG_SUPPRESS_LEDS = 0,
    EXTRA_FLAG_CUT_SLOT_POWER = 1,
    EXTRA_FLAG_CUT_SLEEP_WIFI = 2,
    EXTRA_FLAG_INCLUDE_SCREENSHOT_TITLE_ID = 3,
    EXTRA_FLAG_SCREENSHOT_DATE_FOLDERS = 4,
    EXTRA_FLAG_SCREENSHOT_COMBINED = 5,
    EXTRA_FLAG_TOGGLE_LCD_COMBO = 6,
    EXTRA_FLAG_COUNT
};

static u32 extraConfigFlags = 0;

typedef struct {
    const char *name;
    bool *configPtr;
    int flagIndex;
} ConfigItem;

static const ConfigItem configItems[] = {
    { "Automatically suppress LEDs", &configExtra.suppressLeds, EXTRA_FLAG_SUPPRESS_LEDS },
    { "Cut power to TWL Flashcards", &configExtra.cutSlotPower, EXTRA_FLAG_CUT_SLOT_POWER },
    { "Cut 3DS Wifi in sleep mode", &configExtra.cutSleepWifi, EXTRA_FLAG_CUT_SLEEP_WIFI },
    { "Include title ID in screenshot filename", &configExtra.includeScreenshotTitleId, EXTRA_FLAG_INCLUDE_SCREENSHOT_TITLE_ID },
    { "Save screenshots in date folders", &configExtra.screenshotDateFolders, EXTRA_FLAG_SCREENSHOT_DATE_FOLDERS },
    { "Combine top/bottom screenshots", &configExtra.screenshotCombined, EXTRA_FLAG_SCREENSHOT_COMBINED },
    { "Toggle bottom LCD backlight (start+select)", &configExtra.toggleLcdCombo, EXTRA_FLAG_TOGGLE_LCD_COMBO }
};

static const s32 nbConfigItems = sizeof(configItems) / sizeof(ConfigItem);

config_extra configExtra = { .suppressLeds = false, .cutSlotPower = false, .cutSleepWifi = false, .includeScreenshotTitleId = true, .screenshotDateFolders = true, .screenshotCombined = true, .toggleLcdCombo = false };
bool configExtraSaved = false;

static int scrollOffset = 0;
static u32 lastSelectedHash = 0;
static int scrollDir = 1;
static int scrollWait = 0;
static const int scrollSpeed = 2;
static const int scrollWaitFrames = 40;
static const int scrollInitialWaitFrames = 15;

static inline const char* ConfigExtra_GetCheckboxDisplay(bool value)
{
    return value ? "(x)" : "( )";
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
    s32 selected = 0;
    
    do {
        Draw_Lock();
        Draw_DrawMenuFrame("Extra config menu");

        for (s32 i = 0; i < nbConfigItems; i++) {
            u32 yPos = 40 + i * SPACING_Y;
            const char *checkbox = ConfigExtra_GetCheckboxDisplay(*configItems[i].configPtr);
            
            if (i == selected) {
                Draw_DrawString(15, yPos, COLOR_ORANGE, ">>");
                Draw_DrawString(250, yPos, COLOR_ORANGE, "<<        ");
                Draw_DrawString(35, yPos, COLOR_CYAN, checkbox);
                ConfigExtra_DrawScrollableText(59, yPos, configItems[i].name, true);
            } else {
                Draw_DrawString(15, yPos, COLOR_GRAY, " *");
                Draw_DrawString(250, yPos, COLOR_WHITE, "  ");
                Draw_DrawString(35, yPos, COLOR_WHITE, checkbox);
                Draw_DrawString(59, yPos, COLOR_WHITE, configItems[i].name);
            }
        }
        
        Draw_FlushFramebuffer();
        Draw_Unlock();
        
        u32 pressed = waitInputWithTimeout(30);
        
        if(pressed & KEY_B)
            break;
        else if(pressed & KEY_A)
        {
            *configItems[selected].configPtr = !(*configItems[selected].configPtr);

            if (*configItems[selected].configPtr) {
                EXTRACONFIG_SET(configItems[selected].flagIndex);
            } else {
                EXTRACONFIG_CLEAR(configItems[selected].flagIndex);
            }
            
            ConfigExtra_WriteConfigExtra();
            configExtraSaved = true;
        }
        else if(pressed & KEY_DOWN)
            selected++;
        else if(pressed & KEY_UP)
            selected--;
            
        if(selected < 0)
            selected = nbConfigItems - 1;
        else if(selected >= nbConfigItems)
            selected = 0;
            
    } while(!menuShouldExit);
}

void ConfigExtra_ReadConfigExtra(void)
{
    extraConfigFlags = 0;
    for (s32 i = 0; i < nbConfigItems; i++) {
        if (*configItems[i].configPtr) {
            EXTRACONFIG_SET(configItems[i].flagIndex);
        }
    }
}

void ConfigExtra_WriteConfigExtra(void)
{
    for (s32 i = 0; i < nbConfigItems; i++) {
        *configItems[i].configPtr = EXTRACONFIG_GET(configItems[i].flagIndex);
    }

    LumaConfig_RequestSaveSettings();
}
