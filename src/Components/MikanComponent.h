#pragma once

#include <memory>
#include <typeinfo>
#include "MulticastDelegate.h"

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;
typedef std::shared_ptr<MikanObject> MikanObjectPtr;

class MikanComponent;
typedef std::shared_ptr<MikanComponent> MikanComponentPtr;
typedef std::weak_ptr<MikanComponent> MikanComponentWeakPtr;

class MikanComponent : public std::enable_shared_from_this<MikanComponent>
{
public:
	MikanComponent(MikanObjectWeakPtr owner);
	virtual ~MikanComponent();
	
	MikanObjectPtr getOwnerObject() const { return m_ownerObject.lock(); }

	template <class t_derived_type>
	std::shared_ptr<t_derived_type> getSelfPtr()
	{
		return std::dynamic_pointer_cast<t_derived_type>(shared_from_this());
	}

	template <class t_derived_type>
	std::weak_ptr<t_derived_type> getSelfWeakPtr()
	{
		return getSelfPtr<t_derived_type>();
	}

	virtual void init();
	virtual void dispose();
	virtual void update();

protected:
	MikanObjectWeakPtr m_ownerObject;
};

template<class t_derived_type>
std::shared_ptr<t_derived_type> ComponentCast(MikanComponentPtr component)
{
	return std::dynamic_pointer_cast<t_derived_type>(component);
}

template<class t_component_type>
const char* ComponentTypeName(MikanComponentPtr component)
{
	return typeid(*component.get()).name();
}

template<class t_component_type>
const char* ComponentTypeName()
{
	return typeid(t_component_type).name();
}
