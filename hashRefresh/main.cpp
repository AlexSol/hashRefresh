#include "main.h"

#pragma comment(lib , "comctl32.lib")

HINSTANCE hInst;
ListView* viewFile;
static WNDPROC OriginaProcListView = NULL;
bool UI_ENABLE = false;

// first = dir, second = file
std::list<std::pair<std::string, std::string>> vListFile;

std::string fileSHA256(const char*  file);
std::string memorySHA256(const char*  base, long size);

bool createNewFile(const char*  file);

BOOL CALLBACK DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProcListView(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void event(DWORD command, HWND& hWnd);
void initDialog(HWND hWnd);
void resizeDialog(HWND hWnd);
void uiEnbale(HWND hWnd, bool b);
bool openFileDlg(HWND hWnd);
bool proceeed(HWND hWnd);

void getOptions(OPTIONS& op, HWND hWnd);

LPBYTE OpenPEFile(const char* fileName);

int MAIN()
{
	InitCommonControls();
	hInst = ::GetModuleHandle(NULL);

	std::unique_ptr<ListView> viewFile(new ListView);
	::viewFile = viewFile.get();

	int res;
	res = DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);

	return res;
}

BOOL CALLBACK DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE: {
			break;
		}
		case WM_INITDIALOG:
		{
			initDialog(hWnd);
			uiEnbale(hWnd, false);
			return TRUE;
		}
		case WM_CLOSE:
		{
			EndDialog(hWnd, 0);
			return TRUE;
		}
		case WM_SIZE:{
			resizeDialog(hWnd);
			break;
		}
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);

			event(LOWORD(wParam), hWnd);
			return TRUE;
		}
		default: {
			uiEnbale(hWnd, UI_ENABLE);
		}
	}

	return FALSE;
}

LRESULT CALLBACK WindowProcListView(HWND hWnd,	UINT uMsg,	WPARAM wParam,	LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DROPFILES: {
		HDROP hDropInfo = (HDROP)wParam;
		char sItem[MAX_PATH];

		for (size_t i = 0; DragQueryFile(hDropInfo, i, (LPSTR)sItem, sizeof(sItem)); ++i)
		{
			if (GetFileAttributes(sItem) & FILE_ATTRIBUTE_DIRECTORY) {
				;
			}
			else {
				SetFileAttributes(sItem, FILE_ATTRIBUTE_NORMAL);

				char* fileName = strrchr(sItem, '\\');
				if (fileName == nullptr) { continue; }
				++fileName;

				viewFile->InsertItem(0, 0, fileName, MAX_PATH, 0);
				viewFile->InsertItem(0, 1, fileSHA256(sItem).c_str(), MAX_PATH, 0);

				vListFile.push_front(std::pair<std::string, std::string>("", sItem));

				UI_ENABLE = true;
			}
		}
		DragFinish(hDropInfo);		
		break;
	}
	}
	return CallWindowProc(OriginaProcListView, hWnd, uMsg, wParam, lParam);
}

void event(DWORD command, HWND& hWnd)
{
	switch (command)
	{
	case GUI::OPENFILE: {
		openFileDlg(hWnd);
		break;
	}
	case GUI::CLEAR: {
		viewFile->delAllItems();
		vListFile.clear();
		UI_ENABLE = false;
		break;
	}
	case GUI::PROCEED: {
		proceeed(hWnd);
		break;
	}
	}

}

void initDialog(HWND hWnd)
{
	::SendMessage(GetDlgItem(hWnd, GUI::SIZEADD), EM_LIMITTEXT, 3, 0);
	::SetWindowTextA(GetDlgItem(hWnd, GUI::SIZEADD), "3");

	viewFile->setPOint(7, 150);
	viewFile->m_hListView = GetDlgItem(hWnd, GUI::FILELIST);
	viewFile->InitListView();

	viewFile->InsertColumn(0, "File name", 200);
	viewFile->InsertColumn(1, "Old sha-256", 380);
	viewFile->InsertColumn(2, "New sha-256", 380);

	WNDPROC oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(GetDlgItem(hWnd, GUI::FILELIST), GWLP_WNDPROC, (LONG_PTR)&WindowProcListView));
	OriginaProcListView = oldProc;
}

