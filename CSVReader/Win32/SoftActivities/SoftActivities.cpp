// SoftActivities.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SoftActivities.h"
#include "..\..\Utilities.h"

CSoftActivities::CSoftActivities(HINSTANCE hInstance) {
	_instance = hInstance;
	_consoleApp = new CApp(0, NULL);
}

CSoftActivities::~CSoftActivities() {
	delete _consoleApp;
	_consoleApp = NULL;
}

const int CSoftActivities::run() {
	return DialogBoxParamW(_instance, MAKEINTRESOURCE(IDD_SOFTACTIVITIES_DIALOG), NULL, CSoftActivities::dlgProc, reinterpret_cast<long>(this));
}

void CSoftActivities::initialize(HWND dialog) {
	_dialog = dialog;

	auto c = readConfig();
	if (c->defaultGroupPath != NULL) {
		loadActivityGroups(c->defaultGroupPath);
	}
	releaseConfig(c);
}

const bool CSoftActivities::loadActivityGroups(const wchar_t *path) {
	bool pathAllocated = false;

	if (path == NULL) {
		path = getPathByDialog();
		pathAllocated = true;
	}

	if (path == NULL) {
		return false;
	}

	if (!_consoleApp->readGroups(path)) {
		// failed to load groups!
		return false;
	}

	auto groups = _consoleApp->getGroups();

	releaseGroupViews();
	for (auto it = groups->begin(); it != groups->end(); ++it) {
		auto itemId = SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_ADDSTRING, NULL, reinterpret_cast<long>(it->first.c_str()));
		SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_SETITEMDATA, itemId, reinterpret_cast<long>(new wstring(it->first)));
	}

	if (pathAllocated) {
		auto c = readConfig();
		StringCchCopyW(c->defaultGroupPath, MAX_PATH, path);
		saveConfig(c);
		releaseConfig(c);

		releasePath(path);
	}

	return true;
}

const bool CSoftActivities::loadActivities() {
	static bool viewInitiated = false;

	auto path = getPathByDialog();
	if (path == NULL) {
		return false;
	}

	auto view = GetDlgItem(_dialog, IDC_GROUP_RESULT);
	if (!viewInitiated) {

		auto flags = LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
		ListView_SetExtendedListViewStyleEx(view, flags, flags);

		LVCOLUMNW c;
		ZeroMemory(&c, sizeof(LVCOLUMNW));

		c.mask    = LVCF_TEXT | LVCF_WIDTH | LVCF_MINWIDTH;
		c.pszText = L"Aktivitet";
		c.cx      = 300;
		c.cxMin   = 200;
		ListView_InsertColumn(view, 0, &c);

		c.pszText = L"Timmar";
		c.cx      = 300;
		c.cxMin   = 200;
		ListView_InsertColumn(view, 1, &c);

		viewInitiated = true;
	} else {
		ListView_DeleteAllItems(view);
	}

	if (!_consoleApp->readHours(path)) {
		// Load failed
		MessageBoxExW(_dialog, L"Failed to load list of activities. Is the file formatted correctly?", L"Error", MB_OK | MB_ICONERROR, NULL);
	}

	double total = 0;
	wstring title;
	bool chargeable;
	int chargeableCount = 0;

	auto groups = _consoleApp->group(total);
	for (auto it = groups->begin(); it != groups->end(); ++it) {
		LVITEMW item;
		ZeroMemory(&item, sizeof(LVITEMW));

		title = it->first;
		chargeable = it->first.at(0) != L'*';
		if (!chargeable) {
			title = title.substr(1);
		} else {
			chargeableCount += 1;
		}

		item.mask     = LVIF_TEXT | LVIF_PARAM;
		item.pszText  = (LPWSTR) title.c_str();
		item.iSubItem = 0;
		item.iItem    = ListView_GetItemCount(view);
		item.lParam   = chargeable;

		ListView_InsertItem(view, &item);

		item.mask     = LVIF_TEXT;
		item.pszText  = (wchar_t *) it->second.c_str();
		item.iSubItem = 1;

		ListView_SetItem(view, &item);
	}

	if (total > 0) {
		auto totalString = Utilities::toString(total);
		
		chargeable = chargeableCount > 0;

		LVITEMW item;
		ZeroMemory(&item, sizeof(LVITEMW));

		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText  = L"Total";
		item.iSubItem = 0;
		item.iItem    = ListView_GetItemCount(view);
		item.lParam   = chargeable;

		ListView_InsertItem(view, &item);

		item.mask     = LVIF_TEXT;
		item.pszText  = (LPWSTR) totalString.c_str();
		item.iSubItem = 1;

		ListView_SetItem(view, &item);
	}

	delete groups;
	groups = NULL;

	releasePath(path);
	return true;
}

const void CSoftActivities::expandSelectedGroup() {
	int  itemIndex = SendDlgItemMessage(_dialog, IDC_ACTIVITY_GROUPS, LB_GETCURSEL, NULL, NULL);
	if (itemIndex == LB_ERR) {
		return;
	}

	auto groups = _consoleApp->getGroups();

	// just pick one selected item
	auto groupKey = reinterpret_cast<wstring *>(SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_GETITEMDATA, itemIndex, NULL));
	auto group = groups->at(*groupKey);
	
	SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_RESETCONTENT, NULL, NULL);
	for (auto it = group.begin(); it != group.end(); ++it) {
		SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_ADDSTRING, NULL, reinterpret_cast<long>(it->c_str()));
	}
}

