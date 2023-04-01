#pragma once

#include "MikanComponent.h"

#include <memory>
#include <vector>

class MikanSceneComponent;

class MikanObject final
{
public:
	MikanObject();
	~MikanObject();

	template<class t_component_type>
	t_component_type* addComponent()
	{

		t_component_type* component= std::make_shared<t_component_type>(this);
		m_components.push_back(component);
		return component;
	}

	inline std::vector<MikanComponent*>& getComponentsConst() { return m_components; }
	inline const std::vector<MikanComponent*>& getComponentsConst() const { return m_components; }

	template<class t_component_type>
	t_component_type* getComponentOfType()
	{
		for(MikanComponent* component : m_components)
		{
			t_component_type* derivedComponent= ComponentCast<t_component_type>(component);

			if (derivedComponent != nullptr)
			{
				return derivedComponent;
			}
		}

		return nullptr;
	}

	template<class t_component_type>
	void getComponentsOfType(std::vector<t_component_type*>& outComponents)
	{
		for (MikanComponent* component : m_components)
		{
			t_component_type* derivedComponent = ComponentCast<t_component_type>(component);

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
	MikanSceneComponent* m_rootSceneComponent= nullptr;
	std::vector<MikanComponent*> m_components;
};
typedef std::shared_ptr<MikanObject> MikanObjectPtr;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;