#include "CompositorObjectSystem.h"
#include "CompositorComponent.h"
#include "IMkSceneRenderable.h"
#include "ModalSceneAddCompositor/ModalDialog_SceneAddCompositor.h"
#include "MikanObject.h"
#include "MikanSceneTypes.h"
#include "MkScene.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"
#include "SceneComponent.h"
#include "SceneComponentScriptContext.h"
#include "SelectionComponent.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "TransformComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <easy/profiler.h>

#include <queue>

// -- SceneComponentDefinition -----
const std::string SceneComponentDefinition::k_parentStagePropertyId = "parent_stage_id";
const std::string SceneComponentDefinition::k_compositorListPropertyId = "compositor_list";
const std::string SceneComponentDefinition::k_displayCompositorIdPropertyId = "display_compositor_id";

SceneComponentDefinition::SceneComponentDefinition()
	: TransformComponentDefinition()
	, m_parentStageId(INVALID_MIKAN_ID)
{}

SceneComponentDefinition::SceneComponentDefinition(
	MikanSceneID sceneId)
	: TransformComponentDefinition(sceneId, "", glm_transform_to_MikanTransform(GlmTransform()))
	, m_parentStageId(INVALID_MIKAN_ID)
{}

configuru::Config SceneComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_parentStagePropertyId] = m_parentStageId;
	writeStdValueVector(pt, k_compositorListPropertyId, m_compositorIDs);
	pt[k_displayCompositorIdPropertyId] = m_displayCompositorId;

	return pt;
}

void SceneComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_parentStageId = pt.get_or<int>(k_parentStagePropertyId, INVALID_MIKAN_ID);
	readStdValueVector(pt, k_compositorListPropertyId, m_compositorIDs);
	m_displayCompositorId = pt.get_or<int>(k_displayCompositorIdPropertyId, INVALID_MIKAN_ID);
}

void SceneComponentDefinition::setParentStageId(MikanStageID stageId)
{
	if (m_parentStageId != stageId)
	{
		m_parentStageId = stageId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_parentStagePropertyId));
	}
}

void SceneComponentDefinition::addCompositorID(MikanCompositorID compositorId)
{
	auto it = std::find(m_compositorIDs.begin(), m_compositorIDs.end(), compositorId);
	if (it == m_compositorIDs.end())
	{
		m_compositorIDs.push_back(compositorId);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_compositorListPropertyId));
	}
}

void SceneComponentDefinition::removeCompositorID(MikanCompositorID compositorId)
{
	auto it = std::find(m_compositorIDs.begin(), m_compositorIDs.end(), compositorId);
	if (it != m_compositorIDs.end())
	{
		ConfigPropertyChangeSet changeSet;

		m_compositorIDs.erase(it);
		changeSet.addPropertyName(k_compositorListPropertyId);

		// Update the display compositor ID if we are deleting the reference to the current one
		if (compositorId == m_displayCompositorId)
		{
			if (!m_compositorIDs.empty())
			{
				m_displayCompositorId = m_compositorIDs[0];
			}
			else
			{
				m_displayCompositorId = INVALID_MIKAN_ID;
			}

			changeSet.addPropertyName(k_compositorListPropertyId);
		}

		notifyPropertyChanged(changeSet);
	}
}

void SceneComponentDefinition::setDisplayCompositorId(MikanCompositorID compositorId)
{
	if (m_displayCompositorId != compositorId)
	{
		m_displayCompositorId = compositorId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_displayCompositorIdPropertyId));
	}
}

// -- SceneComponent -----
SceneComponent::SceneComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
	, m_mkScene(std::make_shared<MkScene>())
{
}

// -- IEntityAccessor ----
rfk::Struct const* SceneComponent::getClientAPIValuesStructType() const
{
	return &MikanSceneComponentValues::staticGetArchetype();
}

void SceneComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	TransformComponent::setDefinition(definition);

	// Setup initial attachment
	auto sceneComponentConfigPtr = std::static_pointer_cast<SceneComponentDefinition>(definition);
	MikanStageID currentParentId = sceneComponentConfigPtr->getParentStageId();
	attachTransformComponentToStage(currentParentId);
}

StageComponentPtr SceneComponent::getParentStage() const
{
	MikanStageID parentStageId= getSceneComponentDefinition()->getParentStageId();
	if (parentStageId != INVALID_MIKAN_ID)
	{
		return getObjectSystemOfType<StageObjectSystem>()->getStageById(parentStageId);
	}

	return StageComponentPtr();
}

const std::vector<MikanCompositorID>& SceneComponent::getOutputCompositorIDs() const
{
	return getSceneComponentDefinition()->getCompositorIDs();
}

