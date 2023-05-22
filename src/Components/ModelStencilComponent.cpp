#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlMaterialInstance.h"
#include "GlTextRenderer.h"
#include "GlStaticMeshInstance.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "StaticMeshComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"
#include "MathGLM.h"
#include "MathMikan.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "ModelStencilComponent.h"
#include "StringUtils.h"

#include <glm/gtx/matrix_decompose.hpp>

// -- ModelStencilConfig -----
const std::string ModelStencilConfig::k_modelStencilScalePropertyId = "model_scale";
const std::string ModelStencilConfig::k_modelStencilRotatorPropertyId = "model_rotator";
const std::string ModelStencilConfig::k_modelStencilPositionPropertyId = "model_position";
const std::string ModelStencilConfig::k_modelStencilObjPathPropertyId = "model_path";
const std::string ModelStencilConfig::k_modelStencilDisabledPropertyId = "is_disabled";
const std::string ModelStencilConfig::k_modelStencilNamePropertyId = "stencil_name";

ModelStencilConfig::ModelStencilConfig()
{
	memset(&m_modelInfo, 0, sizeof(MikanStencilModel));
	m_modelInfo.stencil_id = INVALID_MIKAN_ID;
	m_modelInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

ModelStencilConfig::ModelStencilConfig(const MikanStencilModel& modelInfo)
{
	m_modelInfo= modelInfo;
}

configuru::Config ModelStencilConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["model_path"] = m_modelPath.string();
	pt["stencil_id"] = m_modelInfo.stencil_id;
	pt["parent_anchor_id"] = m_modelInfo.parent_anchor_id;
	pt["is_disabled"] = m_modelInfo.is_disabled;
	pt["stencil_name"] = m_modelInfo.stencil_name;

	writeVector3f(pt, "model_position", m_modelInfo.model_position);
	writeRotator3f(pt, "model_rotator", m_modelInfo.model_rotator);
	writeVector3f(pt, "model_scale", m_modelInfo.model_scale);

	return pt;
}

void ModelStencilConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	if (pt.has_key("stencil_id"))
	{
		memset(&m_modelInfo, 0, sizeof(MikanStencilModel));

		m_modelPath = pt.get<std::string>("model_path");
		m_modelInfo.stencil_id = pt.get<int>("stencil_id");
		m_modelInfo.parent_anchor_id = pt.get_or<int>("parent_anchor_id", -1);
		readVector3f(pt, "model_position", m_modelInfo.model_position);
		readRotator3f(pt, "model_rotator", m_modelInfo.model_rotator);
		readVector3f(pt, "model_scale", m_modelInfo.model_scale);
		m_modelInfo.is_disabled = pt.get_or<bool>("is_disabled", false);

		const std::string stencil_name = pt.get_or<std::string>("stencil_name", "");
		StringUtils::formatString(m_modelInfo.stencil_name, sizeof(m_modelInfo.stencil_name), "%s", stencil_name.c_str());
	}
}

const glm::mat4 ModelStencilConfig::getModelMat4() const
{
	return getModelTransform().getMat4();
}

void ModelStencilConfig::setModelMat4(const glm::mat4& mat4)
{
	setModelTransform(GlmTransform(mat4));
}

const GlmTransform ModelStencilConfig::getModelTransform() const
{
	return GlmTransform(
		MikanVector3f_to_glm_vec3(m_modelInfo.model_position),
		MikanRotator3f_to_glm_quat(m_modelInfo.model_rotator),
		MikanVector3f_to_glm_vec3(m_modelInfo.model_scale));
}

void ModelStencilConfig::setModelTransform(const GlmTransform& transform)
{
	glm::mat4 xform = transform.getMat4();

	m_modelInfo.model_position= glm_vec3_to_MikanVector3f(transform.getPosition());
	m_modelInfo.model_rotator= glm_quat_to_MikanRotator3f(transform.getOrientation());
	m_modelInfo.model_scale= glm_vec3_to_MikanVector3f(transform.getScale());

	markDirty(ConfigPropertyChangeSet()
				.addPropertyName(k_modelStencilScalePropertyId)
				.addPropertyName(k_modelStencilRotatorPropertyId)
				.addPropertyName(k_modelStencilPositionPropertyId));
}

void ModelStencilConfig::setModelScale(const MikanVector3f& scale)
{
	m_modelInfo.model_scale= scale;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilScalePropertyId));
}

void ModelStencilConfig::setModelRotator(const MikanRotator3f& rotator)
{
	m_modelInfo.model_rotator= rotator;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilRotatorPropertyId));
}

void ModelStencilConfig::setModelPosition(const MikanVector3f& position)
{
	m_modelInfo.model_position= position;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilPositionPropertyId));
}

void ModelStencilConfig::setModelPath(const std::filesystem::path& path)
{
	m_modelPath= path;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilPositionPropertyId));
}

