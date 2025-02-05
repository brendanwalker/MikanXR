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

	virtual bool init();
	virtual void dispose();
	virtual void update();
	virtual void customRender();

	MikanObjectPtr newObject();
	void deleteObject(MikanObjectPtr objectPtr);
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) { }
	inline const MikanObjectList& getObjectList() const { return m_objects; }

	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectPtr)> OnObjectInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectConstPtr)> OnObjectDisposed;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentPtr)> OnComponentInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentConstPtr)> OnComponentDisposed;

protected:
	MikanObjectList m_objects;

	MulticastDelegate<void()> onUpdate;
	MulticastDelegate<void()> onCustomRender;

	friend class MikanComponent;
};