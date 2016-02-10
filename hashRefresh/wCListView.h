#pragma once
#include <commctrl.h>

#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#define LVS_EX_GRIDLINES        0x00000001
#define LVS_EX_DOUBLEBUFFER     0x00010000
#define LVS_EX_LABELTIP         0x00004000
#define LVS_EX_BORDERSELECT     0x00008000

typedef struct m_tagLVITEMA
{
	UINT mask;
	int iItem;
	int iSubItem;
	UINT state;
	UINT stateMask;
	LPSTR pszText;
	int cchTextMax;
	int iImage;
	LPARAM lParam;
#if (_WIN32_IE >= 0x0300)
	int iIndent;
#endif

	int iGroupId;
	UINT cColumns; // tile view columns
	PUINT puColumns;

#if _WIN32_WINNT >= 0x0600 // Will be unused downlevel, but sizeof(LVITEMA) must be equal to sizeof(LVITEMW)
	int* piColFmt;
	int iGroup; // readonly. only valid for owner data.
#endif
};

class ListView
{
public:
	ListView();
	~ListView();
	void setPOint(size_t x, size_t y);
	enum ICON { Information, Warning, Error};
	void SetHandleList(int IDC_ListView, int x, int y, int w, int h, HWND hwndDlg, HINSTANCE hInst);
	void InitListView();
	int InsertColumn(int iCol, TCHAR* text, int iSize, int textPos = LVCFMT_LEFT);
	int InsertItem(int iItem, int iSubItem, const TCHAR* text, int iSize, int nIcon );
	int InsertItem();

	void delAllItems() { ListView_DeleteAllItems(m_hListView); }

	bool SetTextColor(COLORREF color);
	bool SetTextBkColor(COLORREF color);
	bool SetBkColor(COLORREF color);

	void EnableGroupView(const bool);
	void AddListViewGroup(WCHAR *szText, int iGroupId);
	void AddItemToListGroup(TCHAR *szItem, int iItemIdx, int iImageIdx, int iGroupId);
	void AddItemToListGroup(TCHAR *szItem, int iItemIdx, int iItemSubIdx, int iImageIdx, int iGroupId);


	int GetSelectedItem();
	HWND GetHandle() { return m_hListView; }
	void ResetMember();

	void refreshSize(HWND hWin);

	// protected:
	HWND m_hListView;
	LV_COLUMN m_LvC;
	m_tagLVITEMA m_LvI;
	HIMAGELIST m_hImage;
	int m_item;
	LONGLONG m_fSize;

	struct
	{
		int iItem;
		int iSubItem;
		LPSTR pszText;
		int iImage;
	}Item;

private:
	COLORREF COLOR_RGB = (10, 255, 60);
	POINT m_point;
};