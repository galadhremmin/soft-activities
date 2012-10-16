#pragma once
#include <string>
#include "HTMLElement.h"

namespace HTML
{
	class CHTMLDocument : public CHTMLElement
	{
	public:
		CHTMLDocument(void);
		~CHTMLDocument(void);

		void CHTMLDocument::add(const CHTMLElement * const child);
		const wstring toString(void) const;

	private:
		CHTMLElement *_body;
	};
}

