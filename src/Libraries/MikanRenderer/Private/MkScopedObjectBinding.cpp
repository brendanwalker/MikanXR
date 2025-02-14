#include "MkScopedObjectBinding.h"
#include "MkStateStack.h"
#include "IMkState.h"

#include <assert.h>

struct MkScopedObjectBindingData
{
	IMkBindableObjectWeakPtr boundObject;
	IMkState* mkState;
};

MkScopedObjectBinding::MkScopedObjectBinding(
	IMkState* parentMkState,
	const std::string& scopeName,
	IMkBindableObjectPtr bindableObject)
	: m_data(new MkScopedObjectBindingData())
{
	m_data->boundObject= bindableObject;
	m_data->mkState= parentMkState->getOwnerStateStack().pushState(scopeName);

	IMkBindableObjectPtr boundObject= getBoundObject();
	if (boundObject)
	{
		assert(!boundObject->getIsBound());
		boundObject->bindObject(m_data->mkState);
	}
}

MkScopedObjectBinding::~MkScopedObjectBinding()
{
	// Restore all the GL state we modified
	assert(m_data->mkState->getOwnerStateStack().getCurrentStackDepth() == m_data->mkState->getStackDepth());
	m_data->mkState->getOwnerStateStack().popState();

	IMkBindableObjectPtr boundObject= getBoundObject();
	if (boundObject)
	{
		assert(boundObject->getIsBound());
		boundObject->unbindObject();
	}

	m_data->boundObject.reset();
	m_data->mkState= nullptr;
	delete m_data;
}

IMkBindableObjectPtr MkScopedObjectBinding::getBoundObject() const 
{ 
	return m_data->boundObject.lock(); 
}

MkScopedObjectBinding::operator bool() const 
{ 
	IMkBindableObjectPtr boundObject= getBoundObject();

	return boundObject && boundObject->getIsBound(); 
}

IMkState* MkScopedObjectBinding::getMkState() 
{ 
	return m_data->mkState; 
}
