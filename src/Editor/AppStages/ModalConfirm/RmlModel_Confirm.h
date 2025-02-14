#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_Confirm : public RmlModel
{
public:
	void setTitle(const Rml::String& title);
	void setQuestion(const Rml::String& question);

	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnAcceptQuestion;
	SinglecastDelegate<void()> OnRejectQuestion;

protected:
	Rml::String m_title;
	Rml::String m_question;
};
