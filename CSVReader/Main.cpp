#include "App.h"

int main(int argc, const char **argv) {
	CApp app(argc, argv);
	return app.run();


	/*
	CEntryReader reader(args.files().at(0));
	reader.configure(args);
	if (reader.read()) {
		TActivityEntryMap activities = reader.get();
		wofstream html("output.html");

		html << L"<!DOCTYPE html>" << endl 
		     << L"<html>"
			 << L"<head>"
			 << L"<meta charset=\"utf-8\"><title>Export fr&aring;n Softweb</title>"
			 << L"<style>body{font:14px'segoe ui', helvetica, sans-serif;}</style>"
			 << L"</head>"
			 << L"<body><table cellpadding=\"4\" cellspacing=\"0\">" 
			 << L"<thead><tr><th scope=\"col\">Aktivitet</th><th scope=\"col\">Debitering</th><th scope=\"col\">Timmar</th></tr></thead><tbody>";

		float total = 0;
		for (TActivityEntryMap::const_iterator activity = activities.cbegin(); activity != activities.end(); ++activity) {
			float hours = 0;
			wstring rateType;
			for (TEntryVector::const_iterator entry = (*activity).second.begin(); entry != (*activity).second.end(); ++entry) {
				if (!hours) {
					rateType = (*entry)->getRateType();
				}

				hours += (*entry)->getHours();
			}

			html << L"<tr>" 
				 << L"<td>" << (*activity).first << L"</td>" 
				 << L"<td>" << rateType          << L"</td>"
			     << L"<td>" << hours             << L"</td>" 
				 << L"</tr>";

			total += hours;
		}


		html << L"</tbody><tfoot><tr><th colspan=\"2\" scope=\"row\">Total</th><th>" << total << L"</th></tr></tfoot></table></body>";

		html.close();
		system("explorer output.html");
	}
	*/
	return 0;
}