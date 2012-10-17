#include <sstream>
#include "Arguments.h"

CArguments::CArguments(int argc, const char **argv) {
	wstring attr;
	for (int i = 1; i < argc; ++i) {
		// Convert ASCII char to wide char (UTF-8)
		size_t size = strnlen(argv[i], 4096) + 1;
		wchar_t *utf8arg = new wchar_t[size];
		memset(utf8arg, 0, size);
		mbstowcs(utf8arg, argv[i], size);
		
		// Divide the parameters depending on its function: arguments and 
		// file references (names)
		wstring arg(utf8arg);
		if (arg[0] == L'-') {
			arg = arg.substr(1); // remove initial hyphen
			if (attr.size() < 1) {
				attr = arg;
			} else {
				_args[attr] = NULL;
				_args[arg] = NULL;

				attr = L"";
			}
		} else {
			if (attr.size() < 1) {
				_refFiles.push_back(arg);
			} else {
				_args[attr] = new wstring(arg);
			}

			attr = L"";
		}

		delete [] utf8arg;
	}

	if (attr.size() > 0) {
		_args[attr] = NULL;
	}
}

CArguments::~CArguments(void) {
	this->free();
}

const CArguments::referenced_files_t CArguments::files(void) const {
	return _refFiles;
}

const CArguments::arguments_map_t CArguments::args(void) const {
	return _args;
}

void CArguments::free(void) {
	for (arguments_map_t::const_iterator it = _args.begin(); it != _args.end(); ++it) {
		delete (*it).second;
	}

	_args.clear();
	_refFiles.clear();
}