#include "RmlView.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

RmlView::~RmlView()
{
	dispose();
}

void RmlView::init(Rml::ElementDocument* document)
{
	m_document= document;
}

void RmlView::dispose()
{
	m_document= nullptr;
}