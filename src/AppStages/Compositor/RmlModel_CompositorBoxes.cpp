#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "RmlModel_CompositorBoxes.h"
#include "MathMikan.h"
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
					if (OnModifyBoxStencilEvent && stencil_id >= 0)
					{
						OnModifyBoxStencilEvent(stencil_id);
					}
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
				if (OnModifyBoxStencilEvent && stencil_id >= 0)
				{
					OnModifyBoxStencilEvent(stencil_id);
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

				if (OnModifyBoxStencilParentAnchorEvent &&
					stencil_id != INVALID_MIKAN_ID &&
					new_anchor_id != INVALID_MIKAN_ID)
				{
					OnModifyBoxStencilParentAnchorEvent(stencil_id, new_anchor_id);
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

	// Fill in the data model
	rebuildAnchorList();
	rebuildUIBoxesFromStencilSystem();

	return true;
}

void RmlModel_CompositorBoxes::dispose()
{
	OnAddBoxStencilEvent.Clear();
	OnDeleteBoxStencilEvent.Clear();
	OnModifyBoxStencilEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorBoxes::rebuildAnchorList()
{
	auto& anchorMap= m_anchorSystemPtr->getAnchorMap();

	m_spatialAnchors.clear();
	for (auto it = anchorMap.begin(); it != anchorMap.end(); ++it)
	{
		const MikanSpatialAnchorID anchorId= it->first;

		m_spatialAnchors.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorBoxes::rebuildUIBoxesFromStencilSystem()
{
	auto& stencilMap= m_stencilSystemPtr->getBoxStencilMap();

	m_stencilBoxes.clear();
	for (auto it = stencilMap.begin(); it != stencilMap.end(); ++it)
	{
		BoxStencilComponentPtr stencilPtr= it->second.lock();
		BoxStencilConfigPtr configPtr= stencilPtr->getConfig();
		const MikanStencilBox& box= configPtr->getBoxInfo();

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

void RmlModel_CompositorBoxes::copyUIBoxToStencilSystem(int stencil_id) const
{
	auto it = std::find_if(
		m_stencilBoxes.begin(), m_stencilBoxes.end(),
		[stencil_id](const RmlModel_CompositorBox& box) {
		return box.stencil_id == stencil_id;
	});

	if (it != m_stencilBoxes.end())
	{
		const RmlModel_CompositorBox& uiBox = *it;
		BoxStencilComponentPtr stencilPtr = m_stencilSystemPtr->getBoxStencilById(stencil_id).lock();
		BoxStencilConfigPtr configPtr= stencilPtr->getConfig();
		MikanStencilBox box= configPtr->getBoxInfo();

		box.parent_anchor_id= uiBox.parent_anchor_id;

		EulerAnglesToMikanOrientation(
			uiBox.angles.x, uiBox.angles.y, uiBox.angles.z,
			box.box_x_axis, 
			box.box_y_axis, 
			box.box_z_axis);

		box.box_center= {uiBox.box_center.x, uiBox.box_center.y, uiBox.box_center.z};
		box.box_x_size= uiBox.size.x;
		box.box_y_size= uiBox.size.y;
		box.box_z_size= uiBox.size.z;

		box.is_disabled= uiBox.disabled;

		StringUtils::formatString(
			box.stencil_name, sizeof(box.stencil_name), "%s", 
			uiBox.stencil_name.c_str());

		configPtr->setBoxInfo(box);
	}
}