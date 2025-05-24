#include "SceneComponent.h"
#include "StageComponent.h"
#include "MikanObject.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "IMkSceneRenderable.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"
#include "MkScene.h"
#include "StageObjectSystem.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <queue>

// -- SceneComponentDefinition -----
const std::string SceneComponentDefinition::k_parentStagePropertyId = "parent_stage_id";

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

	return pt;
}

void SceneComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_sceneId = pt.get<int>("scene_id");
	m_parentStageId = pt.get_or<int>(k_parentStagePropertyId, INVALID_MIKAN_ID);
}

void SceneComponentDefinition::setParentStageId(MikanStageID stageId)
{
	if (m_parentStageId != stageId)
	{
		m_parentStageId = stageId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_parentStagePropertyId));
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
}

void SceneComponent::dispose()
{
	m_mkScene= nullptr;
	//m_selectionComponents.clear();

	TransformComponent::dispose();
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

void SceneComponent::render(MikanCameraConstPtr camera, MkStateStack& MkStateStack) const
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

void SceneComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(SceneComponentDefinition::k_parentStagePropertyId);
}

bool SceneComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (TransformComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		outDescriptor = {SceneComponentDefinition::k_parentStagePropertyId, ePropertyDataType::datatype_int, ePropertySemantic::stage_id};
		return true;
	}

	return false;
}

bool SceneComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (TransformComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		outValue = getSceneComponentDefinition()->getParentStageId();
		return true;
	}

	return false;
}

bool SceneComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (TransformComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		MikanStageID stageId = inValue.Get<int>();

		attachTransformComponentToStage(stageId);
		return true;
	}

	return false;
}

// -- IFunctionInterface ----
const std::string SceneComponent::k_deleteSceneFunctionId = "delete_scene";

void SceneComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_deleteSceneFunctionId);
}

bool SceneComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (TransformComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == SceneComponent::k_deleteSceneFunctionId)
	{
		outDescriptor = {SceneComponent::k_deleteSceneFunctionId, "Delete Scene"};
		return true;
	}

	return false;
}

bool SceneComponent::invokeFunction(const std::string& functionName)
{
	if (TransformComponent::invokeFunction(functionName))
		return true;

	if (functionName == SceneComponent::k_deleteSceneFunctionId)
	{
		deleteScene();
	}

	return false;
}

void SceneComponent::deleteScene()
{
	getOwnerObject()->deleteSelfConfig();
}