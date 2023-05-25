#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "BoxColliderComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"
#include "StringUtils.h"
#include "TextStyle.h"

// -- BoxStencilComponent -----
const std::string BoxStencilConfig::k_boxParentAnchorPropertyId = "parent_anchor_id";
const std::string BoxStencilConfig::k_boxStencilXAxisPropertyId = "box_x_axis";
const std::string BoxStencilConfig::k_boxStencilYAxisPropertyId = "box_y_axis";
const std::string BoxStencilConfig::k_boxStencilZAxisPropertyId = "box_z_axis";
const std::string BoxStencilConfig::k_boxStencilCenterPropertyId = "box_center";
const std::string BoxStencilConfig::k_boxStencilXSizePropertyId = "box_x_size";
const std::string BoxStencilConfig::k_boxStencilYSizePropertyId = "box_y_size";
const std::string BoxStencilConfig::k_boxStencilZSizePropertyId = "box_z_size";
const std::string BoxStencilConfig::k_boxStencilDisabledPropertyId = "is_disabled";
const std::string BoxStencilConfig::k_boxStencilNamePropertyId = "stencil_name";

BoxStencilConfig::BoxStencilConfig()
{
	memset(&m_boxInfo, 0, sizeof(MikanStencilBox));
	m_boxInfo.stencil_id = INVALID_MIKAN_ID;
	m_boxInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

BoxStencilConfig::BoxStencilConfig(const MikanStencilBox& box)
{
	m_boxInfo= box;
}

configuru::Config BoxStencilConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["stencil_id"] = m_boxInfo.stencil_id;
	pt["parent_anchor_id"] = m_boxInfo.parent_anchor_id;
	pt["box_x_size"] = m_boxInfo.box_x_size;
	pt["box_y_size"] = m_boxInfo.box_y_size;
	pt["box_z_size"] = m_boxInfo.box_z_size;
	pt["is_disabled"] = m_boxInfo.is_disabled;
	pt["stencil_name"] = m_boxInfo.stencil_name;

	writeVector3f(pt, "box_center", m_boxInfo.box_center);
	writeVector3f(pt, "box_x_axis", m_boxInfo.box_x_axis);
	writeVector3f(pt, "box_y_axis", m_boxInfo.box_y_axis);
	writeVector3f(pt, "box_z_axis", m_boxInfo.box_z_axis);

	return pt;
}

void BoxStencilConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	if (pt.has_key("stencil_id"))
	{
		memset(&m_boxInfo, 0, sizeof(m_boxInfo));

		m_boxInfo.stencil_id = pt.get<int>("stencil_id");
		m_boxInfo.parent_anchor_id = pt.get_or<int>("parent_anchor_id", -1);
		readVector3f(pt, "box_center", m_boxInfo.box_center);
		readVector3f(pt, "box_x_axis", m_boxInfo.box_x_axis);
		readVector3f(pt, "box_y_axis", m_boxInfo.box_y_axis);
		readVector3f(pt, "box_z_axis", m_boxInfo.box_z_axis);
		m_boxInfo.box_x_size = pt.get_or<float>("box_x_size", 0.25f);
		m_boxInfo.box_y_size = pt.get_or<float>("box_y_size", 0.25f);
		m_boxInfo.box_z_size = pt.get_or<float>("box_z_size", 0.25f);
		m_boxInfo.is_disabled = pt.get_or<bool>("is_disabled", false);

		const std::string stencil_name = pt.get_or<std::string>("stencil_name", "");
		StringUtils::formatString(m_boxInfo.stencil_name, sizeof(m_boxInfo.stencil_name), "%s", stencil_name.c_str());
	}
}

void BoxStencilConfig::setParentAnchorId(MikanSpatialAnchorID anchorId)
{
	m_boxInfo.parent_anchor_id = anchorId;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxParentAnchorPropertyId));
}

const glm::mat4 BoxStencilConfig::getBoxMat4() const
{
	return glm::mat4(
		glm::vec4(MikanVector3f_to_glm_vec3(m_boxInfo.box_x_axis), 0.f),
		glm::vec4(MikanVector3f_to_glm_vec3(m_boxInfo.box_y_axis), 0.f),
		glm::vec4(MikanVector3f_to_glm_vec3(m_boxInfo.box_z_axis), 0.f),
		glm::vec4(MikanVector3f_to_glm_vec3(m_boxInfo.box_center), 1.f));
}

