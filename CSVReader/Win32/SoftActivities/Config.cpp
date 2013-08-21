#include "stdafx.h"
#include "Config.h"
#include <fstream>

using namespace std;

CConfig::~CConfig(void)
{
	this->release();
}

bool CConfig::read() {	
	auto path = this->getPath();

	if (path == NULL) {
		return false;
	}
	 
	// try to open the configuration file in binary reading mode
	ifstream f(path, ios::in | ios::binary);
	
	delete [] path;
	path = NULL;

	if (!f.is_open()) {
		return false;
	}

	// release existing configuration
	this->release();

	// read the struct from file, assuming it is correctly formatted
	auto c = new config_t;
	ZeroMemory(c, sizeof(config_t));
	f.read((char *) c, sizeof(*c));
	f.close();

	// do not bother with validation at this stage - just assigned the result as the loaded configuration
	_loadedConfig = c;
	
	return true;
}

CConfig::config_t *CConfig::get() {
	if (_loadedConfig == NULL) {
		_loadedConfig = new CConfig::config_t;
		ZeroMemory(_loadedConfig, sizeof(CConfig::config_t));
	}

	return _loadedConfig;
}

const void CConfig::release() {
	if (_loadedConfig != NULL) {
		delete _loadedConfig;
	}
	_loadedConfig = NULL;
}

const void CConfig::save() {
	auto path = this->getPath();
	if (path == NULL) {
		return;
	}

	ofstream f;
	f.open(path, ios::out | ios::binary);

	delete [] path;
	path = NULL;

	if (!f.is_open()) {
		return;
	}

	f.write((char *) _loadedConfig, sizeof(*_loadedConfig));
	f.close();
}

const wchar_t *CConfig::getPath() const {
	wchar_t *path = new wchar_t[MAX_PATH];
	HWND parentWindow = NULL;
	if (_parent != NULL) {
		parentWindow = *_parent;
	}

	if (FAILED( SHGetFolderPathW(parentWindow, CSIDL_APPDATA, NULL, 0, path) )) {
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