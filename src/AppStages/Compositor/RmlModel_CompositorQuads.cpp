#include "RmlModel_CompositorQuads.h"
#include "MathMikan.h"
#include "ProfileConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorQuads::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorQuads::init(
	Rml::Context* rmlContext,
	const ProfileConfig* profile)
{
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
		"modify_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int listIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnModifyQuadStencilEvent && listIndex >= 0) {
				OnModifyQuadStencilEvent(m_stencilQuads[listIndex].stencil_id);
			}
		});
	constructor.BindEventCallback(
		"delete_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int listIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnDeleteQuadStencilEvent && listIndex >= 0)
			{
				OnDeleteQuadStencilEvent(m_stencilQuads[listIndex].stencil_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList(profile);
	rebuildStencilList(profile);

	return true;
}

void RmlModel_CompositorQuads::dispose()
{
	OnAddQuadStencilEvent.Clear();
	OnDeleteQuadStencilEvent.Clear();
	OnModifyQuadStencilEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorQuads::rebuildAnchorList(const ProfileConfig* profile)
{
	m_spatialAnchors.clear();
	for (const MikanSpatialAnchorInfo& anchorInfo : profile->spatialAnchorList)
	{
		m_spatialAnchors.push_back(anchorInfo.anchor_id);
	}
}

void RmlModel_CompositorQuads::rebuildStencilList(const ProfileConfig* profile)
{
	m_stencilQuads.clear();
	for (const MikanStencilQuad& quad : profile->quadStencilList)
	{
		float angles[3];
		MikanOrientationToEulerAngles(
			quad.quad_x_axis, quad.quad_y_axis, quad.quad_normal,
			angles[0], angles[1], angles[2]);

		RmlModel_CompositorQuad uiQuad = {
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
}