void ModelStencilConfig::setIsDisabled(bool flag)
{
	m_modelInfo.is_disabled = flag;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilDisabledPropertyId));
}

void ModelStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_modelInfo.stencil_name, stencilName.c_str(), sizeof(m_modelInfo.stencil_name) - 1);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilNamePropertyId));
}

// -- ModelStencilComponent -----
ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void ModelStencilComponent::init()
{
	StencilComponent::init();

	// Get a list of all the attached wireframe meshes
	std::vector<StaticMeshComponentPtr> meshComponents;
	getOwnerObject()->getComponentsOfType<StaticMeshComponent>(meshComponents);
	for (auto& meshComponentPtr : meshComponents)
	{
		GlStaticMeshInstancePtr staticMeshPtr= meshComponentPtr->getStaticMesh();
		if (staticMeshPtr && staticMeshPtr->getName() == "wireframe")
		{
			staticMeshPtr->setVisible(false);
			m_wireframeMeshes.push_back(staticMeshPtr);
		}
	}

	SelectionComponentPtr selectionComponentPtr= getOwnerObject()->getComponentOfType<SelectionComponent>();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->OnInteractionRayOverlapEnter+= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit+= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected+= MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected+= MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);
		m_selectionComponentWeakPtr= selectionComponentPtr;
	}

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

void ModelStencilComponent::customRender()
{
	if (!m_config->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", m_config->getStencilId());
	}
}

void ModelStencilComponent::dispose()
{
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected -= MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected -= MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);
		m_selectionComponentWeakPtr = selectionComponentPtr;
	}

	StencilComponent::dispose();
}

void ModelStencilComponent::setConfig(ModelStencilConfigPtr config)
{
	assert(!m_bIsInitialized);
	m_config = config;

	// Initially the model component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);
	m_worldTransform = config->getModelMat4();
	m_relativeTransform = GlmTransform(m_worldTransform);

	// Make the component name match the config name
	m_name= config->getStencilName();

	// Setup initial attachment
	MikanSpatialAnchorID currentParentId = m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
	MikanSpatialAnchorID newParentId = config ? config->getParentAnchorId() : INVALID_MIKAN_ID;
	if (currentParentId != newParentId)
	{
		attachSceneComponentToAnchor(newParentId);
	}
}

void ModelStencilComponent::setRelativePosition(const glm::vec3& position)
{
	m_relativeTransform.setPosition(position);
	StencilComponent::setRelativeTransform(m_relativeTransform);

	m_config->setModelPosition({position.x, position.y, position.z});
}

void ModelStencilComponent::setRelativeOrientation(const glm::vec3& eulerAnglesDegrees)
{
	glm::mat3 rotation;
	glm_euler_angles_to_mat3(
		eulerAnglesDegrees.x * k_degrees_to_radians,
		eulerAnglesDegrees.y * k_degrees_to_radians,
		eulerAnglesDegrees.z * k_degrees_to_radians,
		rotation);

	m_relativeTransform.setOrientation(glm::quat_cast(rotation));
	StencilComponent::setRelativeTransform(m_relativeTransform);

	m_config->setModelRotator({eulerAnglesDegrees.x, eulerAnglesDegrees.y, eulerAnglesDegrees.z});
}

void ModelStencilComponent::setRelativeScale(const glm::vec3& scale)
{
	m_relativeTransform.setScale(scale);
	StencilComponent::setRelativeTransform(m_relativeTransform);

	m_config->setModelPosition({scale.x, scale.y, scale.z});
}

void ModelStencilComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	StencilComponent::setRelativeTransform(newRelativeXform);

	m_config->setModelTransform(newRelativeXform);
}

void ModelStencilComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	StencilComponent::setWorldTransform(newWorldXform);

	m_config->setModelTransform(getRelativeTransform().getMat4());
}

MikanStencilID ModelStencilComponent::getParentAnchorId() const
{
	return m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
}

void ModelStencilComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr && !selectionComponentPtr->getIsSelected())
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA, 
				glm::vec4(Colors::LightGray, 1.f));
			meshPtr->setVisible(true);
		}
	}
}

void ModelStencilComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr && !selectionComponentPtr->getIsSelected())
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->setVisible(false);
		}
	}
}

void ModelStencilComponent::onInteractionSelected()
{
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA, 
				glm::vec4(Colors::Yellow, 1.f));
			meshPtr->setVisible(true);
		}
	}
}

void ModelStencilComponent::onInteractionUnselected()
{
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		if (selectionComponentPtr->getIsHovered())
		{
			for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
			{
				meshPtr->getMaterialInstance()->setVec4BySemantic(
					eUniformSemantic::diffuseColorRGBA, 
					glm::vec4(Colors::LightGray, 1.f));
				meshPtr->setVisible(true);
			}
		}
		else
		{
			for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
			{
				meshPtr->setVisible(false);
			}
		}
	}
}