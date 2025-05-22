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

protected:
	MikanSceneID m_sceneId = INVALID_MIKAN_ID;
	MikanStageID m_parentStageId = INVALID_MIKAN_ID;
};

class SceneComponent final : public TransformComponent
{
public:
	SceneComponent(MikanObjectWeakPtr owner);

	inline SceneComponentDefinitionPtr getSceneComponentDefinition() const
	{
		return std::static_pointer_cast<SceneComponentDefinition>(m_definition);
	}

	// -- MikanComponent ----
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	virtual void init() override;
	virtual void dispose() override;

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteSceneFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	// -- SceneComponent ----
	void attachTransformComponentToStage(MikanStageID newParentId);
	SelectionComponentPtr findClosestSelectionTarget(
		const glm::vec3& rayOrigin,
		const glm::vec3& rayDir,
		ColliderRaycastHitResult& outRaycastResult) const;
	void render(MikanCameraConstPtr camera, class MkStateStack& MkStateStack) const;
	void deleteScene();

private:
	IMkScenePtr m_mkScene;
};