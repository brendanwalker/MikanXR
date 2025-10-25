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
	SceneComponentDefinition(
		MikanSceneID sceneId,
		MikanStageID parentStageId,
		const std::string& componentName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanSceneID getSceneId() const { return m_sceneId; }

	static const std::string k_parentStagePropertyId;
	MikanStageID getParentStageId() const { return m_parentStageId; }
	void setParentStageId(MikanStageID stageId);

	static const std::string k_compositorListPropertyId;
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

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_deleteSceneFunctionId;
	static const std::string k_addCompositorRefFunctionId;
	static const std::string k_removeCompositorRefFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

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
	void onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

	// IRmlFunctionInterface handlers
	void deleteScene();
	void addCompositorRef();
	void removeCompositorRef();

private:
	// Scene Rendering
	IMkScenePtr m_mkScene;
};