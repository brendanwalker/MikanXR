#pragma once

#include "MikanObjectSystem.h"
#include "ColliderQuery.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "RendererFwd.h"
#include "SceneFwd.h"

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class EditorObjectSystem : public MikanObjectSystem
{
public:
	virtual void init() override;
	virtual void dispose() override;

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

	// Object System Events
	void onObjectAdded(MikanObjectSystem& system, MikanObject& object);
	void onObjectRemoved(MikanObjectSystem& system, MikanObject& object);

	// Viewport Events
	void onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onSelectionChanged(SelectionComponentPtr oldComponentPtr, SelectionComponentPtr newComponentPtr);
	void onSelectionTranslationRequested(const glm::vec3& worldSpaceTranslation);
	void onSelectionRotationRequested(const glm::quat& objectSpaceRotation);
	void onSelectionScaleRequested(const glm::vec3& objectSpaceScale);

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
	SelectionComponentWeakPtr findClosestSelectionTarget(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		ColliderRaycastHitResult& outRaycastResult) const;
};