#include "GlScopedObjectBinding.h"
#include "GlStateStack.h"

#include <assert.h>

GlScopedObjectBinding::GlScopedObjectBinding(
	GlState& parentGLState,
	const std::string& scopeName,
	IMkBindableObjectPtr bindableObject)
	: m_boundObject(bindableObject)
	, m_glState(parentGLState.getOwnerStateStack().pushState(scopeName))
{
	if (m_boundObject)
	{
		assert(!m_boundObject->getIsBound());
		m_boundObject->bindObject(m_glState);
	}
}

GlScopedObjectBinding::~GlScopedObjectBinding()
{
	// Restore all the GL state we modified
	assert(m_glState.getOwnerStateStack().getCurrentStackDepth() == m_glState.getStackDepth());
	m_glState.getOwnerStateStack().popState();

	if (m_boundObject)
	{
		assert(m_boundObject->getIsBound());
		m_boundObject->unbindObject();
	}
}