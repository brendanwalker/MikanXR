#pragma once
#include "MulticastDelegate.h"
#include <memory>
#include <type_traits>

class MikanComponent;

class ComponentPropertyAccessor
{
};
using ComponentPropertyAccessorPtr= std::shared_ptr<ComponentPropertyAccessor>;

template <typename t_property_type>
class TComponentPropertyAccessor : public ComponentPropertyAccessor
{
public:
	TComponentPropertyAccessor(t_property_type& propertyValue) 
		: m_propertyValue(propertyValue)
	{
	}

	const t_property_type& getValue() const
	{
		return m_propertyValue;
	}

private:
	t_property_type& m_propertyValue;
};

class ComponentProperty
{
public:
	ComponentProperty(
		MikanComponent& ownerComponent,
		const std::string& propertyTypeString,
		const std::string& propertyName,
		ComponentPropertyAccessorPtr valueAccessor);

	template <class t_requested_type>
	inline const t_requested_type& getValueConst() const
	{
		auto typedAccessor= std::dynamic_pointer_cast<TComponentPropertyAccessor<t_requested_type>>(m_valueAccessor);
		assert(typedAccessor && "Attempting to cast to the incorrect value accessor");

		return typedAccessor.getValue();
	}

private:
	MikanComponent& m_ownerComponent;
	std::string m_propertyTypeString;
	std::string m_propertyName;
	ComponentPropertyAccessorPtr m_valueAccessor;
};

class ComponentPropertyEvents
{
public:
	static MulticastDelegate<void(const ComponentProperty&)> OnComponentPropertyChanged;
	static MulticastDelegate<void(const MikanComponent&)> OnComponentChanged;
};

#define COMPONENT_PROPERTY(PROPERTY_TYPE, PROPERTY_NAME)												\
	PROPERTY_TYPE PROPERTY_NAME;																		\
	inline void notify ## PROPERTY_NAME ## Changed()													\
	{																									\
		auto accessor= std::make_shared< TComponentPropertyAccessor<PROPERTY_TYPE> >(PROPERTY_NAME);	\
		ComponentProperty componentProperty(*this, #PROPERTY_TYPE, #PROPERTY_NAME, accessor);			\
		if (ComponentPropertyEvents::OnComponentPropertyChanged)										\
			ComponentPropertyEvents::OnComponentPropertyChanged(componentProperty);						\
	}																									\
	

