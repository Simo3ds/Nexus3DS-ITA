#include <3ds.h>
#include "fmt.h"
#include "draw.h"
#include "ifile.h"
#include "menu.h"
#include "menus.h"
#include "menus/config_extra.h"

config_extra configExtra = { .suppressLeds = false, .cutSlotPower = false, .cutSleepWifi = false, .autoSave = true };
bool configExtraSaved = false;

static const char menuText[4][32] = {
    "Automatically suppress LEDs",
    "Cut power to TWL Flashcards",
    "Cut 3DS Wifi in sleep mode",
    "Enable auto-save"
};

static char menuDisplay[4][64];

Menu configExtraMenu = {
    "Extra config menu",
    {
        { menuText[0], METHOD, .method = &ConfigExtra_SetSuppressLeds},
        { menuText[1], METHOD, .method = &ConfigExtra_SetCutSlotPower},
        { menuText[2], METHOD, .method = &ConfigExtra_SetCutSleepWifi},
        { menuText[3], METHOD, .method = &ConfigExtra_SetAutoSave},
        {},
    }
};

void ConfigExtra_SetSuppressLeds(void) 
{
    configExtra.suppressLeds = !configExtra.suppressLeds;
    ConfigExtra_UpdateMenuItem(0, configExtra.suppressLeds);
    if (configExtra.autoSave) ConfigExtra_WriteConfigExtra();
}

void ConfigExtra_SetCutSlotPower(void) 
{
    configExtra.cutSlotPower = !configExtra.cutSlotPower;
    ConfigExtra_UpdateMenuItem(1, configExtra.cutSlotPower);
    if (configExtra.autoSave) ConfigExtra_WriteConfigExtra();
}

void ConfigExtra_SetCutSleepWifi(void) 
{
    configExtra.cutSleepWifi = !configExtra.cutSleepWifi;
    ConfigExtra_UpdateMenuItem(2, configExtra.cutSleepWifi);
    if (configExtra.autoSave) ConfigExtra_WriteConfigExtra();
}

void ConfigExtra_SetAutoSave(void)
{
    configExtra.autoSave = !configExtra.autoSave;
    ConfigExtra_UpdateMenuItem(3, configExtra.autoSave);
    ConfigExtra_WriteConfigExtra();
}

void ConfigExtra_UpdateMenuItem(int menuIndex, bool value)
{
    sprintf(menuDisplay[menuIndex], "%s %s", value ? "(x)" : "( )", menuText[menuIndex]);
    configExtraMenu.items[menuIndex].title = menuDisplay[menuIndex];
}

void ConfigExtra_UpdateAllMenuItems(void)
{
    ConfigExtra_UpdateMenuItem(0, configExtra.suppressLeds);
    ConfigExtra_UpdateMenuItem(1, configExtra.cutSlotPower);
    ConfigExtra_UpdateMenuItem(2, configExtra.cutSleepWifi);
    ConfigExtra_UpdateMenuItem(3, configExtra.autoSave);
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