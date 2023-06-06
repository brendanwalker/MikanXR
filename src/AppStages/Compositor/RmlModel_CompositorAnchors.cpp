#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "App.h"
#include "RmlModel_CompositorAnchors.h"
#include "MathMikan.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "ProfileConfig.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorAnchors::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorAnchors::init(
	Rml::Context* rmlContext,
	ProfileConfigPtr profile,
	AnchorObjectSystemPtr anchorSystemPtr)
{
	m_profile= profile;
	m_anchorSystemPtr= anchorSystemPtr;

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
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_spatialAnchors)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("origin_anchor_id", &m_originAnchorId);
	constructor.Bind("spatial_anchors", &m_spatialAnchors);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"update_origin_pose",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			onUpdateOriginEvent();
		});
	constructor.BindEventCallback(
		"add_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			onAddAnchorEvent();
		});
	constructor.BindEventCallback(
		"edit_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int anchor_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (anchor_id >= 0)
			{
				onEditAnchorEvent(anchor_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList();

	// Listen for anchor system config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty+= 
		MakeDelegate(this, &RmlModel_CompositorAnchors::anchorSystemConfigMarkedDirty);

	return true;
}

void RmlModel_CompositorAnchors::dispose()
{
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty-= 
		MakeDelegate(this, &RmlModel_CompositorAnchors::anchorSystemConfigMarkedDirty);

	RmlModel::dispose();
}

void RmlModel_CompositorAnchors::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		rebuildAnchorList();
	}
	else if (changedPropertySet.hasPropertyName(AnchorDefinition::k_anchorNamePropertyID))
	{
		// Mark list as dirty to refresh the anchor names
		m_modelHandle.DirtyVariable("spatial_anchors");
	}
}

void RmlModel_CompositorAnchors::rebuildAnchorList()
{
	m_originAnchorId= m_anchorSystemPtr->getAnchorSystemConfig()->originAnchorId;
	m_modelHandle.DirtyVariable("origin_anchor_id");

	m_spatialAnchors.clear();
	auto anchorMap= m_anchorSystemPtr->getAnchorMap();
	for (auto it= anchorMap.begin(); it != anchorMap.end(); it++)
	{
		const MikanSpatialAnchorID anchorId= it->first;

		RmlModel_CompositorAnchor uiAnchorInfo;
		uiAnchorInfo.anchor_id= anchorId;

		m_spatialAnchors.push_back(uiAnchorInfo);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorAnchors::onUpdateOriginEvent()
{
	VRDeviceViewPtr vrDeviceView =
		VRDeviceListIterator(eDeviceType::VRTracker, m_profile->originVRDevicePath).getCurrent();

	if (vrDeviceView != nullptr)
	{
		AnchorComponentPtr originSpatialAnchor = m_anchorSystemPtr->getOriginSpatialAnchor();
		if (originSpatialAnchor)
		{
			const glm::mat4 devicePose = vrDeviceView->getCalibrationPose();

			glm::mat4 anchorXform = devicePose;
			if (m_profile->originVerticalAlignFlag)
			{
				const glm::vec3 deviceForward = glm_mat4_get_x_axis(devicePose);
				const glm::vec3 devicePosition = glm_mat4_get_position(devicePose);
				const glm::quat yawOnlyOrientation = glm::quatLookAt(deviceForward, glm::vec3(0.f, 1.f, 0.f));

				anchorXform = glm_mat4_from_pose(yawOnlyOrientation, devicePosition);
			}

			// Update origin anchor transform
			originSpatialAnchor->setWorldTransform(anchorXform);
		}
	}
}

void RmlModel_CompositorAnchors::onAddAnchorEvent()
{
	MikanSpatialAnchorInfo anchorInfo;
	memset(&anchorInfo, 0, sizeof(MikanSpatialAnchorInfo));

	const MikanSpatialAnchorID nextAnchorId = m_anchorSystemPtr->getAnchorSystemConfig()->nextAnchorId;
	StringUtils::formatString(
		anchorInfo.anchor_name, sizeof(anchorInfo.anchor_name),
		"Anchor_%d", nextAnchorId);
	anchorInfo.anchor_id = INVALID_MIKAN_ID;
	anchorInfo.anchor_xform = glm_mat4_to_MikanMatrix4f(glm::mat4(1.f));

	AnchorComponentPtr anchorComponent = m_anchorSystemPtr->addNewAnchor(anchorInfo);
	if (anchorComponent != nullptr)
	{
		// Show Anchor Triangulation Tool
		AppStage_AnchorTriangulation* anchorTriangulation = App::getInstance()->pushAppStage<AppStage_AnchorTriangulation>();
		anchorTriangulation->setTargetAnchor(anchorInfo);
	}
}

void RmlModel_CompositorAnchors::onEditAnchorEvent(MikanSpatialAnchorID anchor_id)
{
	AnchorComponentPtr anchorComponent = m_anchorSystemPtr->getSpatialAnchorById(anchor_id);
	if (anchorComponent != nullptr)
	{
		// Show Anchor Triangulation Tool
		AppStage_AnchorTriangulation* anchorTriangulation = App::getInstance()->pushAppStage<AppStage_AnchorTriangulation>();
		anchorTriangulation->setTargetAnchor(anchorComponent->getDefinition()->getAnchorInfo());
	}
}