#include "CSV.h"
#include "Utilities.h"
#include <stdio.h>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

CCSV::CCSV(const wstring fileName, const int flags) {
	_fileName = fileName;
    _flags = flags;
}

CCSV::~CCSV(void) {

}

const bool CCSV::read(void) {
    
    // basic_ifstream requires the file name to be ANSI encoded, but the file name
    // is UTF-8. This performs the conversion.
    char *ansiFile = new char[_fileName.size() + 1];
    sprintf(ansiFile, "%ls", _fileName.c_str());
    wifstream file(ansiFile);
    delete [] ansiFile;
    
	wstring csvRow;
	vector<wstring> row, headers;
	const unsigned bom[] = { 0xEF, 0xBB, 0xBF };

	if (file.bad()) {
		return false;
	}

	for (unsigned i = 0; i < sizeof(bom) / sizeof(unsigned); ++i) {
		if (file.get() != bom[i]) {
			file.seekg(0);
			break;
		}
	}

    row_t rowObj;
    bool useHeaders = !(_flags & CSV_NO_HEADERS);
	while (!file.eof()) {
		getline(file, csvRow);

		if (!explode(csvRow, row)) {
			continue;
		}

		if (useHeaders && headers.size() < 1) {
            
            for (vector<wstring>::const_iterator it = row.begin(); it != row.end(); ++it) {
                headers.push_back(*it);
            }
            
		} else if (useHeaders) {
			wstring value;

			for (vector<wstring>::const_iterator header = headers.begin(), item = row.begin(); header != headers.end(); ++header) {
				value = L"";
                
                // cover columns without values 
				if (item != row.end()) {
					value = *item;
					++item;
				}

				rowObj[*header] = value;
			}

			_rows.push_back(rowObj);
		} else {
            
            vector<wstring>::const_iterator item = row.begin();
            for (int i = 0; item != row.end(); ++i) {
                rowObj[Utilities::toString(i)] = *item;
            
                ++item;
            }
            
            _rows.push_back(rowObj);
            
        }
	}

	return true;
}

CCSV::iterator CCSV::begin(void) {
	return _rows.begin();
}

CCSV::const_iterator CCSV::begin(void) const {
	return _rows.begin();
}

CCSV::iterator CCSV::end(void) {
	return _rows.end();
}

CCSV::const_iterator CCSV::end(void) const {
	return _rows.end();
}

const bool CCSV::explode(const wstring& source, vector<wstring> &destination) {
	bool content = false, quoted = false;
	wostringstream buffer;

	if (source.size() < 1) {
		return false;
	}

	destination.clear();
	for (wstring::const_iterator it = source.begin(); it != source.end(); ) {
		// looking for beginning of new item
		if (!content) {
			switch (*it) {
				case separator:

					if ((it + 1) != source.end() &&  *(it + 1) == separator) { // catch empty items
						destination.push_back(L"");
					}

					quoted = false;
					break;
				case L'"':
					quoted = true;
					break;
				default:
					content = true;
					break;
			}
		}

		if (content) {
			switch (*it) {
				case L'"':

					if (quoted && it + 1 != source.end() && *(it + 1) == L'"') {
						++it; // stop the current character from being added to the buffer, as it is a repetition thus an "escaped" quotation mark
					} else if (quoted) {
						quoted = false;
						content = false;
					}

					break;
				case separator:

					if (!quoted) { // only accept , within strings if quoted
						content = false;
						continue;
					}

					break;
			}
		}

		if (content) {
			buffer << *it;
		}

		++it;

		if ((!content || it == source.end()) && buffer.str().length() > 0) {
            
            if ((_flags & CSV_TRIM) > 0) {
                buffer.str(Utilities::trim(buffer.str()));
            }
        
			destination.push_back(buffer.str());
			buffer.str(L"");
		}
	}

	return true;
}
