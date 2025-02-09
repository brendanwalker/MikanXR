#include "MkScopedState.h"
#include "MkStateStack.h"
#include "IMkState.h"
#include <assert.h>

struct MkScopedStateImpl
{
	IMkStatePtr mkState;
};

MkScopedState::MkScopedState(IMkStatePtr state) : 
	m_impl(new MkScopedStateImpl())
{
	m_impl->mkState= state;
}

MkScopedState::~MkScopedState()
{
	// Make sure we are deleting the state on the top of the stack
	assert(m_impl->mkState->getOwnerStateStack().getCurrentStackDepth() == m_impl->mkState->getStackDepth());

	// Pop the state last since this will invalidate the state reference
	m_impl->mkState->getOwnerStateStack().popState();

	delete m_impl;
}

IMkStatePtr MkScopedState::getStackState() const 
{ 
	return m_impl->mkState; 
}

int MkScopedState::getStackDepth() const 
{ 
	return m_impl->mkState->getStackDepth(); 
}