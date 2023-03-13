#include "RmlModel_CompositorAnchors.h"
#include "MathMikan.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorAnchors::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorAnchors::init(
	Rml::Context* rmlContext,
	const ProfileConfig* profile)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_anchors");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorAnchor>())
		{
			layer_model_handle.RegisterMember("anchor_id", &RmlModel_CompositorAnchor::anchor_id);
			layer_model_handle.RegisterMember("child_fastener_ids", &RmlModel_CompositorAnchor::child_fastener_ids);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_spatialAnchors)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("spatial_anchors", &m_spatialAnchors);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"add_fastener",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int anchor_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnAddFastenerEvent) OnAddFastenerEvent(anchor_id);
		});
	constructor.BindEventCallback(
		"edit_fastener",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int fastener_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnEditFastenerEvent && fastener_id >= 0)
			{
				OnEditFastenerEvent(fastener_id);
			}
		});
	constructor.BindEventCallback(
		"delete_fastener",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int anchor_id = (arguments.size() == 2 ? arguments[0].Get<int>(-1) : -1);
			const int fastener_id = (arguments.size() == 2 ? arguments[1].Get<int>(-1) : -1);
			if (OnDeleteFastenerEvent && fastener_id >= 0)
			{
				OnDeleteFastenerEvent(anchor_id, fastener_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList(profile);

	return true;
}

void RmlModel_CompositorAnchors::dispose()
{
	OnAddFastenerEvent.Clear();
	OnDeleteFastenerEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorAnchors::rebuildAnchorList(const ProfileConfig* profile)
{
	m_spatialAnchors.clear();
	for (const MikanSpatialAnchorInfo& anchorInfo : profile->spatialAnchorList)
	{
		RmlModel_CompositorAnchor uiAnchorInfo;
		uiAnchorInfo.anchor_id= anchorInfo.anchor_id;
		uiAnchorInfo.child_fastener_ids= 
			profile->getSpatialFastenersWithParent(
				MikanFastenerParentType_SpatialAnchor, anchorInfo.anchor_id);

		m_spatialAnchors.push_back(uiAnchorInfo);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}