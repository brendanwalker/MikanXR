#pragma once

#include "MikanComponent.h"
#include "MikanObject.h"

#include <memory>
#include <vector>

class MikanObject;
typedef std::shared_ptr<MikanObject> MikanObjectPtr;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class MikanObjectSystem : public std::enable_shared_from_this<MikanObjectSystem>
{
public:
	MikanObjectSystem();
	virtual ~MikanObjectSystem();

	MikanObjectPtr newObject()
	{
		MikanObjectPtr objectPtr = std::make_shared<MikanObject>();
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

	MulticastDelegate<void(MikanComponent* componentPtr, const std::string& propertyName, const std::string& propertyType)> OnComponentPropertyChaged;

protected:
	std::vector<MikanObjectPtr> m_objects;
};