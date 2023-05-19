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

void ModelStencilConfig::notifyStencilChanged()
{
	StencilObjectSystemConfigPtr stencilConfig = StencilObjectSystem::getSystem()->getStencilSystemConfig();
	if (stencilConfig->OnModelStencilModified)
		stencilConfig->OnModelStencilModified(m_modelInfo.stencil_id);
}

void ModelStencilConfig::setModelInfo(const MikanStencilModel& modelInfo)
{
	m_modelInfo= modelInfo;
	markDirty();
	notifyStencilChanged();
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

	markDirty();
	notifyStencilChanged();
}

void ModelStencilConfig::setModelScale(const MikanVector3f& scale)
{
	m_modelInfo.model_scale= scale;
	markDirty();
	notifyStencilChanged();
}

void ModelStencilConfig::setModelRotator(const MikanRotator3f& rotator)
{
	m_modelInfo.model_rotator= rotator;
	markDirty();
	notifyStencilChanged();
}

void ModelStencilConfig::setModelPosition(const MikanVector3f& position)
{
	m_modelInfo.model_position= position;
	markDirty();
	notifyStencilChanged();
}

void ModelStencilConfig::setModelPath(const std::filesystem::path& path)
{
	m_modelPath= path;
	markDirty();
	notifyStencilChanged();
}

void ModelStencilConfig::setIsDisabled(bool flag)
{
	m_modelInfo.is_disabled = flag;
	markDirty();
	notifyStencilChanged();
}

void ModelStencilConfig::setStencilName(const std::string& stencilName)
{
	strncpy(m_modelInfo.stencil_name, stencilName.c_str(), sizeof(m_modelInfo.stencil_name) - 1);
	markDirty();
	notifyStencilChanged();
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
}

void ModelStencilComponent::customRender()
{
	if (!m_config->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = m_sceneComponent.lock()->getWorldTransform();
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
	MikanSpatialAnchorID currentParentId = m_config ? m_config->getParentAnchorId() : INVALID_MIKAN_ID;
	MikanSpatialAnchorID newParentId = config ? config->getParentAnchorId() : INVALID_MIKAN_ID;
	if (currentParentId != newParentId)
	{
		attachSceneComponentToAnchor(newParentId);
	}

	m_config = config;

	applyConfigTransformToSceneComponent();
}

void ModelStencilComponent::setConfigTransform(const GlmTransform& transform)
{
	if (m_config)
		m_config->setModelTransform(transform);
}

const GlmTransform ModelStencilComponent::getConfigTransform()
{
	return m_config ? m_config->getModelTransform() : GlmTransform();
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