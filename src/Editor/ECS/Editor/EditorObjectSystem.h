#pragma once

#include "MikanObjectSystem.h"
#include "ColliderQuery.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "MikanRendererFwd.h"
#include "SceneFwd.h"

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class EditorObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	EditorObjectSystemDefinition(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_cameraSpeedPropertyId;
	float getCameraSpeed() const { return cameraSpeed; }
	void setCameraSpeed(float speed);

	static const std::string k_currentSceneNamePropertyId;
	const std::string& getCurrentSceneName() const { return currentSceneName; }
	void setCurrentSceneName(const std::string& sceneName);

private:
	float cameraSpeed= 1.f;
	std::string currentSceneName;
};

class EditorObjectSystem : public MikanObjectSystem
{
public:
	EditorObjectSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "EditorObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;

	EditorObjectSystemDefinitionConstPtr getEditorSystemConfigConst() const;
	EditorObjectSystemDefinitionPtr getEditorSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;
	virtual bool getComponentIdList(const std::string& componentClassName, std::vector<int>& outComponentIdList) const override;

	void bindViewport(MikanViewportWeakPtr viewportWeakPtr);
	void unbindViewport(MikanViewportWeakPtr viewportWeakPtr);
	void clearViewports();

	SelectionComponentPtr getSelection() const { return m_selectedComponentWeakPtr.lock(); }
	void setSelection(SelectionComponentPtr newComponentPtr);
	MulticastDelegate<void()> OnSelectionChanged;

	inline MikanObjectPtr getGizmoObject() const { return m_gizmoObjectWeakPtr.lock(); }

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	std::vector<MikanViewportWeakPtr> m_viewports;
	
	ColliderRaycastHitResult m_lastestRaycastResult;
	SelectionComponentWeakPtr m_hoverComponentWeakPtr;
	SelectionComponentWeakPtr m_selectedComponentWeakPtr;

	MikanObjectWeakPtr m_gizmoObjectWeakPtr;
	GizmoTransformComponentWeakPtr m_gizmoComponentWeakPtr;

	// App Events
	void onAppStageEntered(class AppStage* oldAppStage, class AppStage* newAppStage);

	// Object System Events
	void onSceneActivated(SceneComponentPtr newScene);
	void onSceneDeactivated(SceneComponentPtr oldScene);
	void onActorDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component);

	// Keyboard Events
	void onDeletePressed();

	// Viewport Events
	void onMouseExited();
	void onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onSelectionChanged(SelectionComponentPtr oldComponentPtr, SelectionComponentPtr newComponentPtr);

	// Helpers
	void createSceneTransformGizmo(SceneComponentPtr ownerScene);
	void disposeSceneTransformGizmo();
	void createGizmoBoxCollider(
		MikanObjectPtr gizmoObjectPtr, 
		const std::string& name,
		const glm::vec3& center,
		const glm::vec3& halfExtents);
	void createGizmoDiskCollider(
		MikanObjectPtr gizmoObjectPtr,
		const std::string& name,
		const glm::vec3& center,
		const glm::vec3& normal,
		const float radius);
	SelectionComponentPtr findClosestSelectionTarget(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		ColliderRaycastHitResult& outRaycastResult) const;
};