void resizeDialog(HWND hWnd)
{
	viewFile->refreshSize(hWnd);
	RECT rec, retGroutOtp = { 7, 7, 14, 95 };
	GetClientRect(hWnd, &rec);
	MoveWindow(GetDlgItem(hWnd, ID_GROUP_OPT), retGroutOtp.left, retGroutOtp.top, rec.right - retGroutOtp.left * 2, retGroutOtp.bottom, 1);
}

bool openFileDlg(HWND hWnd)
{
	static OPENFILENAMEA ofn;

	static char FilePatch[MAX_PATH];
	static char FileName[MAX_PATH];;
	static char szDefExt[MAX_PATH];;

	ZeroMemory(&FilePatch, sizeof(FilePatch));
	ZeroMemory(&FileName, sizeof(FileName));
	ZeroMemory(&szDefExt, sizeof(szDefExt));
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = FilePatch;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 600;
	ofn.lpstrFilter = "EXE-file(*.exe)\0*.exe\0All-file(*.*)\0*.*\0";
	ofn.lpstrDefExt = szDefExt;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = FileName;
	ofn.nMaxFileTitle = 60;
	ofn.lpstrInitialDir = ".\\";
	ofn.lpstrTitle = "Select file.";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileNameA(&ofn) == TRUE) {
		if (ofn.nFileExtension != 0) {
			viewFile->InsertItem(0, 0, FileName, MAX_PATH, 0);
			viewFile->InsertItem(0, 1, fileSHA256(FileName).c_str(), MAX_PATH, 0);

			vListFile.push_back(std::pair<std::string, std::string>("", FilePatch));
		}
		else {
			std::string pathDir = FilePatch;
			pathDir += "\\";

			char* ptrFile = FilePatch + ofn.nFileOffset;
			int indexFile = 0;

			for (; *(ptrFile + 1) != 0;)
			{
				viewFile->InsertItem(indexFile, 0, ptrFile, MAX_PATH, 0);
				viewFile->InsertItem(indexFile, 1, fileSHA256((pathDir + ptrFile).c_str()).c_str(), MAX_PATH, 0);
				++indexFile;

				vListFile.push_back(std::pair<std::string, std::string>(FilePatch, ptrFile));
			
				ptrFile += ::lstrlenA(ptrFile++);
			}
		}
		UI_ENABLE = true;
	}

	return false;
}

bool proceeed(HWND hWnd)
{
	OPTIONS opt;
	getOptions(opt, hWnd);

	srand(time(NULL));

	//std::unique_ptr<char> dataAdd(new char[opt.sizeByte]);
	//std::memset(static_cast<void*>(dataAdd.get()), 0, opt.sizeByte);

	std::fstream file;

	std::string fileName;

	//for (size_t itemFile = 0; itemFile < vListFile.size(); ++itemFile) {
	int indexINsert = 0;
	for (auto &indexFile: vListFile)
	{
		fileName.clear();

		if (indexFile.first.length() <= 1) {
			fileName = indexFile.second;
		}
		else {
			fileName = indexFile.first;
			fileName += "\\";
			fileName += indexFile.second;
		}

		if (opt.backup == true) {
			CopyFileA(fileName.c_str(), (std::string(fileName) + ".bak").c_str(), true);
		}

		//createNewFile(fileName.c_str());

		auto base = OpenPEFile(fileName.c_str());
		if (base == NULL) { return -1; }
		IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)base;
		if (dos_head->e_magic != IMAGE_DOS_SIGNATURE) { UnmapViewOfFile(base);  return -1 ; }
		IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(base + dos_head->e_lfanew);
		if (nt_head->Signature != IMAGE_NT_SIGNATURE) { UnmapViewOfFile(base); return -1; }
		IMAGE_SECTION_HEADER* sect_head = IMAGE_FIRST_SECTION(nt_head);

		struct f
		{
			int number;
			DWORD pointRaw;
		} minPointSect;
		minPointSect.pointRaw = sect_head->PointerToRawData;
		minPointSect.number = 1;
		for (UINT i = 0; i < nt_head->FileHeader.NumberOfSections; i++, sect_head++) {
			if (sect_head->PointerToRawData < minPointSect.pointRaw) {
				minPointSect.pointRaw = sect_head->PointerToRawData;
				minPointSect.number = i;
			}
		}

		DWORD sizeAllSect = sizeof(IMAGE_SECTION_HEADER) * nt_head->FileHeader.NumberOfSections;
		DWORD offsetSpace = sizeAllSect + nt_head->FileHeader.SizeOfOptionalHeader + 0x18 + dos_head->e_lfanew;
		if (offsetSpace == minPointSect.pointRaw) { return -1; }
		DWORD sizeSpace = nt_head->OptionalHeader.SizeOfHeaders - offsetSpace;
		offsetSpace += 2;
		*(base + offsetSpace) = 0x31;

		int randomSize = rand() % sizeSpace - 1;

		for (int i = 0; i < randomSize; i++)
		{
			*(base + offsetSpace + i) = rand() % 255 + 1;
		}
		
		UnmapViewOfFile(base);

		//file.open(fileName, std::ios_base::binary | std::ios_base::app);
/*
		if (file.is_open()) {
			file.write(dataAdd.get(), opt.sizeByte);
			file.close();
		}
*/
		viewFile->InsertItem(indexINsert++, 2, fileSHA256(fileName.c_str()).c_str(), MAX_PATH, 0);
	}

	return 0;
}