void BoxStencilConfig::setBoxMat4(const glm::mat4& xform)
{
	m_boxInfo.box_x_axis = glm_vec3_to_MikanVector3f(xform[0]);
	m_boxInfo.box_y_axis = glm_vec3_to_MikanVector3f(xform[1]);
	m_boxInfo.box_z_axis = glm_vec3_to_MikanVector3f(xform[2]);
	m_boxInfo.box_center = glm_vec3_to_MikanVector3f(xform[3]);
	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_boxStencilXAxisPropertyId)
				.addPropertyName(k_boxStencilYAxisPropertyId)
				.addPropertyName(k_boxStencilZAxisPropertyId)
				.addPropertyName(k_boxStencilCenterPropertyId));
}

const GlmTransform BoxStencilConfig::getBoxTransform() const
{
	return GlmTransform(
		MikanVector3f_to_glm_vec3(m_boxInfo.box_center),
		glm::quat_cast(glm::mat3(
			MikanVector3f_to_glm_vec3(m_boxInfo.box_x_axis),
			MikanVector3f_to_glm_vec3(m_boxInfo.box_y_axis),
			MikanVector3f_to_glm_vec3(m_boxInfo.box_z_axis))));
}

void BoxStencilConfig::setBoxTransform(const GlmTransform& transform)
{
	glm::mat4 xform = transform.getMat4();

	m_boxInfo.box_x_axis = glm_vec3_to_MikanVector3f(xform[0]);
	m_boxInfo.box_y_axis = glm_vec3_to_MikanVector3f(xform[1]);
	m_boxInfo.box_z_axis = glm_vec3_to_MikanVector3f(xform[2]);
	m_boxInfo.box_center = glm_vec3_to_MikanVector3f(xform[3]);
	markDirty(ConfigPropertyChangeSet()
			  .addPropertyName(k_boxStencilXAxisPropertyId)
			  .addPropertyName(k_boxStencilYAxisPropertyId)
			  .addPropertyName(k_boxStencilZAxisPropertyId)
			  .addPropertyName(k_boxStencilCenterPropertyId));
}

void BoxStencilConfig::setBoxXAxis(const MikanVector3f& xAxis)
{
	m_boxInfo.box_x_axis = xAxis;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilXAxisPropertyId));
}

void BoxStencilConfig::setBoxYAxis(const MikanVector3f& yAxis)
{
	m_boxInfo.box_y_axis = yAxis;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilYAxisPropertyId));
}

void BoxStencilConfig::setBoxZAxis(const MikanVector3f& zAxis)
{
	m_boxInfo.box_z_axis = zAxis;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilZAxisPropertyId));
}

void BoxStencilConfig::setBoxOrientation(const glm::mat3& R)
{
	m_boxInfo.box_x_axis = glm_vec3_to_MikanVector3f(R[0]);
	m_boxInfo.box_y_axis = glm_vec3_to_MikanVector3f(R[1]);
	m_boxInfo.box_z_axis = glm_vec3_to_MikanVector3f(R[2]);
	markDirty(ConfigPropertyChangeSet()
		.addPropertyName(k_boxStencilXAxisPropertyId)
		.addPropertyName(k_boxStencilYAxisPropertyId)
		.addPropertyName(k_boxStencilZAxisPropertyId));
}

void BoxStencilConfig::setBoxCenter(const MikanVector3f& center)
{
	m_boxInfo.box_center = center;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilCenterPropertyId));
}

void BoxStencilConfig::setBoxXSize(float size)
{
	m_boxInfo.box_x_size = size;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilXSizePropertyId));
}

void BoxStencilConfig::setBoxYSize(float size)
{
	m_boxInfo.box_y_size = size;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilYSizePropertyId));
}

void BoxStencilConfig::setBoxZSize(float size)
{
	m_boxInfo.box_z_size = size;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilZSizePropertyId));
}

