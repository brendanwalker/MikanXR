#pragma once

#include "ComponentFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_EntityAccessor.h"
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

	inline RmlModel_EntityAccessorPtr getPropertyInterface() const { return m_entityAccessor; }

	MikanComponentPtr getComponent() const;
	virtual bool setComponent(MikanComponentPtr component);
	virtual void onComponentPropertyChanged(
		IEntityAccessorPtr accessorPtr,
		const ConfigPropertyChangeSet& changedPropertySet) {}

	// IRmlModel
	virtual Rml::Context* getContext() override;
	virtual Rml::DataModelHandle& getModelHandle() override;

	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;

	virtual void addModelUpdateCallback(std::function<void()> callback) override;

protected:
	template <class t_component_type>
	bool initTypedPropertyInterface(Rml::Context* rmlContext)
	{
		return
			m_entityAccessor->init<t_component_type>(
				rmlContext,
				t_component_type::k_componentClassName,
				[this](Rml::DataModelConstructor& constructor) -> bool
				{
					return onConstruct(constructor);
				});
	}

	MikanComponentWeakPtr m_component;
	RmlModel_EntityAccessorPtr m_entityAccessor;
	RmlDataBinding_ScriptTriggerListPtr m_scriptTriggerList;
};
