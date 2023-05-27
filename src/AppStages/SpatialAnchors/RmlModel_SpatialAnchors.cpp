#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "RmlModel_SpatialAnchors.h"
#include "MathMikan.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_SpatialAnchors::s_bHasRegisteredTypes = false;

bool RmlModel_SpatialAnchors::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr)
{
	m_anchorSystemPtr = anchorSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "spatial_anchor_settings");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_SpatialAnchor>())
		{
			layer_model_handle.RegisterMember("anchor_id", &RmlModel_SpatialAnchor::anchor_id);
			layer_model_handle.RegisterMember("anchor_name", &RmlModel_SpatialAnchor::anchor_name);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_spatialAnchors)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("vr_device_list", &m_vrDeviceList);
	constructor.Bind("anchor_vr_device_path", &m_anchorVRDevicePath);
	constructor.Bind("spatial_anchors", &m_spatialAnchors);
	constructor.Bind("max_spatial_anchors", &m_maxSpatialAnchors);
	constructor.Bind("origin_anchor_id", &m_originAnchorId);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"add_new_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnAddNewAnchor) OnAddNewAnchor();
		});
	constructor.BindEventCallback(
		"update_anchor_pose",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int anchorId = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnUpdateAnchorPose) OnUpdateAnchorPose(anchorId);
		});
	constructor.BindEventCallback(
		"update_anchor_vr_device_path",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const Rml::String stringValue = ev.GetParameter("value", Rml::String());
			if (OnUpdateAnchorVRDevicePath) OnUpdateAnchorVRDevicePath(stringValue);
		});
	constructor.BindEventCallback(
		"update_anchor_name",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int anchorId = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			const bool isLineBreak = ev.GetParameter("linebreak", false);

			if (isLineBreak)
			{
				const Rml::String stringValue = ev.GetParameter("value", Rml::String());
				if (OnUpdateAnchorName) OnUpdateAnchorName(anchorId, stringValue);
			}
		});
	constructor.BindEventCallback(
		"erase_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int deleteAnchorId = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : 0);
			if (OnDeleteAnchor) OnDeleteAnchor(deleteAnchorId);
		});
	constructor.BindEventCallback(
		"goto_main_menu",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnGotoMainMenu) OnGotoMainMenu();
		});

	// Fill in the data model
	rebuildVRDevicePaths();
	rebuildAnchorList();

	// Listen for anchor system config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty += MakeDelegate(this, &RmlModel_SpatialAnchors::anchorSystemConfigMarkedDirty);

	return true;
}

void RmlModel_SpatialAnchors::dispose()
{
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -= MakeDelegate(this, &RmlModel_SpatialAnchors::anchorSystemConfigMarkedDirty);

	OnAddNewAnchor.Clear();
	OnUpdateAnchorVRDevicePath.Clear();
	OnUpdateAnchorPose.Clear();
	OnUpdateAnchorName.Clear();
	OnDeleteAnchor.Clear();
	OnGotoMainMenu.Clear();
	RmlModel::dispose();
}

void RmlModel_SpatialAnchors::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	rebuildAnchorList();
}

void RmlModel_SpatialAnchors::rebuildVRDevicePaths()
{
	VRDeviceList vrTrackers = VRDeviceManager::getInstance()->getFilteredVRDeviceList(eDeviceType::VRTracker);

	m_vrDeviceList.clear();
	for (VRDeviceViewPtr vrTrackerPtr : vrTrackers)
	{
		m_vrDeviceList.push_back(vrTrackerPtr->getDevicePath());
	}
	m_modelHandle.DirtyVariable("vr_device_list");
}

void RmlModel_SpatialAnchors::rebuildAnchorList()
{
	m_anchorVRDevicePath = m_anchorSystemPtr->getAnchorSystemConfig()->anchorVRDevicePath;
	m_modelHandle.DirtyVariable("anchor_vr_device_path");

	m_originAnchorId = m_anchorSystemPtr->getAnchorSystemConfig()->originAnchorId;
	m_modelHandle.DirtyVariable("origin_anchor_id");

	m_spatialAnchors.clear();
	auto anchorMap = m_anchorSystemPtr->getAnchorMap();
	for (auto it = anchorMap.begin(); it != anchorMap.end(); it++)
	{
		const MikanSpatialAnchorID anchorId = it->first;
		AnchorComponentPtr anchorComponent =  it->second.lock();

		RmlModel_SpatialAnchor uiAnchorInfo;
		uiAnchorInfo.anchor_id = anchorId;
		uiAnchorInfo.anchor_name = anchorComponent->getName();

		m_spatialAnchors.push_back(uiAnchorInfo);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}