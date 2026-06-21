#include "App.h"
#include "CompositorObjectSystem.h"
#include "CompositorComponent.h"
#include "ComponentScriptContext.h"
#include "ColliderComponent.h"
#include "Windows/CompositorOutputEditorWindow.h"
#include "IMkSceneRenderable.h"
#include "ModalSceneAddCompositor/ModalDialog_SceneAddCompositor.h"
#include "MikanObject.h"
#include "MikanSceneTypes.h"
#include "MkScene.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "SceneObjectSystem.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "TransformComponent.h"
#include "VRObjectSystem.h"
#include "VRTrackingVolumeComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <easy/profiler.h>

#include <queue>

// -- SceneComponentDefinition -----
const std::string SceneComponentDefinition::k_parentStagePropertyId= "parent_stage_id";
const std::string SceneComponentDefinition::k_displayCompositorIdPropertyId= "display_compositor_id";

SceneComponentDefinition::SceneComponentDefinition()
	: TransformComponentDefinition()
	, m_parentStageId(INVALID_MIKAN_ID)
{
}

SceneComponentDefinition::SceneComponentDefinition(MikanSceneID sceneId)
	: TransformComponentDefinition(sceneId, "", glm_transform_to_MikanTransform(GlmTransform()))
	, m_parentStageId(INVALID_MIKAN_ID)
{
}

configuru::Config SceneComponentDefinition::writeToJSON()
{
	configuru::Config pt= TransformComponentDefinition::writeToJSON();

	pt[k_parentStagePropertyId]= m_parentStageId;
	pt[k_displayCompositorIdPropertyId]= m_displayCompositorId;

	return pt;
}

void SceneComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_parentStageId= pt.get_or<int>(k_parentStagePropertyId, INVALID_MIKAN_ID);
	m_displayCompositorId= pt.get_or<int>(k_displayCompositorIdPropertyId, INVALID_MIKAN_ID);
}

bool SceneComponentDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
												  const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TransformComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues= initParams.getTypedPointer<MikanSceneComponentValues>();
	if (componentValues)
	{
		m_parentStageId= componentValues->parent_stage_id;
		m_displayCompositorId= componentValues->display_compositor_id;

		// Make sure our parent is always the stage component (if a stage was given)
		if (m_parentStageId != INVALID_MIKAN_ID)
		{
			m_parentTransformId= m_parentStageId;
		}
	}

	if (m_parentStageId == INVALID_MIKAN_ID)
	{
		auto stageObjectSystem= ownerObjectSystem->getObjectSystemOfType<StageObjectSystem>();

		m_parentStageId= stageObjectSystem->getFirstComponentId();
	}

	return true;
}

void SceneComponentDefinition::setParentStageId(MikanStageID stageId)
{
	if (m_parentStageId != stageId)
	{
		m_parentStageId= stageId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_parentStagePropertyId));
	}
}

void SceneComponentDefinition::setDisplayCompositorId(MikanCompositorID compositorId)
{
	if (m_displayCompositorId != compositorId)
	{
		m_displayCompositorId= compositorId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_displayCompositorIdPropertyId));
	}
}

// -- SceneComponent -----
SceneComponent::SceneComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
}

// -- IEntityAccessor ----
rfk::Struct const* SceneComponent::getClientAPIValuesStructType() const
{
	return &MikanSceneComponentValues::staticGetArchetype();
}

MikanStageID SceneComponent::getParentStageId() const { return getSceneComponentDefinition()->getParentStageId(); }

StageComponentPtr SceneComponent::getParentStage() const
{
	MikanStageID parentStageId= getParentStageId();
	if (parentStageId != INVALID_MIKAN_ID)
	{
		return getObjectSystemOfType<StageObjectSystem>()->getStageById(parentStageId);
	}

	return StageComponentPtr();
}

std::vector<MikanCompositorID> SceneComponent::getOutputCompositorIDs() const
{
	std::vector<MikanCompositorID> compositorIDs;

	auto& componentMap= getObjectSystemOfType<CompositorObjectSystem>()->getComponentMap();
	for (const auto& pair : componentMap)
	{
		const CompositorComponentWeakPtr& sceneComponentWeakPtr= pair.second;

		if (sceneComponentWeakPtr.lock()->getOwnerStageId() == getComponentId())
		{
			compositorIDs.push_back(pair.first);
		}
	}

	return compositorIDs;
}

std::vector<CompositorComponentPtr> SceneComponent::getOutputCompositors() const
{
	std::vector<CompositorComponentPtr> outputCompositors;

	const std::vector<MikanCompositorID>& compositorIDs= getOutputCompositorIDs();
	for (MikanCompositorID compositorId : compositorIDs)
	{
		auto compositorSystem= getObjectSystemOfType<CompositorObjectSystem>();
		CompositorComponentPtr compositor= compositorSystem->getCompositorById(compositorId);

		if (compositor)
		{
			outputCompositors.push_back(compositor);
		}
	}

	return outputCompositors;
}