const BOOL CSoftActivities::handleListNotification(const LPARAM param) {
	auto list = GetDlgItem(_dialog, IDC_GROUP_RESULT);

	auto pnm = (LPNMLISTVIEW)param;

    if (pnm->hdr.hwndFrom == list &&pnm->hdr.code == NM_CUSTOMDRAW)
    {
        SetWindowLong(_dialog, DWL_MSGRESULT, (LONG)processCustomListDraw(param));
        return TRUE;
    }

	return FALSE;
}

const LRESULT CSoftActivities::processCustomListDraw(const LPARAM param) {
	auto item = (LPNMLVCUSTOMDRAW)param;

    switch (item->nmcd.dwDrawStage) 
    {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;
            
        case CDDS_ITEMPREPAINT:
			return CDRF_NOTIFYSUBITEMDRAW;
    
        case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			if (item->iSubItem) {
				item->clrText   = RGB(255,255,255);
				
				if (item->nmcd.lItemlParam) {
					item->clrTextBk = RGB(34, 139, 34);
				} else {
					item->clrTextBk = RGB(255, 0, 0);
				}

				return CDRF_NEWFONT;
			}
		default:
			return CDRF_DODEFAULT;
    }
}

const wchar_t *CSoftActivities::getPathByDialog() {
	IFileDialog *dialog;
	IShellItem  *resultItem;
	HRESULT      result;
	wchar_t     *fileName = NULL;

    result = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
	if (SUCCEEDED(result)) {
		result = dialog->Show(NULL);

		if (SUCCEEDED(result)) {
			result = dialog->GetResult(&resultItem);

			if (SUCCEEDED(result)) {
				resultItem->GetDisplayName(SIGDN_FILESYSPATH, &fileName);
				
				resultItem->Release();
				resultItem = NULL;
			}
		}

		dialog->Release();
		dialog = NULL;
	}

	return fileName;
}

const void CSoftActivities::releasePath(const wchar_t *&path) {
	CoTaskMemFree((LPVOID )path);
	path = NULL;
}

const void CSoftActivities::releaseGroupViews() {

	int count = SendDlgItemMessage(_dialog, IDC_ACTIVITY_GROUPS, LB_GETCOUNT, NULL, NULL);
	for (int i = 0; i < count; ++i) {
		auto item = SendDlgItemMessage(_dialog, IDC_ACTIVITY_GROUPS, LB_GETITEMDATA, i, NULL);
		delete reinterpret_cast<wstring *>(item);
	}

	SendDlgItemMessage(_dialog, IDC_ACTIVITY_GROUPS, LB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessage(_dialog, IDC_GROUPED_ACTIVITIES, LB_RESETCONTENT, NULL, NULL);
}

CSoftActivities::config_t *CSoftActivities::readConfig() {
	auto path = getConfigPath();
	auto c = new config_t;
	ZeroMemory(c, sizeof(config_t));

	if (path == NULL) {
		return c;
	}
	 
	ifstream f(path, ios::in | ios::binary);
	
	delete path;
	path = NULL;

	if (!f.is_open()) {
		return c;
	}

	f.read((char *) c, sizeof(*c));

	return c;
}

const void CSoftActivities::releaseConfig(CSoftActivities::config_t *&cptr) {
	if (cptr != NULL) {
		delete cptr;
	}
	cptr = NULL;
}

const void CSoftActivities::saveConfig(const CSoftActivities::config_t *c) {
	auto path = getConfigPath();
	if (path == NULL) {
		return;
	}

	ofstream f;
	f.open(path, ios::out | ios::binary);

	delete path;
	path = NULL;

	if (!f.is_open()) {
		return;
	}

	f.write((char *) c, sizeof(*c));
}

const wchar_t *CSoftActivities::getConfigPath() {
	wchar_t *path = new wchar_t[MAX_PATH];
	
	if (FAILED( SHGetFolderPathW(_dialog, CSIDL_APPDATA, NULL, 0, path) )) {
		return NULL;
	}

	StringCchCatW(path, MAX_PATH, L"\\SoftActivities");

	auto attr = GetFileAttributes(path);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		CreateDirectory(path, NULL);
	}

	StringCchCatW(path, MAX_PATH, L"\\Configuration.dat");

	return path;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
BOOL CALLBACK CSoftActivities::dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CSoftActivities *app;

	int wmId, wmEvent;
	switch (message)
	{
	case WM_INITDIALOG:
		app = reinterpret_cast<CSoftActivities *>(lParam);
		app->initialize(hWnd);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		switch (wmId)
		{
		case ID_LOAD_LOADACTIVITIES:
			app->loadActivities();
			break;
		case ID_LOAD_LOADACTIVITYGROUPS:
			app->loadActivityGroups(NULL);
			break;
		case ID_FILE_QUIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case IDC_ACTIVITY_GROUPS:
			
			switch (wmEvent) {
			case LBN_SELCHANGE:
					app->expandSelectedGroup();
					break;
			}

			break;
		}
		break;
	case WM_NOTIFY:
		wmId = LOWORD(wParam);

		switch (wmId) 
		{
		case IDC_GROUP_RESULT:
			return app->handleListNotification(lParam);
		}

		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}