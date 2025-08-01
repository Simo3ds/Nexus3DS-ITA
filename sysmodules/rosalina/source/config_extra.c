#include <3ds.h>
#include "fmt.h"
#include "draw.h"
#include "ifile.h"
#include "menu.h"
#include "menus.h"
#include "menus/config_extra.h"

config_extra configExtra = { .suppressLeds = false, .cutSlotPower = false, .cutSleepWifi = false, .includeScreenshotTitleId = false, .toggleLcdCombo = false };
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
        "Cut 3DS Wifi in sleep mode",
        "Include title ID in screenshots",
        "Toggle bottom LCD backlight (start+select)"
    };
    
    bool *configValues[] = {
        &configExtra.suppressLeds,
        &configExtra.cutSlotPower,
        &configExtra.cutSleepWifi,
        &configExtra.includeScreenshotTitleId,
        &configExtra.toggleLcdCombo
    };
    
    s32 selected = 0;
    s32 nbOptions = sizeof(configOptions) / sizeof(const char *);
    
    do {
        Draw_Lock();
        Draw_DrawMenuFrame("Extra config menu");

        for (s32 i = 0; i < nbOptions; i++) {
            u32 yPos = 40 + i * SPACING_Y;
            char buf[128];
            const char *checkbox = ConfigExtra_GetCheckboxDisplay(*configValues[i]);
            sprintf(buf, "%s %s", checkbox, configOptions[i]);
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