void BoxStencilConfig::setBoxSize(float xSize, float ySize, float zSize)
{
	m_boxInfo.box_x_size = xSize;
	m_boxInfo.box_y_size = ySize;
	m_boxInfo.box_z_size = zSize;
	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_boxStencilXSizePropertyId)
				.addPropertyName(k_boxStencilYSizePropertyId)
				.addPropertyName(k_boxStencilZSizePropertyId));
}

void BoxStencilConfig::setIsDisabled(bool flag)
{
	m_boxInfo.is_disabled = flag;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilDisabledPropertyId));
}

void BoxStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_boxInfo.stencil_name, stencilName.c_str(), sizeof(m_boxInfo.stencil_name) - 1);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilNamePropertyId));
}

// -- BoxStencilComponent -----
BoxStencilComponent::BoxStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void BoxStencilComponent::init()
{
	StencilComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();
}

void BoxStencilComponent::customRender()
{
	if (!m_config->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const float xSize= m_config->getBoxXSize();
		const float ySize= m_config->getBoxYSize();
		const float zSize= m_config->getBoxZSize();
		const glm::mat4 xform = getWorldTransform();
		const glm::vec3 half_extents(xSize / 2.f, ySize / 2.f, zSize / 2.f);
		const glm::vec3 position = glm::vec3(xform[3]);

		glm::vec3 color= Colors::DarkGray;
		SelectionComponentPtr selectionComponent= m_selectionComponent.lock();
		if (selectionComponent)
		{
			if (selectionComponent->getIsSelected())
				color= Colors::Yellow;
			else if (selectionComponent->getIsHovered())
				color= Colors::LightGray;
		}
		
		drawTransformedBox(xform, half_extents, color);
		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", m_config->getStencilId());
	}
}

void BoxStencilComponent::setConfig(BoxStencilConfigPtr config)
{
	assert(!m_bIsInitialized);
	m_config = config;

	// Initially the model component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);
	m_worldTransform = config->getBoxMat4();
	m_relativeTransform = GlmTransform(m_worldTransform);

	// Make the component name match the config name
	m_name = config->getStencilName();

	// Setup initial attachment
	MikanSpatialAnchorID currentParentId = m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
	attachSceneComponentToAnchor(currentParentId);
}

void BoxStencilComponent::onParentAnchorChanged(MikanSpatialAnchorID newParentId)
{
	if (m_config)
	{
		m_config->setParentAnchorId(newParentId);
	}
}

void BoxStencilComponent::setRelativePosition(const glm::vec3& position)
{
	m_relativeTransform.setPosition(position);
	StencilComponent::setRelativeTransform(m_relativeTransform);

	if (m_bIsInitialized)
		m_config->setBoxCenter({position.x, position.y, position.z});
}

void BoxStencilComponent::setRelativeOrientation(const glm::mat3& rotation)
{
	m_relativeTransform.setOrientation(glm::quat_cast(rotation));
	StencilComponent::setRelativeTransform(m_relativeTransform);

	if (m_bIsInitialized)
		m_config->setBoxOrientation(rotation);
}

void BoxStencilComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	StencilComponent::setRelativeTransform(newRelativeXform);

	if (m_bIsInitialized)
		m_config->setBoxTransform(newRelativeXform);
}

void BoxStencilComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	StencilComponent::setWorldTransform(newWorldXform);

	if (m_bIsInitialized)
	{
		m_config->setBoxMat4(getRelativeTransform().getMat4());
	}
}

void BoxStencilComponent::setName(const std::string& name)
{
	StencilComponent::setName(name);

	if (m_bIsInitialized)
		m_config->setStencilName(name);
}

MikanStencilID BoxStencilComponent::getParentAnchorId() const
{
	return m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
}

void BoxStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider= m_boxCollider.lock();
	if (boxCollider && m_config)
	{
		const float xSize = m_config->getBoxXSize();
		const float ySize = m_config->getBoxYSize();
		const float zSize = m_config->getBoxZSize();

		boxCollider->setHalfExtents(glm::vec3(xSize, ySize, zSize) * 0.5f);
	}
}