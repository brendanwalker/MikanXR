#pragma once

#include "MikanObjectSystem.h"
#include "ColliderQuery.h"
#include "ComponentFwd.h"
#include "SceneFwd.h"

#include <vector>

#include "glm/ext/vector_float3.hpp"

class EditorObjectSystem : public MikanObjectSystem
{
public:
	EditorObjectSystem();
	virtual ~EditorObjectSystem();

	virtual void init() override;
	virtual void dispose() override;

	void bindViewport(GlViewportWeakPtr viewportWeakPtr);
	void clearViewports();

protected:
	MikanScenePtr m_scene;
	std::vector<GlViewportWeakPtr> m_viewports;
	SelectionComponentWeakPtr m_selectionHoverWeakPtr;

	// Object System Events
	void onObjectAdded(MikanObjectSystem& system, MikanObject& object);
	void onObjectRemoved(MikanObjectSystem& system, MikanObject& object);

	// Viewport Events
	void onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);

	// Helpers
	SelectionComponentWeakPtr findClosestSelectionTarget(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		ColliderRaycastHitResult& outRaycastResult) const;
};