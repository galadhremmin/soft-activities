#pragma once
class CDirectoryPathDialog
{
public:
	typedef enum  {
		INITIAL_PATH,
		PATH,
		CANCELLED,
		RELEASED
	} InteractionState;

	CDirectoryPathDialog(const wchar_t *initialPath = NULL);
	~CDirectoryPathDialog();

	const bool open();
	const wchar_t *getPath() const;
	const InteractionState getState() const;

protected:
	void setPath(const wchar_t *path, const bool initial = false);
	void release();

private:
	const wchar_t *_filePath;
	InteractionState _state;
};

