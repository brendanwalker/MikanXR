#pragma once

#include "ColliderQuery.h"
#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "MulticastDelegate.h"

#include <vector>

class SelectionComponent : public MikanComponent
{
public:
	SelectionComponent(MikanObjectWeakPtr owner);
	virtual ~SelectionComponent();

	virtual void init() override;
	virtual void dispose() override;

	bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const;

	void notifyHoverEnter(const ColliderRaycastHitResult& hitResult);
	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult)> OnInteractionRayOverlapEnter;
	void notifyHoverExit(const ColliderRaycastHitResult& hitResult);
	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult)> OnInteractionRayOverlapExit;
	bool getIsHovered() const { return m_bIsHovered; }

	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult, int button)> OnInteractionRayPress;
	MulticastDelegate<void(const ColliderRaycastHitResult& hitResult, int button)> OnInteractionRayRelease;

	void notifySelected();
	MulticastDelegate<void()> OnInteractionSelected;
	void notifyUnselected();
	MulticastDelegate<void()> OnInteractionUnselected;
	bool getIsSelected() const { return m_bIsSelected; }

protected:
	std::vector<ColliderComponentPtr> m_colliders;
	bool m_bIsHovered = false;
	bool m_bIsSelected= false;
};