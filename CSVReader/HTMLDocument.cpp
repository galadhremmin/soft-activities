#include "HTMLDocument.h"
#include <sstream>

using namespace HTML;

CHTMLDocument::CHTMLDocument(void) 
	: CHTMLElement(L"html") {

	CHTMLElement *head = new CHTMLElement(L"head", L"<title>Tidsummering av aktiviteter</title>\n"
        L"<meta charset=\"utf-8\">\n"
        L"<style>body { font: 12pt 'segoe ui', helvetica, sans-serif; }\n"
        L"table { border-collapse:collapse; width:100%; }\n"
        L"td { padding: 5px; border-bottom: 1px solid #e0e0e0; }\n"
        L"th { text-align: left; background-color: #2169A5; color: white; padding: 10px 5px 10px 5px; }\n"
        L"tbody tr:hover { background-color: #eee; }\n"
        L"</style>\n"),

		*body = new CHTMLElement(L"body");

	CHTMLElement::add(head);
	CHTMLElement::add(body);

	_body = body;
}


CHTMLDocument::~CHTMLDocument(void)  {

}

void CHTMLDocument::add(CHTMLElement *child) {
	_body->add(child);
}

const wstring CHTMLDocument::toString(void) const {
	wstringstream output;

	output << "<!DOCTYPE html>" << endl;
	output << CHTMLElement::toString();

	return output.str();
}