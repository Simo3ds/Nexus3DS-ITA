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
    static const char *configOptions[] = {
        "Automatically suppress LEDs",
        "Cut power to TWL Flashcards", 
        "Cut 3DS Wifi in sleep mode",
        "Include title ID in screenshot filename",
        "Save screenshots in date folders",
        "Combine top/bottom screenshots",
        "Toggle bottom LCD backlight (start+select)"
    };
    
    bool *configValues[] = {
        &configExtra.suppressLeds,
        &configExtra.cutSlotPower,
        &configExtra.cutSleepWifi,
        &configExtra.includeScreenshotTitleId,
        &configExtra.screenshotDateFolders,
        &configExtra.screenshotCombined,
        &configExtra.toggleLcdCombo
    };
    
    s32 selected = 0;
    s32 nbOptions = sizeof(configOptions) / sizeof(const char *);
    
    do {
        Draw_Lock();
        Draw_DrawMenuFrame("Extra config menu");

        for (s32 i = 0; i < nbOptions; i++) {
            u32 yPos = 40 + i * SPACING_Y;
            const char *checkbox = ConfigExtra_GetCheckboxDisplay(*configValues[i]);
            
            if (i == selected) {
                Draw_DrawString(15, yPos, COLOR_ORANGE, ">>");
                Draw_DrawString(250, yPos, COLOR_ORANGE, "<<        ");
                Draw_DrawString(35, yPos, COLOR_CYAN, checkbox);
                ConfigExtra_DrawScrollableText(59, yPos, configOptions[i], true);
            } else {
                Draw_DrawString(15, yPos, COLOR_GRAY, " *");
                Draw_DrawString(250, yPos, COLOR_WHITE, "  ");
                Draw_DrawString(35, yPos, COLOR_WHITE, checkbox);
                Draw_DrawString(59, yPos, COLOR_WHITE, configOptions[i]);
            }
        }
        
        Draw_FlushFramebuffer();
        Draw_Unlock();
        
        u32 pressed = waitInputWithTimeout(30);
        
        if(pressed & KEY_B)
            break;
        else if(pressed & KEY_A)
        {
            *configValues[selected] = !(*configValues[selected]);
            LumaConfig_RequestSaveSettings();
        }
        else if(pressed & KEY_DOWN)
            selected++;
        else if(pressed & KEY_UP)
            selected--;
            
        if(selected < 0)
            selected = nbOptions - 1;
        else if(selected >= nbOptions)
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
    } else {
        configExtra.suppressLeds = false;
        configExtra.cutSlotPower = false;
        configExtra.cutSleepWifi = false;
        configExtra.includeScreenshotTitleId = true;
        configExtra.screenshotDateFolders = true;
        configExtra.screenshotCombined = true;
        configExtra.toggleLcdCombo = false;
    }
}
