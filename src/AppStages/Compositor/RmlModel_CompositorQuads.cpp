#include "RmlModel_CompositorQuads.h"
#include "MathMikan.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

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
			const int stencilIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (stencilIndex >= 0 && stencilIndex < (int)m_stencilQuads.size())
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
		"delete_stencil",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (OnDeleteQuadStencilEvent && stencil_id >= 0) {
				OnDeleteQuadStencilEvent(stencil_id);
			}
		});

	// Fill in the data model
	rebuildAnchorList(profile);
	rebuildUIQuadsFromProfile(profile);

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
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorQuads::rebuildUIQuadsFromProfile(const ProfileConfig* profile)
{
	m_stencilQuads.clear();
	for (const MikanStencilQuad& quad : profile->quadStencilList)
	{
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

void RmlModel_CompositorQuads::copyUIQuadToProfile(int stencil_id, ProfileConfig* profile) const
{
	auto it = std::find_if(
		m_stencilQuads.begin(), m_stencilQuads.end(),
		[stencil_id](const RmlModel_CompositorQuad& quad) {
		return quad.stencil_id == stencil_id;
	});
	if (it != m_stencilQuads.end())
	{
		const RmlModel_CompositorQuad& uiQuad = *it;

		MikanVector3f quad_x_axis;
		MikanVector3f quad_y_axis;
		MikanVector3f quad_normal;
		EulerAnglesToMikanOrientation(
			uiQuad.angles.x, uiQuad.angles.y, uiQuad.angles.z,
			quad_x_axis, quad_y_axis, quad_normal);

		MikanStencilQuad quad = {
			uiQuad.stencil_id,
			uiQuad.parent_anchor_id,
			{uiQuad.position.x, uiQuad.position.y, uiQuad.position.z},
			quad_x_axis,
			quad_y_axis,
			quad_normal,
			uiQuad.size.x,
			uiQuad.size.y,
			uiQuad.double_sided,
			uiQuad.disabled
		};

		StringUtils::formatString(quad.stencil_name, sizeof(quad.stencil_name), "%s", uiQuad.stencil_name.c_str());

		profile->updateQuadStencil(quad);
	}
}