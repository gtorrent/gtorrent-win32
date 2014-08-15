// gtorrent-win32
// Copyright (C) 2014  nodev
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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