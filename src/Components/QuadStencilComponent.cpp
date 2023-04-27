#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "BoxColliderComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "QuadStencilComponent.h"
#include "SelectionComponent.h"
#include "StringUtils.h"
#include "TextStyle.h"

#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4_precision.hpp"

// -- QuadConfig -----
QuadStencilConfig::QuadStencilConfig()
{
	memset(&m_quadInfo, 0, sizeof(MikanStencilQuad));
	m_quadInfo.stencil_id = INVALID_MIKAN_ID;
	m_quadInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

QuadStencilConfig::QuadStencilConfig(MikanStencilID stencilId)
{
	memset(&m_quadInfo, 0, sizeof(MikanStencilQuad));
	m_quadInfo.stencil_id = stencilId;
	m_quadInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

configuru::Config QuadStencilConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["stencil_id"] = m_quadInfo.stencil_id;
	pt["parent_anchor_id"] = m_quadInfo.parent_anchor_id;
	pt["quad_width"] = m_quadInfo.quad_width;
	pt["quad_height"] = m_quadInfo.quad_height;
	pt["is_double_sided"] = m_quadInfo.is_double_sided;
	pt["is_disabled"] = m_quadInfo.is_disabled;
	pt["stencil_name"] = m_quadInfo.stencil_name;

	writeVector3f(pt, "quad_center", m_quadInfo.quad_center);
	writeVector3f(pt, "quad_x_axis", m_quadInfo.quad_x_axis);
	writeVector3f(pt, "quad_y_axis", m_quadInfo.quad_y_axis);
	writeVector3f(pt, "quad_normal", m_quadInfo.quad_normal);

	return pt;
}

void QuadStencilConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	if (pt.has_key("stencil_id"))
	{
		MikanStencilQuad stencil;
		memset(&stencil, 0, sizeof(stencil));

		stencil.stencil_id = pt.get<int>("stencil_id");
		stencil.parent_anchor_id = pt.get_or<int>("parent_anchor_id", -1);
		readVector3f(pt, "quad_center", stencil.quad_center);
		readVector3f(pt, "quad_x_axis", stencil.quad_x_axis);
		readVector3f(pt, "quad_y_axis", stencil.quad_y_axis);
		readVector3f(pt, "quad_normal", stencil.quad_normal);
		stencil.quad_width = pt.get_or<float>("quad_width", 0.25f);
		stencil.quad_height = pt.get_or<float>("quad_height", 0.25f);
		stencil.is_double_sided = pt.get_or<bool>("is_double_sided", false);
		stencil.is_disabled = pt.get_or<bool>("is_disabled", false);

		const std::string stencil_name = pt.get_or<std::string>("stencil_name", "");
		StringUtils::formatString(stencil.stencil_name, sizeof(stencil.stencil_name), "%s", stencil_name.c_str());
	}
}

void QuadStencilConfig::setQuadInfo(const MikanStencilQuad& quadInfo)
{
	m_quadInfo= quadInfo;
	markDirty();
}

const glm::mat4 QuadStencilConfig::getQuadMat4() const
{
	return glm::mat4 (
		glm::vec4(MikanVector3f_to_glm_vec3(m_quadInfo.quad_x_axis), 0.f),
		glm::vec4(MikanVector3f_to_glm_vec3(m_quadInfo.quad_y_axis), 0.f),
		glm::vec4(MikanVector3f_to_glm_vec3(m_quadInfo.quad_normal), 0.f),
		glm::vec4(MikanVector3f_to_glm_vec3(m_quadInfo.quad_center), 1.f));
}

void QuadStencilConfig::setQuadMat4(const glm::mat4& xform)
{
	m_quadInfo.quad_x_axis = glm_vec3_to_MikanVector3f(xform[0]);
	m_quadInfo.quad_y_axis = glm_vec3_to_MikanVector3f(xform[1]);
	m_quadInfo.quad_normal = glm_vec3_to_MikanVector3f(xform[2]);
	m_quadInfo.quad_center = glm_vec3_to_MikanVector3f(xform[3]);
	markDirty();
}

