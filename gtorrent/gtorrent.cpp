// Needs some error checking and refactoring
// GUI is pretty hacky atm but thats Win32 for you
//

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdint.h>
#include <queue>

#include "stdafx.h"
#include "gtorrent.h"
#include "util.h"
#include "config.h"


#define MAX_LOADSTRING 100
#define NUM_COLUMNS (sizeof(pszColumnLabels) / sizeof(pszColumnLabels[0]))

gtc::gt_core *gtCore = NULL;
std::deque<TorrentInfo*> torrent_queue;

bool bRunning = FALSE;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hTorrentList;								// Torrent Listview
HWND hStatusTree;								// Sidebar tree
HWND hDetailsTab;								// Details tab pane
HWND hToolBar;
HWND hMainFrame;
HWND hMainWindow;

//static DWORD	Config.UI.VSplitter, dwSplitterH;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
HWND				CreateTorrentListView(HWND hwndParent);
HWND				CreateStatusTreeView(HWND hwndParent);
HWND				CreateDetailTabView(HWND hwndParent);
HWND				CreateToolbar(HWND hWndParent);
HWND				CreateMainFrame(HWND hwndParent);
VOID CALLBACK       UIUpdateCallback(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

void UpdateTorrentListView(void);
void FinalizeConfig(void);

void OnAddTorrent(void);

void GetWindowPos(HWND hWnd, int *x, int *y)
{
	HWND hWndParent = GetParent(hWnd);
	POINT p = { 0 };

	MapWindowPoints(hWnd, hWndParent, &p, 1);

	(*x) = p.x;
	(*y) = p.y;
}

bool IsMouseOver(HWND hWnd, int mousex, int mousey)
{
	int x, y;
	RECT rect;

	GetClientRect(hWnd, &rect);
	GetWindowPos(hWnd, &x, &y);

	if (mousex >= x && mousex <= (rect.right + x)
		&& mousey >= y && mousey <= (rect.bottom + y))
	{
		return TRUE;
	}
	return FALSE;
}

bool InitGTCore(void)
{
	gtCore = gtc::core_create();

	return (gtCore != NULL);
}

void CloseGTCore(void)
{
	if (!torrent_queue.empty())
	{
		for (int i = 0; i < torrent_queue.size(); i++)
		{
			TorrentInfo *t = torrent_queue[i];
			free(t->pszPath);
			free(t);
		}
		torrent_queue.clear();
	}

	if (gtCore)
		gtc::core_shutdown(gtCore);
}

#define CUSTOM_FRAME   _T("CustomFrame")

void CustomPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	GetClientRect(hwnd, &rect);

	hdc = BeginPaint(hwnd, &ps);
	EndPaint(hwnd, &ps);
}


LRESULT CALLBACK CustomProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT:
		CustomPaint(hwnd);
		return 0;

	case WM_COMMAND:
		SendMessage(hMainWindow, uMsg, wParam, lParam);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CustomRegister(void)
{
	WNDCLASS wc = { 0 };

	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = CustomProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = CUSTOM_FRAME;
	RegisterClass(&wc);
}

