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
	return true;
}

const bool CApp::readHours(const wstring fileName) {
	CCSV csv(fileName);

	if (!csv.read()) {
		return false;
	}

	CHourEntry *entry;
	const CCSV::row_t *row;
	for (CCSV::const_iterator it = csv.begin(); it != csv.end(); ++it) {
		row = &*it;

		entry = new CHourEntry(
			Utilities::swedishNotation(row->at(L"textBox2")),
			row->at(L"textBox1"), 
			Utilities::swedishNotation(row->at(L"htmlTextBox1")), 
			0L, 
			row->at(L"textBox5"), 
			row->at(L"CommentsTB")
		);

		_hours[row->at(L"htmlTextBox9")].push_back(entry);
	}

	return true;
}

const bool CApp::output(void) const {
	CHTMLDocument doc;
	CHTMLElement *table = new CHTMLElement(L"table");
	CHTMLElement *row;
	double total = 0;

	for (hours_map_t::const_iterator activity = _hours.begin(); activity != _hours.end(); ++activity) {
		double localTotal = 0;

		for (hours_list_t::const_iterator item = (*activity).second.begin(); item != (*activity).second.end(); ++item) {
			localTotal += (*item)->getHours();
		}

		total += localTotal;

		row = new CHTMLElement(L"tr");
		row->add(new CHTMLElement(L"td", (*activity).first));
		row->add(new CHTMLElement(L"td", Utilities::toString(localTotal)));
		table->add(row);
	}

	row = new CHTMLElement(L"tr");
	row->add(new CHTMLElement(L"th", L"Totalt"));
	row->add(new CHTMLElement(L"th", Utilities::toString(total)));
	table->add(row);
	doc.add(table);

	wofstream htmlFile("output.html");
	htmlFile << doc.toString();
	htmlFile.close();

	system("explorer output.html");

	return true;
}