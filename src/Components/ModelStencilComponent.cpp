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
#include "MathGLM.h"
#include "MathMikan.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "ModelStencilComponent.h"
#include "StringUtils.h"

#include <glm/gtx/matrix_decompose.hpp>

// -- ModelStencilConfig -----
ModelStencilConfig::ModelStencilConfig()
{
	memset(&m_modelInfo, 0, sizeof(MikanStencilModel));
	m_modelInfo.stencil_id = INVALID_MIKAN_ID;
	m_modelInfo.parent_anchor_id = INVALID_MIKAN_ID;
}

ModelStencilConfig::ModelStencilConfig(MikanStencilID stencilId)
{
	memset(&m_modelInfo, 0, sizeof(MikanStencilModel));
	m_modelInfo.stencil_id = stencilId;
	m_modelInfo.parent_anchor_id = INVALID_MIKAN_ID;
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

void ModelStencilConfig::setModelInfo(const MikanStencilModel& modelInfo)
{
	m_modelInfo= modelInfo;
	markDirty();
}

const glm::mat4 ModelStencilConfig::getModelXform() const
{
	MikanVector3f stencilXAxis, stencilYAxis, stencilZAxis;
	EulerAnglesToMikanOrientation(
		m_modelInfo.model_rotator.x_angle, m_modelInfo.model_rotator.y_angle, m_modelInfo.model_rotator.z_angle,
		stencilXAxis, stencilYAxis, stencilZAxis);

	const glm::vec3 xAxis = MikanVector3f_to_glm_vec3(stencilXAxis) * m_modelInfo.model_scale.x;
	const glm::vec3 yAxis = MikanVector3f_to_glm_vec3(stencilYAxis) * m_modelInfo.model_scale.y;
	const glm::vec3 zAxis = MikanVector3f_to_glm_vec3(stencilZAxis) * m_modelInfo.model_scale.z;
	const glm::vec3 position = MikanVector3f_to_glm_vec3(m_modelInfo.model_position);
	const glm::mat4 localXform =
		glm::mat4(
			glm::vec4(xAxis, 0.f),
			glm::vec4(yAxis, 0.f),
			glm::vec4(zAxis, 0.f),
			glm::vec4(position, 1.f));

	return localXform;
}

void ModelStencilConfig::setModelXform(const glm::mat4& xform)
{
	// Extract position scale and rotation from the local transform
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	if (glm::decompose(
		xform,
		scale, orientation, translation, skew, perspective))
	{
		m_modelInfo.model_position = glm_vec3_to_MikanVector3f(translation);
		m_modelInfo.model_scale = glm_vec3_to_MikanVector3f(scale);
		glm_quat_to_euler_angles(
			orientation,
			m_modelInfo.model_rotator.x_angle,
			m_modelInfo.model_rotator.y_angle,
			m_modelInfo.model_rotator.z_angle);
		markDirty();
	}
}

void ModelStencilConfig::setModelPath(const std::filesystem::path& path)
{
	m_modelPath= path;
	markDirty();
}

void ModelStencilConfig::setIsDisabled(bool flag)
{
	m_modelInfo.is_disabled = flag;
	markDirty();
}

void ModelStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_modelInfo.stencil_name, stencilName.c_str(), sizeof(m_modelInfo.stencil_name) - 1);
	markDirty();
}

// -- ModelStencilComponent -----
ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{}

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
}

void ModelStencilComponent::update()
{
	StencilComponent::update();

	if (!IsDisabled)
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = m_sceneComponent.lock()->getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", StencilId);
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
	m_config = config;
	updateSceneComponentTransform();
}

glm::mat4 ModelStencilComponent::getStencilLocalTransform() const
{
	return m_config->getModelXform();
}

glm::mat4 ModelStencilComponent::getStencilWorldTransform() const
{
	const glm::mat4 localXform= getStencilLocalTransform();

	glm::mat4 worldXform = localXform;
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		worldXform = anchorPtr->getAnchorXform() * localXform;
	}

	return worldXform;
}

void ModelStencilComponent::setStencilLocalTransformProperty(const glm::mat4& localXform)
{
	m_config->setModelXform(localXform);
}

void ModelStencilComponent::setStencilWorldTransformProperty(const glm::mat4& worldXform)
{
	glm::mat4 localXform = worldXform;
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		const glm::mat4 invParentXform = glm::inverse(anchorPtr->getAnchorXform());

		// Convert transform to the space of the parent anchor
		localXform = glm_composite_xform(worldXform, invParentXform);
	}

	setStencilLocalTransformProperty(localXform);
}

void ModelStencilComponent::updateSceneComponentTransform()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}

void ModelStencilComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr && !selectionComponentPtr->getIsSelected())
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec3BySemantic(eUniformSemantic::diffuseColorRGB, Colors::LightGray);
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
			meshPtr->getMaterialInstance()->setVec3BySemantic(eUniformSemantic::diffuseColorRGB, Colors::Yellow);
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
				meshPtr->getMaterialInstance()->setVec3BySemantic(eUniformSemantic::diffuseColorRGB, Colors::LightGray);
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