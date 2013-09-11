// SoftActivities.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SoftActivities.h"
#include "DirectoryPathDialog.h"
#include "..\..\Utilities.h"

// Inline helper methods, defined at the end of the file
inline void DeallocateActivityGroupItem(const HWND dialog, const int index);
inline void DeallocateActivityItem(const HWND dialog, const int index);

CSoftActivities::CSoftActivities(HINSTANCE hInstance) {
	_dialog = NULL;
	_instance = hInstance;
	_consoleApp = new CApp(0, NULL);
}

CSoftActivities::~CSoftActivities() {
	delete _consoleApp;
	_consoleApp = NULL;

	if (_config != NULL) {
		delete _config;
		_config = NULL;
	}
}

const int CSoftActivities::run() {
	return DialogBoxParamW(_instance, MAKEINTRESOURCE(IDD_SOFTACTIVITIES_DIALOG), NULL, CSoftActivities::dlgProc, reinterpret_cast<long>(this));
}

void CSoftActivities::initialize(HWND dialog) {
	_dialog = dialog;

	_config = new CConfig(&dialog);
	if (_config->read())
	{
		if (_config->get()->defaultGroupPath != NULL) {
			loadActivityGroups(_config->get()->defaultGroupPath);
		}

		if (_config->get()->defaultActivityPath != NULL) {
			loadActivities(_config->get()->defaultActivityPath);
		}
	}
}

const bool CSoftActivities::loadActivityGroups(const wchar_t *path) {
	CDirectoryPathDialog pathDialog(path);

	if (pathDialog.getState() == CDirectoryPathDialog::InteractionState::RELEASED) {
		if (!pathDialog.open()) {
			return false;
		}
	}

	if (!_consoleApp->readGroups(pathDialog.getPath())) {
		// failed to load groups!
		return false;
	}

	auto groups = _consoleApp->getGroups();

	releaseGroupViews();
	for (auto it = groups->begin(); it != groups->end(); ++it) {
		auto itemId = SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_ADDSTRING, NULL, reinterpret_cast<long>(it->first.c_str()));
		SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_SETITEMDATA, itemId, reinterpret_cast<long>(new wstring(it->first)));
	}

	if (pathDialog.getState() == CDirectoryPathDialog::InteractionState::PATH) {
		StringCchCopyW(_config->get()->defaultGroupPath, MAX_PATH, pathDialog.getPath());
		_config->save();
	}

	return true;
}

const bool CSoftActivities::loadActivities(const wchar_t *path) {
	static bool viewInitiated = false;

	CDirectoryPathDialog pathDialog(path);

	if (pathDialog.getState() == CDirectoryPathDialog::InteractionState::RELEASED) {
		if (!pathDialog.open()) {
			return false;
		}
	}

	auto view = GetDlgItem(_dialog, IDC_GROUP_RESULT);
	if (!viewInitiated) {

		auto flags = LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
		ListView_SetExtendedListViewStyleEx(view, flags, flags);

		LVCOLUMNW c;
		ZeroMemory(&c, sizeof(LVCOLUMNW));

		c.mask    = LVCF_TEXT | LVCF_WIDTH | LVCF_MINWIDTH ;
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

	_consoleApp->clearHours();
	if (!_consoleApp->readHours(pathDialog.getPath())) {
		// Load failed
		MessageBoxExW(_dialog, L"Failed to load list of activities. Is the file formatted correctly?", L"Error", MB_OK | MB_ICONERROR, NULL);
	}

	double total = 0;
	wstring title;
	bool chargeable;
	int chargeableCount = 0;

	ListView_DeleteAllItems(view);

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

	if (pathDialog.getState() == CDirectoryPathDialog::InteractionState::PATH) {
		StringCchCopyW(_config->get()->defaultActivityPath, MAX_PATH, pathDialog.getPath());
		_config->save();
	}

	return true;
}

const void CSoftActivities::expandSelectedGroup() {
	int  itemIndex = SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_GETCURSEL, NULL, NULL);
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

    if (pnm->hdr.hwndFrom != list)
		return FALSE;

	switch (pnm->hdr.code)
    {
		case NM_KEYDOWN: {
				auto key = (LPNMKEY) pnm->lParam;
				if (key->nVKey == 'C' && GetKeyState(VK_CONTROL) < 0) {
					// copy!
				}
			}
			return FALSE;

		case NM_CUSTOMDRAW:
	        SetWindowLong(_dialog, DWL_MSGRESULT, (LONG)processCustomListDraw(param));
			return TRUE;
    }

	return FALSE;
}

