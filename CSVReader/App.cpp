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
            0L, 
            row->at(valueKeys[3]), 
            row->at(valueKeys[4])
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
    system("explorer output.html");
#endif
    
    return true;
}