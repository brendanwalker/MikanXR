#pragma once

#include "ComponentFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "Shared/RmlModelInterface.h"

namespace Rml
{
	class Context;
	class DataModelConstructor;
}

class RmlModel_MikanComponent : public IRmlModel
{
public:
	RmlModel_MikanComponent();
	virtual ~RmlModel_MikanComponent() = default;

	virtual bool init(Rml::Context* rmlContext) = 0;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor);

	inline RmlModel_PropertyInterfacePtr getPropertyInterface() const { return m_propertyInterface; }

	MikanComponentPtr getComponent() const;
	virtual bool setComponent(MikanComponentPtr component);

	// IRmlModel
	virtual Rml::Context* getContext() override;
	virtual Rml::DataModelHandle& getModelHandle() override;

	virtual void dispose() override;
	virtual void update() override;

	virtual void addModelUpdateCallback(std::function<void()> callback) override;

protected:
	template <class t_component_type>
	bool initTypedPropertyInterface(Rml::Context* rmlContext)
	{
		return
			m_propertyInterface->init<t_component_type>(
				rmlContext,
				t_component_type::k_componentClassName,
				[this](Rml::DataModelConstructor& constructor) -> bool
				{
					return onConstruct(constructor);
				});
	}

	MikanComponentWeakPtr m_component;
	RmlModel_PropertyInterfacePtr m_propertyInterface;
	RmlDataBinding_ScriptTriggerListPtr m_scriptTriggerList;
};
