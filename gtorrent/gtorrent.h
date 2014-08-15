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

#pragma once

#include "resource.h"
#include "stdafx.h"

namespace gtc
{
	extern "C"
	{
#include "gtorrent-core.h"
	}
}


typedef struct TorrentInfo
{
	gtc::gt_torrent *t;

	TCHAR *pszPath;
	TCHAR szDateAdded[64];
	TCHAR szDateCompleted[64];

} TorrentInfo;

