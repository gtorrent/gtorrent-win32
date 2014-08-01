
#include "stdafx.h"
#include "config.h"

// TODO Fix this.
#define CFG _T(".\\gtorrent.cfg")

Config_t Config;


void LoadConfig(void)
{
	GetPrivateProfileStruct(_T("UI"), _T("Layout"), &Config.UI, sizeof(Config.UI), CFG);
}

void SaveConfig(void)
{
	WritePrivateProfileStruct(_T("UI"), _T("Layout"), &Config.UI, sizeof(Config.UI), CFG);
}