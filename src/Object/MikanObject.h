#pragma once

#include "MikanComponent.h"

#include <memory>
#include <vector>

class MikanSceneComponent;
typedef std::shared_ptr<MikanSceneComponent> MikanSceneComponentPtr;

class MikanObject final
{
public:
	MikanObject();
	~MikanObject();

	template<class t_component_type>
	std::shared_ptr<t_component_type> addComponent()
	{

		std::shared_ptr<t_component_type> component= std::make_shared<t_component_type>(this);
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

	void init();
	void dispose();
	void update();

protected:
	MikanSceneComponentPtr m_rootSceneComponent;
	std::vector<MikanComponentPtr> m_components;
};
typedef std::shared_ptr<MikanObject> MikanObjectPtr;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;