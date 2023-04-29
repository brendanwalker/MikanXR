#include "AnchorObjectSystem.h"
#include "RmlModel_CompositorQuads.h"
#include "MathMikan.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorQuads::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorQuads::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr)
{
	m_anchorSystemPtr= anchorSystemPtr;
	m_stencilSystemPtr= stencilSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_quads");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorQuad>())
		{
			layer_model_handle.RegisterMember("stencil_name", &RmlModel_CompositorQuad::stencil_name);
			layer_model_handle.RegisterMember("stencil_id", &RmlModel_CompositorQuad::stencil_id);
			layer_model_handle.RegisterMember("parent_anchor_id", &RmlModel_CompositorQuad::parent_anchor_id);
			layer_model_handle.RegisterMember("position", &RmlModel_CompositorQuad::position);
			layer_model_handle.RegisterMember("angles", &RmlModel_CompositorQuad::angles);
			layer_model_handle.RegisterMember("size", &RmlModel_CompositorQuad::size);
			layer_model_handle.RegisterMember("double_sided", &RmlModel_CompositorQuad::double_sided);
			layer_model_handle.RegisterMember("disabled", &RmlModel_CompositorQuad::disabled);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_stencilQuads)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("spatial_anchors", &m_spatialAnchors);
	constructor.Bind("stencil_quads", &m_stencilQuads);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"add_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnAddQuadStencilEvent) OnAddQuadStencilEvent();
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
					if (OnModifyQuadStencilEvent && stencil_id >= 0)
					{
						OnModifyQuadStencilEvent(stencil_id);
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
				if (OnModifyQuadStencilEvent && stencil_id >= 0)
				{
					OnModifyQuadStencilEvent(stencil_id);
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

				if (OnModifyQuadStencilParentAnchorEvent &&
					stencil_id != INVALID_MIKAN_ID &&
					new_anchor_id != INVALID_MIKAN_ID)
				{
					OnModifyQuadStencilParentAnchorEvent(stencil_id, new_anchor_id);
				}
			}
		});
	constructor.BindEventCallback(
		"delete_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnDeleteQuadStencilEvent && stencil_id >= 0) {
				OnDeleteQuadStencilEvent(stencil_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList();
	rebuildUIQuadsFromProfile();

	return true;
}

void RmlModel_CompositorQuads::dispose()
{
	OnAddQuadStencilEvent.Clear();
	OnDeleteQuadStencilEvent.Clear();
	OnModifyQuadStencilEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorQuads::rebuildAnchorList()
{
	m_spatialAnchors.clear();

	auto& anchorMap = m_anchorSystemPtr->getAnchorMap();
	for (auto it = anchorMap.begin(); it != anchorMap.end(); ++it)
	{
		const MikanSpatialAnchorID anchorId= it->first;

		m_spatialAnchors.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorQuads::rebuildUIQuadsFromProfile()
{
	m_stencilQuads.clear();

	auto& stencilMap = m_stencilSystemPtr->getQuadStencilMap();
	for (auto it = stencilMap.begin(); it != stencilMap.end(); ++it)
	{
		QuadStencilComponentPtr stencilPtr = it->second.lock();
		QuadStencilConfigPtr configPtr = stencilPtr->getConfig();
		const MikanStencilQuad& quad = configPtr->getQuadInfo();

		float angles[3]{};
		MikanOrientationToEulerAngles(
			quad.quad_x_axis, quad.quad_y_axis, quad.quad_normal,
			angles[0], angles[1], angles[2]);

		RmlModel_CompositorQuad uiQuad = {
			quad.stencil_name,
			quad.stencil_id,
			quad.parent_anchor_id,
			Rml::Vector3f(quad.quad_center.x, quad.quad_center.y, quad.quad_center.z),
			Rml::Vector3f(angles[0], angles[1], angles[2]),
			Rml::Vector2f(quad.quad_width, quad.quad_height),
			quad.is_double_sided,
			quad.is_disabled
		};
		m_stencilQuads.push_back(uiQuad);
	}
	m_modelHandle.DirtyVariable("stencil_quads");
}

void RmlModel_CompositorQuads::copyUIQuadToProfile(int stencil_id) const
{
	auto it = std::find_if(
		m_stencilQuads.begin(), m_stencilQuads.end(),
		[stencil_id](const RmlModel_CompositorQuad& quad) {
		return quad.stencil_id == stencil_id;
	});
	if (it != m_stencilQuads.end())
	{
		QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);
		QuadStencilConfigPtr configPtr = stencilPtr->getConfig();
		MikanStencilQuad quad = configPtr->getQuadInfo();

		const RmlModel_CompositorQuad& uiQuad = *it;

		EulerAnglesToMikanOrientation(
			uiQuad.angles.x, uiQuad.angles.y, uiQuad.angles.z,
			quad.quad_x_axis, quad.quad_y_axis, quad.quad_normal);

		quad.parent_anchor_id= uiQuad.parent_anchor_id;
		quad.quad_center= {uiQuad.position.x, uiQuad.position.y, uiQuad.position.z};
		quad.quad_width= uiQuad.size.x;
		quad.quad_height= uiQuad.size.y;
		quad.is_double_sided= uiQuad.double_sided;
		quad.is_disabled= uiQuad.disabled;

		StringUtils::formatString(quad.stencil_name, sizeof(quad.stencil_name), "%s", uiQuad.stencil_name.c_str());

		configPtr->setQuadInfo(quad);
	}
}