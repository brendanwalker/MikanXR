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
#include "StringUtils.h"
#include "TextStyle.h"

// -- BoxStencilComponent -----
BoxStencilConfig::BoxStencilConfig()
{
	memset(&m_boxInfo, 0, sizeof(MikanStencilBox));
	m_boxInfo.stencil_id = INVALID_MIKAN_ID;
	m_boxInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

BoxStencilConfig::BoxStencilConfig(MikanStencilID stencilId)
{
	memset(&m_boxInfo, 0, sizeof(MikanStencilBox));
	m_boxInfo.stencil_id = stencilId;
	m_boxInfo.parent_anchor_id = INVALID_MIKAN_ID;
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
		MikanStencilBox stencil;
		memset(&stencil, 0, sizeof(stencil));

		stencil.stencil_id = pt.get<int>("stencil_id");
		stencil.parent_anchor_id = pt.get_or<int>("parent_anchor_id", -1);
		readVector3f(pt, "box_center", stencil.box_center);
		readVector3f(pt, "box_x_axis", stencil.box_x_axis);
		readVector3f(pt, "box_y_axis", stencil.box_y_axis);
		readVector3f(pt, "box_z_axis", stencil.box_z_axis);
		stencil.box_x_size = pt.get_or<float>("box_x_size", 0.25f);
		stencil.box_y_size = pt.get_or<float>("box_y_size", 0.25f);
		stencil.box_z_size = pt.get_or<float>("box_z_size", 0.25f);
		stencil.is_disabled = pt.get_or<bool>("is_disabled", false);

		const std::string stencil_name = pt.get_or<std::string>("stencil_name", "");
		StringUtils::formatString(stencil.stencil_name, sizeof(stencil.stencil_name), "%s", stencil_name.c_str());
	}
}

void BoxStencilConfig::setBoxInfo(const MikanStencilBox& box)
{
	m_boxInfo= box;
	markDirty();
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
	markDirty();
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
	markDirty();
}

void BoxStencilConfig::setBoxXSize(float size)
{
	m_boxInfo.box_x_size = size;
	markDirty();
}

void BoxStencilConfig::setBoxYSize(float size)
{
	m_boxInfo.box_y_size = size;
	markDirty();
}

void BoxStencilConfig::setBoxZSize(float size)
{
	m_boxInfo.box_z_size = size;
	markDirty();
}

void BoxStencilConfig::setIsDisabled(bool flag)
{
	m_boxInfo.is_disabled = flag;
	markDirty();
}

void BoxStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_boxInfo.stencil_name, stencilName.c_str(), sizeof(m_boxInfo.stencil_name) - 1);
	markDirty();
}

// -- BoxStencilComponent -----
BoxStencilComponent::BoxStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{}

void BoxStencilComponent::init()
{
	MikanComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();
}

void BoxStencilComponent::update()
{
	MikanComponent::update();

	if (!m_config->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const float xSize= m_config->getBoxXSize();
		const float ySize= m_config->getBoxYSize();
		const float zSize= m_config->getBoxZSize();
		const glm::mat4 xform = m_sceneComponent.lock()->getWorldTransform();
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
	MikanSpatialAnchorID currentParentId = m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
	MikanSpatialAnchorID newParentId = config ? config->getParentAnchorId() : INVALID_MIKAN_ID;
	if (currentParentId != newParentId)
	{
		attachSceneComponentToAnchor(newParentId);
	}

	m_config= config;

	updateSceneComponentTransform();
	updateBoxColliderExtents();
}

void BoxStencilComponent::onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr)
{
	m_config->setBoxTransform(sceneComponentPtr->getRelativeTransform());
}

void BoxStencilComponent::updateSceneComponentTransform()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}

void BoxStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider= m_boxCollider.lock();
	if (boxCollider)
	{
		const float xSize = m_config->getBoxXSize();
		const float ySize = m_config->getBoxYSize();
		const float zSize = m_config->getBoxZSize();

		boxCollider->setHalfExtents(glm::vec3(xSize, ySize, zSize) * 0.5f);
	}
}