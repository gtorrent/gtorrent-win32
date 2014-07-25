#pragma once

#include "resource.h"
#ifndef _MSC_VER_ // if not using Visual Studio, #include the files that are in stdafx.h
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#endif

#define BOOST_ASIO_SEPARATE_COMPILATION