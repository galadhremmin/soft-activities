#pragma once

#include "resource.h"
#include "..\..\App.h"
#include "Config.h"

class CSoftActivities {

public:
	CSoftActivities(HINSTANCE hInstance);
	~CSoftActivities();

	const INT_PTR run();
	void initialize(HWND);
	const void resize(const WPARAM) const;
	const bool loadActivityGroups(const wchar_t *path);
	const bool loadActivities(const wchar_t *path);
	const void expandSelectedGroup();
	const BOOL handleListNotification(const LPARAM);

	void addActivity(const wchar_t *itemDescription);
	void removeActivity(LRESULT index);

protected:
	void initializeResizing();
	const LRESULT processCustomListDraw(const LPARAM);
	const void releaseGroupViews();

private:

	HINSTANCE _instance;
	HWND _dialog;
	CApp *_consoleApp;
	CConfig *_config;
	int _windowPadding;

	static INT_PTR CALLBACK dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
