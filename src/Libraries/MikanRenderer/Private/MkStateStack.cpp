#include "MkStateStack.h"
#include "IMkGraphicsContext.h"
#include "IMkState.h"

#include <vector>
#include <assert.h>

struct MkStateStackData
{
	std::vector<IMkState*> stateStack;
	class IMkGraphicsContext* ownerContext= nullptr;
	bool bDebugPrint= false;
};

MkStateStack::MkStateStack(IMkGraphicsContext* ownerContext)
	: m_data(new MkStateStackData())
{
	m_data->ownerContext= ownerContext;
	m_data->bDebugPrint= false;
}

MkStateStack::~MkStateStack()
{
	while (!m_data->stateStack.empty())
	{
		popState();
	}

	delete m_data;
}

IMkState* MkStateStack::pushState(const std::string& scopeName)
{
	// Create a new GlState and initialize it from the parent state on this stack
	IMkState* state= createMkState(*this, scopeName, (int)m_data->stateStack.size());

	// Add it to the top of the stack
	m_data->stateStack.push_back(state);

	return state;
}

int MkStateStack::getCurrentStackDepth() const { return (int)m_data->stateStack.size() - 1; }

IMkState* MkStateStack::getState(const int depth) const
{
	return (depth >= 0 && depth < (int)m_data->stateStack.size()) ? m_data->stateStack[depth] : nullptr;
}

IMkState* MkStateStack::getCurrentState() const { return getState(getCurrentStackDepth()); }

IMkGraphicsContext* MkStateStack::getOwnerContext() const { return m_data->ownerContext; }

void MkStateStack::setDebugPrintEnabled(bool bDebugPrint) { m_data->bDebugPrint= bDebugPrint; }

bool MkStateStack::isDebugPrintEnabled() const { return m_data->bDebugPrint; }

void MkStateStack::popState()
{
	const int currentDepth= getCurrentStackDepth();

	if (currentDepth >= 0)
	{
		// Cleaning up the state will undo all the flags it set
		destroyMkState(m_data->stateStack[currentDepth]);

		// Remove the state from the top of the stack
		// Cleaning up the state will undo all the flags it set
		m_data->stateStack.pop_back();
	}
}

MkScopedState MkStateStack::createScopedState(const std::string& scopeName)
{
	// Create a state that will get auto cleaned up when GLScopedState goes out of scope
	return MkScopedState(pushState(scopeName));
}