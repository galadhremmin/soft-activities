#pragma once

#if WIN32 && !_CONSOLE
#define WIN32_GUI
#endif

#ifndef __APP_H__
#define __APP_H__

#include <string>
#include <vector>
#include <map>
#include "Arguments.h"
#include "HourEntry.h"

using namespace std;

class CApp
{
public:
    typedef enum {
        RET_OK                = 0,
        RET_INCORRECT_USE,
        RET_CSV_READ_FAILED,
        RET_CSV_READ_GROUPS_FAILED
    } retcode_t;

    typedef vector<wstring> groups_t;
    typedef map<wstring, groups_t> groups_map_t;
    typedef vector<CHourEntry *> hours_list_t;
    typedef map<wstring, hours_list_t> hours_map_t;

    CApp(const int argc, const char **argv);
    ~CApp(void);

    const retcode_t run(void);
    const bool readGroups(const wstring fileName);
    const bool readHours(const wstring fileName);
    const map<wstring, wstring> *group(double &total) const;
    const bool output(void) const;
	const hours_map_t *getHours() const;
	const groups_map_t *getGroups() const;
	void clearHours();

private:
    CArguments *_args;
    hours_map_t _hours;
    groups_map_t _groups;
};

#endif