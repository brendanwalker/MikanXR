#pragma once

#include "ColliderQuery.h"
#include "MikanComponent.h"
#include "MulticastDelegate.h"

#include <vector>

class SelectionComponent : public MikanComponent
{
public:
	SelectionComponent(MikanObjectWeakPtr owner);

	virtual void init() override;
	virtual void dispose() override;

	void rebindColliders();
	bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const;

	void notifyHoverEnter(const ColliderRaycastHitResult& hitResult);
	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult)> OnInteractionRayOverlapEnter;
	void notifyHoverExit(const ColliderRaycastHitResult& hitResult);
	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult)> OnInteractionRayOverlapExit;
	bool getIsHovered() const { return m_bIsHovered; }

	void notifyGrab(const ColliderRaycastHitResult& hitResult);
	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult)> OnInteractionGrab;
	void notifyMove(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir)> OnInteractionMove;
	void notifyRelease();
	MulticastDelegate<void()> OnInteractionRelease;
	bool getIsGrabbed() const { return m_bIsGrabbed; }

	void notifySelected();
	MulticastDelegate<void()> OnInteractionSelected;
	void notifyUnselected();
	MulticastDelegate<void()> OnInteractionUnselected;
	bool getIsSelected() const { return m_bIsSelected; }

protected:
	std::vector<ColliderComponentWeakPtr> m_colliders;
	bool m_bIsHovered = false;
	bool m_bIsGrabbed = false;
	bool m_bIsSelected= false;
};