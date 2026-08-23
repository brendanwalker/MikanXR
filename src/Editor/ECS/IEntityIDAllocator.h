#pragma once

#include "memory"

class IEntityIDAllocator
{
public:
	virtual int allocateNextId()= 0;
	virtual void ensureNextIdGreaterThan(int32_t id)= 0;
};

using IEntityIDAllocatorPtr= std::shared_ptr<IEntityIDAllocator>;
using IEntityIDAllocatorWeakPtr= std::weak_ptr<IEntityIDAllocator>;