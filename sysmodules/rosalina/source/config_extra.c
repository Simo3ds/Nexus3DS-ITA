#include <3ds.h>
#include "fmt.h"
#include "draw.h"
#include "ifile.h"
#include "menu.h"
#include "menus.h"
#include "menus/config_extra.h"

config_extra configExtra = { .suppressLeds = false, .cutSlotPower = false, .cutSleepWifi = false };
bool configExtraSaved = false;

static inline const char* ConfigExtra_GetCheckboxDisplay(bool value)
{
    return value ? "(x)" : "( )";
}

void ConfigExtra_DrawDetailedMenu(void)
{
    static const char *configOptions[] = {
        "Automatically suppress LEDs",
        "Cut power to TWL Flashcards", 
        "Cut 3DS Wifi in sleep mode"
    };
    
    bool *configValues[] = {
        &configExtra.suppressLeds,
        &configExtra.cutSlotPower,
        &configExtra.cutSleepWifi
    };
    
    s32 selected = 0;
    s32 nbOptions = sizeof(configOptions) / sizeof(const char *);
    
    do {
        Draw_Lock();
        Draw_DrawString(10, 8, COLOR_CYAN, "+");
        for (u32 i = 0; i < 35; i++) Draw_DrawCharacter(18 + i * 6, 8, COLOR_CYAN, '-');
        Draw_DrawString(222, 8, COLOR_CYAN, "+");
        Draw_DrawString(10, 16, COLOR_CYAN, "|");
        Draw_DrawString(222, 16, COLOR_CYAN, "|");
        Draw_DrawString(20, 16, COLOR_ORANGE, "Extra config menu");
        Draw_DrawString(10, 24, COLOR_CYAN, "+");
        for (u32 i = 0; i < 35; i++) Draw_DrawCharacter(18 + i * 6, 24, COLOR_CYAN, '-');
        Draw_DrawString(222, 24, COLOR_CYAN, "+");

        static int scrollOffset = 0;
        static int lastSelected = -1;
        static int scrollDir = 1;
        static int scrollWait = 0;
        const int scrollSpeed = 2;
        const int scrollWaitFrames = 40;
        const int scrollInitialWaitFrames = 20;

        for (s32 i = 0; i < nbOptions; i++) {
            u32 yPos = 40 + i * SPACING_Y;
            char buf[128];
            const char *checkbox = ConfigExtra_GetCheckboxDisplay(*configValues[i]);
            sprintf(buf, "%s %s", checkbox, configOptions[i]);
            char *paren = strchr(buf, '(');
            size_t scrollLen = paren ? (size_t)(paren - buf) : strlen(buf);
            int titleLen = (int)scrollLen;
            if (i == selected) {
                if (lastSelected != i) {
                    scrollOffset = 0;
                    lastSelected = i;
                    scrollDir = 1;
                    scrollWait = scrollInitialWaitFrames;
                }
                if (titleLen > 36) {
                    int maxOffset = (titleLen - 36) * 8;
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
                    Draw_DrawString(15, yPos, COLOR_ORANGE, ">>");
                    char dispBuf[37];
                    strncpy(dispBuf, buf + (scrollOffset/8), 36);
                    dispBuf[36] = '\0';
                    Draw_DrawString(35, yPos, COLOR_CYAN, dispBuf);
                    Draw_DrawString(250, yPos, COLOR_ORANGE, "<<        ");
                } else {
                    Draw_DrawString(15, yPos, COLOR_ORANGE, ">>");
                    Draw_DrawString(35, yPos, COLOR_CYAN, buf);
                    Draw_DrawString(250, yPos, COLOR_ORANGE, "<<        ");
                }
            } else {
                Draw_DrawString(15, yPos, COLOR_GRAY, " *");
                Draw_DrawString(250, yPos, COLOR_WHITE, "  ");
                Draw_DrawString(35, yPos, COLOR_WHITE, buf);
            }
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