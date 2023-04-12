#pragma once

#include "MikanComponent.h"

#include <memory>
#include <vector>

class MikanObject : public std::enable_shared_from_this<MikanObject>
{
public:
	MikanObject();
	~MikanObject();

	template<class t_component_type>
	std::shared_ptr<t_component_type> addComponent()
	{

		std::shared_ptr<t_component_type> component= std::make_shared<t_component_type>(shared_from_this());
		m_components.push_back(component);
		return component;
	}

	inline std::vector<MikanComponentPtr>& getComponentsConst() { return m_components; }
	inline const std::vector<MikanComponentPtr>& getComponentsConst() const { return m_components; }

	template<class t_component_type>
	std::shared_ptr<t_component_type> getComponentOfType()
	{
		for(MikanComponentPtr component : m_components)
		{
			std::shared_ptr<t_component_type> derivedComponent= ComponentCast<t_component_type>(component);

			if (derivedComponent != nullptr)
			{
				return derivedComponent;
			}
		}

		return nullptr;
	}

	template<class t_component_type>
	void getComponentsOfType(std::vector< std::shared_ptr<t_component_type> >& outComponents)
	{
		for (MikanComponentPtr component : m_components)
		{
			std::shared_ptr<t_component_type> derivedComponent = ComponentCast<t_component_type>(component);

			if (derivedComponent != nullptr)
			{
				outComponents.push_back(derivedComponent);
			}
		}
	}

	inline void setRootComponent(SceneComponentPtr sceneComponent) { m_rootSceneComponent= sceneComponent; }

	void init();
	void dispose();
	void update();

protected:
	SceneComponentPtr m_rootSceneComponent;
	std::vector<MikanComponentPtr> m_components;
};

template<class t_derived_type>
std::shared_ptr<t_derived_type> ObjectCast(MikanObjectPtr object)
{
	return std::dynamic_pointer_cast<t_derived_type>(object);
}

template<class t_object_type>
const char* ObjectTypeName(MikanObjectPtr object)
{
	return typeid(*object.get()).name();
}

template<class t_object_type>
const char* ObjectTypeName()
{
	return typeid(t_object_type).name();
}
