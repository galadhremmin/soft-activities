#pragma once

#include "resource.h"
#include "..\..\App.h"

class CSoftActivities {

public:
	CSoftActivities(HINSTANCE hInstance);
	~CSoftActivities();

	const int run();
	void initialize(HWND);
	const bool loadActivityGroups(const wchar_t *path);
	const bool loadActivities();
	const void expandSelectedGroup();
	const BOOL handleListNotification(const LPARAM);

protected:
	typedef struct {
		wchar_t defaultGroupPath[MAX_PATH];
	} config_t;

	const LRESULT processCustomListDraw(const LPARAM);
	const wchar_t *getPathByDialog();
	const void releasePath(const wchar_t *&);
	const void releaseGroupViews();
	config_t *readConfig();
	const void releaseConfig(config_t *&);
	const void saveConfig(const config_t *);

private:
	const wchar_t *getConfigPath();

	HINSTANCE _instance;
	HWND _dialog;
	CApp *_consoleApp;

	static BOOL CALLBACK dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};
