#pragma once

#ifndef __HTMLDOCUMENT_H__
#define __HTMLDOCUMENT_H__

#include <string>
#include "HTMLElement.h"

namespace HTML
{
    class CHTMLDocument : public CHTMLElement
    {
    public:
        CHTMLDocument(void);
        ~CHTMLDocument(void);

        void add(CHTMLElement *child);
        const wstring toString(void) const;

    private:
        CHTMLElement *_body;
    };
}

#endif