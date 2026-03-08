#include "PersistentIDAllocator.h"

#include <configuru.hpp>

PersistentIDAllocator::PersistentIDAllocator()
	: CommonConfig("PersistentIDAllocator")
	, m_nextId(0)
{
}

PersistentIDAllocator::PersistentIDAllocator(int32_t initialNextId)
	: CommonConfig("PersistentIDAllocator")
	, m_nextId(initialNextId)
{}

// Serialization
configuru::Config PersistentIDAllocator::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_id"] = m_nextId;

	return pt;
}

void PersistentIDAllocator::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextId = pt.get_or<int32_t>("next_id", m_nextId);
}

// ID Allocation
int PersistentIDAllocator::allocateNextId()
{
	assert(m_bSafeToAllocate);

	int allocatedId = m_nextId;
	m_nextId++;

	return allocatedId;
}

void PersistentIDAllocator::ensureNextIdGreaterThan(int32_t id)
{
	if (m_nextId <= id)
	{
		m_nextId = id + 1;
	}
}