void getOptions(OPTIONS& opt, HWND hWnd) {
	opt.backup		= SendMessage(GetDlgItem(hWnd, GUI::BackUP), BM_GETSTATE, 0, 0);
	opt.addZero		= SendMessage(GetDlgItem(hWnd, GUI::ADD_ZERO), BM_GETSTATE, 0, 0);
	opt.sizeByte	= GetDlgItemInt(hWnd, GUI::SIZEADD, NULL, NULL);
}

LPBYTE OpenPEFile(const char* fileName)
{
	HANDLE hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	SYSTEMTIME st;
	//GetLocalTime(&st);
	GetSystemTime(&st);

	FILETIME creationTime;
	SystemTimeToFileTime(&st, &creationTime);

	FILETIME lastAccessTime;
	SystemTimeToFileTime(&st, &lastAccessTime);

	FILETIME lastWriteTime;
	SystemTimeToFileTime(&st, &lastWriteTime);

	SetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime);

	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	CloseHandle(hFile);

	LPBYTE pBase = NULL;
	if (hMapping != NULL) {
		pBase = (LPBYTE)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
		CloseHandle(hMapping);
	}

	return pBase;
}

void uiEnbale(HWND hWnd, bool b)
{
	b = UI_ENABLE;
	EnableWindow(GetDlgItem(hWnd, GUI::PROCEED), b);
	EnableWindow(GetDlgItem(hWnd, GUI::CLEAR), b);
}

std::string fileSHA256(const char* FilePatch)
{
	std::ifstream file;
	file.open(FilePatch, std::ios_base::binary);

	if (file.is_open()) {
		SHA256 sha256;

		file.seekg(0, file.end);
		size_t length = file.tellg();

		file.seekg(0, file.beg);

		std::unique_ptr<char> buffer(new char[length]);

		file.read(buffer.get(), length);
		file.close();

		return sha256(buffer.get(), length);
	}
	return "Error:IO";
}

std::string memorySHA256(const char * base, long size)
{
	if(base == NULL) { return "Error:IO"; }

	SHA256 sha256;
	return sha256(base, size);
}

bool createNewFile(const char * filePatch)
{
	std::fstream file;
	file.open(filePatch, std::ios_base::binary | std::ios_base::in);

	if (file.is_open()) {
		file.seekg(0, file.end);
		size_t length = file.tellg();

		file.seekg(0, file.beg);

		std::unique_ptr<char> buffer(new char[length]);

		file.read(buffer.get(), length);
		file.close();

		if (DeleteFile(filePatch) == 0) { return true; }

		file.open(filePatch, std::ios_base::binary | std::ios_base::out);
		if (file.is_open()) {
			file.write(buffer.get(), length);
			file.close();
		}
	}

	return false;
}
