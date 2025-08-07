typedef struct {
	bool suppressLeds;
	bool cutSlotPower;
	bool cutSleepWifi;
	bool includeScreenshotTitleId;
	bool screenshotDateFolders;
	bool toggleLcdCombo;
} config_extra;

extern config_extra configExtra;

void ConfigExtra_DrawDetailedMenu();
void ConfigExtra_ReadConfigExtra();
void ConfigExtra_WriteConfigExtra();
void ConfigExtra_Init();