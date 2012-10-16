#include "HTMLDocument.h"
#include <sstream>

using namespace HTML;

CHTMLDocument::CHTMLDocument(void) 
	: CHTMLElement(L"html") {

	CHTMLElement *head = new CHTMLElement(L"head", L"<title>Tidsummering av aktiviteter</title>"
		L"<style>body { font-family: 11pt 'segoe ui', helvetica, sans-serif; }</style>"
		L"<meta charset=\"utf-8\">"),

		*body = new CHTMLElement(L"body");

	CHTMLElement::add(head);
	CHTMLElement::add(body);

	_body = body;
}


CHTMLDocument::~CHTMLDocument(void)  {

}

void CHTMLDocument::add(const CHTMLElement * const child) {
	_body->add(child);
}

const wstring CHTMLDocument::toString(void) const {
	wstringstream output;

	output << "<!DOCTYPE html>" << endl;
	output << CHTMLElement::toString();

	return output.str();
}