#pragma once

#include "CommonConfig.h"
#include "CommonConfigFwd.h"
#include "IEntityIDAllocator.h"
#include "memory"

class PersistentIDAllocator : 
	public CommonConfig, 
	public IEntityIDAllocator
{
public:
	PersistentIDAllocator();
	PersistentIDAllocator(int32_t initialNextId);

	// Serialization
	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	// ID Allocation
	void markSafeToAllocate() { m_bSafeToAllocate = true; }

	// -- IEntityIDAllocator interface
	virtual int allocateNextId() override;
	virtual void ensureNextIdGreaterThan(int32_t id) override;

private:
	int m_nextId= 0;
	bool m_bSafeToAllocate = false;
};

using PersistentIDAllocatorPtr = std::shared_ptr<PersistentIDAllocator>;
using PersistentIDAllocatorWeakPtr = std::weak_ptr<PersistentIDAllocator>;