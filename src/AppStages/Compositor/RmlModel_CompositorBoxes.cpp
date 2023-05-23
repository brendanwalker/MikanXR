#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "BoxStencilComponent.h"
#include "RmlModel_CompositorBoxes.h"
#include "MathMikan.h"
#include "MathGLM.h"
#include "ProfileConfig.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorBoxes::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorBoxes::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr)
{
	m_anchorSystemPtr= anchorSystemPtr;
	m_stencilSystemPtr= stencilSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_boxes");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorBox>())
		{
			layer_model_handle.RegisterMember("stencil_name", &RmlModel_CompositorBox::stencil_name);
			layer_model_handle.RegisterMember("stencil_id", &RmlModel_CompositorBox::stencil_id);
			layer_model_handle.RegisterMember("parent_anchor_id", &RmlModel_CompositorBox::parent_anchor_id);
			layer_model_handle.RegisterMember("box_center", &RmlModel_CompositorBox::box_center);
			layer_model_handle.RegisterMember("angles", &RmlModel_CompositorBox::angles);
			layer_model_handle.RegisterMember("size", &RmlModel_CompositorBox::size);
			layer_model_handle.RegisterMember("disabled", &RmlModel_CompositorBox::disabled);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_stencilBoxes)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("spatial_anchors", &m_spatialAnchors);
	constructor.Bind("stencil_boxes", &m_stencilBoxes);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"add_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnAddBoxStencilEvent) OnAddBoxStencilEvent();
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
					const RmlModel_CompositorBox* rmlModel= getBoxRmlModel(stencil_id);
					BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);

					if (rmlModel && stencilPtr)
					{
						stencilPtr->setName(rmlModel->stencil_name);
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
				const RmlModel_CompositorBox* rmlModel = getBoxRmlModel(stencil_id);
				BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec= rmlModel->box_center;
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
				const RmlModel_CompositorBox* rmlModel = getBoxRmlModel(stencil_id);
				BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec = rmlModel->angles;
					glm::mat3 orientation;

					glm_euler_angles_to_mat3(
						uiVec.x * k_degrees_to_radians, 
						uiVec.y * k_degrees_to_radians, 
						uiVec.z * k_degrees_to_radians, 
						orientation);
					stencilPtr->setRelativeOrientation(orientation);
				}
			}
		});
	constructor.BindEventCallback(
		"modify_size",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const RmlModel_CompositorBox* rmlModel = getBoxRmlModel(stencil_id);
				BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec = rmlModel->size;

					stencilPtr->getConfig()->setBoxSize(uiVec.x, uiVec.y, uiVec.z);
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
				const RmlModel_CompositorBox* rmlModel = getBoxRmlModel(stencil_id);
				BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					stencilPtr->getConfig()->setIsDisabled(rmlModel->disabled);
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
					BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);
					if (stencilPtr != nullptr)
					{
						stencilPtr->attachSceneComponentToAnchor(new_anchor_id);
					}
				}
			}
		});
	constructor.BindEventCallback(
		"delete_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnDeleteBoxStencilEvent && stencil_id >= 0) {
				OnDeleteBoxStencilEvent(stencil_id);
			}
		});

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty+=
		MakeDelegate(this, &RmlModel_CompositorBoxes::anchorSystemConfigMarkedDirty);

	// Listen for stencil config changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorBoxes::stencilSystemConfigMarkedDirty);

	// Fill in the data model
	rebuildAnchorList();
	rebuildUIBoxesFromStencilSystem();

	return true;
}

void RmlModel_CompositorBoxes::dispose()
{
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorBoxes::stencilSystemConfigMarkedDirty);

	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorBoxes::anchorSystemConfigMarkedDirty);

	OnAddBoxStencilEvent.Clear();
	OnDeleteBoxStencilEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorBoxes::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		rebuildAnchorList();
	}
}

void RmlModel_CompositorBoxes::rebuildAnchorList()
{
	auto& anchorList = m_anchorSystemPtr->getAnchorSystemConfigConst()->spatialAnchorList;

	m_spatialAnchors.clear();
	for (AnchorConfigPtr configPtr : anchorList)
	{
		const MikanSpatialAnchorID anchorId = configPtr->getAnchorId();

		m_spatialAnchors.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorBoxes::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_boxStencilListPropertyId))
	{
		rebuildUIBoxesFromStencilSystem();
	}
	else 
	{
		BoxStencilConfigPtr boxConfigPtr= std::dynamic_pointer_cast<BoxStencilConfig>(configPtr);

		if (boxConfigPtr)
		{
			copyStencilSystemToUIBox(boxConfigPtr->getStencilId());
		}
	}
}

void RmlModel_CompositorBoxes::rebuildUIBoxesFromStencilSystem()
{
	auto& boxStencilList = m_stencilSystemPtr->getStencilSystemConfigConst()->boxStencilList;

	m_stencilBoxes.clear();
	for (BoxStencilConfigPtr configPtr : boxStencilList)
	{
		const MikanStencilBox& box = configPtr->getBoxInfo();

		float angles[3]{};
		MikanOrientationToEulerAngles(
			box.box_x_axis, box.box_y_axis, box.box_z_axis,
			angles[0], angles[1], angles[2]);

		RmlModel_CompositorBox uiQuad = {
			box.stencil_name,
			box.stencil_id,
			box.parent_anchor_id,
			Rml::Vector3f(box.box_center.x, box.box_center.y, box.box_center.z),
			Rml::Vector3f(angles[0], angles[1], angles[2]),
			Rml::Vector3f(box.box_x_size, box.box_y_size, box.box_z_size),
			box.is_disabled
		};
		m_stencilBoxes.push_back(uiQuad);
	}
	m_modelHandle.DirtyVariable("stencil_boxes");
}

RmlModel_CompositorBox* RmlModel_CompositorBoxes::getBoxRmlModel(const int stencil_id)
{
	auto it = std::find_if(
		m_stencilBoxes.begin(), m_stencilBoxes.end(),
		[stencil_id](RmlModel_CompositorBox& box) {
		return box.stencil_id == stencil_id;
	});
	
	if (it != m_stencilBoxes.end())
	{
		RmlModel_CompositorBox& uiBox = *it;
		
		return &uiBox;
	}

	return nullptr;
}

void RmlModel_CompositorBoxes::copyStencilSystemToUIBox(int stencil_id)
{
	auto it = std::find_if(
		m_stencilBoxes.begin(), m_stencilBoxes.end(),
		[stencil_id](const RmlModel_CompositorBox& box) {
		return box.stencil_id == stencil_id;
	});

	if (it != m_stencilBoxes.end())
	{
		BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id);

		if (stencilPtr != nullptr)
		{
			BoxStencilConfigPtr configPtr = stencilPtr->getConfig();
			const MikanStencilBox& box = configPtr->getBoxInfo();

			RmlModel_CompositorBox& uiBox = *it;

			uiBox.parent_anchor_id = box.parent_anchor_id;

			MikanOrientationToEulerAngles(
				box.box_x_axis, box.box_y_axis, box.box_z_axis,
				uiBox.angles.x, uiBox.angles.y, uiBox.angles.z);

			uiBox.box_center = {box.box_center.x, box.box_center.y, box.box_center.z};
			uiBox.size.x = box.box_x_size;
			uiBox.size.y = box.box_y_size;
			uiBox.size.z = box.box_z_size;
			uiBox.disabled = box.is_disabled;
			uiBox.stencil_name = box.stencil_name;
		}
		m_modelHandle.DirtyVariable("stencil_boxes");
	}
}