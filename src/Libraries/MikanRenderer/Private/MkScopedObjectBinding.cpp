#include "MkScopedObjectBinding.h"
#include "MkStateStack.h"
#include "IMkState.h"

#include <assert.h>

struct MkScopedObjectBindingData
{
	IMkBindableObjectPtr boundObject;
	IMkStatePtr mkState;
};

MkScopedObjectBinding::MkScopedObjectBinding(
	IMkStatePtr parentMkState,
	const std::string& scopeName,
	IMkBindableObjectPtr bindableObject)
	: m_data(new MkScopedObjectBindingData())
{
	m_data->boundObject= bindableObject;
	m_data->mkState= parentMkState->getOwnerStateStack().pushState(scopeName);

	if (m_data->boundObject)
	{
		assert(!m_data->boundObject->getIsBound());
		m_data->boundObject->bindObject(m_data->mkState);
	}
}

MkScopedObjectBinding::~MkScopedObjectBinding()
{
	// Restore all the GL state we modified
	assert(m_data->mkState->getOwnerStateStack().getCurrentStackDepth() == m_data->mkState->getStackDepth());
	m_data->mkState->getOwnerStateStack().popState();

	if (m_data->boundObject)
	{
		assert(m_data->boundObject->getIsBound());
		m_data->boundObject->unbindObject();
	}

	delete m_data;
}

IMkBindableObjectPtr MkScopedObjectBinding::getBoundObject() const 
{ 
	return m_data->boundObject; 
}

MkScopedObjectBinding::operator bool() const 
{ 
	return m_data->boundObject->getIsBound(); 
}

IMkStatePtr MkScopedObjectBinding::getMkState() 
{ 
	return m_data->mkState; 
}
