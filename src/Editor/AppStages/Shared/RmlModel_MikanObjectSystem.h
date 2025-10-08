#pragma once

#include "ObjectFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel.h"

#include <memory>

class RmlModel_MikanObjectSystem
{
public:
	RmlModel_MikanObjectSystem();

	virtual bool init(Rml::Context* rmlContext, MikanObjectSystemPtr objectSystem);
	virtual void dispose();

protected:
	MikanObjectSystemWeakPtr m_objectSystem;
	RmlModel_PropertyInterfacePtr m_propertyInterface;
};
