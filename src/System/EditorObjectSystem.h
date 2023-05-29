#pragma once

#include "MikanObjectSystem.h"
#include "ColliderQuery.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "RendererFwd.h"
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

	float cameraSpeed= 1.f;
};

class EditorObjectSystem : public MikanObjectSystem
{
public:
	static EditorObjectSystemPtr getSystem() { return s_editorObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	EditorObjectSystemConfigConstPtr getEditorSystemConfigConst() const;
	EditorObjectSystemConfigPtr getEditorSystemConfig();
	MikanSceneConstPtr getEditorScene() const { return m_scene; }

	void bindViewport(GlViewportWeakPtr viewportWeakPtr);
	void clearViewports();

protected:

	MikanScenePtr m_scene;
	std::vector<GlViewportWeakPtr> m_viewports;
	
	ColliderRaycastHitResult m_lastestRaycastResult;
	SelectionComponentWeakPtr m_hoverComponentWeakPtr;
	SelectionComponentWeakPtr m_selectedComponentWeakPtr;

	MikanObjectWeakPtr m_gizmoObjectWeakPtr;
	GizmoTransformComponentWeakPtr m_gizmoComponentWeakPtr;

	// App Events
	void onAppStageEntered(class AppStage* appStage);

	// Object System Events
	void onComponentInitialized(MikanObjectSystemPtr system, MikanComponentPtr component);
	void onComponentDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component);

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