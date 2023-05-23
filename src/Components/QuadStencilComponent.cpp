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
#include "StencilObjectSystemConfig.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"
#include "TextStyle.h"

#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4_precision.hpp"

// -- QuadConfig -----
const std::string QuadStencilConfig::k_quadStencilXAxisPropertyId = "quad_x_axis";
const std::string QuadStencilConfig::k_quadStencilYAxisPropertyId = "quad_y_axis";
const std::string QuadStencilConfig::k_quadStencilNormalPropertyId = "quad_normal";
const std::string QuadStencilConfig::k_quadStencilCenterPropertyId = "quad_center";
const std::string QuadStencilConfig::k_quadStencilWidthPropertyId = "quad_width";
const std::string QuadStencilConfig::k_quadStencilHeightPropertyId = "quad_height";
const std::string QuadStencilConfig::k_quadStencilDisabledPropertyId = "is_disabled";
const std::string QuadStencilConfig::k_quadStencilDoubleSidedPropertyId = "is_double_sided";
const std::string QuadStencilConfig::k_quadStencilNamePropertyId = "stencil_name";

QuadStencilConfig::QuadStencilConfig()
{
	memset(&m_quadInfo, 0, sizeof(MikanStencilQuad));
	m_quadInfo.stencil_id = INVALID_MIKAN_ID;
	m_quadInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

QuadStencilConfig::QuadStencilConfig(const MikanStencilQuad& quadInfo)
{
	m_quadInfo= quadInfo;
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
		memset(&m_quadInfo, 0, sizeof(m_quadInfo));

		m_quadInfo.stencil_id = pt.get<int>("stencil_id");
		m_quadInfo.parent_anchor_id = pt.get_or<int>("parent_anchor_id", -1);
		readVector3f(pt, "quad_center", m_quadInfo.quad_center);
		readVector3f(pt, "quad_x_axis", m_quadInfo.quad_x_axis);
		readVector3f(pt, "quad_y_axis", m_quadInfo.quad_y_axis);
		readVector3f(pt, "quad_normal", m_quadInfo.quad_normal);
		m_quadInfo.quad_width = pt.get_or<float>("quad_width", 0.25f);
		m_quadInfo.quad_height = pt.get_or<float>("quad_height", 0.25f);
		m_quadInfo.is_double_sided = pt.get_or<bool>("is_double_sided", false);
		m_quadInfo.is_disabled = pt.get_or<bool>("is_disabled", false);

		const std::string stencil_name = pt.get_or<std::string>("stencil_name", "");
		StringUtils::formatString(m_quadInfo.stencil_name, sizeof(m_quadInfo.stencil_name), "%s", stencil_name.c_str());
	}
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
	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_quadStencilXAxisPropertyId)
				.addPropertyName(k_quadStencilYAxisPropertyId)
				.addPropertyName(k_quadStencilNormalPropertyId)
				.addPropertyName(k_quadStencilCenterPropertyId));
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
	markDirty(ConfigPropertyChangeSet()
			  .addPropertyName(k_quadStencilXAxisPropertyId)
			  .addPropertyName(k_quadStencilYAxisPropertyId)
			  .addPropertyName(k_quadStencilNormalPropertyId)
			  .addPropertyName(k_quadStencilCenterPropertyId));
}

void QuadStencilConfig::setQuadXAxis(const MikanVector3f& xAxis)
{
	m_quadInfo.quad_x_axis = xAxis;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilXAxisPropertyId));
}

void QuadStencilConfig::setQuadYAxis(const MikanVector3f& yAxis)
{
	m_quadInfo.quad_y_axis = yAxis;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilYAxisPropertyId));
}

void QuadStencilConfig::setQuadNormal(const MikanVector3f& normal)
{
	m_quadInfo.quad_normal = normal;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilNormalPropertyId));
}