void CustomUnregister(void)
{
	UnregisterClass(CUSTOM_FRAME, NULL);
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	UINT_PTR timer;

	if (!InitGTCore())
	{
		MessageBox(NULL, _T("Error initializing gTorrent-Core!"), _T("ERROR"), MB_OK);
	}

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	CustomRegister();

	LoadConfig();

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GTORRENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GTORRENT));

	timer = SetTimer(NULL, NULL, 1000, UIUpdateCallback);

	while (bRunning)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			GetMessage(&msg, NULL, 0, 0);
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Sleep(10);
	}

	KillTimer(NULL, timer);

	CustomUnregister();
	CloseGTCore();

	// TODO: FinalizeConfig() is called within the WM_DESTROY to flush settings to the memory
	// to save UI settings before everything is kill.
	SaveConfig();

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GTORRENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GTORRENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hMainWindow = hWnd;

   hStatusTree = CreateStatusTreeView(hWnd);
   hMainFrame = CreateMainFrame(hWnd);
		hToolBar = CreateToolbar(hMainFrame);
		hTorrentList = CreateTorrentListView(hMainFrame);
   hDetailsTab = CreateDetailTabView(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void ResizeWindow(DWORD dwSplitterPos)
{
	int x, y, w, h;
	RECT r;

	// TODO: Test this default value
	if (dwSplitterPos == 0)
		dwSplitterPos = 200;

	GetClientRect(hMainWindow, &r);
	x = y = 0;
	w = dwSplitterPos - 1;
	h = r.bottom;
	MoveWindow(hStatusTree, x, y, w, h, TRUE);
	UpdateWindow(hMainWindow);

	x = dwSplitterPos + 2;
	y = 0;
	w = r.right - dwSplitterPos - 2;
	h = (r.bottom * 2) / 3;
	MoveWindow(hMainFrame, x, y, w, h, TRUE);

	GetClientRect(hToolBar, &r);
	MoveWindow(hToolBar, 0, 0, w, r.bottom, TRUE);

	MoveWindow(hTorrentList, 0, r.bottom, w, h - r.bottom, TRUE);
	UpdateWindow(hMainWindow);

	GetClientRect(hMainWindow, &r);
	x = dwSplitterPos + 2;
	y = ((r.bottom - r.top) * 2 ) / 3;
	w = r.right - dwSplitterPos - 2;
	h = r.bottom / 3;
	MoveWindow(hDetailsTab, x, y, w, h, TRUE);
	UpdateWindow(hMainWindow);
}

VOID CALLBACK UIUpdateCallback(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	UpdateTorrentListView();
}

void FinalizeConfig(void)
{
	for (int i = 0; i < 9; i++)
	{
		Config.UI.ListView.Widths[i] = ListView_GetColumnWidth(hTorrentList, i);
	}
//	Config.UI.VSplitter = Config.UI.VSplitter;
}

void UpdateTorrentListView(void)
{
	TorrentInfo* t;
	LVITEM		lvItem;

	// TODO: Fix 64
	TCHAR text[64];

	if (torrent_queue.empty())
		return;

	for (int i = 0; i < torrent_queue.size(); i++)
	{
		//TODO: Columns are hardcoded for now.

		t = torrent_queue[i];

		if (t->t)
		{
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = i;

			// Name
			lvItem.iSubItem = 1;
			_tsplitpath(t->pszPath, NULL, NULL, text, NULL);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// Size
			lvItem.iSubItem = 2;
			int64_t size = gtc::get_size(t->t);
			// TODO: Sizes are shown in MBs right now
			_stprintf(text, _T("%d MB"), size / 1024 / 1024);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// Status
			lvItem.iSubItem = 3;
			float progress = gtc::get_total_progress(t->t);
			_stprintf(text, _T("%3.0f %%"), progress);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// Down speed
			lvItem.iSubItem = 4;
			uint32_t down_rate = gtc::get_download_rate(t->t);
			_stprintf(text, _T("%d KB/s"), down_rate / 1024);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// Up speed
			lvItem.iSubItem = 5;
			uint32_t up_rate = gtc::get_upload_rate(t->t);
			_stprintf(text, _T("%d KB/s"), up_rate / 1024);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// ETA
			lvItem.iSubItem = 6;
			int64_t time = gtc::get_time_remaining(t->t);
			int h, m, s;
			h = time / 3600;
			time %= 3600;
			m = time / 60;
			time %= 60;
			s = time;
			_stprintf(text, _T("%d H : %d M : %d S"), h, m, s);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// Seeds/peers
			lvItem.iSubItem = 7;
			uint32_t seeders = gtc::get_total_seeders(t->t);
			uint32_t leechers = gtc::get_total_leechers(t->t);
			_stprintf(text, _T("%d / %d"), seeders, leechers);
			lvItem.pszText = text;
			ListView_SetItem(hTorrentList, &lvItem);

			// Added On
			lvItem.iSubItem = 8;
			lvItem.pszText = t->szDateAdded;
			ListView_SetItem(hTorrentList, &lvItem);

			// Completed On

			UpdateWindow(hTorrentList);
		}
	}
}

void OnAddTorrent(void)
{
	// TODO: Do error checking
	LVITEM		lvItem;
	OpenFileDialog ofd;

	// TODO: Fix 64
	TCHAR text[64];

	if (ofd.ShowDialog())
	{
		size_t len = _tcscnlen(ofd.FileName, MAX_PATH);
		char *path = (char*)malloc(len+1);
		wcstombs(path, ofd.FileName, len);
		path[len] = '\0';

		TorrentInfo *ti = (TorrentInfo*)malloc(sizeof(TorrentInfo));
		ti->t = gtc::add_torrent(gtCore, path);
		ti->pszPath = _tcsdup(ofd.FileName);
		GetLocalTimeString(ti->szDateAdded);
		torrent_queue.push_back(ti);

		free(path);

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = (int)torrent_queue.size();
		lvItem.iSubItem = 0;
		_stprintf(text, _T("%d"), lvItem.iItem);
		lvItem.pszText = text;
		ListView_InsertItem(hTorrentList, &lvItem);

		UpdateTorrentListView();
	}
}

void OnPauseTorrent(void)
{

}

void OnResumeTorrent(void)
{

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	static HCURSOR 	hCursorWE;
	static HCURSOR 	hCursorNS;
	static BOOL	bSplitterMoving;
	static bool bHSplitter;

	switch (message)
	{

	case WM_CREATE:
		hCursorWE = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
		hCursorNS = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
		bSplitterMoving = FALSE;
		bHSplitter = FALSE;

		GetClientRect(hWnd, &rect);

		// Rough approximation for treeview width
		Config.UI.VSplitter = (rect.right - rect.left) / 7;
//		dwSplitterH = (rect.bottom - rect.top) / 2;

		bRunning = TRUE;
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case IDM_ADD_TORRENT:
			OnAddTorrent();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_SIZE:
		ResizeWindow(Config.UI.VSplitter);
		break;

	case WM_MOUSEMOVE:

		if (IsMouseOver(hDetailsTab, LOWORD(lParam), HIWORD(lParam)) && !bSplitterMoving)
			return 0;

		if (LOWORD(lParam) > 10)
		{
			SetCursor(hCursorWE);
			if ((wParam == MK_LBUTTON) && bSplitterMoving)
			{
				GetClientRect(hWnd, &rect);
				if (LOWORD(lParam) > rect.right)
					return 0;

				Config.UI.VSplitter = LOWORD(lParam);
				SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
			}
		}
		//else if (HIWORD(lParam) > 10)
		//{
		//	SetCursor(hCursorNS);
		//	if ((wParam == MK_LBUTTON) && bSplitterMoving)
		//	{
		//		GetClientRect(hWnd, &rect);
		//		if (HIWORD(lParam) > rect.bottom)
		//			return 0;

		//		dwSplitterH = HIWORD(lParam);
		//		SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
		//	}
		//}
		break;


	case WM_LBUTTONDOWN:
		if (!IsMouseOver(hDetailsTab, LOWORD(lParam), HIWORD(lParam)))
		{
			SetCursor(hCursorWE);
			bSplitterMoving = TRUE;
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		bSplitterMoving = FALSE;
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		bRunning = FALSE;
		FinalizeConfig();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

TCHAR *pszColumnLabels[] =
{
	_T("Number"),
	_T("Name"),
	_T("Size"),
	_T("Status"),
	_T("Down Speed"),
	_T("Up Speed"),
	_T("ETA"),
	_T("Seeds/Peers"),
	_T("Added"),
	_T("Completed On")
};

HWND CreateMainFrame(HWND hwndParent)
{
	RECT rcClient;                       // The parent window's client area.

	GetClientRect(hwndParent, &rcClient);

	HWND hGroupBox = CreateWindow(CUSTOM_FRAME, _T(""),
		WS_CHILD | WS_VISIBLE,
		(rcClient.right - rcClient.left) / 2,
		0,
		(rcClient.right - rcClient.left) / 2,
		(rcClient.bottom - rcClient.top) / 2,
		hwndParent, NULL, hInst, NULL);

	return hGroupBox;
}

HWND CreateTorrentListView(HWND hwndParent)
{
	RECT rcClient;

	// Toolbar must be created before listview
	GetClientRect(hToolBar, &rcClient);
	int iHeight = rcClient.bottom;

	GetClientRect(hwndParent, &rcClient);

	// Create the list-view window in report view with label editing enabled.
	HWND hWndListView = CreateWindow(WC_LISTVIEW,
		_T(""),
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
		0,
		iHeight,
		rcClient.right,
		rcClient.bottom - iHeight,
		hwndParent,
		NULL,
		hInst,
		NULL);


	ListView_DeleteAllItems(hWndListView);

	LV_COLUMN   lvColumn;
	ZeroMemory(&lvColumn, sizeof(lvColumn));

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;

	for (int i = 0; i < NUM_COLUMNS; i++)
	{
		lvColumn.pszText = pszColumnLabels[i];
		ListView_InsertColumn(hWndListView, i, &lvColumn);
		if (Config.UI.ListView.Widths[i])
			ListView_SetColumnWidth(hWndListView, i, Config.UI.ListView.Widths[i]);
		else
			ListView_SetColumnWidth(hWndListView, i, LVSCW_AUTOSIZE_USEHEADER);
	}

	// This is a hack to avoid first column taking all the width
	//TODO : 0 index is fixed for name
	if (Config.UI.ListView.Widths[0] == 0)
		ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);

	ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	return (hWndListView);
}

TCHAR *pszStatusTreeLables[] =
{
	_T("Downloading"),
	_T("Seeding"),
	_T("Completed"),
	_T("Active"),
	_T("Inactive")
};

HWND CreateStatusTreeView(HWND hwndParent)
{
	RECT rcClient;
	HWND hwndTV;

	GetClientRect(hwndParent, &rcClient);
	hwndTV = CreateWindow(WC_TREEVIEW,
		_T("Tree View"),
		WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
		0,
		0,
		rcClient.right/2,
		rcClient.bottom,
		hwndParent,
		NULL,
		hInst,
		NULL);

	TreeView_DeleteAllItems(hwndTV);

	TVINSERTSTRUCT tvins;
	HTREEITEM parent;

	tvins.hParent = TVI_ROOT;
	tvins.hInsertAfter = TVI_FIRST;
	tvins.item.mask = TVIF_TEXT;
	tvins.item.pszText = _T("Torrents");

	parent = TreeView_InsertItem(hwndTV, &tvins);

	tvins.hParent = parent;

	for (int i = 0; i < (sizeof(pszStatusTreeLables) / sizeof(pszStatusTreeLables[0])); i++)
	{
		tvins.hInsertAfter = TVI_LAST;
		tvins.item.pszText = pszStatusTreeLables[i];
		TreeView_InsertItem(hwndTV, &tvins);
	}

	TreeView_Expand(hwndTV, parent, TVE_EXPAND);

	return hwndTV;
}

TCHAR *pszTabLabels[] =
{
	_T("Files"),
	_T("Info"),
	_T("Peers"),
	_T("Trackers"),
	_T("Pieces"),
	_T("Speed"),
	_T("Logger")
};

HWND CreateDetailTabView(HWND hwndParent)
{
	RECT rcClient;
	HWND hwndTab;
	TCITEM tie;

	GetClientRect(hwndParent, &rcClient);

	hwndTab = CreateWindow(WC_TABCONTROL, _T(""),
		WS_CHILD | WS_VISIBLE,
		(rcClient.right - rcClient.left) / 2,
		(rcClient.bottom - rcClient.top) / 2,
		(rcClient.right - rcClient.left) / 2,
		(rcClient.bottom - rcClient.top) / 2,
		hwndParent, NULL, hInst, NULL);

	tie.mask = TCIF_TEXT;

	for (int i = 0; i < (sizeof(pszTabLabels) / sizeof(pszTabLabels[0])); i++)
	{
		tie.pszText = pszTabLabels[i];
		if (TabCtrl_InsertItem(hwndTab, i, &tie) == -1)
		{
			DestroyWindow(hwndTab);
			return NULL;
		}
	}

	return hwndTab;
}

HIMAGELIST hImageList = NULL;

HWND CreateToolbar(HWND hWndParent)
{
	const int ImageListID = 0;
	const int numButtons = 5;
	const int bitmapSize = 32;

	const DWORD buttonStyles = BTNS_AUTOSIZE;

	TBBUTTON tbButtons[numButtons];
	HICON ico;

	HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_LIST,
		0, 0, 0, 0,
		hWndParent, NULL, hInst, NULL);

	if (hWndToolbar == NULL)
		return NULL;

	hImageList = ImageList_Create(bitmapSize, bitmapSize, ILC_COLOR32 | ILC_MASK, numButtons, 0);

	SendMessage(hWndToolbar, TB_SETIMAGELIST,
		(WPARAM)ImageListID,
		(LPARAM)hImageList);

	SendMessage(hWndToolbar, TB_LOADIMAGES,	(WPARAM)IDB_STD_LARGE_COLOR, (LPARAM)HINST_COMMCTRL);

	for (int i = 0; i < numButtons; i++)
	{
		ico = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NEWFILE + i));
		int idx = ImageList_AddIcon(hImageList, ico);
		tbButtons[i].iBitmap = idx;
		tbButtons[i].idCommand = IDM_ADD_TORRENT + i;
		tbButtons[i].fsState = TBSTATE_ENABLED;
		tbButtons[i].fsStyle = buttonStyles;
		tbButtons[i].dwData = 0L;
		tbButtons[i].iString = 0;
	}

	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hWndToolbar, TRUE);

	return hWndToolbar;
}