void SceneComponent::attachToStage(MikanStageID newParentId)
{
	auto stageSystem= getObjectSystemOfType<StageObjectSystem>();
	StageComponentPtr oldStage= stageSystem->getStageById(getParentStageId());
	StageComponentPtr newStage= stageSystem->getStageById(newParentId);

	if (newStage != oldStage)
	{
		// onAttachedToNewParent / onDetachedFromParent will update our parent stage ID
		if (newStage)
		{
			attachToComponent(newStage->getOwnerObject()->getRootComponent());
		}
		else if (oldStage)
		{
			detachFromParent(TransformComponent::eDetachReason::detachFromParent);
		}
	}
}

void SceneComponent::onDefinitionMarkedDirty(CommonConfigPtr configPtr,
											 const ConfigPropertyChangeSet& changedPropertySet)
{
	TransformComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);

	if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_displayCompositorIdPropertyId))
	{
		refreshActiveCompositors();
	}
}

void SceneComponent::showCompositorOutput()
{
	App* app= App::getInstance();
	CompositorOutputEditorWindow* window= app->getWindowOfType<CompositorOutputEditorWindow>();
	if (!window)
	{
		window= app->createAppWindow<CompositorOutputEditorWindow>();
	}
	if (window)
	{
		window->bindSceneComponent(getSelfPtr<SceneComponent>());
	}
}

void SceneComponent::activateScene() { refreshActiveCompositors(); }

void SceneComponent::deactivateScene() {}

void SceneComponent::refreshActiveCompositors()
{
	CompositorObjectSystemPtr compositorSystem= getObjectSystemOfType<CompositorObjectSystem>();
	MikanCompositorID compositorId= getSceneComponentDefinition()->getDisplayCompositorId();

	std::vector<MikanCompositorID> activeCompositorIDs;

	if (compositorId != INVALID_MIKAN_ID)
	{
		activeCompositorIDs.push_back(compositorId);
	}

	// Set active compositors for this scene
	compositorSystem->setActiveCompositors(activeCompositorIDs);
}

void SceneComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(SceneComponentDefinition::k_parentStagePropertyId, MikanVariantType::INT)
			->setDefaultValue(-1)
			->setUIHidden());
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 SceneComponentDefinition::k_displayCompositorIdPropertyId, MikanVariantType::INT)
								 ->setDefaultValue(-1)
								 ->setUIHidden());
}

bool SceneComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		outValue= getSceneComponentDefinition()->getParentStageId();
		return true;
	}
	else if (propertyName == SceneComponentDefinition::k_displayCompositorIdPropertyId)
	{
		outValue= getSceneComponentDefinition()->getDisplayCompositorId();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool SceneComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == SceneComponentDefinition::k_parentStagePropertyId)
	{
		MikanStageID stageId= inValue.getIntValue();

		attachToStage(stageId);
		return true;
	}
	else if (propertyName == SceneComponentDefinition::k_displayCompositorIdPropertyId)
	{
		MikanCompositorID compositorId= inValue.getIntValue();

		getSceneComponentDefinition()->setDisplayCompositorId(compositorId);
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string SceneComponent::k_showCompositorOutputFunctionId= "show_compositor_output";

void SceneComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_showCompositorOutputFunctionId, "Show Compositor Output"));
}

bool SceneComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == SceneComponent::k_showCompositorOutputFunctionId)
	{
		showCompositorOutput();
		return true;
	}

	return TransformComponent::invokeFunction(functionName);
}

// -- Lua Binding ----
void SceneComponent::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<SceneComponent, TransformComponent>(SceneComponent::k_componentClassName.c_str())
		.addProperty("parentStageId", [](SceneComponent* component) -> int
					 { return component->getSceneComponentDefinition()->getParentStageId(); })
		.addProperty("displayCompositorId", [](SceneComponent* component) -> int
					 { return component->getSceneComponentDefinition()->getDisplayCompositorId(); })
		.addFunction("showCompositorOutput", [](SceneComponent* c) { c->showCompositorOutput(); })
		.addFunction("getParentStage", [](SceneComponent* c) -> StageComponent* { return c->getParentStage().get(); })
		.addFunction("getOutputCompositorCount",
					 [](SceneComponent* c) -> int { return static_cast<int>(c->getOutputCompositors().size()); })
		.addFunction("getOutputCompositorAtIndex",
					 [](SceneComponent* c, int i) -> CompositorComponent*
					 {
						 auto v= c->getOutputCompositors();
						 return (i >= 0 && i < static_cast<int>(v.size())) ? v[i].get() : nullptr;
					 })
		.endClass();
}