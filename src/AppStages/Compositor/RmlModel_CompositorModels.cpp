#include "RmlModel_CompositorModels.h"
#include "MathMikan.h"
#include "ProfileConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorModels::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorModels::init(
	Rml::Context* rmlContext,
	const ProfileConfig* profile)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_models");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorModel>())
		{
			layer_model_handle.RegisterMember("stencil_id", &RmlModel_CompositorModel::stencil_id);
			layer_model_handle.RegisterMember("parent_anchor_id", &RmlModel_CompositorModel::parent_anchor_id);
			layer_model_handle.RegisterMember("model_path", &RmlModel_CompositorModel::model_path);
			layer_model_handle.RegisterMember("model_position", &RmlModel_CompositorModel::model_position);
			layer_model_handle.RegisterMember("model_angles", &RmlModel_CompositorModel::model_angles);
			layer_model_handle.RegisterMember("model_scale", &RmlModel_CompositorModel::model_scale);
			layer_model_handle.RegisterMember("disabled", &RmlModel_CompositorModel::disabled);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_stencilModels)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("spatial_anchors", &m_spatialAnchors);
	constructor.Bind("stencil_models", &m_stencilModels);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"add_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnAddModelStencilEvent) OnAddModelStencilEvent();
		});
	constructor.BindEventCallback(
		"modify_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				if (OnModifyModelStencilEvent && stencil_id >= 0)
				{
					OnModifyModelStencilEvent(stencil_id);
				}				
			}
		});
	constructor.BindEventCallback(
		"delete_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnDeleteModelStencilEvent && stencil_id >= 0) {
				OnDeleteModelStencilEvent(stencil_id);
			}
		});
	constructor.BindEventCallback(
		"select_stencil_model_path",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnSelectModelStencilPathEvent && stencil_id >= 0)
			{
				OnSelectModelStencilPathEvent(stencil_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList(profile);
	rebuildUIModelsFromProfile(profile);

	return true;
}

void RmlModel_CompositorModels::dispose()
{
	OnAddModelStencilEvent.Clear();
	OnDeleteModelStencilEvent.Clear();
	OnModifyModelStencilEvent.Clear();
	OnSelectModelStencilPathEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorModels::rebuildAnchorList(const ProfileConfig* profile)
{
	m_spatialAnchors.clear();
	for (const MikanSpatialAnchorInfo& anchorInfo : profile->spatialAnchorList)
	{
		m_spatialAnchors.push_back(anchorInfo.anchor_id);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorModels::rebuildUIModelsFromProfile(const ProfileConfig* profile)
{
	m_stencilModels.clear();
	for (const MikanStencilModelConfig& modelConfig : profile->modelStencilList)
	{
		const MikanStencilModel& modelInfo= modelConfig.modelInfo;

		RmlModel_CompositorModel uiModel = {
			modelInfo.stencil_id,
			modelInfo.parent_anchor_id,
			modelConfig.modelPath,
			Rml::Vector3f(modelInfo.model_position.x, modelInfo.model_position.y, modelInfo.model_position.z),
			Rml::Vector3f(modelInfo.model_rotator.x_angle, modelInfo.model_rotator.y_angle, modelInfo.model_rotator.z_angle),
			Rml::Vector3f(modelInfo.model_scale.x, modelInfo.model_scale.y, modelInfo.model_scale.z),
			modelInfo.is_disabled
		};
		m_stencilModels.push_back(uiModel);
	}
	m_modelHandle.DirtyVariable("stencil_models");
}

void RmlModel_CompositorModels::copyUIModelToProfile(int stencil_id, ProfileConfig* profile) const
{
	auto it = std::find_if(
		m_stencilModels.begin(), m_stencilModels.end(),
		[stencil_id](const RmlModel_CompositorModel& quad) {
			return quad.stencil_id == stencil_id;
		});
	if (it != m_stencilModels.end())
	{
		const RmlModel_CompositorModel& uiModel = *it;

		MikanStencilModel modelInfo = {
			uiModel.stencil_id,
			uiModel.parent_anchor_id,
			{uiModel.model_position.x, uiModel.model_position.y, uiModel.model_position.z},
			{uiModel.model_angles.x, uiModel.model_angles.y, uiModel.model_angles.z},
			{uiModel.model_scale.x, uiModel.model_scale.y, uiModel.model_scale.z},
			uiModel.disabled
		};
		// NOTE: This intentionally excludes the model path
		// That's handled in OnSelectModelStencilPathEvent
		profile->updateModelStencil(modelInfo);
	}
}