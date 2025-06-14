#include "App.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "ClientSourceManager.h"
#include "CompositorComponent.h"
#include "IMkState.h"
#include "IMkTriangulatedMesh.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MkMaterialInstance.h"
#include "MkScopedState.h"
#include "MkStateStack.h"
#include "ProjectConfig.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "TransformComponent.h"
#include "StringUtils.h"

#include "NodeGraphAssetReference.h"
#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <easy/profiler.h>

// -- CompositorConfig -----
const std::string CompositorDefinition::k_compositorGraphPathPropertyId = "script_path";
const std::string CompositorDefinition::k_cameraPropertyId= "camera_id";
const std::string CompositorDefinition::k_ownerScenePropertyId = "owner_scene_id";

CompositorDefinition::CompositorDefinition()
	: MikanComponentDefinition()
	, m_compositorId(INVALID_MIKAN_ID)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{
}

CompositorDefinition::CompositorDefinition(
	MikanCompositorID compositorId,
	MikanSceneID ownerSceneId,
	const std::string& compositorName)
	: MikanComponentDefinition(compositorName)
	, m_compositorId(compositorId)
	, m_ownerSceneId(ownerSceneId)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{}

configuru::Config CompositorDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_compositorId;
	pt[k_cameraPropertyId] = m_cameraId;
	pt[k_ownerScenePropertyId] = m_ownerSceneId;

	if (m_nodeGraphAssetRef)
	{
		pt[k_compositorGraphPathPropertyId] = m_nodeGraphAssetRef->writeToJSON();
	}

	return pt;
}

void CompositorDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_compositorId = pt.get<int>("id");
	m_cameraId = pt.get_or<int>(k_cameraPropertyId, INVALID_MIKAN_ID);
	m_ownerSceneId = pt.get_or<int>(k_ownerScenePropertyId, INVALID_MIKAN_ID);

	m_componentScriptAssetRefConfig = NodeGraphAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_compositorGraphPathPropertyId))
	{
		m_componentScriptAssetRefConfig->readFromJSON(pt[k_compositorGraphPathPropertyId]);
	}
}

void CompositorDefinition::setCameraId(MikanCameraID cameraId)
{
	if (m_cameraId != cameraId)
	{
		m_cameraId = cameraId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_cameraPropertyId));
	}
}

void CompositorDefinition::setOwnerSceneId(MikanSceneID sceneId)
{
	if (m_ownerSceneId != sceneId)
	{
		m_ownerSceneId = sceneId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_ownerScenePropertyId));
	}
}

bool CompositorDefinition::hasCompositorGraphPath() const
{
	return !m_nodeGraphAssetRef->assetPath.empty();
}

std::filesystem::path CompositorDefinition::getCompositorGraphPath() const
{
	return m_nodeGraphAssetRef->assetPath;
}

void CompositorDefinition::setCompositorGraphPath(const std::filesystem::path& graphPath)
{
	if (graphPath != m_nodeGraphAssetRef->assetPath)
	{
		m_nodeGraphAssetRef->assetPath= graphPath.string();
		markDirty(ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentScriptPathPropertyId));
	}
}

// -- CompositorComponent -----
CompositorComponent::CompositorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
	m_bWantsUpdate = true;
}

void CompositorComponent::init()
{
	MikanComponent::init();

	m_layerQuadMesh = createFullscreenQuadMesh(getOwnerWindow(), false);
}

void CompositorComponent::dispose()
{
	m_layerQuadMesh= nullptr;

	MikanComponent::dispose();
}

void CompositorComponent::update(float deltaSeconds)
{
	EASY_FUNCTION();

	if (!getIsRunning())
		return;

	CameraComponentPtr cameraComponent= getCameraComponent();
	if (!cameraComponent)
		return;

	const glm::mat4 cameraXform= cameraComponent->getWorldTransform();

	// Keep track of how long it's been since the last frame has been composited
	// This is used to update the timer in compositorNodeGraph
	m_timeSinceLastFrameComposited += deltaSeconds;

	// Composite the next frame if we got all the renders back from the clients
	if (m_pendingCompositeFrameIndex != 0 && m_nodeGraph)
	{
		auto* clientSourceManager = ClientSourceManager::getInstance();

		std::set<std::string> clientSourceIds;
		m_nodeGraph->gatherAllReferencedClientSourceIDs(clientSourceIds);
		//TODO
	}
}

void CompositorComponent::render() const
{
	IMkTextureConstPtr compositedFrameTexture = getCompositedFrameTexture();
	if (compositedFrameTexture)
	{
		MkMaterialInstancePtr materialInstance = m_layerQuadMesh->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			// Bind the color texture
			materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, compositedFrameTexture);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				MkScopedState scopedState = 
					getOwnerWindow()->getMkStateStack().createScopedState("GlFrameCompositorRender");
				scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

				m_layerQuadMesh->drawElements();
			}
		}
	}

}

bool CompositorComponent::getIsRunning() const
{
	SceneComponentPtr ownerScene= getOwnerSceneComponent();
	if (ownerScene)
	{
		MikanCompositorID selfCompositorId = getCompositorDefinition()->getCompositorId();
		MikanCompositorID outputCompositorId = ownerScene->getSceneComponentDefinition()->getOutputCompositorId();

		return (outputCompositorId == selfCompositorId);
	}

	return false;
}

SceneComponentPtr CompositorComponent::getOwnerSceneComponent() const
{
	MikanSceneID ownerSceneId= getCompositorDefinition()->getOwnerSceneId();

	return SceneObjectSystem::getSystem()->getSceneById(ownerSceneId);
}

CameraComponentPtr CompositorComponent::getCameraComponent() const
{
	MikanCameraID cameraId = getCompositorDefinition()->getCameraId();

	return CameraObjectSystem::getSystem()->getCameraById(cameraId);
}

IMkTextureConstPtr CompositorComponent::getCompositedFrameTexture() const
{
	return m_nodeGraph ? m_nodeGraph->getCompositedFrameTexture() : IMkTextureConstPtr();
}

// -- IPropertyInterface ----
void CompositorComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(CompositorDefinition::k_compositorGraphPathPropertyId);
}

bool CompositorComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		outDescriptor = {CompositorDefinition::k_compositorGraphPathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
		return true;
	}

	return false;
}

bool CompositorComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		Rml::String filepath = getCompositorDefinition()->getCompositorGraphPath().string();

		outValue = filepath;
		return true;
	}

	return false;
}

bool CompositorComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyAttribute(propertyName, attributeName, outValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		if (attributeName == *k_PropertyAttributeFileBrowseTitle)
		{
			outValue = "Select a graph";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilter)
		{
			outValue = ".graph";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilterDesc)
		{
			outValue = "Graph Files (.graph)";
		}
	}

	return false;
}

bool CompositorComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		const Rml::String fileString = inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		getCompositorDefinition()->setCompositorGraphPath(filePath);
		return true;
	}

	return false;
}