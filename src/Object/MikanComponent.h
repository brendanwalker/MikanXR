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
typedef std::weak_ptr<MikanObject> MikanComponentWeakPtr;

template<class t_component_type>
t_component_type* ComponentCast(MikanComponent* component)
{
	return dynamic_cast<t_component_type>(component);
}

template<class t_component_type>
const char* ComponentTypeName(const t_component_type* component)
{
	return typeid(*component).name();
}

template<class t_component_type>
const char* ComponentTypeName()
{
	return typeid(t_component_type).name();
}
