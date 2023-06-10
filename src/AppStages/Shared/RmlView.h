#pragma once

#include "RmlFwd.h"

class RmlView
{
public:
	RmlView() = default;
	virtual ~RmlView();

	Rml::ElementDocument* getDocument() { return m_document; }

	virtual void init(Rml::ElementDocument* document);
	virtual void dispose();

protected:
	Rml::ElementDocument* m_document = nullptr;
};
