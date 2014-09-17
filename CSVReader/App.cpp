#include <iostream>
#include <fstream>
#include "App.h"
#ifdef WIN32_GUI
#include "Win32\SoftActivities\stdafx.h"
#include "Win32\SoftActivities\resource.h"
#elif WIN32
#include <Windows.h>
#endif
#include "CSV.h"
#include "HTMLDocument.h"
#include "HTMLElement.h"
#include "Utilities.h"

using namespace HTML;

CApp::CApp(const int argc, const char **argv)
{
    _args = new CArguments(argc, argv);
}

CApp::~CApp(void)
{
    delete _args;
    _args = NULL;

	this->clearHours();
}

const CApp::retcode_t CApp::run(void) {
    if (_args->files().size() < 1) {
        cerr << "Usage: [-g groups.csv] time-export.csv" << endl;
        return RET_INCORRECT_USE;
    }

    const CArguments::arguments_map_t args = _args->args();
    for (CArguments::arguments_map_t::const_iterator it = args.begin(); it != args.end(); ++it) {

        // load activity groups if specified. This is an optional parameter
        if ((*it).first == L"g" && (*it).second != NULL) {
            if (!this->readGroups(* (*it).second)) {
                return RET_CSV_READ_GROUPS_FAILED;
            }
        }
    }

    const CArguments::referenced_files_t files = _args->files();
    for (CArguments::referenced_files_t::const_iterator it = files.begin(); it != files.end(); ++it) {
        this->readHours(*it);
    }

    if (!this->output()) {
        // do nothing for now
    }
    
    return RET_OK;
}

const bool CApp::readGroups(const wstring fileName) {

#ifdef DEBUG
    wcout << L"Reads activity groups from " << fileName << endl;
#endif
    
    CCSV csv(fileName, CCSV::CSV_TRIM | CCSV::CSV_NO_HEADERS);

    if (!csv.read()) {
        return false;
    }
    
	// clear existing groups
	_groups.clear();

    CCSV::row_t::const_iterator activity;
    for (CCSV::const_iterator it = csv.begin(); it != csv.end(); ++it) {
        activity = (*it).begin();
        
        // ignore empty groups
        if ((*it).size() < 2) {
            continue;
        }
        
        groups_t *group = &_groups[(*it).at(L"0")];
        
#ifdef DEBUG
        wcout << L"Group " << (*it).at(L"0") << L": ";
#endif

        // skip the first activity, as it's the name of the group
        for (++activity; activity != (*it).end(); ++activity) {
            group->push_back((*activity).second);
#ifdef DEBUG
            wcout << (*activity).second << L", ";
#endif
        }
        
#ifdef DEBUG
        wcout << endl << endl;
#endif
    }

    return true;
}

const long CApp::readDate(const wstring value) const {
	if (value.length() < 10) {
		return 0;
	}
	
	long year, month, day;
	wstringstream ss;

	ss << value.substr(0, 4);
	ss >> year;

	ss.clear();
	
	ss << value.substr(5, 2);
	ss >> month;

	ss.clear();

	ss << value.substr(8, 2);
	ss >> day;

	return year * 10000 + month * 100 + day;
}

const bool CApp::readHours(const wstring fileName) {
    
#ifdef DEBUG
    wcout << L"Reads activities from " << fileName << endl;
#endif
    
    CCSV csv(fileName, CCSV::CSV_TRIM);

    if (!csv.read()) {
        return false;
    }
    
    static const wchar_t *valueKeys[] = {
		L"textBox2", L"textBox1", L"htmlTextBox1", L"textBox3", L"textBox5", L"CommentsTB"
    };
    static const size_t valueKeysCount = 5; 

    CHourEntry *entry;
    const CCSV::row_t *row;
    for (CCSV::const_iterator it = csv.begin(); it != csv.end(); ++it) {
        row = &*it;
        
#if DEBUG
        cout << "Reading map: " << endl;
        for (CCSV::row_t::const_iterator itd = row->begin(); itd != row->end(); ++itd) {
            wcout << (*itd).first << L": " << (*itd).second << L", ";
        }
#endif
        
        for (size_t i = 0; i < valueKeysCount; ++i) {
            if (row->find(valueKeys[i]) == row->end()) {
                return false;
            }
        }

        entry = new CHourEntry(
            Utilities::swedishNotation(row->at(valueKeys[0])),
            row->at(valueKeys[1]), 
            Utilities::swedishNotation(row->at(valueKeys[2])), 
            this->readDate(row->at(valueKeys[3])), 
            row->at(valueKeys[4]), 
            row->at(valueKeys[5])
        );

        _hours[row->at(L"htmlTextBox9")].push_back(entry);
    }

    return true;
}

