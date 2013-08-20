#include "stdafx.h"
#include "DirectoryPathDialog.h"

CDirectoryPathDialog::CDirectoryPathDialog(const wchar_t *initialPath) {
	this->setPath(initialPath, true);
}

CDirectoryPathDialog::~CDirectoryPathDialog()
{
	this->release();
}

const bool CDirectoryPathDialog::open() {
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

	this->setPath(fileName);

	// User cancelled?
	if (this->getState() == InteractionState::CANCELLED) {
		return false;
	}

	return true;
}

const wchar_t *CDirectoryPathDialog::getPath() const {
	return _filePath;
}

const CDirectoryPathDialog::InteractionState CDirectoryPathDialog::getState() const {
	return _state;
}

void CDirectoryPathDialog::setPath(const wchar_t *path, const bool initial) {
	this->release();
	
	if (path == NULL) {
		_state = initial ? InteractionState::RELEASED : InteractionState::CANCELLED;
	} else {
		_state = initial ? InteractionState::INITIAL_PATH : InteractionState::PATH;
	}

	_filePath = path;
}

void CDirectoryPathDialog::release() {
	if (_filePath != NULL && _state != InteractionState::INITIAL_PATH) {
		CoTaskMemFree((LPVOID ) _filePath);
	}

	_filePath = NULL;
	_state = InteractionState::RELEASED;
}
