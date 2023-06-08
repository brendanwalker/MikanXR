#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "RmlModel_CompositorQuads.h"
#include "MathMikan.h"
#include "MathGLM.h"
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
					const RmlModel_CompositorQuad* rmlModel = getQuadRmlModel(stencil_id);
					QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

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
					QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);
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
				const RmlModel_CompositorQuad* rmlModel = getQuadRmlModel(stencil_id);
				QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector3f& uiVec = rmlModel->position;
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
				const RmlModel_CompositorQuad* rmlModel = getQuadRmlModel(stencil_id);
				QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

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
				const RmlModel_CompositorQuad* rmlModel = getQuadRmlModel(stencil_id);
				QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					const Rml::Vector2f& uiVec = rmlModel->size;

					stencilPtr->getQuadStencilDefinition()->setQuadSize(uiVec.x, uiVec.y);
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
				const RmlModel_CompositorQuad* rmlModel = getQuadRmlModel(stencil_id);
				QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					stencilPtr->getQuadStencilDefinition()->setIsDisabled(rmlModel->disabled);
				}
			}
		});
	constructor.BindEventCallback(
		"modify_double_sided",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			// Only consider change events when it resulted in a valid value
			if (ev.GetId() == Rml::EventId::Submit)
			{
				const int stencil_id = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
				const RmlModel_CompositorQuad* rmlModel = getQuadRmlModel(stencil_id);
				QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

				if (rmlModel && stencilPtr)
				{
					stencilPtr->getQuadStencilDefinition()->setIsDoubleSided(rmlModel->double_sided);
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

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorQuads::anchorSystemConfigMarkedDirty);

	// Listen for stencil config changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorQuads::stencilSystemConfigMarkedDirty);

	// Fill in the data model
	rebuildAnchorList();
	rebuildUIQuadsFromStencilSystem();

	return true;
}

void RmlModel_CompositorQuads::dispose()
{
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorQuads::stencilSystemConfigMarkedDirty);

	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorQuads::anchorSystemConfigMarkedDirty);

	OnAddQuadStencilEvent.Clear();
	OnDeleteQuadStencilEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorQuads::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		rebuildAnchorList();
	}
}

void RmlModel_CompositorQuads::rebuildAnchorList()
{
	m_spatialAnchors.clear();

	auto& anchorList = m_anchorSystemPtr->getAnchorSystemConfig()->spatialAnchorList;
	for (AnchorDefinitionPtr configPtr : anchorList)
	{
		const MikanSpatialAnchorID anchorId= configPtr->getAnchorId();

		m_spatialAnchors.push_back(anchorId);
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}

void RmlModel_CompositorQuads::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_quadStencilListPropertyId))
	{
		rebuildUIQuadsFromStencilSystem();
	}
	else
	{
		QuadStencilDefinitionPtr quadConfigPtr = std::dynamic_pointer_cast<QuadStencilDefinition>(configPtr);

		if (quadConfigPtr)
		{
			copyStencilSystemToUIQuad(quadConfigPtr->getStencilId());
		}
	}
}

void RmlModel_CompositorQuads::rebuildUIQuadsFromStencilSystem()
{
	auto& stencilList = m_stencilSystemPtr->getStencilSystemConfigConst()->quadStencilList;

	m_stencilQuads.clear();
	for (QuadStencilDefinitionPtr definitionPtr : stencilList)
	{
		const GlmTransform quad_xform= definitionPtr->getRelativeTransform();
		const glm::vec3 quad_center= quad_xform.getPosition();
		const glm::quat quad_orientation= quad_xform.getOrientation();
		
		float angles[3]{};
		glm_quat_to_euler_angles(quad_orientation, angles[0], angles[1], angles[2]);
		angles[0]*= k_radians_to_degrees;
		angles[1]*= k_radians_to_degrees;
		angles[2]*= k_radians_to_degrees;

		RmlModel_CompositorQuad uiQuad = {
			definitionPtr->getComponentName(),
			definitionPtr->getStencilId(),
			definitionPtr->getParentAnchorId(),
			Rml::Vector3f(quad_center.x, quad_center.y, quad_center.z),
			Rml::Vector3f(angles[0], angles[1], angles[2]),
			Rml::Vector2f(definitionPtr->getQuadWidth(), definitionPtr->getQuadHeight()),
			definitionPtr->getIsDoubleSided(),
			definitionPtr->getIsDisabled()
		};
		m_stencilQuads.push_back(uiQuad);
	}
	m_modelHandle.DirtyVariable("stencil_quads");
}

RmlModel_CompositorQuad* RmlModel_CompositorQuads::getQuadRmlModel(const int stencil_id)
{
	auto it = std::find_if(
		m_stencilQuads.begin(), m_stencilQuads.end(),
		[stencil_id](RmlModel_CompositorQuad& box) {
		return box.stencil_id == stencil_id;
	});

	if (it != m_stencilQuads.end())
	{
		RmlModel_CompositorQuad& uiQuad = *it;

		return &uiQuad;
	}

	return nullptr;
}

void RmlModel_CompositorQuads::copyStencilSystemToUIQuad(int stencil_id)
{
	auto it = std::find_if(
		m_stencilQuads.begin(), m_stencilQuads.end(),
		[stencil_id](RmlModel_CompositorQuad& quad) {
			return quad.stencil_id == stencil_id;
		});
	if (it != m_stencilQuads.end())
	{
		QuadStencilComponentPtr stencilPtr = m_stencilSystemPtr->getQuadStencilById(stencil_id);

		if (stencilPtr != nullptr)
		{
			QuadStencilDefinitionPtr definitionPtr= stencilPtr->getQuadStencilDefinition();
			const GlmTransform quad_xform = definitionPtr->getRelativeTransform();
			const glm::vec3 quad_center = quad_xform.getPosition();
			const glm::quat quad_orientation = quad_xform.getOrientation();
			
			RmlModel_CompositorQuad& uiQuad = *it;

			glm_quat_to_euler_angles(quad_orientation, uiQuad.angles.x, uiQuad.angles.y, uiQuad.angles.z);
			uiQuad.angles.x *= k_radians_to_degrees;
			uiQuad.angles.y *= k_radians_to_degrees;
			uiQuad.angles.z *= k_radians_to_degrees;

			uiQuad.parent_anchor_id = definitionPtr->getParentAnchorId();
			uiQuad.position = {quad_center.x, quad_center.y, quad_center.z};
			uiQuad.size.x = definitionPtr->getQuadWidth();
			uiQuad.size.y = definitionPtr->getQuadHeight();
			uiQuad.double_sided = definitionPtr->getIsDoubleSided();
			uiQuad.disabled = definitionPtr->getIsDisabled();
			uiQuad.stencil_name = definitionPtr->getComponentName();
		}
		m_modelHandle.DirtyVariable("stencil_quads");
	}
}