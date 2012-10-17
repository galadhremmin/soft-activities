#include <iostream>
#include <fstream>
#include "App.h"
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

	for (hours_map_t::const_iterator activity = _hours.begin(); activity != _hours.end(); ++activity) {
		for (hours_list_t::const_iterator item = (*activity).second.begin(); item != (*activity).second.end(); ++item) {
			delete *item;
		}
	}
	_hours.clear();
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

const bool CApp::readHours(const wstring fileName) {
    
#ifdef DEBUG
    wcout << L"Reads activities from " << fileName << endl;
#endif
    
	CCSV csv(fileName, CCSV::CSV_TRIM);

	if (!csv.read()) {
		return false;
	}
    
    static const wchar_t *valueKeys[] = {
        L"textBox2", L"textBox1", L"htmlTextBox1", L"textBox5", L"CommentsTB"
    };
    static const size_t valueKeysCount = 5; 

	CHourEntry *entry;
	const CCSV::row_t *row;
	for (CCSV::const_iterator it = csv.begin(); it != csv.end(); ++it) {
		row = &*it;
        
#if DEBUG
        cout << "Reading map: " << endl;
        for (CCSV::row_t::const_iterator itd = row->begin(); itd != row->end(); ++itd) {
            wcout << (*itd).first << L": " << (*itd).second << ", ";
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
			0L, 
			row->at(valueKeys[3]), 
			row->at(valueKeys[4])
		);

		_hours[row->at(L"htmlTextBox9")].push_back(entry);
	}

	return true;
}

const map<wstring, wstring> *CApp::getGroups(void) const {
    return NULL;
}

const bool CApp::output(void) const {
	CHTMLDocument doc;
	CHTMLElement *table = new CHTMLElement(L"table"),
                 *thead  = new CHTMLElement(L"thead"),
                 *tbody  = new CHTMLElement(L"tbody"),
                 *tfoot  = new CHTMLElement(L"tfoot"),
                 *row;
	double total = 0;
    
	row = new CHTMLElement(L"tr");
	row->add(new CHTMLElement(L"th", L"Aktivitet"));
	row->add(new CHTMLElement(L"th", L"Timmar"));
	thead->add(row);

	for (hours_map_t::const_iterator activity = _hours.begin(); activity != _hours.end(); ++activity) {
		double localTotal = 0;

		for (hours_list_t::const_iterator item = (*activity).second.begin(); item != (*activity).second.end(); ++item) {
			localTotal += (*item)->getHours();
		}

		total += localTotal;

		row = new CHTMLElement(L"tr");
		row->add(new CHTMLElement(L"td", (*activity).first));
		row->add(new CHTMLElement(L"td", Utilities::toString(localTotal)));
		tbody->add(row);
	}

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
	system("explorer output.html");
#endif
    
	return true;
}