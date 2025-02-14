#include "RmlModel_Confirm.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_Confirm::init(Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "modal_confirm");
	if (!constructor)
		return false;

	constructor.Bind("title", &m_title);
	constructor.Bind("question", &m_question);	
	constructor.BindEventCallback(
		"accept_question",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnAcceptQuestion) OnAcceptQuestion();
		});
	constructor.BindEventCallback(
		"reject_question",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnRejectQuestion) OnRejectQuestion();
		});

	return true;
}

void RmlModel_Confirm::dispose()
{
	OnAcceptQuestion.Clear();
	OnRejectQuestion.Clear();
	RmlModel::dispose();
}

void RmlModel_Confirm::setTitle(const Rml::String& title)
{
	m_title = title;
	m_modelHandle.DirtyVariable("title");
}

void RmlModel_Confirm::setQuestion(const Rml::String& question)
{
	m_question = question;
	m_modelHandle.DirtyVariable("question");
}