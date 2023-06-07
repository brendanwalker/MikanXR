#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "ComponentFwd.h"
#include "FileBrowser/ModalDialog_FileBrowser.h"
#include "ModelStencilComponent.h"
#include "RmlModel_CompositorModels.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorModels::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorModels::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr)
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
					stencilPtr->getStencilComponentDefinition()->setIsDisabled(rmlModel->disabled);
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
			if (stencil_id >= 0)
			{
				std::filesystem::path current_dir;
				std::filesystem::path current_file;

				ModelStencilComponentPtr modelStencil = m_stencilSystemPtr->getModelStencilById(stencil_id);
				if (modelStencil != nullptr)
				{
					current_file = modelStencil->getModelStencilDefinition()->getModelPath();
					current_dir = current_file.remove_filename();
				}

				ModalDialog_FileBrowser::browseFile(
					"Select Stencil Model",
					current_dir,
					current_file,
					{".obj"},
					[this, modelStencil](const std::filesystem::path& filepath) {
					modelStencil->setModelPath(filepath);
				});
			}
		});

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorModels::anchorSystemConfigMarkedDirty);

	// Listen for stencil config changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorModels::stencilSystemConfigMarkedDirty);

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

	OnAddModelStencilEvent.Clear();
	OnDeleteModelStencilEvent.Clear();
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
	for (AnchorDefinitionPtr configPtr : anchorList)
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
	if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_modelStencilListPropertyId))
	{
		rebuildStencilUIModelsFromProfile();
	}
	else
	{
		ModelStencilDefinitionPtr modelConfigPtr = std::dynamic_pointer_cast<ModelStencilDefinition>(configPtr);

		if (modelConfigPtr)
		{
			const MikanStencilID stencilId= modelConfigPtr->getStencilId();
			auto it = std::find_if(
				m_stencilModels.begin(), m_stencilModels.end(),
				[stencilId](const RmlModel_CompositorModel& model) {
				return model.stencil_id == stencilId;
			});
			if (it != m_stencilModels.end())
			{
				RmlModel_CompositorModel& uiModel = *it;
				bool bAnyDirty= false;

				if (changedPropertySet.hasPropertyName(StencilComponentDefinition::k_parentAnchorPropertyId))
				{
					uiModel.parent_anchor_id = modelConfigPtr->getParentAnchorId();
					bAnyDirty= true;
				}
				if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeTranslationPropertyId))
				{
					MikanVector3f position= modelConfigPtr->getRelativeTranslation();
					uiModel.model_position = {position.x, position.y, position.z};
					bAnyDirty= true;
				}
				if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeQuatPropertyId))
				{
					glm::quat model_orientation= MikanQuatf_to_glm_quat(modelConfigPtr->getRelativeQuat());

					float angles[3]{};
					glm_quat_to_euler_angles(model_orientation, angles[0], angles[1], angles[2]);
					angles[0] *= k_radians_to_degrees;
					angles[1] *= k_radians_to_degrees;
					angles[2] *= k_radians_to_degrees;

					uiModel.model_angles = {angles[0], angles[1], angles[2]};
					bAnyDirty= true;
				}
				if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeScalePropertyId))
				{
					MikanVector3f scale = modelConfigPtr->getRelativeScale();

					uiModel.model_scale = {scale.x, scale.y, scale.z};
					bAnyDirty= true;
				}
				if (changedPropertySet.hasPropertyName(ModelStencilDefinition::k_modelStencilObjPathPropertyId))
				{
					uiModel.model_path= modelConfigPtr->getModelPath().string();
					bAnyDirty = true;
				}
				if (changedPropertySet.hasPropertyName(StencilComponentDefinition::k_stencilDisabledPropertyId))
				{
					uiModel.disabled = modelConfigPtr->getIsDisabled();
					bAnyDirty = true;
				}
				if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
				{
					uiModel.stencil_name = modelConfigPtr->getComponentName();
					bAnyDirty = true;
				}

				if (bAnyDirty)
				{
					m_modelHandle.DirtyVariable("stencil_models");
				}
			}
		}
	}
}

void RmlModel_CompositorModels::rebuildStencilUIModelsFromProfile()
{
	auto& stencilList = m_stencilSystemPtr->getStencilSystemConfigConst()->modelStencilList;

	m_stencilModels.clear();
	for (ModelStencilDefinitionPtr definitionPtr : stencilList)
	{
		const GlmTransform model_xform = definitionPtr->getRelativeTransform();
		const glm::vec3 model_center = model_xform.getPosition();
		const glm::quat model_orientation = model_xform.getOrientation();
		const glm::vec3 model_scale = model_xform.getScale();

		float angles[3]{};
		glm_quat_to_euler_angles(model_orientation, angles[0], angles[1], angles[2]);
		angles[0] *= k_radians_to_degrees;
		angles[1] *= k_radians_to_degrees;
		angles[2] *= k_radians_to_degrees;

		RmlModel_CompositorModel uiModel = {
			definitionPtr->getComponentName(),
			definitionPtr->getStencilId(),
			definitionPtr->getParentAnchorId(),
			definitionPtr->getModelPath().string(),
			Rml::Vector3f(model_center.x, model_center.y, model_center.z),
			Rml::Vector3f(angles[0], angles[1], angles[2]),
			Rml::Vector3f(model_scale.x, model_scale.y, model_scale.z),
			definitionPtr->getIsDisabled()
		};
		m_stencilModels.push_back(uiModel);
	}
	m_modelHandle.DirtyVariable("stencil_models");
}