#include "CompositorObjectSystem.h"
#include "CompositorComponent.h"
#include "IMkSceneRenderable.h"
#include "MikanObject.h"
#include "MkScene.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "TransformComponent.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <easy/profiler.h>

#include <queue>

// -- SceneComponentDefinition -----
const std::string SceneComponentDefinition::k_parentStagePropertyId = "parent_stage_id";
const std::string SceneComponentDefinition::k_compositorListPropertyId = "compositor_list";

SceneComponentDefinition::SceneComponentDefinition()
	: m_sceneId(INVALID_MIKAN_ID)
	, m_parentStageId(INVALID_MIKAN_ID)
{}

SceneComponentDefinition::SceneComponentDefinition(
	MikanSceneID sceneId,
	MikanStageID parentStageId,
	const std::string& componentName)
	: TransformComponentDefinition(componentName, glm_transform_to_MikanTransform(GlmTransform()))
	, m_sceneId(sceneId)
	, m_parentStageId(parentStageId)
{}

configuru::Config SceneComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["scene_id"] = m_sceneId;
	pt[k_parentStagePropertyId] = m_parentStageId;
	readStdValueVector(pt, "compositors", m_compositorIDs);

	return pt;
}

void SceneComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_sceneId = pt.get<int>("scene_id");
	m_parentStageId = pt.get_or<int>(k_parentStagePropertyId, INVALID_MIKAN_ID);
	readStdValueVector(pt, "compositors", m_compositorIDs);
}

void SceneComponentDefinition::setParentStageId(MikanStageID stageId)
{
	if (m_parentStageId != stageId)
	{
		m_parentStageId = stageId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_parentStagePropertyId));
	}
}

void SceneComponentDefinition::addCompositorID(MikanCompositorID compositorId)
{
	auto it = std::find(m_compositorIDs.begin(), m_compositorIDs.end(), compositorId);
	if (it == m_compositorIDs.end())
	{
		m_compositorIDs.push_back(compositorId);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_compositorListPropertyId));
	}
}

void SceneComponentDefinition::removeCompositorID(MikanCompositorID compositorId)
{
	auto it = std::find(m_compositorIDs.begin(), m_compositorIDs.end(), compositorId);
	if (it != m_compositorIDs.end())
	{
		m_compositorIDs.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_compositorListPropertyId));
	}
}

// -- SceneComponent -----
SceneComponent::SceneComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
	, m_mkScene(std::make_shared<MkScene>())
{
}

void SceneComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	TransformComponent::setDefinition(definition);

	// Setup initial attachment
	auto sceneComponentConfigPtr = std::static_pointer_cast<SceneComponentDefinition>(definition);
	MikanStageID currentParentId = sceneComponentConfigPtr->getParentStageId();
	attachTransformComponentToStage(currentParentId);
}

std::vector<CompositorComponentPtr> SceneComponent::getOutputCompositors() const
{
	std::vector<CompositorComponentPtr> outputCompositors;

	const std::vector<MikanCompositorID>& compositorIDs =
		getSceneComponentDefinition()->getCompositorIDs();
	for (MikanCompositorID compositorId : compositorIDs)
	{
		CompositorComponentPtr compositor =
			CompositorObjectSystem::getSystem()->getCompositorById(compositorId);
		if (compositor)
		{
			outputCompositors.push_back(compositor);
		}
	}

	return outputCompositors;
}

void SceneComponent::attachTransformComponentToStage(MikanStageID newParentId)
{
	if (newParentId != INVALID_MIKAN_ID)
	{
		StageComponentPtr stage = StageObjectSystem::getSystem()->getStageById(newParentId);

		if (stage)
		{
			if (attachToComponent(stage->getOwnerObject()->getRootComponent()))
			{
				getSceneComponentDefinition()->setParentStageId(newParentId);
			}
		}
		else
		{
			detachFromParent(eDetachReason::detachFromParent);
			getSceneComponentDefinition()->setParentStageId(INVALID_MIKAN_ID);
		}
	}
	else
	{
		detachFromParent(eDetachReason::detachFromParent);
		getSceneComponentDefinition()->setParentStageId(INVALID_MIKAN_ID);
	}
}

void SceneComponent::init()
{
	TransformComponent::init();

	// Listen for changes to the scene definition
	getSceneComponentDefinition()->OnMarkedDirty += MakeDelegate(this, &SceneComponent::onDefinitionChanged);
}

