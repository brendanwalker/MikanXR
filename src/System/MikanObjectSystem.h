#pragma once

#include "ComponentFwd.h"
#include "MulticastDelegate.h"
#include "ObjectFwd.h"

#include <vector>

using MikanObjectList = std::vector<MikanObjectPtr>;

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
	const MikanObjectList& getObjectList() const;

	MulticastDelegate<void(MikanObjectSystem&, MikanObject&)> OnObjectAdded;
	MulticastDelegate<void(MikanObjectSystem&, MikanObject&)> OnObjectRemoved;

protected:
	MikanObjectList m_objects;
};