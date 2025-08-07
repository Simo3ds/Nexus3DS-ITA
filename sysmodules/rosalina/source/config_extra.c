#include <3ds.h>
#include "fmt.h"
#include "draw.h"
#include "ifile.h"
#include "menu.h"
#include "menus.h"
#include "menus/config_extra.h"

config_extra configExtra = { .suppressLeds = false, .cutSlotPower = false, .cutSleepWifi = false, .includeScreenshotTitleId = true, .screenshotDateFolders = true, .toggleLcdCombo = false };
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
    static const char *configOptions[] = {
        "Automatically suppress LEDs",
        "Cut power to TWL Flashcards", 
        "Cut 3DS Wifi in sleep mode",
        "Include title ID in screenshot filename",
        "Save screenshots in date folders",
        "Toggle bottom LCD backlight (start+select)"
    };
    
    bool *configValues[] = {
        &configExtra.suppressLeds,
        &configExtra.cutSlotPower,
        &configExtra.cutSleepWifi,
        &configExtra.includeScreenshotTitleId,
        &configExtra.screenshotDateFolders,
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
            ConfigExtra_WriteConfigExtra();
            configExtraSaved = true;
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

void ConfigExtra_Init(void)
{
    ConfigExtra_ReadConfigExtra();
}

void ConfigExtra_ReadConfigExtra(void)
{
    IFile file;
    Result res = 0;

    res = IFile_Open(&file, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""),
            fsMakePath(PATH_ASCII, "/luma/configExtra.bin"), FS_OPEN_READ);

    if(R_SUCCEEDED(res))
    {
        u64 total;
        res = IFile_Read(&file, &total, &configExtra, sizeof(configExtra));
        IFile_Close(&file);
        if(R_SUCCEEDED(res)) 
        {
            configExtraSaved = true;
        }
    }
}

void ConfigExtra_WriteConfigExtra(void)
{
    IFile file;
    Result res = 0;

    res = IFile_Open(&file, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""),
            fsMakePath(PATH_ASCII, "/luma/configExtra.bin"), FS_OPEN_CREATE | FS_OPEN_WRITE);

    if(R_SUCCEEDED(res))
    {
        u64 total;
        res = IFile_Write(&file, &total, &configExtra, sizeof(configExtra), 0);
        IFile_Close(&file);

        if(R_SUCCEEDED(res)) 
        {
            configExtraSaved = true;
        }
    }
}