#pragma once

#include "ComponentFwd.h"
#include "MulticastDelegate.h"
#include "ObjectFwd.h"

#include <vector>

class MikanObjectSystem : public std::enable_shared_from_this<MikanObjectSystem>
{
public:
	MikanObjectSystem();
	virtual ~MikanObjectSystem();

	virtual void init();
	virtual void dispose();
	virtual void update();

	MikanObjectWeakPtr newObject();
	void deleteObject(MikanObjectWeakPtr objectPtr);

	MulticastDelegate<void(MikanObjectSystem&, MikanObject&)> OnObjectAdded;
	MulticastDelegate<void(MikanObjectSystem&, MikanObject&)> OnObjectRemoved;

protected:
	std::vector<MikanObjectPtr> m_objects;
};