std::vector<CompositorComponentPtr> SceneComponent::getOutputCompositors() const
{
	std::vector<CompositorComponentPtr> outputCompositors;

	const std::vector<MikanCompositorID>& compositorIDs = getOutputCompositorIDs();
	for (MikanCompositorID compositorId : compositorIDs)
	{
		auto compositorSystem= getObjectSystemOfType<CompositorObjectSystem>();
		CompositorComponentPtr compositor = compositorSystem->getCompositorById(compositorId);

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
		auto stageSystem = getObjectSystemOfType<StageObjectSystem>();
		StageComponentPtr stage = stageSystem->getStageById(newParentId);

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
	getSceneComponentDefinition()->OnPropertyChanged += MakeDelegate(this, &SceneComponent::onDefinitionChanged);
}

void SceneComponent::dispose()
{
	getSceneComponentDefinition()->OnPropertyChanged -= MakeDelegate(this, &SceneComponent::onDefinitionChanged);

	m_mkScene= nullptr;

	TransformComponent::dispose();
}

ComponentScriptContextPtr SceneComponent::allocateScriptContext()
{
	return std::make_shared<SceneComponentScriptContext>(getSelfPtr<SceneComponent>());
}

void SceneComponent::onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_compositorListPropertyId))
	{
		auto CompositorObjectSystemPtr = getObjectSystemOfType<CompositorObjectSystem>();
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
	CompositorObjectSystemPtr compositorSystem = getObjectSystemOfType<CompositorObjectSystem>();
	MikanCompositorID compositorId= getSceneComponentDefinition()->getDisplayCompositorId();

	std::vector<MikanCompositorID> activeCompositorIDs;

	if (compositorId != INVALID_MIKAN_ID)
	{
		activeCompositorIDs.push_back(compositorId);
	}

	// Set active compositors for this scene
	compositorSystem->setActiveCompositors(activeCompositorIDs);
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

void SceneComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			SceneComponentDefinition::k_parentStagePropertyId, MikanVariantType::INT)
		->setDefaultValue(-1));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			SceneComponentDefinition::k_compositorListPropertyId, MikanVariantType::INT_ARRAY)
		->setReadOnly());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			SceneComponentDefinition::k_displayCompositorIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(-1));
}

bool SceneComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		outValue = getSceneComponentDefinition()->getParentStageId();
		return true;
	}
	else if (propertyName == SceneComponentDefinition::k_displayCompositorIdPropertyId)
	{
		outValue = getSceneComponentDefinition()->getDisplayCompositorId();
		return true;
	}
	else if (propertyName == SceneComponentDefinition::k_compositorListPropertyId)
	{
		outValue = getSceneComponentDefinition()->getCompositorIDs();
		return true;
	}
	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool SceneComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		MikanStageID stageId = inValue.getIntValue();

		attachTransformComponentToStage(stageId);
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string SceneComponent::k_deleteSceneFunctionId = "delete_scene";
const std::string SceneComponent::k_addCompositorRefFunctionId = "add_compositor_ref";
const std::string SceneComponent::k_removeCompositorRefFunctionId = "remove_compositor_ref";

void SceneComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_deleteSceneFunctionId, "Delete Scene"));
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_addCompositorRefFunctionId, "Add Compositor Reference"));
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_removeCompositorRefFunctionId, "Remove Compositor Reference"));
}

bool SceneComponent::invokeFunction(FunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionId = functionDesc->getFunctionName();

	if (functionId == k_deleteSceneFunctionId)
	{
		deleteScene();
		return true;
	}
	else if (functionId == k_addCompositorRefFunctionId)
	{
		addCompositorRef();
		return true;
	}
	else if (functionId == k_removeCompositorRefFunctionId)
	{
		removeCompositorRef();
		return true;
	}

	return TransformComponent::invokeFunction(functionDesc);
}

void SceneComponent::deleteScene()
{
	getOwnerObject()->deleteSelfConfig();
}

void SceneComponent::addCompositorRef()
{
	ModalDialog_SceneAddCompositor::selectNewCompositor(
		getSelfPtr<SceneComponent>(),
		[this](MikanCompositorID compositorId) {
			getSceneComponentDefinition()->addCompositorID(compositorId);
		});
}

void SceneComponent::removeCompositorRef()
{
	auto definition= getSceneComponentDefinition();
	MikanCompositorID selectedCompositorId= definition->getDisplayCompositorId();
	if (selectedCompositorId != INVALID_MIKAN_ID)
	{
		definition->removeCompositorID(selectedCompositorId);
	}
}

// -- Lua Binding ----
void SceneComponent::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<SceneComponent, TransformComponent>(SceneComponent::k_componentClassName.c_str())
		.addProperty("parentStageId",
			[](SceneComponent* component) -> int {
				return component->getSceneComponentDefinition()->getParentStageId();
			})
		//TODO
		//.addProperty("parentStage",
		//	[](SceneComponent* component) -> StageComponent* {
		//		return component->getParentStage().get();
		//	})
		.addProperty("compositorCount",
			[](SceneComponent* component) -> int {
				return component->getSceneComponentDefinition()->getCompositorCount();
			})
		//TODO
		//.addFunction("getCompositorByIndex",
		//	[](int index) -> CompositorComponent* {
		//		return component->getCompositorByIndex(index).get();
		//	})
		.addProperty("displayCompositorId",
			[](SceneComponent* component) -> LuaVec3f {
				return LuaVec3f(component->getRelativeScale());
			})
		//TODO
		//.addProperty("quadStencilCount",
		//	[](SceneComponent* component) -> int {
		//		return component->getQuadStencilCount();
		//	})
		//.addFunction("getQuadStencilByIndex",
		//	[](int index) -> QuadStencilComponent* {
		//		return component->getQuadStencilByIndex(index).get();
		//	})
		//.addProperty("boxStencilCount",
		//	[](SceneComponent* component) -> int {
		//		return component->getBoxStencilCount();
		//	})
		//.addFunction("getBoxStencilByIndex",
		//	[](int index) -> BoxStencilComponent* {
		//		return component->getBoxStencilByIndex(index).get();
		//	})
		//.addProperty("modelStencilCount",
		//	[](SceneComponent* component) -> int {
		//		return component->getModelStencilCount();
		//	})
		//.addFunction("getModelStencilByIndex",
		//	[](int index) -> ModelStencilComponent* {
		//		return component->getModelStencilByIndex(index).get();
		//	})
		.endClass();
}