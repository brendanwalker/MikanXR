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

	MulticastDelegate<void()> OnInteractionRayOverlapEnter;
	MulticastDelegate<void()> OnInteractionRayOverlapExit;
	MulticastDelegate<void(int button)> OnInteractionRayPress;
	MulticastDelegate<void(int button)> OnInteractionRayRelease;

protected:
	std::vector<ColliderComponentPtr> m_colliders;
};