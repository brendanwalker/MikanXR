#pragma once

#include "ObjectFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlModel_PropertyInterface.h"

#include <memory>

class RmlModel_MikanObjectSystem
{
public:
	RmlModel_MikanObjectSystem();

	virtual bool init(Rml::Context* rmlContext) = 0;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor);
	virtual void dispose();

	MikanObjectSystemPtr getObjectSystem() const;
	void setObjectSystem(MikanObjectSystemPtr objectSystem);

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
