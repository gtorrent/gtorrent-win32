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

