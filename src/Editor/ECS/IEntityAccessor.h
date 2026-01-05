#pragma once

#include "CommonConfigFwd.h"
#include "FunctionInterface.h"
#include "PropertyInterface.h"

class IEntityAccessor :
	public IPropertyInterface,
	public IFunctionInterface
{
public:
	virtual CommonConfigPtr getEntityConfig() = 0;
};
using IEntityAccessorPtr = std::shared_ptr<IEntityAccessor>;
using IEntityAccessorConstPtr = std::shared_ptr<const IEntityAccessor>;
using IEntityAccessorWeakPtr = std::weak_ptr<IEntityAccessor>;