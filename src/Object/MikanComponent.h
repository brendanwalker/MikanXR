#pragma once

#include <memory>
#include <typeinfo>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;
typedef std::shared_ptr<MikanObject> MikanObjectPtr;

class MikanComponent
{
public:
	MikanComponent(MikanObjectWeakPtr owner);
	virtual ~MikanComponent();
	
	MikanObjectPtr getOwnerObject() const { return m_ownerObject.lock(); }

	virtual void init();
	virtual void dispose();
	virtual void update();

protected:
	MikanObjectWeakPtr m_ownerObject;
};
typedef std::shared_ptr<MikanComponent> MikanComponentPtr;
typedef std::weak_ptr<MikanComponent> MikanComponentWeakPtr;

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
