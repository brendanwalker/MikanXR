#pragma once

#include "CommonConfigFwd.h"
#include "FunctionInterface.h"
#include "PropertyInterface.h"
#include "MulticastDelegate.h"

namespace rfk
{
	class Struct;
};

class IEntityAccessor :
	public IPropertyInterface,
	public IFunctionInterface
{
public:
	MulticastDelegate<void(const IEntityAccessor* selfPtr)> onDisposed;
	
	virtual std::string getEntityUIIdentifier() const = 0;
	virtual CommonConfigPtr getEntityConfig() = 0;
	virtual rfk::Struct const* getClientAPIValuesStructType() const = 0;
};
using IEntityAccessorPtr = std::shared_ptr<IEntityAccessor>;
using IEntityAccessorConstPtr = std::shared_ptr<const IEntityAccessor>;
using IEntityAccessorWeakPtr = std::weak_ptr<IEntityAccessor>;