void SceneComponent::dispose()
{
	getSceneComponentDefinition()->OnMarkedDirty -= MakeDelegate(this, &SceneComponent::onDefinitionChanged);

	m_mkScene= nullptr;

	TransformComponent::dispose();
}

void SceneComponent::onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_compositorListPropertyId))
	{
		auto CompositorObjectSystemPtr = CompositorObjectSystem::getSystem();
		const std::vector<MikanCompositorID>& activeCompositorIdList=
			getSceneComponentDefinition()->getCompositorIDs();

		CompositorObjectSystemPtr->setActiveCompositors(activeCompositorIdList);
	}
}

SelectionComponentPtr SceneComponent::findClosestSelectionTarget(
	const glm::vec3& rayOrigin,
	const glm::vec3& rayDir,
	ColliderRaycastHitResult& outRaycastResult) const
{
	struct
	{
		ColliderRaycastHitRequest request;
		ColliderRaycastHitResult result;
		SelectionComponentPtr closestSelectionComponent;
	} raycastQuery;
	
	raycastQuery.request.rayOrigin= rayOrigin;
	raycastQuery.request.rayDirection= rayDir;

	raycastQuery.result.hitDistance = k_real_max;
	raycastQuery.result.hitPriority = 0;
	raycastQuery.result.hitLocation = glm::vec3();
	raycastQuery.result.hitNormal = glm::vec3();

	raycastQuery.closestSelectionComponent.reset();

	visitAllTransformComponentsConst(
		[&raycastQuery](const TransformComponent* transformComponent) {
			MikanObjectPtr ownerObjectPtr = transformComponent->getOwnerObject();
			SelectionComponentPtr selectionComponentPtr = ownerObjectPtr->getComponentOfType<SelectionComponent>();
			if (selectionComponentPtr)
			{
				ColliderRaycastHitResult result;

				if (selectionComponentPtr->computeRayIntersection(raycastQuery.request, result) &&
					(result.hitDistance < raycastQuery.result.hitDistance || 
					 result.hitPriority > raycastQuery.result.hitPriority))
				{
					raycastQuery.closestSelectionComponent = selectionComponentPtr;
					raycastQuery.result= result;
				}
			}
		});

	outRaycastResult= raycastQuery.result;

	return raycastQuery.closestSelectionComponent;
}

void SceneComponent::activateScene()
{
	CompositorObjectSystemPtr compositorSystem = CompositorObjectSystem::getSystem();
	const std::vector<MikanCompositorID>& compositorIDs =
		getSceneComponentDefinition()->getCompositorIDs();

	// Set active compositors for this scene
	compositorSystem->setActiveCompositors(compositorIDs);
}

void SceneComponent::deactivateScene()
{

}

void SceneComponent::renderEditorScene(MikanCameraConstPtr camera, MkStateStack& MkStateStack) const
{
	IMkScene* mkScene = m_mkScene.get();

	// Rebuild list of renderables
	mkScene->removeAllInstances();
	visitAllTransformComponentsConst(
		[mkScene](const TransformComponent* transformComponent) {
			IMkSceneRenderableConstPtr renderable = transformComponent->getGlSceneRenderableConst();
			if (renderable)
			{
				mkScene->addInstance(renderable);
			}
		});

	// Render the scene
	mkScene->render(camera, MkStateStack);
}

void SceneComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			SceneComponentDefinition::k_parentStagePropertyId));
}

bool SceneComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		outValue = getSceneComponentDefinition()->getParentStageId();
		return true;
	}

	return TransformComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool SceneComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		MikanStageID stageId = inValue.Get<int>();

		attachTransformComponentToStage(stageId);
		return true;
	}

	return TransformComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
const std::string SceneComponent::k_deleteSceneFunctionId = "delete_scene";

void SceneComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteSceneFunctionId, "Delete Scene"));
}

bool SceneComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionId = functionDesc->getFunctionName();

	if (functionId == k_deleteSceneFunctionId)
	{
		deleteScene();
		return true;
	}

	return TransformComponent::invokeFunctionFromRml(functionDesc);
}

void SceneComponent::deleteScene()
{
	getOwnerObject()->deleteSelfConfig();
}