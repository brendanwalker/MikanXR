#include "MonotonicIDAllocator.h"

#include <assert.h>

MonotonicIDAllocator::MonotonicIDAllocator(int32_t initialNextId, int32_t idRange)
	: m_nextId(initialNextId)
	, m_maxId(initialNextId + idRange)
{
}

// -- IEntityIDAllocator interface
int MonotonicIDAllocator::allocateNextId()
{
	assert(m_nextId < m_maxId);

	int allocatedId= m_nextId;
	m_nextId++;

	return allocatedId;
}

void MonotonicIDAllocator::ensureNextIdGreaterThan(int32_t id)
{
	if (m_nextId <= id)
	{
		m_nextId= id + 1;
		assert(m_nextId < m_maxId);
	}
}