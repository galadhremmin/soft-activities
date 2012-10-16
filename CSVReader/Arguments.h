#pragma once
#include <vector>
#include <string>
#include <map>
using namespace std;

class CArguments
{
public:
	typedef vector<wstring> referenced_files_t;
	typedef map<wstring, wstring*> arguments_map_t;

	CArguments(int argc, const char **argv);
	~CArguments(void);

	const referenced_files_t files(void) const;
	const arguments_map_t args(void) const;

	void free(void);

private:
	referenced_files_t _refFiles;
	arguments_map_t _args;
};

