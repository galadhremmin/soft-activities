#pragma once

#ifndef __HTMLELEMENT_H__
#define __HTMLELEMENT_H__

#include <map>
#include <vector>
#include <string>

using namespace std;

namespace HTML {

	class CHTMLElement
	{
	public:
		CHTMLElement(const wstring tagName, const wstring innerText = L"");
		~CHTMLElement(void);

		virtual void add(CHTMLElement *child);
		void attr(const wstring key, const wstring value);
		const wstring *attr(const wstring key) const;
	
		virtual const wstring toString(void) const;

	protected:
		typedef vector<CHTMLElement *> child_list_t;
		typedef map<wstring, wstring *> attr_list_t;

	private:
		wstring *_innerText;
		wstring _tagName;
		child_list_t _children;
		attr_list_t _attr;
	};

}

#endif