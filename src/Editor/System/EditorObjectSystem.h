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

class EditorObjectSystemConfig : public CommonConfig
{
public:
	EditorObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
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
	static EditorObjectSystemPtr getSystem() { return s_editorObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	EditorObjectSystemConfigConstPtr getEditorSystemConfigConst() const;
	EditorObjectSystemConfigPtr getEditorSystemConfig();
	SceneComponentConstPtr getEditorScene() const { return m_sceneWeakPtr.lock(); }

	void bindViewport(MikanViewportWeakPtr viewportWeakPtr);
	void clearViewports();

	SelectionComponentPtr getSelection() const { return m_selectedComponentWeakPtr.lock(); }
	void setSelection(SelectionComponentPtr newComponentPtr);
	MulticastDelegate<void()> OnSelectionChanged;

	inline MikanObjectPtr getGizmoObject() const { return m_gizmoObjectWeakPtr.lock(); }

protected:

	SceneComponentWeakPtr m_sceneWeakPtr;
	std::vector<MikanViewportWeakPtr> m_viewports;
	
	ColliderRaycastHitResult m_lastestRaycastResult;
	SelectionComponentWeakPtr m_hoverComponentWeakPtr;
	SelectionComponentWeakPtr m_selectedComponentWeakPtr;

	MikanObjectWeakPtr m_gizmoObjectWeakPtr;
	GizmoTransformComponentWeakPtr m_gizmoComponentWeakPtr;

	// App Events
	void onAppStageEntered(class AppStage* oldAppStage, class AppStage* newAppStage);

	// Object System Events
	void onSceneDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component);
	void onSceneObjectDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component);

	// Keyboard Events
	void onDeletePressed();

	// Viewport Events
	void onMouseExited();
	void onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onSelectionChanged(SelectionComponentPtr oldComponentPtr, SelectionComponentPtr newComponentPtr);

	// Helpers
	void createTransformGizmo();
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

	static EditorObjectSystemWeakPtr s_editorObjectSystem;
};