#include "RmlModel_Snap.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_Snap::init(Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "modal_snap");
	if (!constructor)
		return false;

	constructor.Bind("sourceFastenerId", &m_sourceFastenerId);
	constructor.Bind("targetFastenerId", &m_targetFastenerId);
	constructor.Bind("targetFastenerIds", &m_compatibleTargetFastenerIds);
	constructor.BindEventCallback(
		"request_snap",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnRequestFastenerSnap) OnRequestFastenerSnap(m_sourceFastenerId, m_targetFastenerId);
		});
	constructor.BindEventCallback(
		"cancel_snap",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancelFastenerSnap) OnCancelFastenerSnap();
		});

	return true;
}

void RmlModel_Snap::dispose()
{
	OnRequestFastenerSnap.Clear();
	OnCancelFastenerSnap.Clear();
	RmlModel::dispose();
}

void RmlModel_Snap::setSourceFastenerId(int sourceFastenerId)
{
	m_sourceFastenerId = sourceFastenerId;
	m_modelHandle.DirtyVariable("sourceFastenerId");

	rebuildCompatibleTargetFasteners();
}

void RmlModel_Snap::rebuildCompatibleTargetFasteners()
{

}

void RmlModel_Snap::setTargetFastenerId(int targetFastenerId)
{
	m_targetFastenerId = targetFastenerId;
	m_modelHandle.DirtyVariable("targetFastenerId");
}