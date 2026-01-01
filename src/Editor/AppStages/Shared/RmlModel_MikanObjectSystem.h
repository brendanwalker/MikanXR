#pragma once

#include "ObjectFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModelInterface.h"
#include "Shared/RmlModel_PropertyInterface.h"

#include <memory>

class RmlModel_MikanObjectSystem : public IRmlModel
{
public:
	RmlModel_MikanObjectSystem();

	virtual bool init(Rml::Context* rmlContext) = 0;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor);

	MikanObjectSystemPtr getObjectSystem() const;
	void setObjectSystem(MikanObjectSystemPtr objectSystem);

	// IRmlModel
	virtual Rml::Context* getContext() override;
	virtual Rml::DataModelHandle& getModelHandle() override;

	virtual void dispose() override;
	virtual void update() override;

	virtual void addModelUpdateCallback(std::function<void()> callback) override;

protected:
	template <class t_object_system_type>
	bool initTypedPropertyInterface(Rml::Context* rmlContext)
	{
		return
			m_propertyInterface->init<t_object_system_type>(
				rmlContext,
				t_object_system_type::k_objectSystemClassName,
				[this](Rml::DataModelConstructor& constructor) -> bool
				{
					return onConstruct(constructor);
				});
	}

protected:
	MikanObjectSystemWeakPtr m_objectSystem;
	RmlModel_PropertyInterfacePtr m_propertyInterface;
};
