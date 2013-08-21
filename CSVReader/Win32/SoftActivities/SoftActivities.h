#pragma once

#include "resource.h"
#include "..\..\App.h"
#include "Config.h"

class CSoftActivities {

public:
	CSoftActivities(HINSTANCE hInstance);
	~CSoftActivities();

	const int run();
	void initialize(HWND);
	const bool loadActivityGroups(const wchar_t *path);
	const bool loadActivities(const wchar_t *path);
	const void expandSelectedGroup();
	const BOOL handleListNotification(const LPARAM);
	void removeActivity(const int index);

protected:
	const LRESULT processCustomListDraw(const LPARAM);
	const void releaseGroupViews();

private:

	HINSTANCE _instance;
	HWND _dialog;
	CApp *_consoleApp;
	CConfig *_config;

	static BOOL CALLBACK dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};
