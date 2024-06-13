#pragma once

#include "Constants_StencilAlignment.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_StencilAlignment : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	eStencilAlignmentMenuState getMenuState() const;
	void setMenuState(eStencilAlignmentMenuState newState);

	SinglecastDelegate<void()> OnOkEvent;
	SinglecastDelegate<void()> OnRedoEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	
	Rml::String m_menuState;
};
