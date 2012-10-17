#pragma once

#ifndef __CSV_H__
#define __CSV_H__

#include <map>
#include <vector>
#include <string>

using namespace std;

class CCSV
{
public:
    typedef enum csv_flags {
        CSV_TRIM       = 0x00000001,
        CSV_NO_HEADERS = 0x00000010
    } csv_flags_t;

	typedef map<wstring, wstring> row_t;
	typedef vector<row_t> row_list_t;
	typedef row_list_t::iterator iterator;
    typedef row_list_t::const_iterator const_iterator;

	static const wchar_t separator = L',';

	CCSV(const wstring filePath, const int flags);
	~CCSV(void);

	const bool read(void);

	iterator begin(void);
	const_iterator begin(void) const;
	iterator end(void);
	const_iterator end(void) const;

protected:
	const bool explode(const wstring& source, vector<wstring> &destination);

private:
	wstring _fileName;
    int _flags;
	row_list_t _rows;
};

#endif