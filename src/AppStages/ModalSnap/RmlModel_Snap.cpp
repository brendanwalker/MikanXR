#include "FastenerObjectSystem.h"
#include "RmlModel_Snap.h"
#include "ProfileConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_Snap::init(
	Rml::Context* rmlContext, 
	ProfileConfig* profile,
	int sourceFastenerId)
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

	m_sourceFastenerId = sourceFastenerId;
	m_modelHandle.DirtyVariable("sourceFastenerId");

	m_compatibleTargetFastenerIds= 
		FastenerObjectSystem::getSystem()->getValidSpatialFastenerSnapTargets(sourceFastenerId);
	m_modelHandle.DirtyVariable("targetFastenerIds");

	m_targetFastenerId = 
		m_compatibleTargetFastenerIds.size() > 0 
		? m_compatibleTargetFastenerIds[0] 
		: INVALID_MIKAN_ID;
	m_modelHandle.DirtyVariable("targetFastenerId");

	return true;
}

void RmlModel_Snap::dispose()
{
	OnRequestFastenerSnap.Clear();
	OnCancelFastenerSnap.Clear();
	RmlModel::dispose();
}