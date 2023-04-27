#include "AnchorObjectSystem.h"
#include "FastenerObjectSystem.h"
#include "ModelStencilComponent.h"
#include "RmlModel_CompositorModels.h"
#include "MathMikan.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorModels::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorModels::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemWeakPtr anchorSystemPtr,
	StencilObjectSystemWeakPtr stencilSystemPtr)
{
	m_anchorSystemPtr = anchorSystemPtr;
	m_stencilSystemPtr = stencilSystemPtr;

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
			layer_model_handle.RegisterMember("stencil_name", &RmlModel_CompositorModel::stencil_name);
			layer_model_handle.RegisterMember("stencil_id", &RmlModel_CompositorModel::stencil_id);
			layer_model_handle.RegisterMember("parent_anchor_id", &RmlModel_CompositorModel::parent_anchor_id);
			layer_model_handle.RegisterMember("child_fastener_ids", &RmlModel_CompositorModel::child_fastener_ids);
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
		"modify_stencil_name",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const bool isLineBreak = ev.GetParameter("linebreak", false);

				if (isLineBreak)
				{
					const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
					if (OnModifyModelStencilEvent && stencil_id >= 0)
					{
						OnModifyModelStencilEvent(stencil_id);
					}
				}
			}
		});
	constructor.BindEventCallback(
		"modify_stencil_parent_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const int new_anchor_id = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

				if (OnModifyModelStencilParentAnchorEvent && 
					stencil_id != INVALID_MIKAN_ID && 
					new_anchor_id != INVALID_MIKAN_ID)
				{
					OnModifyModelStencilParentAnchorEvent(stencil_id, new_anchor_id);
				}
			}
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
	constructor.BindEventCallback(
		"add_fastener",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnAddFastenerEvent) OnAddFastenerEvent(stencil_id);
		});	
	constructor.BindEventCallback(
		"snap_fastener",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnSnapFastenerEvent) OnSnapFastenerEvent(stencil_id);
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
			const int stencil_id = (arguments.size() == 2 ? arguments[0].Get<int>(-1) : -1);
			const int fastener_id = (arguments.size() == 2 ? arguments[1].Get<int>(-1) : -1);
			if (OnDeleteFastenerEvent && fastener_id >= 0)
			{
				OnDeleteFastenerEvent(stencil_id, fastener_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList();
	rebuildUIModelsFromProfile();

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

void RmlModel_CompositorModels::rebuildAnchorList()
{
	AnchorObjectSystemPtr anchorSystem = m_anchorSystemPtr.lock();
	auto& anchorMap = anchorSystem->getAnchorMap();

	m_spatialAnchors.clear();
	for (auto it = anchorMap.begin(); it != anchorMap.end(); ++it)
	{
		const MikanSpatialAnchorID anchorId= it->first;

		m_spatialAnchors.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorModels::rebuildUIModelsFromProfile()
{
	StencilObjectSystemPtr stencilSystem = m_stencilSystemPtr.lock();
	auto& stencilMap = stencilSystem->getModelStencilMap();

	m_stencilModels.clear();
	for (auto it = stencilMap.begin(); it != stencilMap.end(); ++it)
	{
		ModelStencilComponentPtr stencilPtr = it->second.lock();
		ModelStencilConfigPtr configPtr = stencilPtr->getConfig();
		const MikanStencilModel& modelInfo = configPtr->getModelInfo();

		const std::vector<MikanSpatialFastenerID> child_fastener_ids=
			FastenerObjectSystem::getSystem()->getSpatialFastenersWithParent(
				MikanFastenerParentType_Stencil, modelInfo.stencil_id);

		RmlModel_CompositorModel uiModel = {
			modelInfo.stencil_name,
			modelInfo.stencil_id,
			modelInfo.parent_anchor_id,
			child_fastener_ids,
			configPtr->getModelPath().string(),
			Rml::Vector3f(modelInfo.model_position.x, modelInfo.model_position.y, modelInfo.model_position.z),
			Rml::Vector3f(modelInfo.model_rotator.x_angle, modelInfo.model_rotator.y_angle, modelInfo.model_rotator.z_angle),
			Rml::Vector3f(modelInfo.model_scale.x, modelInfo.model_scale.y, modelInfo.model_scale.z),
			modelInfo.is_disabled
		};
		m_stencilModels.push_back(uiModel);
	}
	m_modelHandle.DirtyVariable("stencil_models");
}

void RmlModel_CompositorModels::copyUIModelToProfile(int stencil_id) const
{
	auto it = std::find_if(
		m_stencilModels.begin(), m_stencilModels.end(),
		[stencil_id](const RmlModel_CompositorModel& model) {
			return model.stencil_id == stencil_id;
		});
	if (it != m_stencilModels.end())
	{
		StencilObjectSystemPtr stencilSystem= m_stencilSystemPtr.lock();

		const RmlModel_CompositorModel& uiModel = *it;

		ModelStencilComponentPtr stencilPtr = stencilSystem->getModelStencilById(stencil_id).lock();
		ModelStencilConfigPtr configPtr = stencilPtr->getConfig();
		MikanStencilModel modelInfo = configPtr->getModelInfo();

		modelInfo.parent_anchor_id= uiModel.parent_anchor_id;
		modelInfo.model_position= {uiModel.model_position.x, uiModel.model_position.y, uiModel.model_position.z};
		modelInfo.model_rotator= {uiModel.model_angles.x, uiModel.model_angles.y, uiModel.model_angles.z};
		modelInfo.model_scale= {uiModel.model_scale.x, uiModel.model_scale.y, uiModel.model_scale.z};
		modelInfo.is_disabled= uiModel.disabled;

		StringUtils::formatString(modelInfo.stencil_name, sizeof(modelInfo.stencil_name), "%s", uiModel.stencil_name.c_str());

		// NOTE: This intentionally excludes the model path
		// That's handled in OnSelectModelStencilPathEvent
		configPtr->setModelInfo(modelInfo);
	}
}