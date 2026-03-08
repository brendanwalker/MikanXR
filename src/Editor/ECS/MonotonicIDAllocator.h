#pragma once

#include "IEntityIDAllocator.h"

#include "memory"

class MonotonicIDAllocator : public IEntityIDAllocator
{
public:
	MonotonicIDAllocator(int32_t initialNextId, int32_t idRange);

	// -- IEntityIDAllocator interface
	int allocateNextId() override;
	void ensureNextIdGreaterThan(int32_t id) override;

private:
	int m_nextId = 0;
	int m_maxId = 0;
};

using MonotonicIDAllocatorPtr = std::shared_ptr<MonotonicIDAllocator>;
using MonotonicIDAllocatorWeakPtr = std::weak_ptr<MonotonicIDAllocator>;