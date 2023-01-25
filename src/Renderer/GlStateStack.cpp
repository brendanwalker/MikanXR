#include "GlStateStack.h"
#include "GlCommon.h"
#include <assert.h>

GLenum g_glFlagTypeMapping[(int)eGlStateFlagType::COUNT] = {
	GL_LIGHT0,				// light0,
	GL_TEXTURE_2D,			// texture2d,
	GL_DEPTH_TEST,			// depthTest,
	GL_CULL_FACE,			// cullFace,
	GL_PROGRAM_POINT_SIZE,	// programPointSize,
};

// -- GLScopedState -----
GLState::GLState(GlStateStack& ownerStack, const int stackDepth)
	: m_ownerStack(ownerStack)
	, m_parentState(ownerStack.getState(stackDepth - 1))
	, m_stackDepth(stackDepth)
{
	if (m_parentState != nullptr)
	{
		// init our stack state with a copy of our parent state
		memcpy(m_flags, m_parentState->m_flags, sizeof(m_flags));
	}
	else
	{
		// Initialize all stack vars with defaults
		memset(&m_flags, 0, sizeof(m_flags));
	}
}

GLState::~GLState()
{
	for (int flagIndex = 0; flagIndex < (int)eGlStateFlagType::COUNT; ++flagIndex)
	{
		const eGlStateFlagValue flagValue = m_flags[flagIndex];
		const eGlStateFlagValue parentFlagValue= 
			(m_parentState != nullptr) 
			? m_parentState->m_flags[flagIndex] 
			: eGlStateFlagValue::unset;

		if (flagValue != parentFlagValue)
		{
			GLenum glFlag= g_glFlagTypeMapping[flagIndex];

			if (flagValue == eGlStateFlagValue::enabled)
			{
				glDisable(glFlag);
			}
			else if (flagValue == eGlStateFlagValue::disabled)
			{
				glEnable(glFlag);
			}
		}
	}
}

GLState& GLState::enableFlag(eGlStateFlagType flagType)
{
	const eGlStateFlagValue flagValue= m_flags[(int)flagType];
	if (flagValue != eGlStateFlagValue::enabled)
	{
		glEnable(g_glFlagTypeMapping[(int)flagType]);
		m_flags[(int)flagType]= eGlStateFlagValue::enabled;
	}

	return *this;
}

GLState& GLState::disableFlag(eGlStateFlagType flagType)
{
	const eGlStateFlagValue flagValue = m_flags[(int)flagType];
	if (flagValue != eGlStateFlagValue::disabled)
	{
		glDisable(g_glFlagTypeMapping[(int)flagType]);
		m_flags[(int)flagType] = eGlStateFlagValue::disabled;
	}

	return *this;
}

// -- GlScopedState -----
GLScopedState::GLScopedState(GLState& state) : m_state(state)
{
}

GLScopedState::~GLScopedState()
{
	m_state.getOwnerStateStack().freeState(m_state);
}

// -- GlStateStack -----
GlStateStack::~GlStateStack()
{
	while (!m_stateStack.empty())
	{
		freeState(*m_stateStack[m_stateStack.size() - 1]);
	}
}

GLState& GlStateStack::createState()
{
	// Create a new GlState and initialize it from the parent state on this stack
	GLState* state = new GLState(*this, (int)m_stateStack.size());

	// Add it to the top of the stack
	m_stateStack.push_back(state);

	return *state;
}

GLState* GlStateStack::getState(const int depth) const
{
	return (depth >= 0 && depth < (int)m_stateStack.size()) ? m_stateStack[depth] : nullptr;
}

void GlStateStack::freeState(GLState& state)
{
	// Make sure we are deleting the state on the top of the stack
	const int currentDepth = (int)m_stateStack.size() - 1;
	assert(currentDepth == state.getStackDepth());

	// Cleaning up the state will undo all the flags it set
	delete m_stateStack[currentDepth];

	// Remove the state from the top of the stack
	m_stateStack.pop_back();
}

GLScopedState GlStateStack::createScopedState()
{
	// Create a state that will get auto cleaned up when GLScopedState goes out of scope
	return GLScopedState(createState());
}