void QuadStencilConfig::setQuadOrientation(const glm::mat3& R)
{
	m_quadInfo.quad_x_axis = glm_vec3_to_MikanVector3f(R[0]);
	m_quadInfo.quad_y_axis = glm_vec3_to_MikanVector3f(R[1]);
	m_quadInfo.quad_normal = glm_vec3_to_MikanVector3f(R[2]);
	markDirty(ConfigPropertyChangeSet()
			  .addPropertyName(k_quadStencilXAxisPropertyId)
			  .addPropertyName(k_quadStencilYAxisPropertyId)
			  .addPropertyName(k_quadStencilNormalPropertyId));
}

void QuadStencilConfig::setQuadCenter(const MikanVector3f& center)
{
	m_quadInfo.quad_center = center;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilCenterPropertyId));
}

void QuadStencilConfig::setQuadWidth(float width)
{
	m_quadInfo.quad_width = width;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilWidthPropertyId));
}

void QuadStencilConfig::setQuadHeight(float height)
{
	m_quadInfo.quad_height = height;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilHeightPropertyId));
}

void QuadStencilConfig::setQuadSize(float width, float height)
{
	m_quadInfo.quad_width = width;
	m_quadInfo.quad_height = height;
	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_quadStencilWidthPropertyId)
				.addPropertyName(k_quadStencilHeightPropertyId));
}

void QuadStencilConfig::setIsDoubleSided(bool flag)
{
	m_quadInfo.is_double_sided = flag;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilDoubleSidedPropertyId));
}

void QuadStencilConfig::setIsDisabled(bool flag)
{
	m_quadInfo.is_disabled = flag;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilDisabledPropertyId));
}

void QuadStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_quadInfo.stencil_name, stencilName.c_str(), sizeof(m_quadInfo.stencil_name) - 1);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilNamePropertyId));
}

// -- QuadStencilComponent -----
QuadStencilComponent::QuadStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void QuadStencilComponent::init()
{
	StencilComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	updateBoxColliderExtents();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

void QuadStencilComponent::customRender()
{
	if (!m_config->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = getWorldTransform();
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
	assert(!m_bIsInitialized);
	m_config = config;

	// Initially the quad component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);
	m_worldTransform = config->getQuadMat4();
	m_relativeTransform = GlmTransform(m_worldTransform);

	// Make the component name match the config name
	m_name= config->getStencilName();

	// Setup initial attachment
	MikanSpatialAnchorID currentParentId= m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
	attachSceneComponentToAnchor(currentParentId);
}

void QuadStencilComponent::setRelativePosition(const glm::vec3& position)
{
	m_relativeTransform.setPosition(position);
	StencilComponent::setRelativeTransform(m_relativeTransform);

	if (m_bIsInitialized)
		m_config->setQuadCenter({position.x, position.y, position.z});
}

void QuadStencilComponent::setRelativeOrientation(const glm::mat3& rotation)
{
	m_relativeTransform.setOrientation(glm::quat_cast(rotation));
	StencilComponent::setRelativeTransform(m_relativeTransform);

	if (m_bIsInitialized)
		m_config->setQuadOrientation(rotation);
}

void QuadStencilComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	StencilComponent::setRelativeTransform(newRelativeXform);

	if (m_bIsInitialized)
		m_config->setQuadTransform(newRelativeXform);
}

void QuadStencilComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	StencilComponent::setWorldTransform(newWorldXform);

	if (m_bIsInitialized)
		m_config->setQuadMat4(getRelativeTransform().getMat4());
}

void QuadStencilComponent::setName(const std::string& name)
{
	StencilComponent::setName(name);

	if (m_bIsInitialized)
		m_config->setStencilName(name);
}

MikanStencilID QuadStencilComponent::getParentAnchorId() const
{
	return m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
}

void QuadStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider = m_boxCollider.lock();
	if (boxCollider)
	{
		boxCollider->setHalfExtents(glm::vec3(m_config->getQuadWidth(), m_config->getQuadHeight(), 0.01f) * 0.5f);
	}
}