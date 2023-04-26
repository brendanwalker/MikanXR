#pragma once

#include "ComponentFwd.h"
#include "ComponentProperty.h"
#include "ObjectFwd.h"
#include "MulticastDelegate.h"

#include <memory>
#include <typeinfo>


class MikanComponent : public std::enable_shared_from_this<MikanComponent>
{
public:
	MikanComponent(MikanObjectWeakPtr owner);
	virtual ~MikanComponent();
	
	void setName(const std::string& name) { m_name= name; }
	const std::string& getName() const { return m_name; }

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

	MulticastDelegate<void(const ComponentProperty&)> OnComponentPropertyChanged;

protected:
	virtual void notifyComponentPropertyChanged(const ComponentProperty& property);

	std::string m_name;
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
