#pragma once
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

		virtual void add(const CHTMLElement * const child);
		void attr(const wstring key, const wstring value);
		const wstring *attr(const wstring key) const;
	
		virtual const wstring toString(void) const;

	protected:
		typedef vector<const CHTMLElement * const> child_list_t;
		typedef map<const wstring, const wstring *> attr_list_t;

	private:
		wstring *_innerText;
		wstring _tagName;
		child_list_t _children;
		attr_list_t _attr;
	};

}