void CSoftActivities::addActivity(const wchar_t *itemDescription) {
	const int itemTextLength = 255;
	wchar_t itemText[itemTextLength];

	if (itemDescription == NULL) {
		GetDlgItemText(_dialog, IDC_ACTIVITY_ID, itemText, itemTextLength);
	} else {
		StringCchCopyW(itemText, itemTextLength, itemDescription);
	}

	if (*itemText == '\0') {
		return;
	}

	// Try to find items with the exact same string
	int itemCount = SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_GETCOUNT, NULL, NULL);
	for (int i = 0; i < itemCount; i += 1) {
		// Get the string length for the current item and allocate a buffer of sufficient size
		int length = SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_GETTEXTLEN, i, NULL) + 1;

		// automatically skip items longer than the max buffer size
		if (length > itemTextLength) {
			continue;
		}

		wchar_t *tmp = new wchar_t[length];
		
		// make sure to null terminate the buffer
		memset(tmp, 0, sizeof(wchar_t) * length);

		// Acquire the string into the buffer
		SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_GETTEXT, i, (LPARAM) (LPWSTR) tmp);

		// compare the two strings and determine if there would be doublettes if this item was added
		if (!lstrcmpW(tmp, itemText)) {
			MessageBoxW(_dialog, L"Denna aktivitet har redan tilldelats denna grupp.", L"Ny aktivitet", MB_OK | MB_ICONINFORMATION);
			delete [] tmp;
			return;
		}

		delete [] tmp;
		tmp = NULL;
	}

	// Add the item to the collection
	auto index = SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_ADDSTRING, NULL, reinterpret_cast<long>(itemText));
	if (index == LB_ERR) {
		return;
	}

	SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_SETCURSEL, index, NULL);

	// Empty the text field
	SendDlgItemMessageW(_dialog, IDC_ACTIVITY_ID, WM_SETTEXT, NULL, NULL);
}

void CSoftActivities::removeActivity(int index) {
	if (index < 0) {
		index = SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_GETCURSEL, NULL, NULL);
		if (index == LB_ERR) {
			return;
		}
	}

	// Make sure that the index is within the interval of correct indexes
	auto itemCount = SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_GETCOUNT, NULL, NULL);
	if (index < 0 || index >= itemCount) {
		return;
	}

	// Deallocate potential resources defined to the specified item and delete it entirely from the list box
	DeallocateActivityItem(_dialog, index);
	SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_DELETESTRING, index, NULL);
	
	// Select the closest next item in the list
	if (itemCount - 1 < 1) {
		return;
	}

	index -= 1;
	if (index < 0) {
		index += 1;

		if (index >= itemCount - 1) {
			return;
		}
	}

	SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_SETCURSEL, index, NULL);
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

const void CSoftActivities::releaseGroupViews() {

	int count = SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_GETCOUNT, NULL, NULL);
	for (int i = 0; i < count; ++i) {
		DeallocateActivityGroupItem(_dialog, i);
	}

	SendDlgItemMessageW(_dialog, IDC_ACTIVITY_GROUPS, LB_RESETCONTENT, NULL, NULL);
	SendDlgItemMessageW(_dialog, IDC_GROUPED_ACTIVITIES, LB_RESETCONTENT, NULL, NULL);
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
		case IDC_ADD_ACTIVITY:
			app->addActivity(NULL);
			break;
		case IDC_REMOVE_ACTIVITY:
			app->removeActivity(-1);
			break;
		case ID_LOAD_LOADACTIVITIES:
			app->loadActivities(NULL);
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

void DeallocateActivityGroupItem(const HWND dialog, const int index) {
	auto item = SendDlgItemMessageW(dialog, IDC_ACTIVITY_GROUPS, LB_GETITEMDATA, index, NULL);
	auto data = reinterpret_cast<wstring *>(item);
	if (data != NULL) {
		delete data;
	}
}

void DeallocateActivityItem(const HWND dialog, const int index) {
	// there is nothing to deallocate
}