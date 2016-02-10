#include <windows.h>
#include <Commdlg.h>
#include "Shellapi.h"  // for ExtractIconEx
//#include <commctrl.h>

#include "wCListView.h"

//#pragma comment(lib, "Comctl32.lib")

ListView::ListView()
{
	m_hListView = NULL;
	m_item = 0;
	m_fSize = 0;
	m_hImage = NULL;
}

ListView::~ListView() {}

void ListView::setPOint(size_t x, size_t y)
{
	m_point.x = x;
	m_point.y = y;
}

void ListView::SetHandleList(int IDC_ListView, int x, int y, int w, int h, HWND hwndDlg, HINSTANCE hInst)
{
	m_hListView = CreateWindowEx(0, WC_LISTVIEW, " ", WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT, x, y, w, h, hwndDlg, (HMENU)IDC_ListView, hInst, 0);
	bool isRemote = true;
	//SendMessage(m_hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_DOUBLEBUFFER, isRemote ? 0 : LVS_EX_DOUBLEBUFFER);
}

void ListView::InitListView()
{
	if (!m_hListView) return;

	SendMessage(m_hListView, (UINT)0x1000 + 54, (WPARAM)32, (LPARAM)32);

	SendMessage(m_hListView, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS, (LPARAM)LVS_EX_BORDERSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_AUTOSIZECOLUMNS);

	HICON  SmallIconInf;

	ExtractIconEx("%SystemRoot%\\system32\\shell32.dll", 0, NULL, &SmallIconInf, 1);

	ICONINFO iconinfoInf = { 0 };

	GetIconInfo(SmallIconInf, &iconinfoInf);
	DestroyIcon(SmallIconInf);

	HIMAGELIST hlT = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 0);
	ImageList_Add(hlT, iconinfoInf.hbmColor, iconinfoInf.hbmMask);
	ListView_SetImageList(m_hListView, hlT, LVSIL_SMALL);
}

int ListView::InsertColumn(int iCol, TCHAR* text, int iSize, int textPos)
{
	memset(&m_LvC, 0, sizeof(LV_COLUMN));

	m_LvC.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH; //LVCF_IMAGE
	m_LvC.fmt = textPos; //LVCFMT_LEFT;
	m_LvC.cx = iSize;
	m_LvC.pszText = text;
	m_LvC.cchTextMax = strlen(text) + 1;
	m_LvC.iSubItem = 0;
	return ListView_InsertColumn(this->m_hListView, iCol, &m_LvC);
}

int ListView::InsertItem(int iItem, int iSubItem, const TCHAR* text, int iSize, int nIcon = 0 )
{
	memset(&m_LvI, 0, sizeof(LV_ITEM));
	char* strBuff = new char[(lstrlen(text) + 1)];
	lstrcpy(strBuff, text);

	m_LvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	m_LvI.state = 0;
	m_LvI.iItem = iItem;
	m_LvI.iSubItem = iSubItem;
	m_LvI.pszText = strBuff;
	m_LvI.cchTextMax = strlen(strBuff) + 1;
	m_LvI.iImage = nIcon;
	ListView_InsertItem(this->m_hListView, &m_LvI);

	ListView_SetItemText(this->m_hListView, iItem, iSubItem, strBuff);
	delete[] strBuff;
	return 0;
}

int ListView::InsertItem()
{
	m_LvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	m_LvI.state = 0;
	m_LvI.iItem = Item.iItem;
	m_LvI.iSubItem = Item.iSubItem;
	m_LvI.pszText = Item.pszText;
	m_LvI.cchTextMax = strlen(Item.pszText) + 1;
	m_LvI.iImage = Item.iImage;
	ListView_InsertItem(this->m_hListView, &m_LvI);

	ListView_SetItemText(this->m_hListView, Item.iItem, Item.iSubItem, Item.pszText);
	return 0;
}

void ListView::refreshSize(HWND hWin)
{
	RECT rec;
	GetClientRect(hWin, &rec);
	MoveWindow(m_hListView, m_point.x, m_point.y, rec.right - m_point.x * 2, rec.bottom - m_point.y-10, 1); 
}

