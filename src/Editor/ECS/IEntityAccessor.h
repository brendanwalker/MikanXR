#pragma once

#include "CommonConfigFwd.h"
#include "RmlFunctionInterface.h"
#include "RmlPropertyInterface.h"

class IEntityAccessor :
	public IRmlPropertyInterface,
	public IRmlFunctionInterface
{
public:
	virtual CommonConfigPtr getEntityConfig() = 0;
};
using IEntityAccessorPtr = std::shared_ptr<IEntityAccessor>;
using IEntityAccessorConstPtr = std::shared_ptr<const IEntityAccessor>;
using IEntityAccessorWeakPtr = std::weak_ptr<IEntityAccessor>;