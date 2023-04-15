#pragma once

#include "ComponentFwd.h"
#include "MulticastDelegate.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "SceneFwd.h"

#include <vector>

class ObjectEvents
{
public:
	static MulticastDelegate<void(MikanObject&)> OnObjectInitialized;
	static MulticastDelegate<void(const MikanObject&)> OnObjectDisposed;
};

class MikanObject final : public std::enable_shared_from_this<MikanObject>
{
public:
	MikanObject(MikanObjectSystemWeakPtr ownerSystemPtr);
	~MikanObject();

	template<class t_component_type>
	std::shared_ptr<t_component_type> addComponent()
	{

		std::shared_ptr<t_component_type> component= std::make_shared<t_component_type>(shared_from_this());
		m_components.push_back(component);
		return component;
	}

	template<class t_component_type>
	std::shared_ptr<t_component_type> addComponent(const std::string& name)
	{

		std::shared_ptr<t_component_type> component = std::make_shared<t_component_type>(shared_from_this());
		component->setName(name);
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
	std::shared_ptr<t_component_type> getComponentOfTypeAndName(const std::string& name)
	{
		for (MikanComponentPtr component : m_components)
		{
			std::shared_ptr<t_component_type> derivedComponent = ComponentCast<t_component_type>(component);

			if (derivedComponent != nullptr && component->getName() == name)
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

	inline MikanObjectSystemWeakPtr getOwnerSystem() const { return m_ownerObjectSystem; }
	inline SceneComponentWeakPtr getRootComponent() const { return m_rootSceneComponent; }
	inline void setRootComponent(SceneComponentWeakPtr sceneComponent) { m_rootSceneComponent= sceneComponent; }

	void init();
	void dispose();
	void update();

protected:
	MikanObjectSystemWeakPtr m_ownerObjectSystem;
	SceneComponentWeakPtr m_rootSceneComponent;
	std::vector<MikanComponentPtr> m_components;
};
