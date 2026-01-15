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

	MikanSceneID getSceneId() const { return m_sceneId; }

	static const std::string k_parentStagePropertyId;
	MikanStageID getParentStageId() const { return m_parentStageId; }
	void setParentStageId(MikanStageID stageId);

	static const std::string k_compositorListPropertyId;
	inline int getCompositorCount() const { return static_cast<int>(m_compositorIDs.size()); }
	const std::vector<MikanCompositorID>& getCompositorIDs() const { return m_compositorIDs; }
	void addCompositorID(MikanCompositorID compositorId);
	void removeCompositorID(MikanCompositorID compositorId);

	static const std::string k_displayCompositorIdPropertyId;
	MikanCompositorID getDisplayCompositorId() const { return m_displayCompositorId; }
	void setDisplayCompositorId(MikanCompositorID compositorId);

protected:
	MikanSceneID m_sceneId = INVALID_MIKAN_ID;
	MikanStageID m_parentStageId = INVALID_MIKAN_ID;
	std::vector<MikanCompositorID> m_compositorIDs;
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
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	virtual void init() override;
	virtual void dispose() override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteSceneFunctionId;
	static const std::string k_addCompositorRefFunctionId;
	static const std::string k_removeCompositorRefFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

	// -- SceneComponent ----
	inline MikanSceneID getSceneId() const { return getSceneComponentDefinition()->getSceneId(); }
	StageComponentPtr getParentStage() const;
	const std::vector<MikanCompositorID>& getOutputCompositorIDs() const;
	std::vector<CompositorComponentPtr> getOutputCompositors() const;
	void attachTransformComponentToStage(MikanStageID newParentId);
	SelectionComponentPtr findClosestSelectionTarget(
		const glm::vec3& rayOrigin,
		const glm::vec3& rayDir,
		ColliderRaycastHitResult& outRaycastResult) const;
	void activateScene();
	void deactivateScene();
	void renderEditorScene(MikanCameraConstPtr camera, class MkStateStack& MkStateStack) const;

protected:
	virtual ComponentScriptContextPtr allocateScriptContext() override;

	void onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

	// IFunctionInterface handlers
	void deleteScene();
	void addCompositorRef();
	void removeCompositorRef();

private:
	// Scene Rendering
	IMkScenePtr m_mkScene;
};