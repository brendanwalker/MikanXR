#pragma once

#include "ComponentFwd.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "Shared/RmlModel.h"

#include <memory>

class RmlModel_MikanComponent
{
public:
	RmlModel_MikanComponent();

	virtual bool init(Rml::Context* rmlContext);
	virtual bool setComponent(MikanComponentPtr component);
	virtual void dispose();

protected:
	MikanComponentWeakPtr m_component;
	RmlModel_PropertyInterfacePtr m_propertyInterface;
};

using RmlModel_MikanComponentPtr = std::shared_ptr<RmlModel_MikanComponent>;