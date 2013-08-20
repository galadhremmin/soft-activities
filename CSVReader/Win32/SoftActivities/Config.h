#pragma once

class CConfig
{
public:
	typedef struct {
		wchar_t defaultGroupPath[MAX_PATH];
		wchar_t defaultActivityPath[MAX_PATH];
	} config_t;

	CConfig(const HWND *parentWindow) : _parent(parentWindow), _loadedConfig(NULL) {}
	~CConfig();

	bool read();
	config_t *get();
	const void save();

protected:
	const void release();
	const wchar_t *getPath() const;

private:
	const HWND *_parent;
	config_t *_loadedConfig;
};

