#pragma once

#include "ColliderQuery.h"
#include "MikanComponent.h"
#include "MulticastDelegate.h"

#include <vector>

class SelectionComponent : public MikanComponent
{
public:
	SelectionComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "SelectionComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	virtual void init() override;
	virtual void dispose() override;

	inline bool getIsTransformGizmoAllowed() const
	{ return m_bIsTransformGizmoAllowed; }
	inline void setIsTransformGizmoAllowed(bool bAllowed) 
	{ m_bIsTransformGizmoAllowed= bAllowed; }

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

	void notifyTransformGizmoBound();
	MulticastDelegate<void()> OnTransformGizmoBound;
	void notifyTransformGizmoUnbound();
	MulticastDelegate<void()> OnTransformGizmoUnbound;
	bool getIsTransformGizmoAttached() const { return m_bIsTransformGizmoBound; }

protected:
	std::vector<ColliderComponentWeakPtr> m_colliders;
	// Interaction Permissions
	bool m_bIsTransformGizmoAllowed= true;
	// Interaction Flags
	bool m_bIsHovered = false;
	bool m_bIsGrabbed = false;
	bool m_bIsSelected= false;
	bool m_bIsTransformGizmoBound = false;
};