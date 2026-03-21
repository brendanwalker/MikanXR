#pragma once

#include "ComponentFwd.h"
#include "ColliderQuery.h"
#include "ObjectFwd.h"
#include "MikanSceneTypes.h"
#include "MikanRendererFwd.h"
#include "TransformComponent.h"

#include <functional>

class SceneComponentDefinition : public TransformComponentDefinition
{
public:
	SceneComponentDefinition();
	SceneComponentDefinition(MikanSceneID sceneId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual bool readFromInitParams(const Serialization::PolymorphicObjectPtr& initParams) override;

	MikanSceneID getSceneId() const { return getComponentId(); }

	static const std::string k_parentStagePropertyId;
	MikanStageID getParentStageId() const { return m_parentStageId; }
	void setParentStageId(MikanStageID stageId);

	static const std::string k_displayCompositorIdPropertyId;
	MikanCompositorID getDisplayCompositorId() const { return m_displayCompositorId; }
	void setDisplayCompositorId(MikanCompositorID compositorId);

protected:
	MikanStageID m_parentStageId = INVALID_MIKAN_ID;
	MikanCompositorID m_displayCompositorId = INVALID_MIKAN_ID;
};

class SceneComponent final : public TransformComponent
{
public:
	SceneComponent(MikanObjectWeakPtr owner);

	inline SceneComponentDefinitionPtr getSceneComponentDefinition() const
	{
		return std::static_pointer_cast<SceneComponentDefinition>(m_definition);
	}

	inline static const std::string k_componentClassName = "SceneComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	// -- MikanComponent ----
	virtual void init() override;
	virtual void dispose() override;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

	// -- SceneComponent ----
	inline MikanSceneID getSceneId() const { return getSceneComponentDefinition()->getSceneId(); }
	MikanStageID getParentStageId() const;
	StageComponentPtr getParentStage() const;
	std::vector<MikanCompositorID> getOutputCompositorIDs() const;
	std::vector<CompositorComponentPtr> getOutputCompositors() const;
	void attachToStage(MikanStageID newParentId);
	SelectionComponentPtr findClosestSelectionTarget(
		const glm::vec3& rayOrigin,
		const glm::vec3& rayDir,
		ColliderRaycastHitResult& outRaycastResult) const;
	void activateScene();
	void deactivateScene();
	void renderEditorScene(MikanCameraConstPtr camera, class MkStateStack& MkStateStack) const;

protected:
	virtual ComponentScriptContextPtr allocateScriptContext() override;

private:
	// Scene Rendering
	IMkScenePtr m_mkScene;
};