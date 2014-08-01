#pragma once

typedef struct Config
{
	struct UI
	{
		struct ListView
		{
			int Widths[9];
		} ListView;
		DWORD VSplitter;
	} UI;

} Config_t;

extern Config_t Config;

void LoadConfig(void);
void SaveConfig(void);