const map<wstring, wstring> *CApp::group(double &total) const {
    map<wstring, wstring> *res = new map<wstring, wstring>();
    double localTotal = 0;

    total = 0;

    // no defined groups
    if (_groups.size() < 1) {

        // iterate through all activities and sum their respective hour entries
        for (hours_map_t::const_iterator activity = _hours.begin(); activity != _hours.end(); ++activity) {
            
            localTotal = 0;
            for (hours_list_t::const_iterator item = (*activity).second.begin(); item != (*activity).second.end(); ++item) {
                localTotal += (*item)->getHours();
            }

            // add the local total to the grand total
            total += localTotal;

            (*res)[(*activity).first] = Utilities::toString(localTotal);
        }
    } else {
        vector<wstring> includedAct;

        // iterate through all assigned groups and sum activities
        for (groups_map_t::const_iterator group = _groups.begin(); group != _groups.end(); ++group) {

            localTotal = 0;

            // iterate through all activities (identified by name)
            for (groups_t::const_iterator activityName = (*group).second.begin(); activityName != (*group).second.end(); ++activityName) {

                // sought activity might not be found (or lack recorded hours due to e.g. deprecation)
                if (_hours.find(*activityName) == _hours.end()) {
                    continue;
                }
                
                const hours_list_t *hours = &_hours.at(*activityName);
                if (hours == NULL) {
                    continue;
                }

                for (hours_list_t::const_iterator hour = hours->begin(); hour != hours->end(); ++hour) {
                    localTotal += (*hour)->getHours();
                }

                includedAct.push_back(*activityName);
            }

            // Group names without the * prefix are included in the total count
            if ((*group).first[0] != '*')
                total += localTotal;
                
            (*res)[(*group).first] = Utilities::toString(localTotal);
        }

        // find activities not recorded / included, and add them to a group by their name
        vector<wstring>::const_iterator it;
        for (hours_map_t::const_iterator activity = _hours.begin(); activity != _hours.end(); ++activity) {
            
            it = find(includedAct.begin(), includedAct.end(), (*activity).first);
            if (it != includedAct.end()) {
                continue;
            }

            localTotal = 0;
            for (hours_list_t::const_iterator item = (*activity).second.begin(); item != (*activity).second.end(); ++item) {
                localTotal += (*item)->getHours();
            }

            // add the local total to the grand total
            total += localTotal;

            (*res)[(*activity).first] = Utilities::toString(localTotal);
        }
    }

    return res;
}

const CApp::hours_map_t *CApp::getHours() const {
	return & _hours;
}

const CApp::groups_map_t *CApp::getGroups() const {
	return & _groups;
}

void CApp::clearHours() {
    for (hours_map_t::const_iterator activity = _hours.begin(); activity != _hours.end(); ++activity) {
        for (hours_list_t::const_iterator item = (*activity).second.begin(); item != (*activity).second.end(); ++item) {
            delete *item;
        }
    }
    _hours.clear();
}

const bool CApp::output(void) const {
    CHTMLDocument doc;
    CHTMLElement *table = new CHTMLElement(L"table"),
                 *thead  = new CHTMLElement(L"thead"),
                 *tbody  = new CHTMLElement(L"tbody"),
                 *tfoot  = new CHTMLElement(L"tfoot"),
                 *row;
    double total = 0;
    const map<wstring, wstring> *entries = this->group(total);

    row = new CHTMLElement(L"tr");
    row->add(new CHTMLElement(L"th", L"Aktivitet"));
    row->add(new CHTMLElement(L"th", L"Timmar"));
    thead->add(row);

    for (map<wstring, wstring>::const_iterator entry = entries->begin(); entry != entries->end(); ++entry) {
        row = new CHTMLElement(L"tr");
        row->add(new CHTMLElement(L"td", (*entry).first));
        
        CHTMLElement *elem = new CHTMLElement(L"td", Utilities::toString((*entry).second));
        if ((*entry).first[0] == '*')
            elem->attr(L"style", L"color:red");
        
        row->add(elem);
        tbody->add(row);
    }

    delete entries;
    entries = NULL;

    row = new CHTMLElement(L"tr");
    row->add(new CHTMLElement(L"th", L"Totalt"));
    row->add(new CHTMLElement(L"th", Utilities::toString(total)));
    tfoot->add(row);
    
    table->add(thead);
    table->add(tbody);
    table->add(tfoot);
    
    doc.add(table);

    wofstream htmlFile("output.html");
    htmlFile << doc.toString();
    htmlFile.close();

#ifdef __MACH__
    system("open output.html");
#elif WIN32
	wchar_t directoryPath [MAX_PATH];
	wchar_t arguments[MAX_PATH*2];
	GetCurrentDirectoryW(MAX_PATH, directoryPath);
	wsprintf(arguments, L"\"C:\\Program Files\\Internet Explorer\\iexplore.exe\" \"%lS\\output.html\"", directoryPath);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	wcout << arguments << endl;
	CreateProcess(NULL, arguments, NULL, NULL, FALSE, 0, NULL, directoryPath, &si, &pi);
#endif
    
    return true;
}