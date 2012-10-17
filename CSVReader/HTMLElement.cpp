#include "HTMLElement.h"
#include <sstream>

using namespace HTML;

CHTMLElement::CHTMLElement(const wstring tagName, const wstring innerText) {
	_tagName = tagName;
	_innerText = NULL;

	if (innerText.length() > 0) {
		_innerText = new wstring(innerText);
	}
}

CHTMLElement::~CHTMLElement(void) {
	if (_innerText != NULL) {
		delete _innerText;
		_innerText = NULL;
	}

	for (attr_list_t::const_iterator it = _attr.begin(); it != _attr.begin(); ++it) {
		delete (*it).second;
	}

	for (child_list_t::const_iterator it = _children.begin(); it != _children.end(); ++it) {
		delete *it;
	}

	_attr.clear();
	_children.clear();
}

void CHTMLElement::add(CHTMLElement *child) {
	_children.push_back(child);
}

void CHTMLElement::attr(const wstring key, const wstring value) {
	wstring *item;

	if (this->attr(key) != NULL) {
		item = _attr.at(key);
		_attr.erase(key);

		delete item;
		item = NULL;
	}

	item = new wstring(value);
	_attr[key] = item;
}

const wstring *CHTMLElement::attr(const wstring key) const {
	return _attr.at(key);
}
	
const wstring CHTMLElement::toString(void) const {
	wostringstream buffer;

	buffer << L"<" << _tagName;
	
	for (attr_list_t::const_iterator it = _attr.begin(); it != _attr.begin(); ++it) {
		buffer << " " << (*it).first << "=\"" << (*it).second << "\"";
	}
	
	buffer << L">";

	if (_innerText != NULL) {
		buffer << *_innerText;

	} else if (_children.size() > 0) {
		buffer << endl;

		for (child_list_t::const_iterator it = _children.begin(); it != _children.end(); ++it) {
			buffer << (*it)->toString() << endl;
		}
	}

	if (_innerText != NULL || _children.size() > 0) {
		buffer << L"</" << _tagName << L">";
	}

	return buffer.str();
}