const GlmTransform QuadStencilConfig::getQuadTransform() const
{
	return GlmTransform(
		MikanVector3f_to_glm_vec3(m_quadInfo.quad_center),
		glm::quat_cast(glm::mat3(
			MikanVector3f_to_glm_vec3(m_quadInfo.quad_x_axis),
			MikanVector3f_to_glm_vec3(m_quadInfo.quad_y_axis),
			MikanVector3f_to_glm_vec3(m_quadInfo.quad_normal))));
}

void QuadStencilConfig::setQuadTransform(const GlmTransform& transform)
{
	glm::mat4 xform= transform.getMat4();

	m_quadInfo.quad_x_axis = glm_vec3_to_MikanVector3f(xform[0]);
	m_quadInfo.quad_y_axis = glm_vec3_to_MikanVector3f(xform[1]);
	m_quadInfo.quad_normal = glm_vec3_to_MikanVector3f(xform[2]);
	m_quadInfo.quad_center = glm_vec3_to_MikanVector3f(xform[3]);
	markDirty();
}

void QuadStencilConfig::setQuadWidth(float width)
{
	m_quadInfo.quad_width = width;
	markDirty();
}

void QuadStencilConfig::setQuadHeight(float height)
{
	m_quadInfo.quad_height = height;
	markDirty();
}

void QuadStencilConfig::setIsDoubleSided(bool flag)
{
	m_quadInfo.is_double_sided = flag;
	markDirty();
}

void QuadStencilConfig::setIsDisabled(bool flag)
{
	m_quadInfo.is_disabled = flag;
	markDirty();
}

void QuadStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_quadInfo.stencil_name, stencilName.c_str(), sizeof(m_quadInfo.stencil_name) - 1);
	markDirty();
}

// -- QuadStencilComponent -----
QuadStencilComponent::QuadStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
}

void QuadStencilComponent::init()
{
	MikanComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	updateSceneComponentTransform();
	updateBoxColliderExtents();
}

void QuadStencilComponent::update()
{
	MikanComponent::update();

	if (!m_config->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = m_sceneComponent.lock()->getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		glm::vec3 color = Colors::DarkGray;
		SelectionComponentPtr selectionComponent = m_selectionComponent.lock();
		if (selectionComponent)
		{
			if (selectionComponent->getIsSelected())
				color = Colors::Yellow;
			else if (selectionComponent->getIsHovered())
				color = Colors::LightGray;
		}

		drawTransformedQuad(xform, m_config->getQuadWidth(), m_config->getQuadHeight(), color);
		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", m_config->getStencilId());
	}
}

void QuadStencilComponent::setConfig(QuadStencilConfigPtr config)
{
	MikanSpatialAnchorID currentParentId= m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
	MikanSpatialAnchorID newParentId= config ? config->getParentAnchorId() : INVALID_MIKAN_ID;
	if (currentParentId != newParentId)
	{
		attachSceneComponentToAnchor(newParentId);
	}

	m_config= config;

	updateSceneComponentTransform();
	updateBoxColliderExtents();
}

void QuadStencilComponent::onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr)
{
	m_config->setQuadTransform(sceneComponentPtr->getRelativeTransform());
}

void QuadStencilComponent::updateSceneComponentTransform()
{
	SceneComponentPtr sceneComponent= m_sceneComponent.lock();
	if (sceneComponent)
	{
		sceneComponent->setRelativeTransform(m_config->getQuadTransform());
	}
}

void QuadStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider = m_boxCollider.lock();
	if (boxCollider)
	{
		boxCollider->setHalfExtents(glm::vec3(m_config->getQuadWidth(), m_config->getQuadHeight(), 0.01f) * 0.5f);
	}
}