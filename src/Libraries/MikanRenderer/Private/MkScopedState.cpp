#include "MkScopedState.h"
#include "MkStateStack.h"
#include "IMkState.h"
#include <assert.h>

struct MkScopedStateImpl
{
	IMkState* mkState;
};

MkScopedState::MkScopedState(IMkState* state) : 
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

	m_impl->mkState= nullptr;
	delete m_impl;
}

IMkState* MkScopedState::getStackState() const 
{ 
	return m_impl->mkState; 
}

int MkScopedState::getStackDepth() const 
{ 
	return m_impl->mkState->getStackDepth(); 
}