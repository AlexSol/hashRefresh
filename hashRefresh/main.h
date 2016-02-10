#pragma once

#include <windows.h>
#include <commctrl.h>
#include <wincrypt.h>
#include "resource.h"

#include <memory>
#include <fstream>
#include <utility>
#include <vector>
#include<ctime>

#ifdef _DEBUG
#include <iostream>

#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#define MAIN() ( main() )
using namespace std;
#else

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#define MAIN() ( WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) )

#endif

#include "wCListView.h"
#include "sha256.h"

#define _WIN32_WINNT 0x0501

enum GUI {
	WIN = DLG_MAIN,
	OPENFILE = IDC_OPENFILE,
	BackUP = IDC_BackUP,
	ADD_ZERO = IDC_ADD_ZERO,
	FILELIST = IDC_FILELIST,
	SIZEADD = IDC_SIZEADD,
	PROCEED = IDC_PROCEED,
	CLEAR = IDC_CLEAR
};

struct OPTIONS
{
	bool backup;
	bool addZero;
	size_t sizeByte;
};