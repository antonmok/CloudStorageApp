// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#define UM_CHECK_LOGIN		WM_APP + 1
#define UM_LOGIN_COMPLETE	WM_APP + 2
#define UM_UPLOAD_FILES		WM_APP + 3
#define UM_FILES_CHANGED	WM_APP + 4
#define UM_HAVE_UPDATES		WM_APP + 5
#define UM_SET_PROGRESS		WM_APP + 6