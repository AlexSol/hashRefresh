#include "main.h"

#pragma comment(lib , "comctl32.lib")

HINSTANCE hInst;
ListView* viewFile;
static WNDPROC OriginaProcListView = NULL;
bool UI_ENABLE = false;

// first = dir, second = file
std::vector<std::pair<std::string, std::string>> vListFile;

std::string fileSHA256(const char*  file);

BOOL CALLBACK DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProcListView(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void event(DWORD command, HWND& hWnd);
void initDialog(HWND hWnd);
void resizeDialog(HWND hWnd);
void uiEnbale(HWND hWnd, bool b);
bool openFileDlg(HWND hWnd);
bool proceeed(HWND hWnd);

void getOptions(OPTIONS& op, HWND hWnd);

int MAIN()
{
	InitCommonControls();
	hInst = ::GetModuleHandle(NULL);

	std::unique_ptr<ListView> viewFile(new ListView);
	::viewFile = viewFile.get();

	int res =  DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);

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

				vListFile.push_back(std::pair<std::string, std::string>("", sItem));

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

	std::unique_ptr<char> dataAdd(new char[opt.sizeByte]);

	std::memset(static_cast<void*>(dataAdd.get()), 0, opt.sizeByte);

	if (opt.addZero != true) {
		for (int i = 0; i < opt.sizeByte; i++)
		{
			dataAdd.get()[i] = rand() % 255 + 1;
		}
	}

	std::fstream file;

	std::string fileName;

	for (size_t itemFile = 0; itemFile < vListFile.size(); ++itemFile) {
		fileName.clear();

		if (vListFile[itemFile].first.length() <= 1) {
			fileName = vListFile[itemFile].second;
		}
		else {
			fileName = vListFile[itemFile].first;
			fileName += "\\";
			fileName += vListFile[itemFile].second;
		}

		if (opt.backup == true) {
			CopyFileA(fileName.c_str(), (std::string(fileName) + ".bak").c_str(), true);
		}

		file.open(fileName, std::ios_base::binary | std::ios_base::app);

		if (file.is_open()) {
			file.write(dataAdd.get(), opt.sizeByte);
			file.close();
		}

		viewFile->InsertItem(itemFile, 2, fileSHA256(fileName.c_str()).c_str(), MAX_PATH, 0);
	}

	return 0;
}

void getOptions(OPTIONS& opt, HWND hWnd) {
	opt.backup		= SendMessage(GetDlgItem(hWnd, GUI::BackUP), BM_GETSTATE, 0, 0);
	opt.addZero		= SendMessage(GetDlgItem(hWnd, GUI::ADD_ZERO), BM_GETSTATE, 0, 0);
	opt.sizeByte	= GetDlgItemInt(hWnd, GUI::SIZEADD, NULL, NULL);
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