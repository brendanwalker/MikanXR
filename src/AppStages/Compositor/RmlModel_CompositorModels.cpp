#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
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
	AnchorObjectSystemPtr anchorSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr,
	FastenerObjectSystemPtr fastenerSystemPtr)
{
	m_anchorSystemPtr = anchorSystemPtr;
	m_stencilSystemPtr = stencilSystemPtr;
	m_fastenerSystemPtr = fastenerSystemPtr;

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
					const RmlModel_CompositorModel* rmlModel = getModelRmlModel(stencil_id);
					ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);

					if (rmlModel && stencilPtr)
					{
						stencilPtr->setName(rmlModel->stencil_name);
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

				if (stencil_id != INVALID_MIKAN_ID &&
					new_anchor_id != INVALID_MIKAN_ID)
				{
					ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);
					if (stencilPtr != nullptr)
					{
						stencilPtr->attachSceneComponentToAnchor(new_anchor_id);
					}
				}
			}
		});
	constructor.BindEventCallback(
		"modify_position",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const RmlModel_CompositorModel* rmlModel = getModelRmlModel(stencil_id);
				ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec = rmlModel->model_position;
					const glm::vec3 position(uiVec.x, uiVec.y, uiVec.z);

					stencilPtr->setRelativePosition(position);
				}
			}
		});
	constructor.BindEventCallback(
		"modify_rotation",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const RmlModel_CompositorModel* rmlModel = getModelRmlModel(stencil_id);
				ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec = rmlModel->model_angles;
					const glm::vec3 rotator(uiVec.x, uiVec.y, uiVec.z);

					stencilPtr->setRelativeOrientation(rotator);
				}
			}
		});
	constructor.BindEventCallback(
		"modify_scale",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const RmlModel_CompositorModel* rmlModel = getModelRmlModel(stencil_id);
				ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec = rmlModel->model_scale;
					const glm::vec3 scale(uiVec.x, uiVec.y, uiVec.z);

					stencilPtr->setRelativeScale(scale);
				}
			}
		});
	constructor.BindEventCallback(
		"modify_disabled",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const RmlModel_CompositorModel* rmlModel = getModelRmlModel(stencil_id);
				ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					stencilPtr->getConfig()->setIsDisabled(rmlModel->disabled);
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

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorModels::anchorSystemConfigMarkedDirty);

	// Listen for stencil config changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorModels::stencilSystemConfigMarkedDirty);

	// Listen for fastener config changes
	m_fastenerSystemPtr->getFastenerSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorModels::fastenerSystemConfigMarkedDirty);

	// Fill in the data model
	rebuildAnchorList();
	rebuildStencilUIModelsFromProfile();

	return true;
}

void RmlModel_CompositorModels::dispose()
{
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorModels::anchorSystemConfigMarkedDirty);

	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorModels::stencilSystemConfigMarkedDirty);

	m_fastenerSystemPtr->getFastenerSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorModels::fastenerSystemConfigMarkedDirty);

	OnAddModelStencilEvent.Clear();
	OnDeleteModelStencilEvent.Clear();
	OnSelectModelStencilPathEvent.Clear();
	RmlModel::dispose();
}

RmlModel_CompositorModel* RmlModel_CompositorModels::getModelRmlModel(const int stencil_id)
{
	auto it = std::find_if(
		m_stencilModels.begin(), m_stencilModels.end(),
		[stencil_id](RmlModel_CompositorModel& box) {
		return box.stencil_id == stencil_id;
	});

	if (it != m_stencilModels.end())
	{
		RmlModel_CompositorModel& uiBox = *it;

		return &uiBox;
	}

	return nullptr;
}

void RmlModel_CompositorModels::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		rebuildAnchorList();
	}
}

void RmlModel_CompositorModels::rebuildAnchorList()
{
	auto& anchorList = m_anchorSystemPtr->getAnchorSystemConfigConst()->spatialAnchorList;

	m_spatialAnchors.clear();
	for (AnchorConfigPtr configPtr : anchorList)
	{
		const MikanSpatialAnchorID anchorId= configPtr->getAnchorId();

		m_spatialAnchors.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorModels::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_quadStencilListPropertyId))
	{
		rebuildStencilUIModelsFromProfile();
	}
	else
	{
		ModelStencilConfigPtr modelConfigPtr = std::dynamic_pointer_cast<ModelStencilConfig>(configPtr);

		if (modelConfigPtr)
		{
			copyStencilSystemToUIModel(modelConfigPtr->getStencilId());
		}
	}
}

void RmlModel_CompositorModels::rebuildStencilUIModelsFromProfile()
{
	auto& modelStencilList= m_stencilSystemPtr->getStencilSystemConfigConst()->modelStencilList;

	m_stencilModels.clear();
	for (ModelStencilConfigPtr configPtr : modelStencilList)
	{
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

void RmlModel_CompositorModels::fastenerSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(FastenerObjectSystemConfig::k_fastenerListPropertyId))
	{
		rebuildStencilUIModelsFromProfile();
	}
}

void RmlModel_CompositorModels::copyStencilSystemToUIModel(int stencil_id)
{
	auto it = std::find_if(
		m_stencilModels.begin(), m_stencilModels.end(),
		[stencil_id](const RmlModel_CompositorModel& model) {
		return model.stencil_id == stencil_id;
	});
	if (it != m_stencilModels.end())
	{
		ModelStencilComponentPtr stencilPtr = m_stencilSystemPtr->getModelStencilById(stencil_id);

		if (stencilPtr != nullptr)
		{
			ModelStencilConfigPtr configPtr = stencilPtr->getConfig();
			const MikanStencilModel& modelInfo = configPtr->getModelInfo();

			RmlModel_CompositorModel& uiModel = *it;

			uiModel.parent_anchor_id = modelInfo.parent_anchor_id;
			uiModel.model_position = {uiModel.model_position.x, uiModel.model_position.y, uiModel.model_position.z};
			uiModel.model_angles = {
				modelInfo.model_rotator.x_angle,
				modelInfo.model_rotator.y_angle,
				modelInfo.model_rotator.z_angle};
			uiModel.model_scale = {modelInfo.model_scale.x, modelInfo.model_scale.y, modelInfo.model_scale.z};
			uiModel.disabled = modelInfo.is_disabled;

			uiModel.stencil_name = modelInfo.stencil_name;
		}
	}
}