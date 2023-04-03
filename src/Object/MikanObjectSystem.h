#pragma once

#include "MikanComponent.h"

#include <memory>
#include <vector>

class MikanObject;
typedef std::shared_ptr<MikanObject> MikanObjectPtr;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class MikanObjectSystem : public std::enable_shared_from_this<MikanObject>
{
public:
	MikanObjectSystem();
	virtual ~MikanObjectSystem();

	template<class t_object_type>
	MikanObjectWeakPtr newObject()
	{
		std::shared_ptr<t_object_type> objectPtr = std::make_shared<t_object_type>();
		objectPtr->init();

		addObject(objectPtr);

		return objectPtr;
	}

	template<class t_object_type>
	void getObjectsOfType(std::vector< std::shared_ptr<t_object_type> >& outObjects)
	{
		for (MikanObjectPtr object : m_objects)
		{
			std::shared_ptr<t_object_type> derivedObject = ObjectCast<t_object_type>(object);

			if (derivedObject != nullptr)
			{
				outObjects.push_back(derivedObject);
			}
		}
	}

	virtual void init();
	virtual void dispose();
	virtual void update();

	virtual void addObject(MikanObjectPtr objectPtr);
	virtual void removeObject(MikanObjectPtr objectPtr);

protected:
	std::vector<MikanObjectPtr> m_objects;
};
typedef std::shared_ptr<MikanObjectSystem> MikanObjectSystemPtr;
typedef std::weak_ptr<MikanObjectSystem> MikanObjectSystemWeakPtr;